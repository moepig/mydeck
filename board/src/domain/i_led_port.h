#pragma once
#include <stdint.h>

// LED 制御をハードウェアから切り離すポートインターフェース
struct ILedPort {
    virtual void setTxLed(bool on) = 0;  // 接続表示 LED
    virtual void setRxLed(bool on) = 0;  // ボタン押下フラッシュ LED
    virtual uint32_t millis() = 0;
    virtual ~ILedPort() = default;
};
