#pragma once
#ifndef UNIT_TEST

#include "../domain/button_port.h"
#include "matrix_index.h"
#include <Arduino.h>

struct ArduinoButtonCtx {
    const uint8_t *row_pins;
    const uint8_t *col_pins;
    uint8_t        row_count;
    uint8_t        col_count;
};

/* pin_index は行優先 (row-major): pin_index = row * col_count + col */
static bool arduino_button_read(void *ctx, uint8_t pin_index) {
    ArduinoButtonCtx *c = (ArduinoButtonCtx *)ctx;
    uint8_t row = matrix_row(pin_index, c->col_count);
    uint8_t col = matrix_col(pin_index, c->col_count);

    digitalWrite(c->row_pins[row], LOW);
    delayMicroseconds(5);
    bool pressed = (digitalRead(c->col_pins[col]) == LOW);
    digitalWrite(c->row_pins[row], HIGH);

    return pressed;
}

static uint32_t arduino_button_millis(void *ctx) {
    (void)ctx;
    return ::millis();
}

static inline ButtonPort arduino_button_port_create(ArduinoButtonCtx *ctx) {
    ButtonPort p = { arduino_button_read, arduino_button_millis, ctx };
    return p;
}

#endif  // UNIT_TEST
