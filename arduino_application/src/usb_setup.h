#pragma once


// #define INCLUDE_DFU_INTERFACE
#define INCLUDE_CDC_INTERFACE

enum
{
    ENDPOINT_HID_IN = 0x81,
    ENDPOINT_HID_OUT = 0x01,
    ENDPOINT_CDC_COMM_IN = 0x83,
    ENDPOINT_CDC_DATA_IN = 0x82,
    ENDPOINT_CDC_DATA_OUT = 0x02
};

#define USB_VID 0x057e
//#define USB_PID 0x2009
#define USB_PID 0x200e

void usb_setup(void);
void usb_poll(void);
uint32_t usb_send_serial_data(void *buf, int len);
uint16_t usb_write_packet(uint8_t ep, void * buf, uint16_t len);
uint16_t usb_read_packet(uint8_t ep, void *buf, uint16_t len);

void hid_rx_cb(uint8_t * buf, uint16_t len);


extern char shared_buf[0x40];


