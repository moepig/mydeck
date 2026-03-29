#pragma once
#ifndef UNIT_TEST

#include "../domain/i_led_port.h"
#include <Arduino.h>

// ILedPort の Arduino 実装。Pro Micro の負論理 LED に対応
class ArduinoLedPort : public ILedPort {
public:
    ArduinoLedPort(uint8_t txPin, uint8_t rxPin);

    // ピンを OUTPUT に設定し消灯した状態で初期化する
    void begin();

    void setTxLed(bool on) override;
    void setRxLed(bool on) override;
    uint32_t millis() override;

private:
    uint8_t txPin_;
    uint8_t rxPin_;
};

#endif  // UNIT_TEST
