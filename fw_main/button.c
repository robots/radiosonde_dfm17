#include "platform.h"

#include "gpio.h"
#include "systime.h"

#include "button.h"

#define BUTTON_LONG 4000
#define BUTTON_SHORT 100

const struct gpio_init_table_t button_gpio[] = {
	{ // btn
		.gpio = GPIOC,
		.pin =  BV(8),
		.mode = GPIO_MODE_IN_FLOATING,
	},
};

struct button_state_t {
	uint32_t t;
	uint32_t state;
} button_state[ARRAY_SIZE(button_gpio)];


button_cb_t button_cb;

void button_init(void)
{
	gpio_init(button_gpio, ARRAY_SIZE(button_gpio));
	for (uint32_t i = 0; i < ARRAY_SIZE(button_gpio); i++) {
		button_state[i].t = systime_get();
		button_state[i].state = gpio_get(&button_gpio[i]);
	}
}

void button_set_ev_handler(button_cb_t cb)
{
	button_cb = cb;
}

void button_process(void)
{
	for (uint32_t i = 0; i < ARRAY_SIZE(button_gpio); i++) {
		uint32_t s = gpio_get(&button_gpio[i]);

		if (button_state[i].state != s) {
			button_state[i].state = s;

			if (s == 1) {
				button_state[i].t = systime_get();
			} else {
				uint32_t diff = systime_get() - button_state[i].t;
				if (diff >= BUTTON_LONG) {
					if (button_cb) button_cb(i, BUTTON_EV_LONG);
				} else if (diff >= BUTTON_SHORT) {
					if (button_cb) button_cb(i, BUTTON_EV_SHORT);
				}
			}
		}
	}
}
