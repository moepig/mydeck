#pragma once
#include "i_button_port.h"
#include "hid_report.h"

static constexpr uint8_t  MAX_BUTTONS       = 16;
static constexpr uint32_t DEBOUNCE_MS       = 50;
static constexpr uint32_t HOLD_THRESHOLD_MS = 500;

struct ButtonState {
    bool     confirmed;      // デバウンス確定済みの押下状態
    bool     raw;            // 最後に読んだ生の入力
    uint32_t rawChangedAt;   // raw が最後に変化した時刻
    uint32_t pressedAt;      // confirmed が true になった時刻（Hold 判定用）
    bool     holdFired;
};

// update() の戻り値。イベントが無い場合は valid = false
struct ButtonPressResult {
    bool        valid;
    uint8_t     buttonId;   // 1 始まり
    ButtonEvent type;
};

class ButtonService {
public:
    ButtonService(IButtonPort& port, uint8_t count);

    // 毎ループ呼び出す。イベントが発生した場合は valid=true の結果を返す
    ButtonPressResult update();

private:
    IButtonPort& port_;
    uint8_t      count_;
    ButtonState  states_[MAX_BUTTONS];
};
