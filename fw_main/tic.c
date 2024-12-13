/**
 * TIM2 - input PA1 - tic trigger connected to PB0 and PA11
 * TIM3 - input PB0 - tic trigger
 * TIM1 - input PA12, output PB13, PA11
 * TIM17 - output PB9 - 1.28MHz test clock
 */
#include "platform.h"

#include "clock.h"
#include "gpio.h"

#include "tic.h"

#define DMA_ENABLE 0x501
#define DMA_DISABLE 0x500

const struct gpio_init_table_t tic_gpio[] = {
	{ // TEMP - temp E
		.gpio = GPIOB,
		.pin =  BV(5), 
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_LOW,
		.state = GPIO_RESET,
	},
	{ // TEMP - temp 0
		.gpio = GPIOC,
		.pin =  BV(11), 
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_LOW,
		.state = GPIO_RESET,
	},
	{ // 20k ref - ref 6
		.gpio = GPIOD,
		.pin =  BV(2), 
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_LOW,
		.state = GPIO_RESET,
	},
	{ // 332k ref - ref a
		.gpio = GPIOB,
		.pin =  BV(4), 
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_LOW,
		.state = GPIO_SET,
	},
	{  // humidity D
		.gpio = GPIOC,
		.pin =  BV(10), 
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_LOW,
		.state = GPIO_RESET,
	},
	{ // TIM2 - CC input
		.gpio = GPIOA,
		.pin =  BV(1), 
		.mode = GPIO_MODE_IN_FLOATING,
	},
	{ // TIM3 - CC input
		.gpio = GPIOB,
		.pin = BV(0),
		.mode = GPIO_MODE_IN_FLOATING,
	},
	{ // TIM1 output TIM1_CH4
		.gpio = GPIOA,
		.pin =  BV(11),
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_LOW,
	},
	{ // TIM1 ETR input
		.gpio = GPIOA,
		.pin =  BV(12), 
		.mode = GPIO_MODE_IPD,
	},
	{ // TIM1 output TIM1_CH1N
		.gpio = GPIOB,
		.pin = BV(13),
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_MED,
	},
	{ // TIM15 output remapped ch2
		.gpio = GPIOB,
		.pin =  BV(15), 
		.mode = GPIO_MODE_AF_OD,
		.speed = GPIO_SPEED_MED,
	},
	{ //  TIM17 cc output 1.28MHz test signal
		.gpio = GPIOB,
		.pin =  BV(9), 
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_MED,
	},
};

volatile uint16_t result_tim3;
volatile uint16_t result_tim2;
volatile uint16_t temp;

void tic_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN | RCC_APB2ENR_TIM17EN | RCC_APB2ENR_TIM15EN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;
	AFIO->MAPR2 |= 1;

	gpio_init(tic_gpio, ARRAY_SIZE(tic_gpio));

	// TIM2 and TIM3 work as 32bit counter for measuring pulse length
	TIM2->CR1   = 1;
	TIM2->CR2   = 0x0020;
	TIM2->DIER  = 0x0600;
	TIM2->CCMR1 = 0x4142;
	TIM2->CCER  = 0x0011;
	TIM2->PSC   = 0x0003;
	TIM2->ARR   = 0xffff;

	TIM3->CR1   = 1;
	TIM3->SMCR  = 0x0017;
	TIM3->DIER  = 0x0800;
	TIM3->CCMR2 = 0x0041;
	TIM3->CCER  = 0x0100;
	TIM3->PSC   = 0x0000;
	TIM3->ARR   = 0xffff;

	TIM15->SMCR = 0x56;
	TIM15->EGR  = 1;
	TIM15->CCMR1 = 0x7001;
	TIM15->CCER = 0x32;
	TIM15->ARR  = 0x3c;
	TIM15->PSC  = 0;
	TIM15->CCR2 = 0x28;
	TIM15->BDTR = 0x8000;
	TIM15->CR1  = 9;

	// initialize TIM1 as oscillator
	TIM1->CR2   = 0x200;
	TIM1->SMCR  = 0x276;
	TIM1->EGR   = 1;
	TIM1->CCMR1 = 0x0070;
	TIM1->CCMR2 = 0x7000;
	TIM1->CCER  = 0x100d;
	TIM1->PSC   = 0;
	TIM1->ARR   = 100;
	TIM1->CCR1  = 5;
	TIM1->CCR4  = 1;
	TIM1->BDTR  = 0x8002;


	TIM1->CR1   = 0x9;

	//TIM17 debug output 1.2MHz
	TIM17->CCMR1 = 0x30;
	TIM17->ARR   = 0x07;
	TIM17->CCER  = 0x01;
	TIM17->CR1   = 0x05;
	TIM17->BDTR  = 0x8000;

	// dma to capture tim2 ccr2 and tim3 ccr3 
	//TIM3_CC3
	DMA1_Channel2->CPAR = (uint32_t)&TIM3->CCR3;
	DMA1_Channel2->CMAR = (uint32_t)&result_tim3;

	//TIM2_CH2
	DMA1_Channel7->CPAR = (uint32_t)&TIM2->CCR2;
	DMA1_Channel7->CMAR = (uint32_t)&result_tim2;

	//TIM2_CH1
	DMA1_Channel5->CPAR = (uint32_t)&TIM2->CCR1;
	DMA1_Channel5->CMAR = (uint32_t )&temp;

	tic_select_channel(TIC_CH_REF_A);
}

void tic_select_channel(int c)
{
	gpio_set(&tic_gpio[0], GPIO_RESET);	
	gpio_set(&tic_gpio[1], GPIO_RESET);	
	gpio_set(&tic_gpio[2], GPIO_RESET);	
	gpio_set(&tic_gpio[3], GPIO_RESET);	
	gpio_set(&tic_gpio[4], GPIO_RESET);	

	TIM15->ARR = 100;
	TIM15->CCR2 = 0x50;

	TIM1->ARR = 200;
	if (c == TIC_CH_HUM_D) {
		TIM1->CCR1=0xb0;
	} else {
		TIM1->CCR1=0x2e;
	}

	switch (c)
	{
		case TIC_CH_TEMP_E:
			gpio_set(&tic_gpio[0], GPIO_SET);
			break;
		case TIC_CH_TEMP_O:
			gpio_set(&tic_gpio[1], GPIO_SET);
			break;
		case TIC_CH_REF_6:
			gpio_set(&tic_gpio[2], GPIO_SET);
			break;
		case TIC_CH_REF_A:
			gpio_set(&tic_gpio[3], GPIO_SET);
			break;
		case TIC_CH_HUM_D:
			gpio_set(&tic_gpio[3], GPIO_SET);
			gpio_set(&tic_gpio[4], GPIO_SET);
			break;
	}
}

void tic_start(void)
{
	// enable dma transfers
	DMA1_Channel2->CNDTR=2;
	DMA1_Channel7->CNDTR=2;
	DMA1_Channel5->CNDTR=0xffff;

	DMA1_Channel2->CCR=DMA_ENABLE;
	DMA1_Channel7->CCR=DMA_ENABLE;
	DMA1_Channel5->CCR=DMA_ENABLE;

	// enable CC inputs
	TIM3->CCER=0x00000100;
	TIM2->CCER=0x00000011;

	// enable counter
	TIM3->CR1=0x00000001;
	TIM2->CR1=0x00000001;
}

void tic_stop(void)
{
	// disable CC inputs
	TIM3->CCER = 0;
	TIM2->CCER = 0;

	// disable counter
	TIM3->CR1 = 0;
	TIM2->CR1 = 0;

	DMA1_Channel2->CCR=DMA_DISABLE;
	DMA1_Channel5->CCR=DMA_DISABLE;
	DMA1_Channel7->CCR=DMA_DISABLE;
}

float tic_get_result(void)
{
	// result_timx contains first capture of timer2+3
	// CCRx registers of TIM2+3 contain last capture

	// total time between start and end:
	uint32_t t = (TIM2->CCR1 - result_tim2) + (TIM3->CCR3 - result_tim3) * 0x10000;

	// number of periods between start and end.
	uint32_t periods = 0xffff - DMA1_Channel5->CNDTR - 1;

	uint32_t tim_khz = clock_get_timer_frequency(TIM2) / (4*1000);
	
	// calculate average period in miliseconds
	float result = (float)t / (float)(periods * tim_khz);

	return result;
}
