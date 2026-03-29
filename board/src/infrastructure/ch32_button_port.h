#pragma once
#ifndef UNIT_TEST

#include "../domain/button_port.h"
#include "ch32fun.h"

typedef struct {
    GPIO_TypeDef *port;
    uint8_t       bit;
} Ch32RowPin;

typedef struct {
    const Ch32RowPin  *rows;
    const uint8_t     *col_bits;
    uint8_t            row_count;
    uint8_t            col_count;
} Ch32ButtonCtx;

static bool ch32_button_read(void *ctx, uint8_t pin_index) {
    Ch32ButtonCtx *c = (Ch32ButtonCtx *)ctx;
    uint8_t row = pin_index / c->col_count;
    uint8_t col = pin_index % c->col_count;

    /* 全 Row を HIGH にリセット (ゴースト防止) */
    for (uint8_t i = 0; i < c->row_count; i++)
        c->rows[i].port->BSHR = (1u << c->rows[i].bit);

    c->rows[row].port->BCR = (1u << c->rows[row].bit);  /* 対象 Row を LOW */
    /* 信号安定待ち (約5us @ 48MHz) */
    __asm__ volatile(
        "li t0, 60\n"
        "1: addi t0,t0,-1\n"
        "   bnez t0,1b\n"
        ::: "t0"
    );
    bool pressed = !(GPIOC->INDR & (1u << c->col_bits[col]));
    c->rows[row].port->BSHR = (1u << c->rows[row].bit);  /* Row を HIGH に戻す */

    return pressed;
}

static uint32_t ch32_button_millis(void *ctx) {
    (void)ctx;
    extern volatile uint32_t g_millis;
    return g_millis;
}

static inline ButtonPort ch32_button_port_create(Ch32ButtonCtx *ctx) {
    ButtonPort p = { ch32_button_read, ch32_button_millis, ctx };
    return p;
}

#endif  /* UNIT_TEST */
