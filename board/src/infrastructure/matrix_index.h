#pragma once
#include <stdint.h>

/* pin_index と行・列の相互変換 (行優先 row-major) */

static inline uint8_t matrix_row(uint8_t pin_index, uint8_t col_count) {
    return pin_index / col_count;
}

static inline uint8_t matrix_col(uint8_t pin_index, uint8_t col_count) {
    return pin_index % col_count;
}

static inline uint8_t matrix_pin_index(uint8_t row, uint8_t col, uint8_t col_count) {
    return row * col_count + col;
}
