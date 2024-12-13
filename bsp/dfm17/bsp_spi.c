#include "platform.h"

#include "gpio.h"
#include "bsp_spi.h"

const struct gpio_init_table_t spi_gpio[] = {
	{ // SCK 
		.gpio = GPIOA,
		.pin = GPIO_Pin_5,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
	},
	{ // MOSI
		.gpio = GPIOA,
		.pin = GPIO_Pin_7,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
	},
	{ // MISO
		.gpio = GPIOA,
		.pin = GPIO_Pin_6,
		.mode = GPIO_MODE_IPU,
		.speed = GPIO_SPEED_HIGH,
	},
};

const struct gpio_init_table_t spi_cs_gpio[] = {
	{ // CS0 
		.gpio = GPIOB,
		.pin = GPIO_Pin_2,
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_HIGH,
		.state = GPIO_SET,
	},
};

const int spi_gpio_cnt = ARRAY_SIZE(spi_gpio);
const int spi_cs_gpio_cnt = ARRAY_SIZE(spi_cs_gpio);

