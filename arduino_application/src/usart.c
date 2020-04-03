#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "compat.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include "usart.h"

#define UART_DBG 1

#define UART_BUFF_SIZE 1024 * 8
static uint8_t uart_buffer[UART_BUFF_SIZE];
static uint8_t *uart_current_ptr;

void usart_init(void)
{
    rcc_periph_clock_enable(RCC_USART2);

    /* Setup GPIO pin GPIO_USART1_TX. */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);

    /* Setup UART parameters. */
    usart_set_baudrate(USART2, 921600 /*115200*/);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    usart_enable(USART2);
    memset(uart_buffer, 0x00, UART_BUFF_SIZE);
    uart_current_ptr = uart_buffer;
}

void usart_send_direct(const char *p)
{
    int len = strlen(p);
    // send uart
    while (len-- > 0)
    {
        usart_send_blocking(USART2, *p++);
    }
}
void usart_send_str(const char *p)
{
#if UART_DBG
    int len = strlen(p);
    memcpy(uart_current_ptr, p, len);
    uart_current_ptr += len;
    *uart_current_ptr++ = '\n';
#endif
}

void uart_flush(void)
{
#if UART_DBG
    char *ptr = (char *)uart_buffer;
    char *end = (char *)uart_current_ptr;
    if (ptr != end)
    {
        // send uart
        while (ptr < end)
        {
            usart_send_blocking(USART2, *ptr++);
        }
    }

    // reset
    uart_current_ptr = uart_buffer;
#endif
}


static char ascii_buffer[0x100] = {};

// #define usart_send_str(X)

void dump_hex(const void *data, size_t size)
{
#if 1
    char *ptr = ascii_buffer;
    size_t i;
    size = min(size, 0x100);
    for (i = 0; i < size; ++i)
    {
        unsigned char b = ((unsigned char *)data)[i];
        ptr += sprintf(ptr, "%02x ", b);
    }

    //usb_send_serial_data(ascii_buffer);
    //usb_poll();
    usart_send_str(ascii_buffer);
#else
    uint32_t *u32_data = (uint32_t *)data;
    char *ptr = ascii_buffer;
    size_t i;
    size = min(size, 0x100) / sizeof(uint32_t);
    for (i = 0; i < size; ++i)
    {
        ptr += sprintf(ptr, "0x%x,", u32_data[i]);
    }

    *ptr++ = '\r';
    *ptr++ = '\n';
    *ptr++ = 0;

    //usb_send_serial_data(ptr, ptr - ascii);
    //usb_poll();
    usart_send_str(ascii_buffer);
#endif
}