#pragma once
#include <stdint.h>

// ---- ボード別ピン設定 ----------------------------------------
// platformio.ini の build_flags で BOARD_PROMICRO または
// BOARD_UIAPDUINO を定義してください。

#if defined(BOARD_PROMICRO)

// SparkFun Pro Micro (ATmega32U4, 16MHz/5V)
static const uint8_t BUTTON_PINS[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10,  // デジタルピン
    14, 15, 16,                   // SPI 兼用ピン（MISO/SCK/MOSI）
    18, 19, 20, 21                // アナログ兼用ピン（A0〜A3）
};
#define BOARD_LED_TX TXLED0
#define BOARD_LED_RX RXLED0

#elif defined(BOARD_UIAPDUINO)

// UIAPduino (ATmega32U4 系)
// ※ 実際のハードウェアのピン配置に合わせて変更してください
static const uint8_t BUTTON_PINS[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,  // デジタルピン
    14, 15, 16, 17                         // アナログ兼用ピン（A0〜A3）
};
#define BOARD_LED_TX TXLED0
#define BOARD_LED_RX RXLED0

#else
#error "ボードが未定義です。platformio.ini の build_flags に -DBOARD_PROMICRO または -DBOARD_UIAPDUINO を追加してください。"
#endif

static constexpr uint8_t BUTTON_COUNT = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);
