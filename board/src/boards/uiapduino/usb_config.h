#ifndef _USB_CONFIG_H
#define _USB_CONFIG_H

// ---- rv003usb 設定 (UIAPduino Pro Micro CH32V003 V1.4) --------
// 配線: D+ = PD3, D- = PD4, Pull-up = PD4 (1.5kΩ to D-)
#define ENDPOINTS   2       // EP0 (control) + EP1 (interrupt IN)
#define USB_PORT    D
#define USB_PIN_DP  3       // PD3 = USB D+
#define USB_PIN_DM  4       // PD4 = USB D-
#define USB_PIN_DPU 4       // PD4 = 1.5k pull-up on D-

#define RV003USB_DEBUG_TIMING      0
#define RV003USB_OPTIMIZE_FLASH    1
#define RV003USB_EVENT_DEBUGGING   0
#define RV003USB_HANDLE_IN_REQUEST 1  // usb_handle_user_in_request() を使用
#define RV003USB_OTHER_CONTROL     1
#define RV003USB_HANDLE_USER_DATA  1  // usb_handle_user_data() を使用
#define RV003USB_HID_FEATURES      1  // SET_REPORT / GET_REPORT を使用

#ifndef __ASSEMBLER__

#include <tinyusb_hid.h>

#ifdef INSTANCE_DESCRIPTORS

// ---- HID レポートディスクリプタ ---------------------------------
// Input  Report ID=0x01: 7 bytes (buttonId, event, reserved[5])
// Feature Report ID=0x02: 7 bytes (command, param, reserved[5])
// ホストは Feature SET_REPORT で LED 制御コマンドを送る
static const uint8_t hid_report_desc[] = {
    0x06, 0x00, 0xff,              // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,                    // Usage (0x01)
    0xa1, 0x01,                    // Collection (Application)

    // Input report: ID=1, 7 bytes
    0x85, 0x01,                    //   Report ID (1)
    0x09, 0x01,                    //   Usage (0x01)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x26, 0xff, 0x00,              //   Logical Maximum (255)
    0x75, 0x08,                    //   Report Size (8)
    0x95, 0x07,                    //   Report Count (7)
    0x81, 0x02,                    //   Input (Data, Variable, Absolute)

    // Feature report: ID=2, 7 bytes (PC → デバイス)
    0x85, 0x02,                    //   Report ID (2)
    0x09, 0x02,                    //   Usage (0x02)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x26, 0xff, 0x00,              //   Logical Maximum (255)
    0x75, 0x08,                    //   Report Size (8)
    0x95, 0x07,                    //   Report Count (7)
    0xb1, 0x02,                    //   Feature (Data, Variable, Absolute)

    0xc0                           // End Collection
};
// sizeof(hid_report_desc) == 38

// ---- デバイスディスクリプタ ------------------------------------
// VID=0x1209 (pid.codes), PID=0xDEC0
// ホスト側アプリでこの VID/PID に合わせること
static const uint8_t device_descriptor[] = {
    18,
    1,           // bDescriptorType (Device)
    0x10, 0x01,  // bcdUSB 1.10
    0x00,        // bDeviceClass (per-interface)
    0x00,
    0x00,
    0x08,        // bMaxPacketSize0 (Low-Speed USB は 8 固定)
    0x09, 0x12,  // idVendor  = 0x1209
    0xc0, 0xde,  // idProduct = 0xDEC0
    0x01, 0x00,  // bcdDevice 0.01
    1,           // iManufacturer
    2,           // iProduct
    0,           // iSerialNumber
    1,           // bNumConfigurations
};

// ---- コンフィギュレーションディスクリプタ (9+9+9+7 = 34 bytes) --
static const uint8_t config_descriptor[] = {
    // Configuration
    9, 0x02, 34, 0x00,   // wTotalLength = 34
    0x01, 0x01, 0x00, 0x80, 0x32,

    // Interface (HID)
    9, 0x04,
    0x00,  // bInterfaceNumber
    0x00,  // bAlternateSetting
    0x01,  // bNumEndpoints
    0x03,  // bInterfaceClass = HID
    0x00,  // bInterfaceSubClass
    0x00,  // bInterfaceProtocol
    0x00,

    // HID descriptor
    9, 0x21,
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType = Report
    sizeof(hid_report_desc), 0x00,

    // Endpoint 1 IN (interrupt)
    7, 0x05,
    0x81,        // bEndpointAddress (IN, EP1)
    0x03,        // bmAttributes (Interrupt)
    0x08, 0x00,  // wMaxPacketSize = 8
    0x0a,        // bInterval = 10ms
};

#define STR_MANUFACTURER u"UIAP"
#define STR_PRODUCT      u"mydeck"

struct usb_string_descriptor_struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wString[];
};
static const struct usb_string_descriptor_struct string0
    __attribute__((section(".rodata"))) = { 4, 3, {0x0409} };
static const struct usb_string_descriptor_struct string1
    __attribute__((section(".rodata"))) = {
    sizeof(STR_MANUFACTURER), 3, STR_MANUFACTURER };
static const struct usb_string_descriptor_struct string2
    __attribute__((section(".rodata"))) = {
    sizeof(STR_PRODUCT), 3, STR_PRODUCT };

static const struct descriptor_list_struct {
    uint32_t       lIndexValue;
    const uint8_t *addr;
    uint8_t        length;
} descriptor_list[] = {
    { 0x00000100, device_descriptor,   sizeof(device_descriptor)   },
    { 0x00000200, config_descriptor,   sizeof(config_descriptor)   },
    { 0x00002200, hid_report_desc,     sizeof(hid_report_desc)     },
    { 0x00002100, config_descriptor + 18, 9 },
    { 0x00000300, (const uint8_t *)&string0, 4 },
    { 0x04090301, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER) },
    { 0x04090302, (const uint8_t *)&string2, sizeof(STR_PRODUCT)      },
};
#define DESCRIPTOR_LIST_ENTRIES \
    (sizeof(descriptor_list) / sizeof(descriptor_list[0]))

#endif // INSTANCE_DESCRIPTORS
#endif // __ASSEMBLER__
#endif // _USB_CONFIG_H
