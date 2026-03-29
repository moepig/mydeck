#include <HID-Project.h>
#include "board_config.h"
#include "domain/hid_report.h"
#include "domain/button_service.h"
#include "domain/led_service.h"
#include "infrastructure/arduino_button_port.h"
#include "infrastructure/arduino_led_port.h"

// ---- インフラストラクチャ層（ハードウェアアダプタ）-----------
static ArduinoButtonPort buttonPort(ROW_PINS, ROW_COUNT, COL_PINS, COL_COUNT);
static ArduinoLedPort    ledPort(BOARD_LED_TX, BOARD_LED_RX);

// ---- ドメイン層（ビジネスロジック）---------------------------
static ButtonService buttonService(buttonPort, BUTTON_COUNT);
static LedService    ledService(ledPort);

// ---- HID カスタムレポート -------------------------------------
static uint8_t hidSendBuf[64];
static uint8_t hidRecvBuf[64];

// ---------------------------------------------------------------
void setup() {
    // Row ピン: OUTPUT, 初期 HIGH（非選択状態）
    for (uint8_t i = 0; i < ROW_COUNT; i++) {
        pinMode(ROW_PINS[i], OUTPUT);
        digitalWrite(ROW_PINS[i], HIGH);
    }
    // Col ピン: INPUT_PULLUP（押下時に Row の LOW が伝わる）
    for (uint8_t i = 0; i < COL_COUNT; i++) {
        pinMode(COL_PINS[i], INPUT_PULLUP);
    }

    ledPort.begin();
    ledService.setConnected(true);

    RawHID.begin(hidRecvBuf, sizeof(hidRecvBuf));

    Serial.begin(115200);
    Serial.println(F("mydeck ready"));
}

void loop() {
    ButtonPressResult ev = buttonService.update();
    if (ev.valid) {
        memset(hidSendBuf, 0, sizeof(hidSendBuf));
        hidSendBuf[0] = REPORT_ID_INPUT;
        hidSendBuf[1] = ev.buttonId;
        hidSendBuf[2] = static_cast<uint8_t>(ev.type);
        RawHID.write(hidSendBuf, sizeof(hidSendBuf));
        ledService.flash();

        // デバッグ: シリアルモニタにボタンイベントを出力
        static const char* EVT_NAMES[] = {"?", "Press", "Release", "Hold"};
        uint8_t ei = static_cast<uint8_t>(ev.type);
        Serial.print(F("btn="));
        Serial.print(ev.buttonId);
        Serial.print(F(" evt="));
        Serial.println(ei <= 3 ? EVT_NAMES[ei] : "?");
    }

    int received = RawHID.available();
    if (received > 0) {
        RawHID.readBytes(hidRecvBuf, received);
        OutputReport* out = reinterpret_cast<OutputReport*>(hidRecvBuf);
        if (out->reportId == REPORT_ID_OUTPUT) {
            if (out->command == static_cast<uint8_t>(OutputCommand::SetLed)) {
                ledService.setConnected(out->param != 0);
            }
        }
    }

    ledService.update();
}
