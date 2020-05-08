#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include <stdio.h>

int main(int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run(); 
}
uint8_t usbTestBuf[0x40];

extern "C"
uint16_t usb_write_packet(uint8_t ep, void *buf, uint16_t len)
{
    memcpy(usbTestBuf, buf, len);
    return len;
}


extern "C" void usart_send_str(char *p) {
    printf("%s\n", p);
}
extern "C" void hw_led_toggle() {}