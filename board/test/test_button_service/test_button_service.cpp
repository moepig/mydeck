#include <unity.h>
#include "../src/domain/button_service.h"
#include "../src/domain/button_service.cpp"  // NOLINT: Unity ビルド

// ---- テスト用 IButtonPort モック ----------------------------
struct MockButtonPort : public IButtonPort {
    bool states[MAX_BUTTONS] = {};
    uint32_t time = 0;

    bool read(uint8_t pinIndex) override { return states[pinIndex]; }
    uint32_t millis() override { return time; }
};

static MockButtonPort mock;

// ---------------------------------------------------------------
void setUp() {
    for (auto& s : mock.states) s = false;
    mock.time = 0;
}
void tearDown() {}

void test_no_event_when_idle() {
    ButtonService s(mock, 4);
    ButtonPressResult ev = s.update();
    TEST_ASSERT_FALSE(ev.valid);
}

// 押下後 DEBOUNCE_MS 安定したとき Press が発火する
void test_press_event_after_debounce_stable() {
    ButtonService s(mock, 4);

    mock.states[0] = true;
    mock.time = 0;
    s.update();

    mock.time = DEBOUNCE_MS;
    ButtonPressResult ev = s.update();

    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(1, ev.buttonId);
    TEST_ASSERT_EQUAL(static_cast<uint8_t>(ButtonEvent::Press),
                      static_cast<uint8_t>(ev.type));
}

// DEBOUNCE_MS 未満で離した場合は Press も Release も発火しない
void test_no_event_if_released_before_debounce() {
    ButtonService s(mock, 4);

    mock.states[0] = true;
    mock.time = 0;
    s.update();

    mock.states[0] = false;
    mock.time = DEBOUNCE_MS - 1;
    ButtonPressResult ev = s.update();

    TEST_ASSERT_FALSE(ev.valid);
}

// Press 確定後、さらに HOLD_THRESHOLD_MS 後に Hold が発火する
void test_hold_event_fires_after_threshold() {
    ButtonService s(mock, 4);

    mock.states[0] = true;
    mock.time = 0;
    s.update();

    mock.time = DEBOUNCE_MS;
    s.update();  // Press 確定（pressedAt = DEBOUNCE_MS）

    mock.time = DEBOUNCE_MS + HOLD_THRESHOLD_MS;
    ButtonPressResult ev = s.update();

    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(1, ev.buttonId);
    TEST_ASSERT_EQUAL(static_cast<uint8_t>(ButtonEvent::Hold),
                      static_cast<uint8_t>(ev.type));
}

void test_hold_fires_only_once() {
    ButtonService s(mock, 4);

    mock.states[0] = true;
    mock.time = 0;
    s.update();

    mock.time = DEBOUNCE_MS;
    s.update();  // Press 確定

    mock.time = DEBOUNCE_MS + HOLD_THRESHOLD_MS;
    s.update();  // Hold 発火

    mock.time = DEBOUNCE_MS + HOLD_THRESHOLD_MS + 100;
    ButtonPressResult ev = s.update();  // 2 回目は発火しない
    TEST_ASSERT_FALSE(ev.valid);
}

// 離し確定後に Release が発火する
void test_release_event_after_debounce() {
    ButtonService s(mock, 4);

    mock.states[0] = true;
    mock.time = 0;
    s.update();
    mock.time = DEBOUNCE_MS;
    s.update();  // Press

    mock.states[0] = false;
    mock.time = DEBOUNCE_MS + 1;
    s.update();  // raw 変化を記録

    mock.time = DEBOUNCE_MS * 2 + 1;
    ButtonPressResult ev = s.update();  // 離し確定 → Release

    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(1, ev.buttonId);
    TEST_ASSERT_EQUAL(static_cast<uint8_t>(ButtonEvent::Release),
                      static_cast<uint8_t>(ev.type));
}

// Hold 発火後にボタンを離しても Release は発火しない
void test_release_not_fired_after_hold() {
    ButtonService s(mock, 4);

    mock.states[0] = true;
    mock.time = 0;
    s.update();

    mock.time = DEBOUNCE_MS;
    s.update();  // Press 確定

    mock.time = DEBOUNCE_MS + HOLD_THRESHOLD_MS;
    s.update();  // Hold 発火

    // ボタンを離す
    mock.states[0] = false;
    mock.time = DEBOUNCE_MS + HOLD_THRESHOLD_MS + 1;
    s.update();  // raw 変化を記録

    mock.time = DEBOUNCE_MS + HOLD_THRESHOLD_MS + 1 + DEBOUNCE_MS;
    ButtonPressResult ev = s.update();  // 離し確定 → Release は抑制される
    TEST_ASSERT_FALSE(ev.valid);
}

void test_second_button_independent() {
    ButtonService s(mock, 4);

    mock.states[1] = true;
    mock.time = 0;
    s.update();

    mock.time = DEBOUNCE_MS;
    ButtonPressResult ev = s.update();  // ボタン2 の Press 確定

    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(2, ev.buttonId);  // ボタン番号は 1 始まり
}

// ---------------------------------------------------------------
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
