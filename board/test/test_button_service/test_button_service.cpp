#include <unity.h>

extern "C" {
#include "../src/domain/button_service.h"
#include "../src/domain/button_service.c"  // NOLINT: Unity ビルド
}

// ---- テスト用 ButtonPort モック ----------------------------
static bool     mock_states[MAX_BUTTONS];
static uint32_t mock_time;

static bool mock_read(void *, uint8_t i) { return mock_states[i]; }
static uint32_t mock_millis(void *) { return mock_time; }

static ButtonPort mock_port = { mock_read, mock_millis, NULL };

// ---------------------------------------------------------------
void setUp() {
    for (int i = 0; i < MAX_BUTTONS; i++) mock_states[i] = false;
    mock_time = 0;
}
void tearDown() {}

void test_no_event_when_idle() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);
    ButtonPressResult ev = button_service_update(&s);
    TEST_ASSERT_FALSE(ev.valid);
}

void test_press_event_after_debounce_stable() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_time = 0;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS;
    ButtonPressResult ev = button_service_update(&s);

    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(1, ev.buttonId);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESS, ev.type);
}

void test_no_event_if_released_before_debounce() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_time = 0;
    button_service_update(&s);

    mock_states[0] = false;
    mock_time = DEBOUNCE_MS - 1;
    ButtonPressResult ev = button_service_update(&s);

    TEST_ASSERT_FALSE(ev.valid);
}

void test_hold_event_fires_after_threshold() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_time = 0;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS;
    button_service_update(&s);  // Press

    mock_time = DEBOUNCE_MS + HOLD_THRESHOLD_MS;
    ButtonPressResult ev = button_service_update(&s);

    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(1, ev.buttonId);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_HOLD, ev.type);
}

void test_hold_fires_only_once() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_time = 0;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS + HOLD_THRESHOLD_MS;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS + HOLD_THRESHOLD_MS + 100;
    ButtonPressResult ev = button_service_update(&s);
    TEST_ASSERT_FALSE(ev.valid);
}

void test_release_event_after_debounce() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_time = 0;
    button_service_update(&s);
    mock_time = DEBOUNCE_MS;
    button_service_update(&s);

    mock_states[0] = false;
    mock_time = DEBOUNCE_MS + 1;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS * 2 + 1;
    ButtonPressResult ev = button_service_update(&s);

    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(1, ev.buttonId);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_RELEASE, ev.type);
}

void test_release_not_fired_after_hold() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_time = 0;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS + HOLD_THRESHOLD_MS;
    button_service_update(&s);

    mock_states[0] = false;
    mock_time = DEBOUNCE_MS + HOLD_THRESHOLD_MS + 1;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS + HOLD_THRESHOLD_MS + 1 + DEBOUNCE_MS;
    ButtonPressResult ev = button_service_update(&s);
    TEST_ASSERT_FALSE(ev.valid);
}

void test_second_button_independent() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[1] = true;
    mock_time = 0;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS;
    ButtonPressResult ev = button_service_update(&s);

    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(2, ev.buttonId);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_no_event_when_idle);
    RUN_TEST(test_press_event_after_debounce_stable);
    RUN_TEST(test_no_event_if_released_before_debounce);
    RUN_TEST(test_hold_event_fires_after_threshold);
    RUN_TEST(test_hold_fires_only_once);
    RUN_TEST(test_release_event_after_debounce);
    RUN_TEST(test_release_not_fired_after_hold);
    RUN_TEST(test_second_button_independent);
    return UNITY_END();
}
