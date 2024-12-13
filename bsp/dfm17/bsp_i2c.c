#include "platform.h"

#include "gpio.h"
#include "bsp_i2c.h"

const struct gpio_init_table_t i2c1_gpio[] = {
	{
		.gpio = GPIOB,
		.pin = GPIO_Pin_6,
		.mode = GPIO_MODE_AF_OD,
		.speed = GPIO_SPEED_LOW,
	},
	{
		.gpio = GPIOB,
		.pin = GPIO_Pin_7,
		.mode = GPIO_MODE_AF_OD,
		.speed = GPIO_SPEED_LOW,
	},
};

const int i2c1_gpio_cnt = ARRAY_SIZE(i2c1_gpio);
