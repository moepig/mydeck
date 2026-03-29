#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    void (*set_tx_led)(void *ctx, bool on);
    void (*set_rx_led)(void *ctx, bool on);
    uint32_t (*millis)(void *ctx);
    void *ctx;
} LedPort;
