#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool (*read)(void *ctx, uint8_t pin_index);
    uint32_t (*millis)(void *ctx);
    void *ctx;
} ButtonPort;
