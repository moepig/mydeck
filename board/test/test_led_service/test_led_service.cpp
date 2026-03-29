#include <unity.h>

extern "C" {
#include "../src/domain/led_service.h"
#include "../src/domain/led_service.c"  // NOLINT: Unity ビルド
}

// ---- テスト用 LedPort モック ----------------------------------
static bool     mock_tx_led;
static bool     mock_rx_led;
static uint32_t mock_time;

static void mock_set_tx(void *, bool on) { mock_tx_led = on; }
static void mock_set_rx(void *, bool on) { mock_rx_led = on; }
static uint32_t mock_millis(void *) { return mock_time; }

static LedPort mock_port = { mock_set_tx, mock_set_rx, mock_millis, NULL };

// ---------------------------------------------------------------
void setUp() {
    mock_tx_led = false;
    mock_rx_led = false;
    mock_time   = 0;
}
void tearDown() {}

void test_set_connected_true_turns_on_tx_led() {
    LedService s;
    led_service_init(&s, &mock_port);
    led_service_set_connected(&s, true);
    TEST_ASSERT_TRUE(mock_tx_led);
}

void test_set_connected_false_turns_off_tx_led() {
    LedService s;
    led_service_init(&s, &mock_port);
    led_service_set_connected(&s, true);
    led_service_set_connected(&s, false);
    TEST_ASSERT_FALSE(mock_tx_led);
}

void test_flash_turns_on_rx_led() {
    LedService s;
    led_service_init(&s, &mock_port);
    led_service_flash(&s);
    TEST_ASSERT_TRUE(mock_rx_led);
}

void test_update_before_duration_keeps_rx_led_on() {
    LedService s;
    led_service_init(&s, &mock_port);
    mock_time = 0;
    led_service_flash(&s);

    mock_time = FLASH_DURATION_MS - 1;
    led_service_update(&s);
    TEST_ASSERT_TRUE(mock_rx_led);
}

void test_update_after_duration_turns_off_rx_led() {
    LedService s;
    led_service_init(&s, &mock_port);
    mock_time = 0;
    led_service_flash(&s);

    mock_time = FLASH_DURATION_MS;
    led_service_update(&s);
    TEST_ASSERT_FALSE(mock_rx_led);
}

void test_flash_resets_timer() {
    LedService s;
    led_service_init(&s, &mock_port);
    mock_time = 0;
    led_service_flash(&s);

    mock_time = FLASH_DURATION_MS - 10;
    led_service_flash(&s);

    mock_time = FLASH_DURATION_MS;
    led_service_update(&s);
    TEST_ASSERT_TRUE(mock_rx_led);

    mock_time = (FLASH_DURATION_MS - 10) + FLASH_DURATION_MS;
    led_service_update(&s);
    TEST_ASSERT_FALSE(mock_rx_led);
}

void test_update_without_flash_does_nothing() {
    LedService s;
    led_service_init(&s, &mock_port);
    mock_rx_led = false;
    led_service_update(&s);
    TEST_ASSERT_FALSE(mock_rx_led);
}

void test_tx_and_rx_are_independent() {
    LedService s;
    led_service_init(&s, &mock_port);
    led_service_set_connected(&s, true);
    led_service_flash(&s);

    mock_time = FLASH_DURATION_MS;
    led_service_update(&s);

    TEST_ASSERT_TRUE(mock_tx_led);
    TEST_ASSERT_FALSE(mock_rx_led);
}

// millis が uint32_t 最大値付近でフラッシュしてもタイマーが正しく動作する
void test_flash_works_near_millis_overflow() {
    LedService s;
    led_service_init(&s, &mock_port);

    mock_time = 0xFFFFFFFF - 10;
    led_service_flash(&s);
    TEST_ASSERT_TRUE(mock_rx_led);

    // オーバーフローをまたいで FLASH_DURATION_MS 経過
    mock_time = 0xFFFFFFFF - 10 + FLASH_DURATION_MS;  // wraps around
    led_service_update(&s);
    TEST_ASSERT_FALSE(mock_rx_led);
}

// flash 完了後の再 flash が正しく動作する
void test_flash_after_previous_flash_completed() {
    LedService s;
    led_service_init(&s, &mock_port);

    mock_time = 0;
    led_service_flash(&s);

    mock_time = FLASH_DURATION_MS;
    led_service_update(&s);
    TEST_ASSERT_FALSE(mock_rx_led);

    // 2 回目の flash
    mock_time = FLASH_DURATION_MS + 100;
    led_service_flash(&s);
    TEST_ASSERT_TRUE(mock_rx_led);

    mock_time = FLASH_DURATION_MS + 100 + FLASH_DURATION_MS;
    led_service_update(&s);
    TEST_ASSERT_FALSE(mock_rx_led);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_set_connected_true_turns_on_tx_led);
    RUN_TEST(test_set_connected_false_turns_off_tx_led);
    RUN_TEST(test_flash_turns_on_rx_led);
    RUN_TEST(test_update_before_duration_keeps_rx_led_on);
    RUN_TEST(test_update_after_duration_turns_off_rx_led);
    RUN_TEST(test_flash_resets_timer);
    RUN_TEST(test_update_without_flash_does_nothing);
    RUN_TEST(test_tx_and_rx_are_independent);
    RUN_TEST(test_flash_works_near_millis_overflow);
    RUN_TEST(test_flash_after_previous_flash_completed);
    return UNITY_END();
}
