
#include "platform.h"

#include "gpio.h"


void gpio_init(const struct gpio_init_table_t *t, int num)
{
	int i;
	for (i = 0; i < num; i++) {
		uint16_t pos = 0;
		uint32_t mode;

		mode = t[i].mode & 0xf;
		if (t[i].mode & 0x10) {
			mode |= t[i].speed;
		}
			
		if (!(t[i].mode == GPIO_MODE_AIN || t[i].mode == GPIO_MODE_IN_FLOATING || t[i].mode == GPIO_MODE_IPD || t[i].mode == GPIO_MODE_IPU)) {
			gpio_set(&t[i], t[i].state);
		}

		if (t[i].pin & 0xff) {
			uint32_t cr;
			cr = t[i].gpio->CRL;
			
			for (pos = 0; pos < 8; pos++) {
				if (!(t[i].pin & (1 << pos))) {
					continue;
				}

				uint32_t pinmask = 0x0f << (4*pos);

				cr &= ~pinmask;
				cr |= mode << (4*pos);

				if (t[i].mode == GPIO_MODE_IPD) {
					t[i].gpio->BRR = 0x01 << pos;
				} else if (t[i].mode == GPIO_MODE_IPU) {
					t[i].gpio->BSRR = 0x01 << pos;
				}
			}

			t[i].gpio->CRL = cr;
		}

		if (t[i].pin & 0xff00) {
			uint32_t cr;
			cr = t[i].gpio->CRH;
			
			for (pos = 0; pos < 8; pos++) {
				if (!(t[i].pin & (1 << (pos+8)))) {
					continue;
				}

				uint32_t pinmask = 0x0f << (4*pos);

				cr &= ~pinmask;
				cr |= mode << (4*pos);

				if (t[i].mode == GPIO_MODE_IPD) {
					t[i].gpio->BRR = 0x01 << pos;
				} else if (t[i].mode == GPIO_MODE_IPU) {
					t[i].gpio->BSRR = 0x01 << pos;
				}
			}

			t[i].gpio->CRH = cr;
		}
	}
}

uint8_t gpio_wait_state(const struct gpio_init_table_t *gpio, uint8_t state)
{
	volatile uint32_t time = 0xFFFFFF;

	while (--time && ((!!(gpio->gpio->IDR & gpio->pin)) != (!!state)));

	return (time > 0)?0:1;
}

void gpio_set(const struct gpio_init_table_t *gpio, uint8_t state)
{
	switch (state) {
		case GPIO_SET:
			gpio->gpio->BSRR = gpio->pin;
			break;
		case GPIO_RESET:
			gpio->gpio->BRR = gpio->pin;
			break;
		default:
			break;
	}
}

uint8_t gpio_get(const struct gpio_init_table_t *gpio)
{
	return !!(gpio->gpio->IDR & gpio->pin);
}
