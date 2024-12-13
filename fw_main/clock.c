#include "platform.h"

#include "clock.h"

#define TIMEOUT 0xfffff

uint32_t clock_frequency;
uint32_t clock_apb1_frequency;
uint32_t clock_apb2_frequency;

/**
 * Initialize RCC to HSI clock, 8MHz
 */
void clock_init(void)
{
  /* Reset the RCC clock configuration to the default reset state(for debug purpose) */
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
  RCC->CFGR &= (uint32_t)0xF8FF0000;
  
  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
  RCC->CFGR &= (uint32_t)0xFF80FFFF;

  /* Disable all interrupts and clear pending bits  */
  RCC->CIR = 0x009F0000;

	clock_frequency = 8000000;
	clock_apb1_frequency = 8000000;
	clock_apb2_frequency = 8000000;
}

/**
 * Initialize RCC to 2.56MHz external clock
 */
int clock_ext(void)
{
	uint32_t timeout;

	timeout = TIMEOUT;
	RCC->CR |= ((uint32_t)RCC_CR_HSEON);
	while (timeout) {
		if (RCC->CR & RCC_CR_HSERDY) {
			break;
		}

		timeout--;
	}

	if (timeout == 0) {
		return 1;
	}

	uint32_t cfgr = RCC->CFGR;
	cfgr &= ~(RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE2_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_ADCPRE_Msk);
	cfgr |= RCC_CFGR_ADCPRE_DIV4;
	cfgr |= RCC_CFGR_HPRE_DIV1;
	cfgr |= RCC_CFGR_PPRE2_DIV2;
	cfgr |= RCC_CFGR_PPRE1_DIV2;

	cfgr &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
	cfgr |= (uint32_t)(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL8);

	RCC->CFGR = cfgr;

	RCC->CR |= RCC_CR_PLLON;

	/* Wait till PLL is ready */
	timeout = TIMEOUT;
	while(timeout) {
		if (RCC->CR & RCC_CR_PLLRDY) {
			break;
		}

		timeout--;
	}

	if (timeout == 0) {
		return 1;
	}

	/* Select PLL as system clock source */
	RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
	RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

	/* Wait till PLL is used as system clock source */
	timeout = TIMEOUT;
	while (timeout) {
		if ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) == (uint32_t)RCC_CFGR_SWS_PLL) {
			break;
		}

		timeout--;
	}

	if (timeout == 0) {
		return 1;
	}

	clock_frequency = 20480000;
	clock_apb1_frequency = 10240000;
	clock_apb2_frequency = 10240000;

	return 0;
}

uint32_t clock_get_i2c_frequency(I2C_TypeDef *i2c)
{
	if ((uintptr_t)i2c < (uintptr_t)APB2PERIPH_BASE)
	{
		// APB1
		return clock_apb1_frequency;
	}

	return clock_apb2_frequency;
}

uint32_t clock_get_timer_frequency(TIM_TypeDef *tim)
{
	uint32_t base_clk;
	uint32_t mul;

	if ((uintptr_t)tim < (uintptr_t)APB2PERIPH_BASE)
	{
		// APB1
		base_clk = clock_apb1_frequency;
		mul = (RCC->CFGR & RCC_CFGR_PPRE1_Msk) ? 2 : 1;
	}
	else
	{
		// apb2
		base_clk = clock_apb2_frequency;
		mul = (RCC->CFGR & RCC_CFGR_PPRE2_Msk) ? 2 : 1;
	}

	return base_clk * mul;
}
