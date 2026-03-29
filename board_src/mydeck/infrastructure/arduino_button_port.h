#pragma once
#ifndef UNIT_TEST

#include "../domain/i_button_port.h"
#include <Arduino.h>

// IButtonPort の Arduino 実装。INPUT_PULLUP 前提で反転読み取りを行う
struct ArduinoButtonPort : public IButtonPort {
    explicit ArduinoButtonPort(const uint8_t* pins) : pins_(pins) {}

    bool read(uint8_t pinIndex) override {
        return digitalRead(pins_[pinIndex]) == LOW;
    }

    uint32_t millis() override {
        return ::millis();
    }

private:
    const uint8_t* pins_;
};

#endif  // UNIT_TEST
