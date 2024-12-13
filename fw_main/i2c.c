
#include "platform.h"

#include "clock.h"

#include "i2c.h"
#include "bsp_i2c.h"

#define I2C_FREQ         100000
#define DELAY_CYCLES     2u
#define DELAY_5MS_CYCLES 5000u
#define MAX_TIMEOUT 0x0000ffff

#define DELAY_HALF() \
do { \
  volatile uint32_t ii = DELAY_CYCLES; \
  while(ii > 0u) ii--; \
} while (0)

#define DELAY_TWR() \
do { \
  volatile uint32_t i = DELAY_5MS_CYCLES; \
  while(i > 0u) i--; \
} while (0)

#define WAIT_COND(x) \
  do {               \
    timeout = MAX_TIMEOUT;\
    while(1)\
    {\
      if (x)\
      {\
        break;\
      }\
\
      timeout --;\
      if (timeout == 0u)\
      {\
        return 1;\
      }\
    }\
  } while(0)


void i2c_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	gpio_init(i2c1_gpio, i2c1_gpio_cnt);

	I2C1->CR1 |= I2C_CR1_SWRST;
	DELAY_HALF();
	I2C1->CR1 &= ~I2C_CR1_SWRST;
	DELAY_HALF();

	uint32_t clock = clock_get_i2c_frequency(I2C1);

	uint32_t f = clock / 1000000;
	I2C1->CR2 = f;

	uint32_t freq_value = clock / (I2C_FREQ * 3);
	freq_value &= I2C_CCR_CCR;

	// configure ccr
	I2C1->CCR &= ~(I2C_CCR_FS | I2C_CCR_DUTY | I2C_CCR_CCR);
	I2C1->CCR |= I2C_CCR_FS;
	I2C1->CCR |= freq_value;

	// calculate trise for fastmode
	I2C1->TRISE = (f * 300u / 1000u) + 1u;

	I2C1->OAR1 = 1UL << 14U;
	I2C1->CR1 |= I2C_CR1_PE;
}

int i2c_start(void)
{
	uint32_t timeout;

	// If line is busy, try to recover
	if ((I2C1->SR2 & I2C_SR2_BUSY) != 0) {
		i2c_init();
	}

	// generate start
	I2C1->CR1 |= I2C_CR1_ACK;
	I2C1->CR1 |= I2C_CR1_START;

	// wait for MASTER select
	WAIT_COND((I2C1->SR1 & I2C_SR1_SB) == I2C_SR1_SB);

	return 0;
}

int i2c_stop(void)
{
	uint32_t timeout;

	// generate stop
	I2C1->CR1 |= I2C_CR1_STOP;

	// wait for STOP to be sent
	WAIT_COND((I2C1->CR1 & I2C_CR1_STOP) == 0);

	return 0;
}

int i2c_write_addr(uint8_t addr)
{
	uint32_t timeout;

	// send write address
	I2C1->DR = addr << 1;

	// wait for ADDR bit
	WAIT_COND((I2C1->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR);

	// clear ADDR flag
	volatile uint8_t sr;
	sr = I2C1->SR1 | I2C1->SR2;
	(void)sr;

	return 0;
}

int i2c_write(uint8_t *data, uint8_t count)
{
	uint32_t timeout;

	// write whole buffer
	for (uint8_t i = 0; i < count; i++) {
		WAIT_COND((I2C1->SR1 & I2C_SR1_TXE) != 0);
		I2C1->DR = data[i];
	}

	// wait for last byte to be sent
	WAIT_COND((I2C1->SR1 & I2C_SR1_BTF) != 0);

	return 0;
}

int i2c_read(uint8_t addr, uint8_t *data, uint8_t count)
{
	uint32_t timeout;

	if (count == 0u) {
		return 0;
	}

	// single byte is easy
	if (count == 1u) {
		// send read address
		I2C1->DR = (addr << 1u) | 1u;
		WAIT_COND((I2C1->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR);

		// clear ack before clearing ADDR
		I2C1->CR1 &= ~I2C_CR1_ACK;

		// clear addr bit
		volatile uint8_t sr;
		sr = I2C1->SR1 | I2C1->SR2;
		(void)sr;
		I2C1->CR1 |= I2C_CR1_STOP;

		// wait for RXNE
		WAIT_COND((I2C1->SR1 & I2C_SR1_RXNE) == I2C_SR1_RXNE);

		// read data
		data[0] = I2C1->DR;
	} else { // count > 1

		// send read address
		I2C1->DR = (addr << 1u) | 1u;
		WAIT_COND((I2C1->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR);

		// clear addr bit
		volatile uint8_t sr;
		sr = I2C1->SR1 | I2C1->SR2;
		(void)sr;

		// read until there are 2 left
		while (count > 2u)
		{
			// wait for RXNE
			WAIT_COND((I2C1->SR1 & I2C_SR1_RXNE) != 0);

			data[0] = I2C1->DR;
			data++;
			count--;
		}

		// receive Second to last byte

		// wait for RXNE
		WAIT_COND((I2C1->SR1 & I2C_SR1_RXNE) != 0);
		data[0] = I2C1->DR;

		// clear ACK, and set STOP generation
		I2C1->CR1 &= ~I2C_CR1_ACK;
		I2C1->CR1 |= I2C_CR1_STOP;
		data++;
		count--;

		// read last byte

		// wait for RXNE
		WAIT_COND((I2C1->SR1 & I2C_SR1_RXNE) != 0);
		data[0] = I2C1->DR;
	}

	return 0;
}
