#pragma once
#ifndef UNIT_TEST

#include "../domain/i_button_port.h"
#include <Arduino.h>

// IButtonPort の Arduino 実装。Row/Column マトリクススキャンで読み取りを行う。
// pinIndex はリニアなボタン番号（0 始まり）で、row = pinIndex / colCount,
// col = pinIndex % colCount に変換してスキャンする。
struct ArduinoButtonPort : public IButtonPort {
    ArduinoButtonPort(const uint8_t* rowPins, uint8_t rowCount,
                      const uint8_t* colPins, uint8_t colCount)
        : rowPins_(rowPins), colPins_(colPins),
          rowCount_(rowCount), colCount_(colCount) {}

    bool read(uint8_t pinIndex) override {
        uint8_t row = pinIndex / colCount_;
        uint8_t col = pinIndex % colCount_;

        // 対象 Row を LOW に駆動してスキャン
        digitalWrite(rowPins_[row], LOW);
        delayMicroseconds(5);  // 信号安定待ち
        bool pressed = (digitalRead(colPins_[col]) == LOW);
        digitalWrite(rowPins_[row], HIGH);

        return pressed;
    }

    uint32_t millis() override {
        return ::millis();
    }

private:
    const uint8_t* rowPins_;
    const uint8_t* colPins_;
    uint8_t        rowCount_;
    uint8_t        colCount_;
};

#endif  // UNIT_TEST
