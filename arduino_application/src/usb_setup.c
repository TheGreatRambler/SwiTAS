#include <stdlib.h>
#include <string.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>

#include "compat.h"
#include "usb_setup.h"

#ifdef INCLUDE_CDC_INTERFACE
#include <libopencm3/usb/cdc.h>
#endif

#ifdef INCLUDE_DFU_INTERFACE
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/dfu.h>
#endif

#define _HID_IFACE_NB 1
#define _DFU_IFACE_NB 1
#define _CDC_IFACE_NB 2

#define INTERFACE_CDC_COMM 1
#define INTERFACE_CDC_DATA 2

#define _USB_IFACE_NB _HID_IFACE_NB
#if defined(INCLUDE_DFU_INTERFACE) && !defined(INCLUDE_CDC_INTERFACE)
#define _USB_IFACE_NB (_HID_IFACE_NB + _DFU_IFACE_NB)
#endif
#if !defined(INCLUDE_DFU_INTERFACE) && defined(INCLUDE_CDC_INTERFACE)
#define _USB_IFACE_NB (_HID_IFACE_NB + _CDC_IFACE_NB)
#endif
#if defined(INCLUDE_DFU_INTERFACE) && defined(INCLUDE_CDC_INTERFACE)
#define _USB_IFACE_NB (_HID_IFACE_NB + _DFU_IFACE_NB + _CDC_IFACE_NB)
#endif

#include "compat.h"
#include "joystick.h"
#include "usb_setup.h"

static usbd_device *usbd_dev;

static const struct usb_device_descriptor dev_descr = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const uint8_t hid_report_descriptor[] = {
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x15, 0x00, // Logical Minimum (0)
    0x09, 0x04, // Usage (Joystick)
    0xA1, 0x01, // Collection (Application)
    // report 0x30
    0x85, 0x30,                   //   Report ID (48)
    0x05, 0x01,                   //   Usage Page (Generic Desktop Ctrls)
    0x05, 0x09,                   //   Usage Page (Button)
    0x19, 0x01,                   //   Usage Minimum (0x01)
    0x29, 0x0A,                   //   Usage Maximum (0x0A)
    0x15, 0x00,                   //   Logical Minimum (0)
    0x25, 0x01,                   //   Logical Maximum (1)
    0x75, 0x01,                   //   Report Size (1)
    0x95, 0x0A,                   //   Report Count (10)
    0x55, 0x00,                   //   Unit Exponent (0)
    0x65, 0x00,                   //   Unit (None)
    0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,                   //   Usage Page (Button)
    0x19, 0x0B,                   //   Usage Minimum (0x0B)
    0x29, 0x0E,                   //   Usage Maximum (0x0E)
    0x15, 0x00,                   //   Logical Minimum (0)
    0x25, 0x01,                   //   Logical Maximum (1)
    0x75, 0x01,                   //   Report Size (1)
    0x95, 0x04,                   //   Report Count (4)
    0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,                   //   Report Size (1)
    0x95, 0x02,                   //   Report Count (2)
    0x81, 0x03,                   //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x0B, 0x01, 0x00, 0x01, 0x00, //   Usage (0x010001)
    0xA1, 0x00,                   //   Collection (Physical)
    0x0B, 0x30, 0x00, 0x01, 0x00, //     Usage (0x010030)
    0x0B, 0x31, 0x00, 0x01, 0x00, //     Usage (0x010031)
    0x0B, 0x32, 0x00, 0x01, 0x00, //     Usage (0x010032)
    0x0B, 0x35, 0x00, 0x01, 0x00, //     Usage (0x010035)
    0x15, 0x00,                   //     Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00, //     Logical Maximum (65534)
    0x75, 0x10,                   //     Report Size (16)
    0x95, 0x04,                   //     Report Count (4)
    0x81, 0x02,                   //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,                         //   End Collection
    0x0B, 0x39, 0x00, 0x01, 0x00, //   Usage (0x010039)
    0x15, 0x00,                   //   Logical Minimum (0)
    0x25, 0x07,                   //   Logical Maximum (7)
    0x35, 0x00,                   //   Physical Minimum (0)
    0x46, 0x3B, 0x01,             //   Physical Maximum (315)
    0x65, 0x14,                   //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,                   //   Report Size (4)
    0x95, 0x01,                   //   Report Count (1)
    0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,                   //   Usage Page (Button)
    0x19, 0x0F,                   //   Usage Minimum (0x0F)
    0x29, 0x12,                   //   Usage Maximum (0x12)
    0x15, 0x00,                   //   Logical Minimum (0)
    0x25, 0x01,                   //   Logical Maximum (1)
    0x75, 0x01,                   //   Report Size (1)
    0x95, 0x04,                   //   Report Count (4)
    0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,                   //   Report Size (8)
    0x95, 0x34,                   //   Report Count (52)
    0x81, 0x03,                   //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,             //   Usage Page (Vendor Defined 0xFF00)

    // report 0x21
    0x85, 0x21, //   Report ID (33)
    0x09, 0x01, //   Usage (0x01)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    // report 0x81
    0x85, 0x81, //   Report ID (-127)
    0x09, 0x02, //   Usage (0x02)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    // report out 0x01
    0x85, 0x01, //   Report ID (1)
    0x09, 0x03, //   Usage (0x03)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    // report out 0x10
    0x85, 0x10, //   Report ID (16)
    0x09, 0x04, //   Usage (0x04)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    // report out 0x80
    0x85, 0x80, //   Report ID (-128)
    0x09, 0x05, //   Usage (0x05)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    // report out 0x82
    0x85, 0x82, //   Report ID (-126)
    0x09, 0x06, //   Usage (0x06)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x3F, //   Report Count (63)
    0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)
    0xC0,       // End Collection
};

static const struct
{
    struct usb_hid_descriptor hid_descriptor;
    struct
    {
        uint8_t bReportDescriptorType;
        uint16_t wDescriptorLength;
    } __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
    .hid_descriptor = {
        .bLength = sizeof(hid_function),
        .bDescriptorType = USB_DT_HID,
        .bcdHID = 0x0100,
        .bCountryCode = 0,
        .bNumDescriptors = 1,
    },
    .hid_report = {
        .bReportDescriptorType = USB_DT_REPORT,
        .wDescriptorLength = sizeof(hid_report_descriptor),
    }};

static const struct usb_endpoint_descriptor hid_endpoint[] = {
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = ENDPOINT_HID_IN,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = 0x40,
        .bInterval = 0x08,
    },
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = ENDPOINT_HID_OUT,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = 0x40,
        .bInterval = 0x08,
    }};

static const struct usb_interface_descriptor hid_iface = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_HID,
    .bInterfaceSubClass = 0, /* noboot */
    .bInterfaceProtocol = 0, /* Joystick */
    .iInterface = 0,

    .endpoint = hid_endpoint,

    .extra = &hid_function,
    .extralen = sizeof(hid_function),
};

#ifdef INCLUDE_CDC_INTERFACE
/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {{
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = ENDPOINT_CDC_COMM_IN,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 16,
    .bInterval = 255,
}};

static const struct usb_endpoint_descriptor data_endp[] = {
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = ENDPOINT_CDC_DATA_OUT,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1,
    },
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = ENDPOINT_CDC_DATA_IN,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1,
    }};

static const struct
{
    struct usb_cdc_header_descriptor header;
    struct usb_cdc_call_management_descriptor call_mgmt;
    struct usb_cdc_acm_descriptor acm;
    struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
    .header = {
        .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
        .bcdCDC = 0x0110,
    },
    .call_mgmt = {
        .bFunctionLength = sizeof(struct usb_cdc_call_management_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
        .bmCapabilities = 0,
        .bDataInterface = 1,
    },
    .acm = {
        .bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_ACM,
        .bmCapabilities = 0,
    },
    .cdc_union = {
        .bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_UNION,
        .bControlInterface = 0,
        .bSubordinateInterface0 = 1,
    },
};

static const struct usb_interface_descriptor comm_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = INTERFACE_CDC_COMM,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_CDC,
    .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
    .bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
    .iInterface = 0,

    .endpoint = comm_endp,

    .extra = &cdcacm_functional_descriptors,
    .extralen = sizeof(cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor data_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = INTERFACE_CDC_DATA,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_DATA,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,

    .endpoint = data_endp,
}};

// An interface association allows the device to group a set of interfaces to
// represent one logical device to be managed by one host driver.
static const struct usb_iface_assoc_descriptor cdc_acm_interface_association = {
    // The size of an interface association descriptor: 8
    .bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
    // A value of 11 indicates that this descriptor describes an interface
    // association.
    .bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
    // The first interface that is part of this group.
    .bFirstInterface = INTERFACE_CDC_COMM,
    // The number of included interfaces. This implies that the bundled
    // interfaces must be continugous.
    .bInterfaceCount = _CDC_IFACE_NB,
    // The class, subclass, and protocol of device represented by this
    // association. In this case a communication device.
    .bFunctionClass = USB_CLASS_CDC,
    // Using Abstract Control Model
    .bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
    // With AT protocol (or Hayes compatible).
    .bFunctionProtocol = USB_CDC_PROTOCOL_AT,
    // A string representing this interface. Zero means not provided.
    .iFunction = 0,
};

#endif

#ifdef INCLUDE_DFU_INTERFACE
static const struct usb_dfu_descriptor dfu_function = {
    .bLength = sizeof(struct usb_dfu_descriptor),
    .bDescriptorType = DFU_FUNCTIONAL,
    .bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
    .wDetachTimeout = 255,
    .wTransferSize = 1024,
    .bcdDFUVersion = 0x011A,
};

static const struct usb_interface_descriptor dfu_iface = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = _USB_IFACE_NB - 1, // last...
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = 0xFE,
    .bInterfaceSubClass = 1,
    .bInterfaceProtocol = 1,
    .iInterface = 0,

    .extra = &dfu_function,
    .extralen = sizeof(dfu_function),
};
#endif

static const struct usb_interface ifaces[] = {
    {
        .num_altsetting = 1,
        .altsetting = &hid_iface,
    },
#ifdef INCLUDE_CDC_INTERFACE
    {
        .num_altsetting = 1,
        .altsetting = comm_iface,
        .iface_assoc = &cdc_acm_interface_association,
    },
    {
        .num_altsetting = 1,
        .altsetting = data_iface,
    },
#endif
#ifdef INCLUDE_DFU_INTERFACE
    {
        .num_altsetting = 1,
        .altsetting = &dfu_iface,
    },
#endif

};

static const struct usb_config_descriptor config = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = _USB_IFACE_NB,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xC0,
    .bMaxPower = 0x32,

    .interface = ifaces,
};

static const char *usb_strings[] = {
    "Nintendo Co., Ltd.",
    "Pro Controller",
    "000000000001",
};

/* Buffer to be used for control requests. */
static uint8_t usbd_control_buffer[128];

char shared_buf[0x40] = {0};

static enum usbd_request_return_codes vendor_control_request(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf, uint16_t *len,
    void (**complete)(usbd_device *, struct usb_setup_data *))
{
    (void)complete;
    (void)dev;

#if 0
    if ((req->bmRequestType == USB_REQ_TYPE_VENDOR))
    {
        *complete = hid_out_complete;
        gpio_clear(GPIOB, GPIO12);
        return USBD_REQ_HANDLED;
    }
#endif
    return USBD_REQ_NOTSUPP;
}

static void hid_callback_complete(usbd_device * dev, struct usb_setup_data * data) {    
    systick_interrupt_enable();
}

static enum usbd_request_return_codes hid_control_request(
    usbd_device *dev, struct usb_setup_data *req,
    uint8_t **buf, uint16_t *len,
    void (**complete)(usbd_device *, struct usb_setup_data *))
{
    *complete = hid_callback_complete;

    if ((req->bmRequestType == 0x81) &&
        (req->bRequest == USB_REQ_GET_DESCRIPTOR) &&
        (req->wValue == 0x2200))
    {

        /* Handle the HID report descriptor. */
        *buf = (uint8_t *)hid_report_descriptor;
        *len = sizeof(hid_report_descriptor);

        return USBD_REQ_HANDLED;
    }

    return USBD_REQ_NOTSUPP;
}

#ifdef INCLUDE_DFU_INTERFACE
static void dfu_detach_complete(usbd_device *dev, struct usb_setup_data *req)
{
    (void)req;
    (void)dev;

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO10);
    gpio_set(GPIOA, GPIO10);
    scb_reset_core();
}

static enum usbd_request_return_codes dfu_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
                                                          void (**complete)(usbd_device *, struct usb_setup_data *))
{
    (void)buf;
    (void)len;
    (void)dev;

    if ((req->bmRequestType != 0x21) || (req->bRequest != DFU_DETACH))
        return USBD_REQ_NOTSUPP; /* Only accept class request. */

    *complete = dfu_detach_complete;

    return USBD_REQ_HANDLED;
}
#endif

#ifdef INCLUDE_CDC_INTERFACE
static enum usbd_request_return_codes cdcacm_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
                                                             uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)complete;
    (void)buf;
    (void)usbd_dev;

    switch (req->bRequest)
    {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
    {
        /*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
        char local_buf[10];
        struct usb_cdc_notification *notif = (void *)local_buf;

        /* We echo signals back to host as notification. */
        notif->bmRequestType = 0xA1;
        notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
        notif->wValue = 0;
        notif->wIndex = 0;
        notif->wLength = 2;
        local_buf[8] = req->wValue & 3;
        local_buf[9] = 0;
        // usbd_ep_write_packet(0x83, buf, 10);
        return USBD_REQ_HANDLED;
    }
    case USB_CDC_REQ_SET_LINE_CODING:
        if (*len < sizeof(struct usb_cdc_line_coding))
            return USBD_REQ_NOTSUPP;
        return USBD_REQ_HANDLED;
    }
    return USBD_REQ_NOTSUPP;
}

static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
    (void)ep;
    (void)usbd_dev;

    char buf[64];
    int len = usbd_ep_read_packet(usbd_dev, ENDPOINT_CDC_DATA_OUT, buf, 64);

    if (len)
    {
        usbd_ep_write_packet(usbd_dev, ENDPOINT_CDC_DATA_IN, buf, len);
        buf[len] = 0;
    }
}

#endif

static void hid_data_rx_cb(usbd_device *_usbd_dev, uint8_t ep)
{
    (void)ep;

    uint8_t buf[0x40];
    int len = usbd_ep_read_packet(_usbd_dev, ENDPOINT_HID_OUT, buf, 0x40);
    if (len)
    {
        hid_rx_cb(buf, len);
    }
}

static void hid_set_config(usbd_device *dev, uint16_t wValue)
{
    (void)wValue;
    (void)dev;

    usbd_ep_setup(dev, ENDPOINT_HID_IN, USB_ENDPOINT_ATTR_INTERRUPT, 0x40, NULL);
    usbd_ep_setup(dev, ENDPOINT_HID_OUT, USB_ENDPOINT_ATTR_INTERRUPT, 0x40, hid_data_rx_cb);

#ifdef INCLUDE_CDC_INTERFACE
    usbd_ep_setup(usbd_dev, ENDPOINT_CDC_DATA_OUT, USB_ENDPOINT_ATTR_BULK, 0x40, cdcacm_data_rx_cb);
    usbd_ep_setup(usbd_dev, ENDPOINT_CDC_DATA_IN, USB_ENDPOINT_ATTR_BULK, 0x40, NULL);
    usbd_ep_setup(usbd_dev, ENDPOINT_CDC_COMM_IN, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);
#endif

    usbd_register_control_callback(
        dev,
        USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        hid_control_request);

    usbd_register_control_callback(
        dev,
        USB_REQ_TYPE_VENDOR,
        USB_REQ_TYPE_TYPE,
        vendor_control_request);

#ifdef INCLUDE_DFU_INTERFACE
    usbd_register_control_callback(
        dev,
        USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        dfu_control_request);
#endif

#ifdef INCLUDE_CDC_INTERFACE
    usbd_register_control_callback(
        usbd_dev,
        USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        cdcacm_control_request);
#endif
}

uint32_t usb_send_serial_data(void *buf, int len)
{
#ifdef INCLUDE_CDC_INTERFACE
    return usbd_ep_write_packet(usbd_dev, ENDPOINT_CDC_DATA_IN, buf, len);
#else
    return 0;
#endif
}

void usb_setup()
{
    usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
    usbd_register_set_config_callback(usbd_dev, hid_set_config);
}

void usb_poll()
{
    usbd_poll(usbd_dev);
}

uint16_t usb_write_packet(uint8_t ep, void *buf, uint16_t len)
{
    return usbd_ep_write_packet(usbd_dev, ep, buf, len);
}

uint16_t usb_read_packet(uint8_t ep, void *buf, uint16_t len)
{
    return usbd_ep_read_packet(usbd_dev, ep, buf, len);
}
