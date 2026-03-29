#include "led_service.h"

void led_service_init(LedService *svc, LedPort *port) {
    svc->port       = port;
    svc->flashUntil = 0;
}

void led_service_set_connected(LedService *svc, bool connected) {
    svc->port->set_tx_led(svc->port->ctx, connected);
}

void led_service_flash(LedService *svc) {
    svc->flashUntil = svc->port->millis(svc->port->ctx) + FLASH_DURATION_MS;
    svc->port->set_rx_led(svc->port->ctx, true);
}

void led_service_update(LedService *svc) {
    if (svc->flashUntil > 0 && svc->port->millis(svc->port->ctx) >= svc->flashUntil) {
        svc->flashUntil = 0;
        svc->port->set_rx_led(svc->port->ctx, false);
    }
}
