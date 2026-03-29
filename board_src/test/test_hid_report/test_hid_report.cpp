#include <unity.h>
#include "../mydeck/domain/hid_report.h"
#include <string.h>

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
    r.clear();

    TEST_ASSERT_EQUAL(REPORT_ID_INPUT, r.reportId);
    TEST_ASSERT_EQUAL(0, r.buttonId);
    TEST_ASSERT_EQUAL(0, r.event);
    TEST_ASSERT_EQUAL(0, r.modifier);
    for (auto b : r.reserved) {
        TEST_ASSERT_EQUAL(0, b);
    }
}

void test_report_id_constants() {
    TEST_ASSERT_EQUAL(0x01, REPORT_ID_INPUT);
    TEST_ASSERT_EQUAL(0x02, REPORT_ID_OUTPUT);
}

void test_button_event_values() {
    TEST_ASSERT_EQUAL(0x01, static_cast<uint8_t>(ButtonEvent::Press));
    TEST_ASSERT_EQUAL(0x02, static_cast<uint8_t>(ButtonEvent::Release));
    TEST_ASSERT_EQUAL(0x03, static_cast<uint8_t>(ButtonEvent::Hold));
}

void test_output_command_values() {
    TEST_ASSERT_EQUAL(0x01, static_cast<uint8_t>(OutputCommand::SetLed));
    TEST_ASSERT_EQUAL(0x02, static_cast<uint8_t>(OutputCommand::Echo));
}

// OutputReport のフィールドが期待どおりのバイト位置にあることを確認する
void test_output_report_fields_layout() {
    OutputReport r;
    memset(&r, 0, sizeof(r));

    r.reportId = REPORT_ID_OUTPUT;
    r.command  = static_cast<uint8_t>(OutputCommand::SetLed);
    r.param    = 0xAB;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(&r);
    TEST_ASSERT_EQUAL(REPORT_ID_OUTPUT,                           raw[0]);
    TEST_ASSERT_EQUAL(static_cast<uint8_t>(OutputCommand::SetLed), raw[1]);
    TEST_ASSERT_EQUAL(0xAB,                                       raw[2]);
    // reserved bytes (3-7) はすべて 0
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
