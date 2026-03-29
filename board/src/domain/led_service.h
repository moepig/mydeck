#pragma once
#include "led_port.h"

#define FLASH_DURATION_MS 80

typedef struct {
    LedPort *port;
    uint32_t flashUntil;
} LedService;

void led_service_init(LedService *svc, LedPort *port);
void led_service_set_connected(LedService *svc, bool connected);
void led_service_flash(LedService *svc);
void led_service_update(LedService *svc);
