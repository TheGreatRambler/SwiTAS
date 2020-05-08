#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/sam/gpio.h>
#include <libopencm3/sam/rcc.h>
#include <stdlib.h>

#include "descriptors.hpp"

#define UART_BUFF_SIZE 1024 * 8
static uint8_t uart_buffer[UART_BUFF_SIZE];
static uint8_t* uart_current_ptr;

void initHardware() {
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	rcc_periph_clock_enable(RCC_AFIO);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
	gpio_clear(GPIOA, GPIO12);

	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);

	gpio_clear(GPIOB, GPIO12);

	// Do nothing for a bit
	for(unsigned i = 0; i < 800000; i++) {
		__asm__("nop");
	}

	hw_led_off();

	usart_init();

	usb_setup();

	systick_iterrupt_init();
}

void usart_init(void) {
	rcc_periph_clock_enable(RCC_USART2);

	/* Setup GPIO pin GPIO_USART1_TX. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);

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

void uart_flush(void) {
#if UART_DBG
	char* ptr = (char*)uart_buffer;
	char* end = (char*)uart_current_ptr;
	if(ptr != end) {
		// send uart
		while(ptr < end) {
			usart_send_blocking(USART2, *ptr++);
		}
	}

	// reset
	uart_current_ptr = uart_buffer;
#endif
}

void usb_setup() {
	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, hid_set_config);
}

static void systick_iterrupt_init(void) {
	static int frequency = 60 * 4; // 4 callback, each every 60hz
	systick_set_frequency(60, rcc_ahb_frequency);
	systick_counter_enable();
}

// LED stuff, I dunno
void hw_led_on() {
	gpio_clear(GPIOB, GPIO12);
}

void hw_led_off() {
	gpio_set(GPIOB, GPIO12);
}

void hw_led_toggle() {
	gpio_toggle(GPIOB, GPIO12);
}