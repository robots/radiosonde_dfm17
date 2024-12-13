#include "platform.h"

#include "gpio.h"
#include "bsp_uart.h"

const struct gpio_init_table_t uart1_gpio[] = {
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_9,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
	},
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_10,
		.mode = GPIO_MODE_IPU,
		.speed = GPIO_SPEED_HIGH,
	},
};

const struct gpio_init_table_t uart2_gpio[] = {
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_2,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
	},
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_3,
		.mode = GPIO_MODE_IPU,
		.speed = GPIO_SPEED_HIGH,
	},
};

const struct gpio_init_table_t uart3_gpio[] = {
	{
		.gpio = GPIOB,
		.pin = GPIO_Pin_10,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
	},
	{
		.gpio = GPIOB,
		.pin = GPIO_Pin_11,
		.mode = GPIO_MODE_IPU,
		.speed = GPIO_SPEED_HIGH,
	},
};

const int uart1_gpio_cnt = ARRAY_SIZE(uart1_gpio);
const int uart2_gpio_cnt = ARRAY_SIZE(uart2_gpio);
const int uart3_gpio_cnt = ARRAY_SIZE(uart3_gpio);
