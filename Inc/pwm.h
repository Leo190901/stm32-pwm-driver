/*
 * pwm.h
 *
 *  Created on: 31.10.2026
 *      Author: leonp
 */

#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>
#include "stm32f4xx.h"


/* Valid PWM frequency range (REQ-005) */
#define PWM_FREQ_MIN_HZ        1U
#define PWM_FREQ_MAX_HZ        100000U

/* Duty cycle is given in 0.1 % steps: 0 = 0 %, 500 = 50 %, 1000 = 100 % */
#define PWM_DUTY_MAX_PERMILLE  1000U

/* Timer channels */
#define PWM_CHANNEL_1          1U
#define PWM_CHANNEL_2          2U
#define PWM_CHANNEL_3          3U
#define PWM_CHANNEL_4          4U


typedef struct
{
    TIM_TypeDef *timer;
    uint32_t frequency_hz;      /* PWM_FREQ_MIN_HZ .. PWM_FREQ_MAX_HZ */
} PWM_TimerConfig_t;


typedef struct
{
    TIM_TypeDef *timer;
    uint8_t channel;            /* PWM_CHANNEL_1 .. PWM_CHANNEL_4 */
    uint16_t duty_permille;     /* 0 .. PWM_DUTY_MAX_PERMILLE */
} PWM_ChannelConfig_t;

typedef enum
{
    PWM_OK = 0,

    PWM_INVALID_FREQUENCY = 1,

    PWM_INVALID_TIMER = 2,

    PWM_NULL_POINTER = 3,

    PWM_INVALID_CHANNEL = 4,    /* channel out of range or no pin mapping */

    PWM_INVALID_DUTY = 5        /* duty_permille > PWM_DUTY_MAX_PERMILLE */

} PWM_Status_t;

/*
 * Configure timer clock, prescaler and period for the requested frequency.
 * The counter is not started; call PWM_Start() after PWM_InitChannel().
 * On error, no register is modified (REQ-014).
 */
PWM_Status_t PWM_InitTimer(const PWM_TimerConfig_t *config);

/*
 * Configure the GPIO pin and output-compare unit of one channel
 * (PWM mode 1, preloaded compare value) and set its initial duty cycle.
 * PWM_InitTimer() must have been called for the same timer before.
 * On error, no register is modified (REQ-014).
 */
PWM_Status_t PWM_InitChannel(const PWM_ChannelConfig_t *config);

/*
 * Enable the channel output and start the timer counter.
 * PWM_InitTimer() and PWM_InitChannel() must have been called before.
 * On error, no register is modified (REQ-014).
 */
PWM_Status_t PWM_Start(TIM_TypeDef *timer, uint8_t channel);


#endif /* PWM_H_ */
