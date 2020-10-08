#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include "config.h"
#include "one_wire.h"

void one_wire_stop(void)
{
	timer_disable_counter(ONE_WIRE_WRITE_TIMER);
}

/** Get write status
 *
 * @returns int. 1 if sent operation completed, 0 otherwise.
 */
static int one_wire_write_sent()
{
	if (timer_get_flag(ONE_WIRE_WRITE_TIMER, ONE_WIRE_WRITE_TIMER_CCIF)) {
		timer_clear_flag(ONE_WIRE_WRITE_TIMER, ONE_WIRE_WRITE_TIMER_CCIF);
		return 1;
	}
	return 0;
}

static void one_wire_write_sent_wait(void)
{
	while (one_wire_write_sent() != 1)
		;
}


/* one_wire functions 
 *
 * one_wire_write_setup():  enable PA0 and ONE_WIRE_WRITE_TIMER for write
 * */

/** Setup onewire and send reset.
 *
 * Setup the needed hardware like timer, input and output.
 * Creates a bus reset and returns 1 if slave sends a
 * presence pulse, 0 otherwise.
 * If no presence pulse was detected, it deactivates the
 * hardware again.
 */

int one_wire_setup(void)
{

	// setup one wire read pin
	rcc_periph_clock_enable(ONE_WIRE_READ_RCC_BIT);
	gpio_set_mode(ONE_WIRE_READ_PORT, GPIO_MODE_INPUT,
		GPIO_CNF_INPUT_FLOAT, ONE_WIRE_READ_PIN);

	// setup one wire write pin
	rcc_periph_clock_enable(ONE_WIRE_WRITE_RCC_BIT);
	gpio_set_mode(ONE_WIRE_WRITE_PORT, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, ONE_WIRE_WRITE_PIN);

	rcc_periph_clock_enable(ONE_WIRE_WRITE_TIMER_RCC);
	rcc_periph_reset_pulse(ONE_WIRE_WRITE_TIMER_RST);

	timer_set_oc_mode(ONE_WIRE_WRITE_TIMER, ONE_WIRE_WRITE_TIMER_OC,
			  TIM_OCM_PWM2);

	timer_set_mode(ONE_WIRE_WRITE_TIMER, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	timer_set_prescaler(ONE_WIRE_WRITE_TIMER, 72);

	timer_continuous_mode(ONE_WIRE_WRITE_TIMER);
	timer_enable_preload(ONE_WIRE_WRITE_TIMER);

	timer_set_period(ONE_WIRE_WRITE_TIMER, 1000);
	timer_set_oc_value(ONE_WIRE_WRITE_TIMER, ONE_WIRE_WRITE_TIMER_OC, 500);

	timer_enable_oc_output(ONE_WIRE_WRITE_TIMER, ONE_WIRE_WRITE_TIMER_OC);
	timer_enable_oc_preload(ONE_WIRE_WRITE_TIMER, ONE_WIRE_WRITE_TIMER_OC);

	timer_generate_event(ONE_WIRE_WRITE_TIMER, TIM_EGR_UG);

	while (timer_get_flag(ONE_WIRE_WRITE_TIMER, TIM_SR_UIF) == 0)
		;

	timer_clear_flag(ONE_WIRE_WRITE_TIMER, TIM_SR_UIF);
	timer_enable_counter(ONE_WIRE_WRITE_TIMER);

	timer_set_period(ONE_WIRE_WRITE_TIMER, 70);
	one_wire_write_sent_wait();

	int presence_pulse = 0;
	while (timer_get_counter(ONE_WIRE_WRITE_TIMER) < (500 + 250)) {
		if (gpio_get(ONE_WIRE_READ_PORT, ONE_WIRE_READ_PIN) == 0)
			presence_pulse = 1;
	}

	//timer_clear_flag(ONE_WIRE_WRITE_TIMER, TIM_SR_UIF);
	if (presence_pulse == 0)
		one_wire_stop();

	return presence_pulse;
}

/** Write byte
 */
static int one_wire_write_byte(uint8_t data)
{
	int bitmask = 1;// << 7;
	int data_bit;
	int capture = 0;
	for (int i=0; i < 8; i++) {
		data_bit = data & bitmask;

		if (data_bit != 0)
			timer_set_oc_value(ONE_WIRE_WRITE_TIMER,
					   ONE_WIRE_WRITE_TIMER_OC, 3);
		else
			timer_set_oc_value(ONE_WIRE_WRITE_TIMER,
					   ONE_WIRE_WRITE_TIMER_OC, 65);

		one_wire_write_sent_wait();

		if (gpio_get(ONE_WIRE_READ_PORT, ONE_WIRE_READ_PIN))
			capture |= bitmask;

		bitmask <<= 1;
	}
	return capture;
}

/** Send reset, bytes and read answer.
 *
 * Sends a reset, writes given w_bytes and and reads the answer back into
 * r_bytes.
 */
void one_wire_write_read(int *w_bytes, int *r_bytes, int w_size, int r_size)
{
	for (int i = 0; i < w_size; i++)
		one_wire_write_byte(w_bytes[i]);

	for (int i = 0; i < r_size; i++)
		r_bytes[i] = one_wire_write_byte(0xff);

	one_wire_stop();
}

int one_wire_crc(int *data, int size)
{
	int crc = 0;
	for (int i = 0; i < size; i++) {
		int inbyte = data[i];
		for (int j = 0; j < 8; j++) {
			int mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}

