#include <unity.h>
#include "../src/domain/led_service.h"
#include "../src/domain/led_service.cpp"  // NOLINT: Unity ビルド

// ---- テスト用 ILedPort モック --------------------------------
struct MockLedPort : public ILedPort {
    bool     txLed = false;
    bool     rxLed = false;
    uint32_t time  = 0;

    void setTxLed(bool on) override { txLed = on; }
    void setRxLed(bool on) override { rxLed = on; }
    uint32_t millis() override { return time; }
};

static MockLedPort mock;

// ---------------------------------------------------------------
void setUp() {
    mock.txLed = false;
    mock.rxLed = false;
    mock.time  = 0;
}
void tearDown() {}

// setConnected(true) で TX LED が点灯する
void test_set_connected_true_turns_on_tx_led() {
    LedService s(mock);
    s.setConnected(true);
    TEST_ASSERT_TRUE(mock.txLed);
}

// setConnected(false) で TX LED が消灯する
void test_set_connected_false_turns_off_tx_led() {
    LedService s(mock);
    s.setConnected(true);
    s.setConnected(false);
    TEST_ASSERT_FALSE(mock.txLed);
}

// flash() 直後に RX LED が点灯する
void test_flash_turns_on_rx_led() {
    LedService s(mock);
    s.flash();
    TEST_ASSERT_TRUE(mock.rxLed);
}

// update() を FLASH_DURATION_MS 前に呼んでも RX LED は消えない
void test_update_before_duration_keeps_rx_led_on() {
    LedService s(mock);
    mock.time = 0;
    s.flash();

    mock.time = FLASH_DURATION_MS - 1;
    s.update();
    TEST_ASSERT_TRUE(mock.rxLed);
}

// update() を FLASH_DURATION_MS 後に呼ぶと RX LED が消灯する
void test_update_after_duration_turns_off_rx_led() {
    LedService s(mock);
    mock.time = 0;
    s.flash();

    mock.time = FLASH_DURATION_MS;
    s.update();
    TEST_ASSERT_FALSE(mock.rxLed);
}

// flash() を複数回呼んだ場合、最後の flash() からタイマーがリセットされる
void test_flash_resets_timer() {
    LedService s(mock);
    mock.time = 0;
    s.flash();

    // FLASH_DURATION_MS の手前で再度 flash() → タイマーが延長される
    mock.time = FLASH_DURATION_MS - 10;
    s.flash();

    // 最初の flash() から FLASH_DURATION_MS 経過しても消灯しない
    mock.time = FLASH_DURATION_MS;
    s.update();
    TEST_ASSERT_TRUE(mock.rxLed);

    // 2 回目の flash() から FLASH_DURATION_MS 経過したら消灯
    mock.time = (FLASH_DURATION_MS - 10) + FLASH_DURATION_MS;
    s.update();
    TEST_ASSERT_FALSE(mock.rxLed);
}

// flash() なしで update() を呼んでも RX LED は変化しない
void test_update_without_flash_does_nothing() {
    LedService s(mock);
    mock.rxLed = false;
    s.update();
    TEST_ASSERT_FALSE(mock.rxLed);
}

// TX LED と RX LED は独立して動作する
void test_tx_and_rx_are_independent() {
    LedService s(mock);
    s.setConnected(true);
    s.flash();

    mock.time = FLASH_DURATION_MS;
    s.update();  // RX 消灯

    TEST_ASSERT_TRUE(mock.txLed);   // TX は変化しない
    TEST_ASSERT_FALSE(mock.rxLed);
}

// ---------------------------------------------------------------
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
    return UNITY_END();
}
