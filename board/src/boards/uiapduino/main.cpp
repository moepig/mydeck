// mydeck — UIAPduino Pro Micro CH32V003 V1.4
// ch32fun + rv003usb による USB HID マクロパッド

#include "ch32fun.h"
#include <string.h>
extern "C" {
#include "rv003usb.h"
}

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

// ---- C++ ランタイムスタブ (組み込み環境: libstdc++ なし) --------
extern "C" {
void __cxa_pure_virtual() { while (1); }
int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
void *__dso_handle = 0;
}
void operator delete(void *, unsigned int) noexcept {}
void operator delete(void *) noexcept {}

// ---- millis カウンタ (ch32_button_port.h / ch32_led_port.h と共有) --
volatile uint32_t g_millis;

extern "C" void SysTick_Handler() __attribute__((interrupt));
extern "C" void SysTick_Handler()
{
    g_millis++;
    SysTick->SR = 0;
}

static void millis_init()
{
    SysTick->CTLR = 0;
    SysTick->SR   = 0;
    SysTick->CNT  = 0;
    SysTick->CMP  = FUNCONF_SYSTEM_CORE_CLOCK / 1000 - 1;  // 1ms
    SysTick->CTLR = 0xF;  // 有効化・HCLK・割り込み・自動リロード
    NVIC_EnableIRQ(SysTick_IRQn);
}

// ---- GPIO 初期化 -----------------------------------------------
static void matrix_init()
{
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

    // PC0: push-pull 出力 (LED3 モード表示)
    // PC1-PC2: push-pull 出力 (Row1, Row2)
    // PC3-PC7: 入力 pull-up (Col)
    GPIOC->CFGLR =
        (0x1u <<  0) |  // PC0 LED3
        (0x1u <<  4) |  // PC1 Row1
        (0x1u <<  8) |  // PC2 Row2
        (0x8u << 12) |  // PC3 Col0
        (0x8u << 16) |  // PC4 Col1
        (0x8u << 20) |  // PC5 Col2
        (0x8u << 24) |  // PC6 Col3
        (0x8u << 28);   // PC7 Col4
    GPIOC->BSHR = 0xFFu;  // 全ピン HIGH

    // PD2: push-pull 出力 10MHz (Row0)
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xFu << 8)) | (0x1u << 8);
    GPIOD->BSHR = (1u << 2);  // PD2 HIGH
}

// ---- シームレススイッチ ----------------------------------------
static void seamless_switch_init()
{
    if (FLASH->STATR & (1 << 14)) NVIC_SystemReset();
    FLASH->KEYR          = FLASH_KEY1;
    FLASH->KEYR          = FLASH_KEY2;
    FLASH->BOOT_MODEKEYR = FLASH_KEY1;
    FLASH->BOOT_MODEKEYR = FLASH_KEY2;
    FLASH->STATR        |= (1 << 14);
    FLASH->CTLR          = CR_LOCK_Set;
    funPinMode(PD4, GPIO_CFGLR_OUT_10Mhz_PP);
}

// ---- ドメイン層 ------------------------------------------------
static const Ch32ButtonPort::RowPin kRows[ROW_COUNT] = {
    { GPIOD, ROW0_BIT },  // Row0: PD2 (PC0/LED3 から移動)
    { GPIOC, ROW1_BIT },  // Row1: PC1
    { GPIOC, ROW2_BIT },  // Row2: PC2
};
static const uint8_t kColBits[COL_COUNT] = { COL_BIT_0, COL_BIT_1, COL_BIT_2, COL_BIT_3, COL_BIT_4 };

static Ch32ButtonPort buttonPort(kRows, ROW_COUNT, kColBits, COL_COUNT);
static Ch32LedPort    ledPort;
static ButtonService  buttonService(buttonPort, BUTTON_COUNT);
static LedService     ledService(ledPort);

// ---- HID 送信バッファ ------------------------------------------
static volatile InputReport g_pending;
static volatile bool        g_hasPending;

// ---- エントリポイント -------------------------------------------
extern "C" int main()
{
    SystemInit();
    seamless_switch_init();
    millis_init();
    matrix_init();

    g_hasPending = false;

    Delay_Ms(1);  // USB 再列挙待ち (USB spec: TDDIS > 2.5µs)
    usb_setup();

    ledService.setConnected(true);

    // モード表示 LED: 実行モードでは LED3 (PC0) を常時点灯
    // 書き込み待機モード（ブートローダ）ではファームウェアが動かないため消灯
    GPIOC->BCR = (1u << LED_MODE_BIT);  // PC0 LOW → LED3 点灯

    DBG_PRINTF("mydeck: ready (rows=%d cols=%d)\n", ROW_COUNT, COL_COUNT);

    while (1) {
        ButtonPressResult ev = buttonService.update();
        if (ev.valid && !g_hasPending) {
            InputReport rpt;
            rpt.clear();
            rpt.buttonId = ev.buttonId;
            rpt.event    = static_cast<uint8_t>(ev.type);
            memcpy((void *)&g_pending, &rpt, sizeof(rpt));
            g_hasPending = true;
            DBG_PRINTF("btn %d ev %d\n", ev.buttonId, static_cast<int>(ev.type));
        }
        ledService.update();
    }
}

// ================================================================
// rv003usb コールバック (C リンケージ — rv003usb.c からリンクされる)
// ================================================================
extern "C" {

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
    e->max_len = (reqLen < REPORT_SIZE - 1) ? reqLen : REPORT_SIZE - 1;
}

void usb_handle_hid_get_report_start(struct usb_endpoint *e,
                                      int reqLen,
                                      uint32_t lValueLSBIndexMSB)
{
    usb_send_empty(0);
}

void usb_handle_user_data(struct usb_endpoint *e,
                           int current_endpoint,
                           uint8_t *data,
                           int len,
                           struct rv003usb_internal *ist)
{
    if (len < 2) return;
    // data[0]=command, data[1]=param (report ID はヘッダ側で処理済み)
    if (data[0] == static_cast<uint8_t>(OutputCommand::SetLed)) {
        ledService.setConnected(data[1] != 0);
    }
}

void usb_handle_other_control_message(struct usb_endpoint *e,
                                        struct usb_urb *s,
                                        struct rv003usb_internal *ist) {}

}  // extern "C"
