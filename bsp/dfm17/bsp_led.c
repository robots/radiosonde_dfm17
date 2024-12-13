
#include "platform.h"

#include "gpio.h"
#include "bsp_led.h"

const struct gpio_init_table_t led_gpio[] = {
	{ // red
		.gpio = GPIOB,
		.pin = GPIO_Pin_12,
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_MED,
	},
	{ // yellow
		.gpio = GPIOC,
		.pin = GPIO_Pin_7,
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_MED,
	},
	{ // green
		.gpio = GPIOC,
		.pin = GPIO_Pin_6,
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_MED,
	},
};

const uint8_t led_pol[] = {
	0, 0, 0,
};

const uint32_t led_gpio_cnt = ARRAY_SIZE(led_gpio);

