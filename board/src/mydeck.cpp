#include <Arduino.h>
#include <HID-Project.h>
#include "board_config.h"

extern "C" {
#include "domain/hid_report.h"
#include "domain/button_service.h"
#include "domain/led_service.h"
}

#include "infrastructure/arduino_button_port.h"
#include "infrastructure/arduino_led_port.h"

// ---- インフラストラクチャ層 ------------------------------------
static ArduinoButtonCtx buttonCtx = { ROW_PINS, COL_PINS, ROW_COUNT, COL_COUNT };
static ButtonPort       buttonPort;

static ArduinoLedCtx ledCtx = { BOARD_LED_TX, BOARD_LED_RX };
static LedPort       ledPort;

// ---- ドメイン層 ------------------------------------------------
static ButtonService buttonService;
static LedService    ledService;

// ---- HID バッファ ----------------------------------------------
static uint8_t hidSendBuf[64];
static uint8_t hidRecvBuf[64];

// ---------------------------------------------------------------
void setup() {
    for (uint8_t i = 0; i < ROW_COUNT; i++) {
        pinMode(ROW_PINS[i], OUTPUT);
        digitalWrite(ROW_PINS[i], HIGH);
    }
    for (uint8_t i = 0; i < COL_COUNT; i++) {
        pinMode(COL_PINS[i], INPUT_PULLUP);
    }

    buttonPort = arduino_button_port_create(&buttonCtx);
    arduino_led_port_begin(&ledCtx);
    ledPort = arduino_led_port_create(&ledCtx);

    button_service_init(&buttonService, &buttonPort, BUTTON_COUNT);
    led_service_init(&ledService, &ledPort);
    led_service_set_connected(&ledService, true);

    RawHID.begin(hidRecvBuf, sizeof(hidRecvBuf));

    Serial.begin(115200);
    Serial.println(F("mydeck ready"));
}

void loop() {
    ButtonPressResult ev = button_service_update(&buttonService);
    if (ev.valid) {
        memset(hidSendBuf, 0, sizeof(hidSendBuf));
        hidSendBuf[0] = REPORT_ID_INPUT;
        hidSendBuf[1] = ev.buttonId;
        hidSendBuf[2] = (uint8_t)ev.type;
        RawHID.write(hidSendBuf, sizeof(hidSendBuf));
        led_service_flash(&ledService);

        static const char* EVT_NAMES[] = {"?", "Press", "Release", "Hold"};
        uint8_t ei = (uint8_t)ev.type;
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
            if (out->command == (uint8_t)OUTPUT_CMD_SET_LED) {
                led_service_set_connected(&ledService, out->param != 0);
            }
        }
    }

    led_service_update(&ledService);
}
