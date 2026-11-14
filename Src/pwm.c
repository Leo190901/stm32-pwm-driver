/*
 * pwm.c
 *
 *  Created on: 31.10.2026
 *      Author: leonp
 */
#include "pwm.h"
#include <stddef.h>

/*
 * Timer kernel clock.
 * No clock configuration runs at startup (SystemInit is not implemented),
 * so the MCU stays in its reset state: HSI = 16 MHz, AHB /1, APB1 /1, APB2 /1.
 * With an APB prescaler of 1 the timer clock equals PCLK (RM0368, clock tree),
 * so TIM1..TIM4 all run at 16 MHz.
 * If the clock tree is changed later, this value must be updated.
 */
#define PWM_TIMER_CLOCK_HZ   16000000U

PWM_Status_t PWM_InitTimer(const PWM_TimerConfig_t *config)
{
	//validate config
	if (config == NULL)
	{
	    return PWM_NULL_POINTER;
	}

	if (config->frequency_hz < PWM_FREQ_MIN_HZ || config->frequency_hz > PWM_FREQ_MAX_HZ)
	{
	    return PWM_INVALID_FREQUENCY;
	}

	if (config->timer != TIM1 &&
	    config->timer != TIM2 &&
	    config->timer != TIM3 &&
	    config->timer != TIM4)
	{
	    return PWM_INVALID_TIMER;
	}

	// enable peripheral clock of the selected timer
	// (TIM1 sits on APB2, TIM2..TIM4 on APB1)
	if(config->timer == TIM1)
	{
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	}
	else if(config->timer == TIM2)
	{
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	}
	else if(config->timer == TIM3)
	{
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	}
	else if(config->timer == TIM4)
	{
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	}

	// determine prescaler value
	uint32_t timer_clock = PWM_TIMER_CLOCK_HZ;
	uint32_t frequency = config->frequency_hz;

	uint32_t psc;
	uint32_t arr;

	uint64_t divisor = (uint64_t)frequency * 65536ULL;

	psc = (uint32_t)((timer_clock + divisor - 1ULL) / divisor - 1ULL);

	// determine arr value
	arr = (uint32_t)((uint64_t)timer_clock / ((uint64_t)frequency * (psc + 1ULL)) - 1ULL);

	// write calculated values to appropriate registers
	TIM_TypeDef *timer = config->timer;

	// ARPE: buffer ARR in a shadow register, so later period changes
	// only take effect at the next update event (period boundary, REQ-010)
	timer->CR1 |= TIM_CR1_ARPE;

	timer->PSC = psc;
	timer->ARR = arr;

	// UG: generate an update event by software.
	// PSC is always buffered, so without this the new prescaler would only
	// be used after the first overflow. UG copies PSC/ARR into the shadow
	// registers immediately and resets the counter to 0.
	// The counter is NOT started here (CEN stays 0) -> see PWM_Start().
	timer->EGR = TIM_EGR_UG;

	return PWM_OK;

}

