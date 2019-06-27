/*
 * pwm.h
 *
 *  Created on: Jun 20, 2019
 *      Author: jack
 */

#ifndef PWM_H_
#define PWM_H_

void init_pwm(void);

/* @param uint16_t duty_cycle: 25%-100% */
void change_pwm_duty_cycle(uint16_t duty_cycle);

uint8_t calc_new_brightness(uint32_t light_sample);

void init_dimming_timer(void);

void trigger_dimming_timer(uint32_t old_brightness, uint32_t new_brightness);

void prvChangePWM(void *pvParameters);


#endif /* PWM_H_ */
