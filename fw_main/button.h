#ifndef BUTTON_h_
#define BUTTON_h_

enum {
	BUTTON_EV_NONE,
	BUTTON_EV_SHORT,
	BUTTON_EV_LONG,
};

typedef void (*button_cb_t)(uint32_t, uint32_t);

void button_init(void);
void button_set_ev_handler(button_cb_t cb);
void button_process(void);

#endif
