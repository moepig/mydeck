#include <HID-Project.h>
#include "board_config.h"
#include "domain/hid_report.h"
#include "domain/button_service.h"
#include "domain/led_service.h"
#include "infrastructure/arduino_button_port.h"
#include "infrastructure/arduino_led_port.h"

// ---- インフラストラクチャ層（ハードウェアアダプタ）-----------
static ArduinoButtonPort buttonPort(BUTTON_PINS);
static ArduinoLedPort    ledPort(BOARD_LED_TX, BOARD_LED_RX);

// ---- ドメイン層（ビジネスロジック）---------------------------
static ButtonService buttonService(buttonPort, BUTTON_COUNT);
static LedService    ledService(ledPort);

// ---- HID カスタムレポート -------------------------------------
static uint8_t hidSendBuf[64];
static uint8_t hidRecvBuf[64];

// ---------------------------------------------------------------
void setup() {
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    }

    ledPort.begin();
    ledService.setConnected(true);

    RawHID.begin(hidRecvBuf, sizeof(hidRecvBuf));
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
