#ifndef UNIT_TEST

#include "arduino_led_port.h"

ArduinoLedPort::ArduinoLedPort(uint8_t txPin, uint8_t rxPin)
    : txPin_(txPin), rxPin_(rxPin) {}

void ArduinoLedPort::begin() {
    pinMode(txPin_, OUTPUT);
    pinMode(rxPin_, OUTPUT);
    // Pro Micro は負論理（HIGH = 消灯）
    digitalWrite(txPin_, HIGH);
    digitalWrite(rxPin_, HIGH);
}

void ArduinoLedPort::setTxLed(bool on) {
    digitalWrite(txPin_, on ? LOW : HIGH);
}

void ArduinoLedPort::setRxLed(bool on) {
    digitalWrite(rxPin_, on ? LOW : HIGH);
}

uint32_t ArduinoLedPort::millis() {
    return ::millis();
}

#endif  // UNIT_TEST
