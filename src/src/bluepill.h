#ifndef _BLUEPILL_H
#define _BLUEPILL_H

#define BLUEPILL_LED_RCC_BIT RCC_GPIOC
#define BLUEPILL_LED_PORT GPIOC
#define BLUEPILL_LED_PIN GPIO13

void bluepill_led_setup(void);
void bluepill_uart_setup(void);
#endif
