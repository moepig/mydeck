#pragma once
#include <stdint.h>

// SparkFun Pro Micro (ATmega32U4, 16MHz/5V) — 4×4 マトリクス (16 ボタン)
static const uint8_t ROW_PINS[] = {2, 3, 4, 5};
static const uint8_t COL_PINS[] = {6, 7, 8, 9};
#define BOARD_LED_TX TXLED0
#define BOARD_LED_RX RXLED0

static constexpr uint8_t ROW_COUNT    = sizeof(ROW_PINS) / sizeof(ROW_PINS[0]);
static constexpr uint8_t COL_COUNT    = sizeof(COL_PINS) / sizeof(COL_PINS[0]);
static constexpr uint8_t BUTTON_COUNT = ROW_COUNT * COL_COUNT;
