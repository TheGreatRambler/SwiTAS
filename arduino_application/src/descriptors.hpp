#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/sam/gpio.h>
#include <libopencm3/sam/rcc.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/usb/usbd.h>
#include <stdlib.h>
#include <string.h>

static const struct usb_device_descriptor dev_descr = {
	.bLength            = USB_DT_DEVICE_SIZE,
	.bDescriptorType    = USB_DT_DEVICE,
	.bcdUSB             = 0x0200,
	.bDeviceClass       = 0,
	.bDeviceSubClass    = 0,
	.bDeviceProtocol    = 0,
	.bMaxPacketSize0    = 64,
	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.bcdDevice          = 0x0200,
	.iManufacturer      = 1,
	.iProduct           = 2,
	.iSerialNumber      = 3,
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

static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
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

static const struct usb_endpoint_descriptor hid_endpoint[] = { {
																   .bLength          = USB_DT_ENDPOINT_SIZE,
																   .bDescriptorType  = USB_DT_ENDPOINT,
																   .bEndpointAddress = ENDPOINT_HID_IN,
																   .bmAttributes     = USB_ENDPOINT_ATTR_INTERRUPT,
																   .wMaxPacketSize   = 0x40,
																   .bInterval        = 0x08,
															   },
	{
		.bLength          = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType  = USB_DT_ENDPOINT,
		.bEndpointAddress = ENDPOINT_HID_OUT,
		.bmAttributes     = USB_ENDPOINT_ATTR_INTERRUPT,
		.wMaxPacketSize   = 0x40,
		.bInterval        = 0x08,
	} };

static const struct usb_interface_descriptor hid_iface = {
	.bLength            = USB_DT_INTERFACE_SIZE,
	.bDescriptorType    = USB_DT_INTERFACE,
	.bInterfaceNumber   = 0,
	.bAlternateSetting  = 0,
	.bNumEndpoints      = 2,
	.bInterfaceClass    = USB_CLASS_HID,
	.bInterfaceSubClass = 0, /* noboot */
	.bInterfaceProtocol = 0, /* Joystick */
	.iInterface         = 0,

	.endpoint = hid_endpoint,

	.extra    = &hid_function,
	.extralen = sizeof(hid_function),
};

#ifdef INCLUDE_CDC_INTERFACE
/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = { {
	.bLength          = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  = USB_DT_ENDPOINT,
	.bEndpointAddress = ENDPOINT_CDC_COMM_IN,
	.bmAttributes     = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize   = 16,
	.bInterval        = 255,
} };

static const struct usb_endpoint_descriptor data_endp[] = { {
																.bLength          = USB_DT_ENDPOINT_SIZE,
																.bDescriptorType  = USB_DT_ENDPOINT,
																.bEndpointAddress = ENDPOINT_CDC_DATA_OUT,
																.bmAttributes     = USB_ENDPOINT_ATTR_BULK,
																.wMaxPacketSize   = 64,
																.bInterval        = 1,
															},
	{
		.bLength          = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType  = USB_DT_ENDPOINT,
		.bEndpointAddress = ENDPOINT_CDC_DATA_IN,
		.bmAttributes     = USB_ENDPOINT_ATTR_BULK,
		.wMaxPacketSize   = 64,
		.bInterval        = 1,
	} };

static const struct {
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

static const struct usb_interface_descriptor comm_iface[] = { {
	.bLength            = USB_DT_INTERFACE_SIZE,
	.bDescriptorType    = USB_DT_INTERFACE,
	.bInterfaceNumber   = INTERFACE_CDC_COMM,
	.bAlternateSetting  = 0,
	.bNumEndpoints      = 1,
	.bInterfaceClass    = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface         = 0,

	.endpoint = comm_endp,

	.extra    = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors),
} };

static const struct usb_interface_descriptor data_iface[] = { {
	.bLength            = USB_DT_INTERFACE_SIZE,
	.bDescriptorType    = USB_DT_INTERFACE,
	.bInterfaceNumber   = INTERFACE_CDC_DATA,
	.bAlternateSetting  = 0,
	.bNumEndpoints      = 2,
	.bInterfaceClass    = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface         = 0,

	.endpoint = data_endp,
} };

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
	.bLength         = sizeof(struct usb_dfu_descriptor),
	.bDescriptorType = DFU_FUNCTIONAL,
	.bmAttributes    = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
	.wDetachTimeout  = 255,
	.wTransferSize   = 1024,
	.bcdDFUVersion   = 0x011A,
};

static const struct usb_interface_descriptor dfu_iface = {
	.bLength            = USB_DT_INTERFACE_SIZE,
	.bDescriptorType    = USB_DT_INTERFACE,
	.bInterfaceNumber   = _USB_IFACE_NB - 1, // last...
	.bAlternateSetting  = 0,
	.bNumEndpoints      = 0,
	.bInterfaceClass    = 0xFE,
	.bInterfaceSubClass = 1,
	.bInterfaceProtocol = 1,
	.iInterface         = 0,

	.extra    = &dfu_function,
	.extralen = sizeof(dfu_function),
};
#endif

static const struct usb_interface ifaces[] = {
	{
		.num_altsetting = 1,
		.altsetting     = &hid_iface,
	},
#ifdef INCLUDE_CDC_INTERFACE
	{
		.num_altsetting = 1,
		.altsetting     = comm_iface,
		.iface_assoc    = &cdc_acm_interface_association,
	},
	{
		.num_altsetting = 1,
		.altsetting     = data_iface,
	},
#endif
#ifdef INCLUDE_DFU_INTERFACE
	{
		.num_altsetting = 1,
		.altsetting     = &dfu_iface,
	},
#endif

};

static const struct usb_config_descriptor config = {
	.bLength             = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType     = USB_DT_CONFIGURATION,
	.wTotalLength        = 0,
	.bNumInterfaces      = _USB_IFACE_NB,
	.bConfigurationValue = 1,
	.iConfiguration      = 0,
	.bmAttributes        = 0xC0,
	.bMaxPower           = 0x32,

	.interface = ifaces,
};

static const char* usb_strings[] = {
	"Nintendo Co., Ltd.",
	"Pro Controller",
	"000000000001",
};