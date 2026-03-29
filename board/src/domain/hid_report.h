#pragma once
#include <stdint.h>
#include <string.h>

/* HID レポートレイアウト（8バイト固定）
 *
 * Input Report（デバイス → PC）
 * Byte 0 : Report ID  固定値 REPORT_ID_INPUT (0x01)
 * Byte 1 : ボタン番号 0x00〜0x0F (0 始まり = pin_index)
 * Byte 2 : イベント種別 ButtonEvent を参照
 * Byte 3-7 : 予約（0x00）
 *
 * Output Report（PC → デバイス）
 * Byte 0 : Report ID  固定値 REPORT_ID_OUTPUT (0x02)
 * Byte 1 : コマンド種別 OutputCommand を参照
 * Byte 2 : パラメータ
 * Byte 3-7 : 予約（0x00）
 */

#define REPORT_ID_INPUT  0x01
#define REPORT_ID_OUTPUT 0x02
#define REPORT_SIZE      8

enum ButtonEvent {
    BUTTON_EVENT_PRESS   = 0x01,
    BUTTON_EVENT_RELEASE = 0x02,
    BUTTON_EVENT_HOLD    = 0x03,
};

enum OutputCommand {
    OUTPUT_CMD_SET_LED = 0x01,
};

typedef struct {
    uint8_t reportId;
    uint8_t buttonId;
    uint8_t event;
    uint8_t reserved[5];
} InputReport;

typedef struct {
    uint8_t reportId;
    uint8_t command;
    uint8_t param;
    uint8_t reserved[5];
} OutputReport;

static inline void input_report_clear(InputReport *r) {
    memset(r, 0, sizeof(*r));
    r->reportId = REPORT_ID_INPUT;
}

#ifdef __cplusplus
static_assert(sizeof(InputReport)  == REPORT_SIZE, "InputReport size mismatch");
static_assert(sizeof(OutputReport) == REPORT_SIZE, "OutputReport size mismatch");
#else
_Static_assert(sizeof(InputReport)  == REPORT_SIZE, "InputReport size mismatch");
_Static_assert(sizeof(OutputReport) == REPORT_SIZE, "OutputReport size mismatch");
#endif
