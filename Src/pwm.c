/*
 * pwm.c
 *
 *  Created on: 31.10.2026
 *      Author: leonp
 */
#include "pwm.h"
#include <stdbool.h>
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

/*
 * True if 'timer' is one of the timers this driver supports (REQ-001).
 * NULL is rejected too, because it does not match any TIMx base address.
 * Only reads the pointer value, never the peripheral -> safe to call
 * before any validation (REQ-014).
 */
static bool pwm_is_supported_timer(const TIM_TypeDef *timer)
{
	return (timer == TIM1) ||
	       (timer == TIM2) ||
	       (timer == TIM3) ||
	       (timer == TIM4);
}

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

	if (!pwm_is_supported_timer(config->timer))
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

PWM_Status_t PWM_InitChannel(const PWM_ChannelConfig_t *config)
{
	// validate config (REQ-014: no register may be touched before this passes)
	// Step 1 supports only TIM2_CH1 on PA5 (AF1); more mappings follow later (CON-004).
	if (config == NULL)
	{
		return PWM_NULL_POINTER;
	}

	// not one of the supported timers at all
	if (!pwm_is_supported_timer(config->timer))
	{
		return PWM_INVALID_TIMER;
	}

	// channel number out of range
	if (config->channel < PWM_CHANNEL_1 || config->channel > PWM_CHANNEL_4)
	{
		return PWM_INVALID_CHANNEL;
	}

	// valid timer/channel, but no pin mapping implemented yet (CON-004):
	// the code below only handles TIM2_CH1 -> PA5 / AF1 / CCR1
	if (config->timer != TIM2 || config->channel != PWM_CHANNEL_1)
	{
		return PWM_INVALID_CHANNEL;
	}

	if (config->duty_permille > PWM_DUTY_MAX_PERMILLE)
	{
		return PWM_INVALID_DUTY;
	}

	TIM_TypeDef *timer = config->timer;

	// --- GPIO: PA5 -> alternate function 1 (TIM2_CH1), RM0368 section 8.4 ---
	// enable GPIOA clock on AHB1, otherwise writes to GPIOA are ignored
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	// MODER5 = 10 (alternate function): clear the 2-bit field, then set it
	GPIOA->MODER &= ~GPIO_MODER_MODER5;
	GPIOA->MODER |= GPIO_MODER_MODER5_1;

	// AFRL5 = 0001 (AF1 = TIM1/TIM2), see datasheet DS10086 table 9
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL5;
	GPIOA->AFR[0] |= (1UL << GPIO_AFRL_AFSEL5_Pos);

	// --- Output compare unit CH1, RM0368 section 13.4.7 (TIM2..5 CCMR1) ---
	// CC1S = 00 -> channel is an output
	// OC1M = 110 -> PWM mode 1: output HIGH while CNT < CCR1
	// OC1PE = 1 -> CCR1 is preloaded, new duty applies at the next update event (REQ-009)
	timer->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M);
	timer->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;

	// compare value: CCR1 = (ARR + 1) * duty / 1000, rounded to nearest.
	// duty 1000 -> CCR1 = ARR + 1 > CNT always -> 100 % HIGH; duty 0 -> 0 % HIGH.
	// Max intermediate: 65536 * 1000 = 65.5 M, fits easily into uint32_t.
	uint32_t period = timer->ARR + 1UL;
	timer->CCR1 = (period * config->duty_permille + 500UL) / 1000UL;

	// UG: copy the preloaded CCR1 into the active register now.
	// Safe here because the counter is not running yet (it also resets CNT).
	timer->EGR = TIM_EGR_UG;

	return PWM_OK;
}

PWM_Status_t PWM_Start(TIM_TypeDef *timer, uint8_t channel)
{
	// validate parameters (REQ-014: no register may be touched before this passes)
	// Same rules and codes as PWM_InitChannel.
	// No separate NULL check needed: pwm_is_supported_timer(NULL) is false.
	if (!pwm_is_supported_timer(timer))
	{
		return PWM_INVALID_TIMER;
	}

	if (channel < PWM_CHANNEL_1 || channel > PWM_CHANNEL_4)
	{
		return PWM_INVALID_CHANNEL;
	}

	// valid timer/channel, but only TIM2_CH1 is mapped so far (CON-004).
	// '||' matters: {TIM2, CH2} or {TIM3, CH1} must both be rejected.
	if (timer != TIM2 || channel != PWM_CHANNEL_1)
	{
		return PWM_INVALID_CHANNEL;
	}

	// --- Enable the channel output, RM0368 section 13.4.9 (TIMx_CCER) ---
	// CC1E = 1 -> the OC1 signal is driven onto the pin (via the GPIO AF set in PWM_InitChannel).
	// CC1P stays 0 -> active high: HIGH while CNT < CCR1 (PWM mode 1).
	// Read-modify-write (|=): CCER holds the enable bits of all 4 channels,
	// a plain '=' would switch off the other channels (REQ-013).
	timer->CCER |= TIM_CCER_CC1E;

	// --- Start the counter, RM0368 section 13.4.1 (TIMx_CR1) ---
	// Done after CC1E, so the very first period already appears on the pin.
	// |= keeps ARPE (set in PWM_InitTimer). Setting CEN again on a running
	// timer is harmless, so starting a second channel later is fine.
	timer->CR1 |= TIM_CR1_CEN;

	return PWM_OK;
}

