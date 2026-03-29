#include "led_service.h"

LedService::LedService(ILedPort& port) : port_(port), flashUntil_(0) {}

void LedService::setConnected(bool connected) {
    port_.setTxLed(connected);
}

void LedService::flash() {
    flashUntil_ = port_.millis() + FLASH_DURATION_MS;
    port_.setRxLed(true);
}

void LedService::update() {
    if (flashUntil_ > 0 && port_.millis() >= flashUntil_) {
        flashUntil_ = 0;
        port_.setRxLed(false);
    }
}
