#pragma once
#ifndef UNIT_TEST

#include "../domain/i_led_port.h"

// ILedPort の ch32fun 実装
// UIAPduino: LED3 (PC0) はモード表示に使用 (main.cpp で直接制御)
// LedService 経由の TX/RX LED 制御は行わない。
struct Ch32LedPort : public ILedPort {
    void setTxLed(bool on) override { (void)on; }
    void setRxLed(bool on) override { (void)on; }

    uint32_t millis() override {
        extern volatile uint32_t g_millis;
        return g_millis;
    }
};

#endif  // UNIT_TEST
