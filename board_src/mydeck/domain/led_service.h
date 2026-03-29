#pragma once
#include "i_led_port.h"

static constexpr uint32_t FLASH_DURATION_MS = 80;

class LedService {
public:
    explicit LedService(ILedPort& port);

    void setConnected(bool connected);
    void flash();   // ボタン押下時の短時間点灯
    void update();  // 毎ループ呼び出す（点灯タイマ管理）

private:
    ILedPort& port_;
    uint32_t  flashUntil_;
};
