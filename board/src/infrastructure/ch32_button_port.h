#pragma once
#ifndef UNIT_TEST

#include "../domain/i_button_port.h"
#include "ch32fun.h"

// IButtonPort の ch32fun 実装
// Row ピンは複数の GPIO ポートに分散可能 (RowPin で port + bit を指定)
// Col ピンは全て GPIOC (ビット番号で指定)
struct Ch32ButtonPort : public IButtonPort {
    struct RowPin {
        GPIO_TypeDef *port;
        uint8_t       bit;
    };

    Ch32ButtonPort(const RowPin *rows, uint8_t rowCount,
                   const uint8_t *colBits, uint8_t colCount)
        : rows_(rows), colBits_(colBits),
          rowCount_(rowCount), colCount_(colCount) {}

    bool read(uint8_t pinIndex) override {
        uint8_t row = pinIndex / colCount_;
        uint8_t col = pinIndex % colCount_;

        // 全 Row を HIGH にリセット (ゴースト防止)
        for (uint8_t i = 0; i < rowCount_; i++)
            rows_[i].port->BSHR = (1u << rows_[i].bit);

        rows_[row].port->BCR = (1u << rows_[row].bit);  // 対象 Row を LOW に駆動
        // 信号安定待ち (約5µs @ 48MHz)
        __asm__ volatile(
            "li t0, 60\n"
            "1: addi t0,t0,-1\n"
            "   bnez t0,1b\n"
            ::: "t0"
        );
        bool pressed = !(GPIOC->INDR & (1u << colBits_[col]));
        rows_[row].port->BSHR = (1u << rows_[row].bit);  // Row を HIGH に戻す

        return pressed;
    }

    uint32_t millis() override {
        extern volatile uint32_t g_millis;
        return g_millis;
    }

private:
    const RowPin  *rows_;
    const uint8_t *colBits_;
    uint8_t        rowCount_;
    uint8_t        colCount_;
};

#endif  // UNIT_TEST
