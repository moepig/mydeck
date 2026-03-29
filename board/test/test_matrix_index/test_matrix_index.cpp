#include <unity.h>

extern "C" {
#include "../src/infrastructure/matrix_index.h"
}

void setUp() {}
void tearDown() {}

// ---- matrix_row / matrix_col ------------------------------------

void test_first_element() {
    TEST_ASSERT_EQUAL(0, matrix_row(0, 4));
    TEST_ASSERT_EQUAL(0, matrix_col(0, 4));
}

void test_last_col_in_first_row() {
    TEST_ASSERT_EQUAL(0, matrix_row(3, 4));
    TEST_ASSERT_EQUAL(3, matrix_col(3, 4));
}

void test_first_col_in_second_row() {
    TEST_ASSERT_EQUAL(1, matrix_row(4, 4));
    TEST_ASSERT_EQUAL(0, matrix_col(4, 4));
}

void test_last_element_4x4() {
    TEST_ASSERT_EQUAL(3, matrix_row(15, 4));
    TEST_ASSERT_EQUAL(3, matrix_col(15, 4));
}

void test_3x5_middle() {
    // pin_index=7 → row=1, col=2
    TEST_ASSERT_EQUAL(1, matrix_row(7, 5));
    TEST_ASSERT_EQUAL(2, matrix_col(7, 5));
}

void test_3x5_last() {
    // pin_index=14 → row=2, col=4
    TEST_ASSERT_EQUAL(2, matrix_row(14, 5));
    TEST_ASSERT_EQUAL(4, matrix_col(14, 5));
}

// ---- matrix_pin_index -------------------------------------------

void test_pin_index_first() {
    TEST_ASSERT_EQUAL(0, matrix_pin_index(0, 0, 4));
}

void test_pin_index_row1_col0() {
    TEST_ASSERT_EQUAL(4, matrix_pin_index(1, 0, 4));
}

void test_pin_index_last_4x4() {
    TEST_ASSERT_EQUAL(15, matrix_pin_index(3, 3, 4));
}

void test_pin_index_last_3x5() {
    TEST_ASSERT_EQUAL(14, matrix_pin_index(2, 4, 5));
}

// ---- round-trip -------------------------------------------------

void test_round_trip_4x4() {
    for (uint8_t i = 0; i < 16; i++) {
        uint8_t r = matrix_row(i, 4);
        uint8_t c = matrix_col(i, 4);
        TEST_ASSERT_EQUAL(i, matrix_pin_index(r, c, 4));
    }
}

void test_round_trip_3x5() {
    for (uint8_t i = 0; i < 15; i++) {
        uint8_t r = matrix_row(i, 5);
        uint8_t c = matrix_col(i, 5);
        TEST_ASSERT_EQUAL(i, matrix_pin_index(r, c, 5));
    }
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_first_element);
    RUN_TEST(test_last_col_in_first_row);
    RUN_TEST(test_first_col_in_second_row);
    RUN_TEST(test_last_element_4x4);
    RUN_TEST(test_3x5_middle);
    RUN_TEST(test_3x5_last);
    RUN_TEST(test_pin_index_first);
    RUN_TEST(test_pin_index_row1_col0);
    RUN_TEST(test_pin_index_last_4x4);
    RUN_TEST(test_pin_index_last_3x5);
    RUN_TEST(test_round_trip_4x4);
    RUN_TEST(test_round_trip_3x5);
    return UNITY_END();
}
