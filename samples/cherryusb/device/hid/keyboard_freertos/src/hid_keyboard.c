/*
 * Copyright (c) 2022-2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "usbd_core.h"
#include "usbd_hid.h"
#include "board.h"
#include "hpm_gpio_drv.h"

#define HID_INT_EP          0x81
#define HID_INT_EP_SIZE     8
#define HID_INT_EP_INTERVAL 10

#define USB_HID_CONFIG_DESC_SIZ       34
#define HID_KEYBOARD_REPORT_DESC_SIZE 63

static const uint8_t hid_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0002, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_HID_CONFIG_DESC_SIZ, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),

    /************** Descriptor of Joystick Mouse interface ****************/
    /* 09 */
    0x09,                          /* bLength: Interface Descriptor size */
    USB_DESCRIPTOR_TYPE_INTERFACE, /* bDescriptorType: Interface descriptor type */
    0x00,                          /* bInterfaceNumber: Number of Interface */
    0x00,                          /* bAlternateSetting: Alternate setting */
    0x01,                          /* bNumEndpoints */
    0x03,                          /* bInterfaceClass: HID */
    0x01,                          /* bInterfaceSubClass : 1=BOOT, 0=no boot */
    0x01,                          /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
    0,                             /* iInterface: Index of string descriptor */
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* 18 */
    0x09,                    /* bLength: HID Descriptor size */
    HID_DESCRIPTOR_TYPE_HID, /* bDescriptorType: HID */
    0x11,                    /* bcdHID: HID Class Spec release number */
    0x01,
    0x00,                          /* bCountryCode: Hardware target country */
    0x01,                          /* bNumDescriptors: Number of HID class descriptors to follow */
    0x22,                          /* bDescriptorType */
    HID_KEYBOARD_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
    0x00,
    /******************** Descriptor of Mouse endpoint ********************/
    /* 27 */
    0x07,                         /* bLength: Endpoint Descriptor size */
    USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
    HID_INT_EP,                   /* bEndpointAddress: Endpoint Address (IN) */
    0x03,                         /* bmAttributes: Interrupt endpoint */
    HID_INT_EP_SIZE,              /* wMaxPacketSize: 4 Byte max */
    0x00,
    HID_INT_EP_INTERVAL, /* bInterval: Polling Interval */
    /* 34 */
    /*
     * string0 descriptor
     */
    USB_LANGID_INIT(USBD_LANGID_STRING),
    /*
     * string1 descriptor
     */
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    /*
     * string2 descriptor
     */
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'H', 0x00,                  /* wcChar10 */
    'I', 0x00,                  /* wcChar11 */
    'D', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    /*
     * string3 descriptor
     */
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '2', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
    /*
     * device qualifier descriptor
     */
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};

static const uint8_t hid_keyboard_report_desc[HID_KEYBOARD_REPORT_DESC_SIZE] = {
    0x05, 0x01, /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x06, /* USAGE (Keyboard) */
    0xa1, 0x01, /* COLLECTION (Application) */
    0x05, 0x07, /* USAGE_PAGE (Keyboard) */
    0x19, 0xe0, /* USAGE_MINIMUM (Keyboard LeftControl) */
    0x29, 0xe7, /* USAGE_MAXIMUM (Keyboard Right GUI) */
    0x15, 0x00, /* LOGICAL_MINIMUM (0) */
    0x25, 0x01, /* LOGICAL_MAXIMUM (1) */
    0x75, 0x01, /* REPORT_SIZE (1) */
    0x95, 0x08, /* REPORT_COUNT (8) */
    0x81, 0x02, /* INPUT (Data,Var,Abs) */
    0x95, 0x01, /* REPORT_COUNT (1) */
    0x75, 0x08, /* REPORT_SIZE (8) */
    0x81, 0x03, /* INPUT (Cnst,Var,Abs) */
    0x95, 0x05, /* REPORT_COUNT (5) */
    0x75, 0x01, /* REPORT_SIZE (1) */
    0x05, 0x08, /* USAGE_PAGE (LEDs) */
    0x19, 0x01, /* USAGE_MINIMUM (Num Lock) */
    0x29, 0x05, /* USAGE_MAXIMUM (Kana) */
    0x91, 0x02, /* OUTPUT (Data,Var,Abs) */
    0x95, 0x01, /* REPORT_COUNT (1) */
    0x75, 0x03, /* REPORT_SIZE (3) */
    0x91, 0x03, /* OUTPUT (Cnst,Var,Abs) */
    0x95, 0x06, /* REPORT_COUNT (6) */
    0x75, 0x08, /* REPORT_SIZE (8) */
    0x15, 0x00, /* LOGICAL_MINIMUM (0) */
    0x25, 0xFF, /* LOGICAL_MAXIMUM (255) */
    0x05, 0x07, /* USAGE_PAGE (Keyboard) */
    0x19, 0x00, /* USAGE_MINIMUM (Reserved (no event indicated)) */
    0x29, 0x65, /* USAGE_MAXIMUM (Keyboard Application) */
    0x81, 0x00, /* INPUT (Data,Ary,Abs) */
    0xc0        /* END_COLLECTION */
};

static USB_NOCACHE_RAM_SECTION uint8_t s_sendbuffer[8];
SemaphoreHandle_t semaphore_tx_done;

void usbd_configure_done_callback(void)
{
    /* no out ep, do nothing */
}

#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1

void usbd_hid_int_callback(uint8_t ep, uint32_t nbytes)
{
    if (semaphore_tx_done != NULL) {
        if (xSemaphoreGive(semaphore_tx_done) != pdPASS) {
            USB_LOG_ERR("xSemaphoreGive error\r\n");
        }
    }
}

static struct usbd_endpoint hid_in_ep = {
    .ep_cb = usbd_hid_int_callback,
    .ep_addr = HID_INT_EP
};

struct usbd_interface intf0;

void hid_keyboard_init(void)
{
    semaphore_tx_done = xSemaphoreCreateBinary();
    if (semaphore_tx_done == NULL) {
        USB_LOG_ERR("xSemaphoreCreateBinary creation failed!.\r\n");
        for (;;) {
            vTaskDelay(100);
        }
    }
    usbd_desc_register(hid_descriptor);
    usbd_add_interface(usbd_hid_init_intf(&intf0, hid_keyboard_report_desc, HID_KEYBOARD_REPORT_DESC_SIZE));
    usbd_add_endpoint(&hid_in_ep);

    usbd_initialize();
    memset(s_sendbuffer, 0, 8);
}

void hid_keyboard_test(void)
{
    bool key_pushed = false;

    if (gpio_read_pin(BOARD_APP_GPIO_CTRL, BOARD_APP_GPIO_INDEX, BOARD_APP_GPIO_PIN) == 0u) {
        key_pushed = true;
        s_sendbuffer[2] = HID_KBD_USAGE_A;    /* a */
        int ret = usbd_ep_start_write(HID_INT_EP, s_sendbuffer, 8);
        if (ret < 0) {
            return;
        }

        if (xSemaphoreTake(semaphore_tx_done, portMAX_DELAY) != pdPASS) {
            USB_LOG_ERR("xSemaphoreTake error %d\r\n", __LINE__);
        }
    }

    if (key_pushed) {
        key_pushed = false;
        while (gpio_read_pin(BOARD_APP_GPIO_CTRL, BOARD_APP_GPIO_INDEX, BOARD_APP_GPIO_PIN) == 0u) {
        }
        s_sendbuffer[2] = 0;
        int ret = usbd_ep_start_write(HID_INT_EP, s_sendbuffer, 8);
        if (ret < 0) {
            return;
        }
        if (xSemaphoreTake(semaphore_tx_done, portMAX_DELAY) != pdPASS) {
            USB_LOG_ERR("xSemaphoreTake error %d\r\n", __LINE__);
        }
    }
}
