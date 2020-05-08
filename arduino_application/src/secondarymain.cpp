#include "init.hpp"

static int main(void) {
	initHardware();

	// Have connected
	// usart_send_str("========== start =========\r\n====================\r\n====================\r\n");
	// uart_flush();

	hw_led_on();

	sns_init(&controller_1);

	uint8_t _packet[0x06] = {};

	while(1) {
		handle_input_0x30();
		usb_poll();
		// handle_packet();

		sns_poll(&controller_1);
		sns_request(&controller_1);
		uart_flush();
	}

	return 0;
}