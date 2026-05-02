#pragma once
// UIAPduino Pro Micro CH32V003 V1.4 — ch32fun ピン定義
// Arduino D2-D9 = CH32V003 PC0-PC7

#define ROW_COUNT    2
#define COL_COUNT    6
#define BUTTON_COUNT (ROW_COUNT * COL_COUNT)  // 12

// Row ピン — ポート・ビット (main.c の kRows[] で GPIO_TypeDef* と組み合わせる)
// Row0 は PD2 に配置し、PC0 を LED3 モード表示専用に解放
#define ROW0_BIT  2u  // PD2
#define ROW1_BIT  1u  // PC1 = Arduino D3

// Col ピン — GPIOC ビット番号 (全て GPIOC)
#define COL_BIT_0  2u  // PC2 = Arduino D4
#define COL_BIT_1  3u  // PC3 = Arduino D5
#define COL_BIT_2  4u  // PC4 = Arduino D6
#define COL_BIT_3  5u  // PC5 = Arduino D7 (左右ピン共通)
#define COL_BIT_4  6u  // PC6 = Arduino D8 (左右ピン共通)
#define COL_BIT_5  7u  // PC7 = Arduino D9 (左右ピン共通)

// LED3 モード表示 (PC0) — 実行モード: 点滅 / 書き込み待機: 消灯
#define LED_MODE_BIT  0u  // GPIOC bit 0
