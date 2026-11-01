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


typedef struct
{
    TIM_TypeDef *timer;
    uint32_t frequency_hz;
} PWM_TimerConfig_t;


typedef struct
{
    TIM_TypeDef *timer;
    uint8_t channel;
    float duty_cycle;
} PWM_ChannelConfig_t;

typedef enum
{
    PWM_OK = 0,

    PWM_INVALID_FREQUENCY = 1,

    PWM_INVALID_TIMER = 2,

    PWM_NULL_POINTER = 3

} PWM_Status_t;

PWM_Status_t PWM_InitTimer(const PWM_TimerConfig_t *config);


#endif /* PWM_H_ */
