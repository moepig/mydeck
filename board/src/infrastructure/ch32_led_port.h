#pragma once
#ifndef UNIT_TEST

#include "../domain/led_port.h"

/* UIAPduino: LED3 (PC0) はモード表示に使用 (main.c で直接制御)
 * LedService 経由の TX/RX LED 制御は行わない */

static void ch32_led_set_tx(void *ctx, bool on) { (void)ctx; (void)on; }
static void ch32_led_set_rx(void *ctx, bool on) { (void)ctx; (void)on; }

static uint32_t ch32_led_millis(void *ctx) {
    (void)ctx;
    extern volatile uint32_t g_millis;
    return g_millis;
}

static inline LedPort ch32_led_port_create(void) {
    LedPort p = { ch32_led_set_tx, ch32_led_set_rx, ch32_led_millis, NULL };
    return p;
}

#endif  /* UNIT_TEST */
