#include "platform.h"

#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "led.h"
#include "spi.h"
#include "systime.h"

#include "button.h"
#include "clock.h"
#include "eeprom.h"
#include "si4063.h"
#include "tic.h"
#include "tehu.h"
#include "ui.h"

const struct gpio_init_table_t pwr_gpio[] = {
	{ // pwr hold
		.gpio = GPIOC,
		.pin =  BV(0),
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_LOW,
		.state = GPIO_SET,
	},
};

static void button_handler(uint32_t btn, uint32_t ev)
{
	if (btn == 0) {
		if (ev == BUTTON_EV_LONG) {
			gpio_set(&pwr_gpio, GPIO_RESET);
			led_set(1, LED_ON);
		} else if (ev == BUTTON_EV_SHORT) {
			// TODO:
		}
	}
}

int main(void)
{
	clock_init();

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_AFIOEN;
	AFIO->MAPR |= (0x02 << 24); // only swj

	delay_init();

	spi_init();
	si4063_init();
	
	gpio_init(pwr_gpio, ARRAY_SIZE(pwr_gpio));
	button_init();
	button_set_ev_handler(button_handler);

	clock_ext();

	systime_init();
	led_init();
	i2c_init();
	eeprom_init();

	tic_init();
	tehu_init();

	led_set(1, LED_BLINK_SLOW);

	ui_init();
	ui_splash();

	while (1) {
		systime_process();
		led_process();
		//radio_process();
		tehu_process();
		button_process();

		if (tehu_is_new_results()) {
			ui_update();
		}
	}
} 
