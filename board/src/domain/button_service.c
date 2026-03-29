#include "button_service.h"

void button_service_init(ButtonService *svc, ButtonPort *port, uint8_t count) {
    svc->port  = port;
    svc->count = count;
    for (uint8_t i = 0; i < count; i++) {
        svc->states[i].confirmed    = false;
        svc->states[i].raw          = false;
        svc->states[i].rawChangedAt = 0;
        svc->states[i].pressedAt    = 0;
        svc->states[i].holdFired    = false;
    }
}

ButtonPressResult button_service_update(ButtonService *svc) {
    uint32_t now = svc->port->millis(svc->port->ctx);
    ButtonPressResult none = { false, 0, BUTTON_EVENT_PRESS };

    for (uint8_t i = 0; i < svc->count; i++) {
        bool newRaw = svc->port->read(svc->port->ctx, i);
        ButtonState *s = &svc->states[i];

        if (newRaw != s->raw) {
            s->raw          = newRaw;
            s->rawChangedAt = now;
        }

        if (s->raw != s->confirmed && (now - s->rawChangedAt) >= DEBOUNCE_MS) {
            s->confirmed = s->raw;
            if (s->confirmed) {
                s->pressedAt = now;
                s->holdFired = false;
                ButtonPressResult r = { true, (uint8_t)(i + 1), BUTTON_EVENT_PRESS };
                return r;
            } else {
                if (!s->holdFired) {
                    ButtonPressResult r = { true, (uint8_t)(i + 1), BUTTON_EVENT_RELEASE };
                    return r;
                }
            }
        }

        if (s->confirmed && !s->holdFired && (now - s->pressedAt) >= HOLD_THRESHOLD_MS) {
            s->holdFired = true;
            ButtonPressResult r = { true, (uint8_t)(i + 1), BUTTON_EVENT_HOLD };
            return r;
        }
    }

    return none;
}
