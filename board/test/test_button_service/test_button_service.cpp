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
    TEST_ASSERT_EQUAL(0, ev.buttonId);
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
    TEST_ASSERT_EQUAL(0, ev.buttonId);
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
    TEST_ASSERT_EQUAL(0, ev.buttonId);
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
    TEST_ASSERT_EQUAL(1, ev.buttonId);
}

// デバウンス中にチャタリング（ON→OFF→ON）が起きた場合、
// タイマーがリセットされ最終状態から再度 DEBOUNCE_MS 待つ
void test_bounce_resets_debounce_timer() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_time = 0;
    button_service_update(&s);

    // チャタリング: DEBOUNCE_MS 前に OFF→ON
    mock_states[0] = false;
    mock_time = 20;
    button_service_update(&s);

    mock_states[0] = true;
    mock_time = 30;
    button_service_update(&s);

    // 最初の ON から DEBOUNCE_MS 経過しても、タイマーリセットで未確定
    mock_time = DEBOUNCE_MS;
    ButtonPressResult ev = button_service_update(&s);
    TEST_ASSERT_FALSE(ev.valid);

    // 最後の ON (t=30) から DEBOUNCE_MS 経過で確定
    mock_time = 30 + DEBOUNCE_MS;
    ev = button_service_update(&s);
    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESS, ev.type);
}

// 2 番目以降のボタンでも Hold が正しく発火する
void test_hold_on_second_button() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[2] = true;
    mock_time = 0;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS;
    ButtonPressResult ev = button_service_update(&s);
    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(2, ev.buttonId);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESS, ev.type);

    mock_time = DEBOUNCE_MS + HOLD_THRESHOLD_MS;
    ev = button_service_update(&s);
    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(2, ev.buttonId);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_HOLD, ev.type);
}

// 2 つのボタンが同時に押された場合、update() 1 回につき 1 イベントのみ返す
// インデックスの小さいボタンが先に処理される
void test_two_buttons_pressed_returns_one_event_per_update() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_states[1] = true;
    mock_time = 0;
    button_service_update(&s);

    mock_time = DEBOUNCE_MS;
    ButtonPressResult ev1 = button_service_update(&s);
    TEST_ASSERT_TRUE(ev1.valid);
    TEST_ASSERT_EQUAL(0, ev1.buttonId);

    // 次の update() で 2 番目のボタンのイベントが返る
    ButtonPressResult ev2 = button_service_update(&s);
    TEST_ASSERT_TRUE(ev2.valid);
    TEST_ASSERT_EQUAL(1, ev2.buttonId);
}

// millis が uint32_t の最大値付近でもデバウンスが正しく動作する
void test_debounce_works_near_millis_overflow() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    mock_states[0] = true;
    mock_time = 0xFFFFFFFF - 10;  // オーバーフロー直前
    button_service_update(&s);

    // オーバーフローをまたいで DEBOUNCE_MS 経過
    mock_time = 0xFFFFFFFF - 10 + DEBOUNCE_MS;  // wraps around
    ButtonPressResult ev = button_service_update(&s);
    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESS, ev.type);
}

// Press 確定後すぐに離して再度押した場合、Release → Press の順でイベントが発火する
void test_press_release_press_sequence() {
    ButtonService s;
    button_service_init(&s, &mock_port, 4);

    // 1 回目の Press
    mock_states[0] = true;
    mock_time = 0;
    button_service_update(&s);
    mock_time = DEBOUNCE_MS;
    button_service_update(&s);  // Press 確定

    // Release
    mock_states[0] = false;
    mock_time = DEBOUNCE_MS + 1;
    button_service_update(&s);
    mock_time = DEBOUNCE_MS * 2 + 1;
    ButtonPressResult ev = button_service_update(&s);
    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_RELEASE, ev.type);

    // 2 回目の Press
    mock_states[0] = true;
    mock_time = DEBOUNCE_MS * 2 + 2;
    button_service_update(&s);
    mock_time = DEBOUNCE_MS * 3 + 2;
    ev = button_service_update(&s);
    TEST_ASSERT_TRUE(ev.valid);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESS, ev.type);
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
    RUN_TEST(test_bounce_resets_debounce_timer);
    RUN_TEST(test_hold_on_second_button);
    RUN_TEST(test_two_buttons_pressed_returns_one_event_per_update);
    RUN_TEST(test_debounce_works_near_millis_overflow);
    RUN_TEST(test_press_release_press_sequence);
    return UNITY_END();
}
