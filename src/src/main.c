#include <stdio.h>
#include <errno.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/usart.h>
#include "config.h"
#include "newlibio.h"
#include "bluepill.h"
#include "temperature.h"

int main(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	bluepill_uart_setup();

	// setup bluepill LED
	bluepill_led_setup();

	// setup frontpanel button
	rcc_periph_clock_enable(BTN_RCC_BIT);
	gpio_set_mode(BTN_PORT, GPIO_MODE_INPUT,
		GPIO_CNF_INPUT_PULL_UPDOWN, BTN_PIN);

	// setup relay
	rcc_periph_clock_enable(RELAY_RCC_BIT);
	gpio_set_mode(RELAY_OUTPUT_PORT, GPIO_MODE_OUTPUT_2_MHZ,
		GPIO_CNF_OUTPUT_OPENDRAIN, RELAY_OUTPUT_PIN);

	// setup RTC
	rtc_auto_awake(RCC_LSE, 0x7fff);
	rtc_set_prescale_val(0x7fff);
	rtc_set_counter_val(0);

	// make sure relay is open(heater turned off)
	gpio_set(RELAY_OUTPUT_PORT, RELAY_OUTPUT_PIN);

	// disable led
	gpio_set(BLUEPILL_LED_PORT, BLUEPILL_LED_PIN);

	printf("Baker initialized\r\n");

	int pushed = 0;
	int temp = 0;
	while (1) {
		if(pushed == 0){
			if(gpio_get(BTN_PORT, BTN_PIN) != 0){
				// enable led
				gpio_clear(BLUEPILL_LED_PORT,BLUEPILL_LED_PIN);

				// enable relais
				gpio_clear(RELAY_OUTPUT_PORT, RELAY_OUTPUT_PIN);
				pushed = 1;
			}
		}

		if (pushed) {
			temp = one_wire_read_temp();
			printf("temp: %i\r\n", temp);
			if (temp > 1 && temp < TEMP_MAX) {
			  gpio_toggle(BLUEPILL_LED_PORT, BLUEPILL_LED_PIN);
			} else if (temp >= TEMP_MAX) {
			  gpio_set(BLUEPILL_LED_PORT, BLUEPILL_LED_PIN);
			  // disable relais
			  gpio_set(RELAY_OUTPUT_PORT, RELAY_OUTPUT_PIN);
			  pushed = 0;
			}
		}
	}
	return 0;
}
