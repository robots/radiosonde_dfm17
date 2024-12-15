#include "platform.h"

#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "led.h"
#include "spi.h"
#include "systime.h"

#include "clock.h"
#include "eeprom.h"
#include "si4063.h"
#include "tic.h"
#include "tehu.h"
#include "ui.h"


int main(void)
{
	clock_init();

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_AFIOEN;
	AFIO->MAPR |= (0x02 << 24); // only swj

	delay_init();

	spi_init();
	si4063_init();
	
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

		if (tehu_is_new_results()) {
			ui_update();
		}
	}
} 
