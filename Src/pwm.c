/*
 * pwm.c
 *
 *  Created on: 31.10.2026
 *      Author: leonp
 */
#include "pwm.h"
#include <stddef.h>

PWM_Status_t PWM_InitTimer(const PWM_TimerConfig_t *config)
{
	//validate config
	if (config == NULL)
	{
	    return PWM_NULL_POINTER;
	}

	if (config->frequency_hz < 1 || config->frequency_hz > 100000)
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

	// enable appropriate Timer
	if(config->timer == TIM1)
	{
		RCC->APB2ENR |= (1U << 0);
	}
	else if(config->timer == TIM2)
	{
		RCC->APB1ENR |= (1U << 0);
	}
	else if(config->timer == TIM3)
	{
		RCC->APB1ENR |= (1U << 1);
	}
	else if(config->timer == TIM4)
	{
		RCC->APB1ENR |= (1U << 2);
	}

	// determine prescaler value
	uint32_t timer_clock = 16000000U;
	uint32_t frequency = config->frequency_hz;

	uint32_t psc;
	uint32_t arr;

	uint64_t divisor = (uint64_t)frequency * 65536ULL;

	psc = (uint32_t)((timer_clock + divisor - 1ULL) / divisor - 1ULL);

	// determine arr value
	arr = (uint32_t)((uint64_t)timer_clock / ((uint64_t)frequency * (psc + 1ULL)) - 1ULL);

	// write calculated values to appropriate registers
	TIM_TypeDef *timer = config->timer;

	timer->PSC = psc;
	timer->ARR = arr;

	return PWM_OK;

}

