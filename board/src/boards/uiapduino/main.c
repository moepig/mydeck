/* mydeck — UIAPduino Pro Micro CH32V003 V1.4
 * ch32fun + rv003usb による USB HID マクロパッド */

#include "ch32fun.h"
#include <string.h>
#include "rv003usb.h"

#include "board_config.h"

#ifdef MYDECK_DEBUG
#include <stdio.h>
#define DBG_PRINTF printf
#else
#define DBG_PRINTF(...) ((void)0)
#endif

#include "domain/hid_report.h"
#include "domain/button_service.h"
#include "domain/led_service.h"
#include "infrastructure/ch32_button_port.h"
#include "infrastructure/ch32_led_port.h"

/* ---- millis カウンタ ------------------------------------------ */
// SysTick を free-running (CTLR=5) で使用し、割り込みなしで更新する。
// ブートローダと同じ設定にすることで SysTick ISR が USB ISR を遅延させる問題を回避。
volatile uint32_t g_millis;

static void millis_init(void) {
    SysTick->CTLR = 0;
    SysTick->CNT  = 0;
    SysTick->CTLR = 5;  // free-running, HCLK, 割り込みなし (ブートローダと同じ)
}

static void millis_update(void) {
    static uint32_t last = 0;
    uint32_t now   = SysTick->CNT;
    uint32_t delta = now - last;
    uint32_t ms    = delta / (FUNCONF_SYSTEM_CORE_CLOCK / 1000);
    if (ms > 0) {
        g_millis += ms;
        last     += ms * (FUNCONF_SYSTEM_CORE_CLOCK / 1000);
    }
}

/* ---- GPIO 初期化 ---------------------------------------------- */
static void matrix_init(void) {
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

    /* PC0: push-pull 出力 (LED3 モード表示)
     * PC1: push-pull 出力 (Row1)
     * PC2-PC7: 入力 pull-up (Col0-Col5) */
    GPIOC->CFGLR =
        (0x1u <<  0) |  /* PC0 LED3 */
        (0x1u <<  4) |  /* PC1 Row1 */
        (0x8u <<  8) |  /* PC2 Col0 */
        (0x8u << 12) |  /* PC3 Col1 */
        (0x8u << 16) |  /* PC4 Col2 */
        (0x8u << 20) |  /* PC5 Col3 */
        (0x8u << 24) |  /* PC6 Col4 */
        (0x8u << 28);   /* PC7 Col5 */
    GPIOC->BSHR = 0xFFu;

    /* PD2: push-pull 出力 10MHz (Row0) */
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xFu << 8)) | (0x1u << 8);
    GPIOD->BSHR = (1u << 2);
}

/* ---- シームレススイッチ --------------------------------------- */
static void seamless_switch_init(void) {
    if (FLASH->STATR & (1 << 14)) NVIC_SystemReset();
    FLASH->KEYR          = FLASH_KEY1;
    FLASH->KEYR          = FLASH_KEY2;
    FLASH->BOOT_MODEKEYR = FLASH_KEY1;
    FLASH->BOOT_MODEKEYR = FLASH_KEY2;
    FLASH->STATR        |= (1 << 14);
    FLASH->CTLR          = CR_LOCK_Set;
    funPinMode(PD4, GPIO_CFGLR_OUT_10Mhz_PP);
}

/* ---- ドメイン層 ----------------------------------------------- */
static const Ch32RowPin kRows[ROW_COUNT] = {
    { GPIOD, ROW0_BIT },
    { GPIOC, ROW1_BIT },
};
static const uint8_t kColBits[COL_COUNT] = {
    COL_BIT_0, COL_BIT_1, COL_BIT_2, COL_BIT_3, COL_BIT_4, COL_BIT_5,
};

static Ch32ButtonCtx buttonCtx;
static ButtonPort    buttonPort;
static LedPort       ledPort;
static ButtonService buttonService;
static LedService    ledService;

/* ---- HID 送信バッファ ----------------------------------------- */
static volatile InputReport g_pending;
static volatile bool        g_hasPending;

/* ---- Feature レポート応答バッファ ----------------------------- */
static uint8_t s_feature_report[REPORT_SIZE] = { REPORT_ID_OUTPUT };

/* ---- エントリポイント ------------------------------------------ */
int main(void) {
    SystemInit();
    seamless_switch_init();
    millis_init();
    matrix_init();

    buttonCtx.rows      = kRows;
    buttonCtx.col_bits  = kColBits;
    buttonCtx.row_count = ROW_COUNT;
    buttonCtx.col_count = COL_COUNT;
    buttonPort = ch32_button_port_create(&buttonCtx);
    ledPort    = ch32_led_port_create();

    button_service_init(&buttonService, &buttonPort, BUTTON_COUNT);
    led_service_init(&ledService, &ledPort);

    g_hasPending = false;

    Delay_Ms(1);
    usb_setup();

    led_service_set_connected(&ledService, true);

    /* モード表示 LED: 実行モード = LED3 点灯, 書き込み待機 = 消灯 */
    GPIOC->BCR = (1u << LED_MODE_BIT);

    DBG_PRINTF("mydeck: ready (rows=%d cols=%d)\n", ROW_COUNT, COL_COUNT);

    while (1) {
        millis_update();
        ButtonPressResult ev = button_service_update(&buttonService);
        if (ev.valid && !g_hasPending) {
            InputReport rpt;
            input_report_clear(&rpt);
            rpt.buttonId = ev.buttonId;
            rpt.event    = (uint8_t)ev.type;
            memcpy((void *)&g_pending, &rpt, sizeof(rpt));
            g_hasPending = true;
            DBG_PRINTF("btn %d ev %d\n", ev.buttonId, (int)ev.type);
        }

        led_service_update(&ledService);
    }
}

/* ================================================================
 * rv003usb コールバック
 * ================================================================ */

void usb_handle_user_in_request(struct usb_endpoint *e,
                                 uint8_t *scratch,
                                 int endp,
                                 uint32_t sendtok,
                                 struct rv003usb_internal *ist)
{
    if (endp != 1) { usb_send_empty(sendtok); return; }
    if (g_hasPending) {
        usb_send_data((uint8_t *)&g_pending, REPORT_SIZE, 0, sendtok);
        g_hasPending = false;
    } else {
        usb_send_empty(sendtok);
    }
}

void usb_handle_hid_set_report_start(struct usb_endpoint *e,
                                      int reqLen,
                                      uint32_t lValueLSBIndexMSB)
{
    e->max_len = (reqLen < REPORT_SIZE) ? reqLen : REPORT_SIZE;
}

void usb_handle_hid_get_report_start(struct usb_endpoint *e,
                                      int reqLen,
                                      uint32_t lValueLSBIndexMSB)
{
    e->opaque  = s_feature_report;
    e->max_len = (reqLen < REPORT_SIZE) ? reqLen : REPORT_SIZE;
}

void usb_handle_user_data(struct usb_endpoint *e,
                           int current_endpoint,
                           uint8_t *data,
                           int len,
                           struct rv003usb_internal *ist)
{
    if (len < 3) return;
    if (data[0] == REPORT_ID_OUTPUT && data[1] == (uint8_t)OUTPUT_CMD_SET_LED) {
        led_service_set_connected(&ledService, data[2] != 0);
    }
}

void usb_handle_other_control_message(struct usb_endpoint *e,
                                        struct usb_urb *s,
                                        struct rv003usb_internal *ist)
{
    (void)e; (void)s; (void)ist;
}
