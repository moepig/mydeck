#pragma once
#include <stdint.h>

// HID レポートレイアウト（8バイト固定）
//
// Input Report（デバイス → PC）
// Byte 0 : Report ID  固定値 REPORT_ID_INPUT (0x01)
// Byte 1 : ボタン番号 0x01〜0x10
// Byte 2 : イベント種別 ButtonEvent を参照
// Byte 3 : 修飾フラグ（将来拡張用 bitmask）
// Byte 4-7 : 予約（0x00）
//
// Output Report（PC → デバイス）
// Byte 0 : Report ID  固定値 REPORT_ID_OUTPUT (0x02)
// Byte 1 : コマンド種別 OutputCommand を参照
// Byte 2 : パラメータ
// Byte 3-7 : 予約（0x00）

static constexpr uint8_t REPORT_ID_INPUT  = 0x01;
static constexpr uint8_t REPORT_ID_OUTPUT = 0x02;
static constexpr uint8_t REPORT_SIZE      = 8;

enum class ButtonEvent : uint8_t {
    Press   = 0x01,
    Release = 0x02,
    Hold    = 0x03,
};

enum class OutputCommand : uint8_t {
    SetLed  = 0x01,
    Echo    = 0x02,
};

struct InputReport {
    uint8_t reportId;
    uint8_t buttonId;
    uint8_t event;
    uint8_t modifier;
    uint8_t reserved[4];

    void clear() {
        reportId = REPORT_ID_INPUT;
        buttonId = 0;
        event    = 0;
        modifier = 0;
        reserved[0] = reserved[1] = reserved[2] = reserved[3] = 0;
    }
};

struct OutputReport {
    uint8_t reportId;
    uint8_t command;
    uint8_t param;
    uint8_t reserved[5];
};

static_assert(sizeof(InputReport)  == REPORT_SIZE, "InputReport size mismatch");
static_assert(sizeof(OutputReport) == REPORT_SIZE, "OutputReport size mismatch");
