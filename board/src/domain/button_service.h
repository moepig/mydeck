#pragma once
#include "button_port.h"
#include "hid_report.h"

#define MAX_BUTTONS       16
#define DEBOUNCE_MS       50
#define HOLD_THRESHOLD_MS 500

typedef struct {
    bool     confirmed;
    bool     raw;
    uint32_t rawChangedAt;
    uint32_t pressedAt;
    bool     holdFired;
} ButtonState;

typedef struct {
    bool             valid;
    uint8_t          buttonId;   /* 0 始まり (= pin_index) */
    enum ButtonEvent type;
} ButtonPressResult;

typedef struct {
    ButtonPort *port;
    uint8_t     count;
    ButtonState states[MAX_BUTTONS];
} ButtonService;

void button_service_init(ButtonService *svc, ButtonPort *port, uint8_t count);
ButtonPressResult button_service_update(ButtonService *svc);
