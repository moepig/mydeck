#include "button_service.h"

ButtonService::ButtonService(IButtonPort& port, uint8_t count)
    : port_(port), count_(count)
{
    for (uint8_t i = 0; i < count_; i++) {
        states_[i] = {false, false, 0, 0, false};
    }
}

ButtonPressResult ButtonService::update() {
    uint32_t now = port_.millis();

    for (uint8_t i = 0; i < count_; i++) {
        bool newRaw = port_.read(i);
        ButtonState& s = states_[i];

        // 生入力が変化したとき時刻を記録
        if (newRaw != s.raw) {
            s.raw          = newRaw;
            s.rawChangedAt = now;
        }

        // DEBOUNCE_MS 以上同じ状態が続いたら確定
        if (s.raw != s.confirmed && (now - s.rawChangedAt) >= DEBOUNCE_MS) {
            s.confirmed = s.raw;
            if (s.confirmed) {
                // 押下確定
                s.pressedAt = now;
                s.holdFired = false;
                return {true, static_cast<uint8_t>(i + 1), ButtonEvent::Press};
            } else {
                // 離し確定
                if (!s.holdFired) {
                    return {true, static_cast<uint8_t>(i + 1), ButtonEvent::Release};
                }
            }
        }

        // Hold 判定
        if (s.confirmed && !s.holdFired && (now - s.pressedAt) >= HOLD_THRESHOLD_MS) {
            s.holdFired = true;
            return {true, static_cast<uint8_t>(i + 1), ButtonEvent::Hold};
        }
    }

    return {false, 0, ButtonEvent::Press};
}
