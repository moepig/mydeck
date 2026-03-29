#include <unity.h>
#include <string.h>

extern "C" {
#include "../src/domain/hid_report.h"
}

void setUp() {}
void tearDown() {}

void test_input_report_size_is_8() {
    TEST_ASSERT_EQUAL(8, sizeof(InputReport));
}

void test_output_report_size_is_8() {
    TEST_ASSERT_EQUAL(8, sizeof(OutputReport));
}

void test_input_report_clear() {
    InputReport r;
    r.buttonId = 0xFF;
    r.event    = 0xFF;
    input_report_clear(&r);

    TEST_ASSERT_EQUAL(REPORT_ID_INPUT, r.reportId);
    TEST_ASSERT_EQUAL(0, r.buttonId);
    TEST_ASSERT_EQUAL(0, r.event);
    TEST_ASSERT_EQUAL(0, r.modifier);
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL(0, r.reserved[i]);
    }
}

void test_report_id_constants() {
    TEST_ASSERT_EQUAL(0x01, REPORT_ID_INPUT);
    TEST_ASSERT_EQUAL(0x02, REPORT_ID_OUTPUT);
}

void test_button_event_values() {
    TEST_ASSERT_EQUAL(0x01, BUTTON_EVENT_PRESS);
    TEST_ASSERT_EQUAL(0x02, BUTTON_EVENT_RELEASE);
    TEST_ASSERT_EQUAL(0x03, BUTTON_EVENT_HOLD);
}

void test_output_command_values() {
    TEST_ASSERT_EQUAL(0x01, OUTPUT_CMD_SET_LED);
    TEST_ASSERT_EQUAL(0x02, OUTPUT_CMD_ECHO);
}

void test_output_report_fields_layout() {
    OutputReport r;
    memset(&r, 0, sizeof(r));

    r.reportId = REPORT_ID_OUTPUT;
    r.command  = OUTPUT_CMD_SET_LED;
    r.param    = 0xAB;

    const uint8_t *raw = (const uint8_t *)&r;
    TEST_ASSERT_EQUAL(REPORT_ID_OUTPUT, raw[0]);
    TEST_ASSERT_EQUAL(OUTPUT_CMD_SET_LED, raw[1]);
    TEST_ASSERT_EQUAL(0xAB, raw[2]);
    for (int i = 3; i < 8; i++) {
        TEST_ASSERT_EQUAL(0, raw[i]);
    }
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_input_report_size_is_8);
    RUN_TEST(test_output_report_size_is_8);
    RUN_TEST(test_input_report_clear);
    RUN_TEST(test_report_id_constants);
    RUN_TEST(test_button_event_values);
    RUN_TEST(test_output_command_values);
    RUN_TEST(test_output_report_fields_layout);
    return UNITY_END();
}
