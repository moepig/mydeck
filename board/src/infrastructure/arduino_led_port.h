#pragma once
#ifndef UNIT_TEST

#include "../domain/led_port.h"
#include <Arduino.h>

struct ArduinoLedCtx {
    uint8_t tx_pin;
    uint8_t rx_pin;
};

static void arduino_led_set_tx(void *ctx, bool on) {
    ArduinoLedCtx *c = (ArduinoLedCtx *)ctx;
    digitalWrite(c->tx_pin, on ? LOW : HIGH);  /* Pro Micro は負論理 */
}

static void arduino_led_set_rx(void *ctx, bool on) {
    ArduinoLedCtx *c = (ArduinoLedCtx *)ctx;
    digitalWrite(c->rx_pin, on ? LOW : HIGH);
}

static uint32_t arduino_led_millis(void *ctx) {
    (void)ctx;
    return ::millis();
}

static inline void arduino_led_port_begin(ArduinoLedCtx *ctx) {
    pinMode(ctx->tx_pin, OUTPUT);
    pinMode(ctx->rx_pin, OUTPUT);
    digitalWrite(ctx->tx_pin, HIGH);  /* 消灯 */
    digitalWrite(ctx->rx_pin, HIGH);
}

static inline LedPort arduino_led_port_create(ArduinoLedCtx *ctx) {
    LedPort p = { arduino_led_set_tx, arduino_led_set_rx, arduino_led_millis, ctx };
    return p;
}

#endif  // UNIT_TEST
