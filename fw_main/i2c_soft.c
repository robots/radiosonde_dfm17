#include "platform.h"

#include "delay.h"
#include "gpio.h"


#define DELAY 50
#define __delay_cycles(x) delay(x)

#define sda_state()       (!!(GPIOB->IDR & GPIO_Pin_10))
#define sda_setlow()      GPIOB->BRR = GPIO_Pin_10
#define sda_sethigh()     GPIOB->BSRR = GPIO_Pin_10
#define scl_setlow()      GPIOB->BRR = GPIO_Pin_11
#define scl_sethigh()     GPIOB->BSRR = GPIO_Pin_11


static void softi2c_start();
static void softi2c_stop();
static uint8_t softi2c_recvack();
static uint8_t softi2c_write(uint8_t byte);
#if 0
static void softi2c_sendack(uint8_t ack);
static uint8_t softi2c_read(uint8_t ack);

static uint8_t softi2c_reg_read8(uint8_t addr, uint8_t reg);
static void softi2c_reg_write8(uint8_t addr, uint8_t reg, uint8_t val);
#endif

const struct gpio_init_table_t i2c2_soft_gpio[] = {
	{ // scl
		.gpio = GPIOB,
		.pin = GPIO_Pin_11,
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_HIGH,
	},
	{ // sda
		.gpio = GPIOB,
		.pin = GPIO_Pin_10,
		.mode = GPIO_MODE_OUT_OD,
		.speed = GPIO_SPEED_HIGH,
	},
};


void i2c2_init(uint32_t x)
{
	(void)x;
	gpio_init(i2c2_soft_gpio, ARRAY_SIZE(i2c2_soft_gpio));

	sda_sethigh();
	scl_sethigh();
}

void i2c2_start(int tx, uint8_t addr)
{
	(void)tx;
	softi2c_start();

	softi2c_write(addr << 1);
}

void i2c2_write_byte(uint8_t data)
{
	softi2c_write(data);
}

void i2c2_stop(void)
{
	softi2c_stop();
}

static void softi2c_start()
{
	sda_setlow();
	__delay_cycles(DELAY);
	scl_setlow();
	__delay_cycles(DELAY);
}

static void softi2c_stop()
{
	sda_setlow();
	__delay_cycles(DELAY);
	scl_sethigh();
	__delay_cycles(DELAY);
	sda_sethigh();
	__delay_cycles(DELAY*2);
}

static uint8_t softi2c_recvack()
{
	int ack;
	sda_sethigh();
	scl_sethigh();
	__delay_cycles(DELAY/2);
	ack = sda_state();
	__delay_cycles(DELAY/2);
	scl_setlow();
	__delay_cycles(DELAY);

	return !ack;
}

static uint8_t softi2c_write(uint8_t byte)
{
	uint8_t cnt = 8;

	while (cnt) {
		if (byte & 0x80)
			sda_sethigh();
		else
			sda_setlow();

		byte <<= 1;

		scl_sethigh();
		__delay_cycles(DELAY);
		scl_setlow();
		__delay_cycles(DELAY);

		cnt--;
	}

	return softi2c_recvack();
}
