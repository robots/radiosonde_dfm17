#ifndef CLOCK_h_
#define CLOCK_h_

extern uint32_t clock_frequency;
extern uint32_t clock_apb_frequency;

void clock_init(void);
int clock_ext(void);
uint32_t clock_get_i2c_frequency(I2C_TypeDef *i2c);
uint32_t clock_get_timer_frequency(TIM_TypeDef *tim);

#endif
