#pragma once


void usart_init(void);
void usart_send_str(const char *p);
void usart_send_direct(const char *p);

// should be called once per loop
void uart_flush(void);

void dump_hex(const void *data, size_t size);