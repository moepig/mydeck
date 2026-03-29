#pragma once
#include <stdint.h>

// ボタン入力の読み取りをハードウェアから切り離すポートインターフェース
struct IButtonPort {
    virtual bool read(uint8_t pinIndex) = 0;  // true = 押下中
    virtual uint32_t millis() = 0;
    virtual ~IButtonPort() = default;
};
