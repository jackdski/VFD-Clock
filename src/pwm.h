/*
 * pwm.h
 *
 *  Created on: Jun 20, 2019
 *      Author: jack
 */

#ifndef PWM_H_
#define PWM_H_

#define PWM_SOURCE_FREQUENCY	1000000		// 8MHz with prescalar of 8
#define	PWM_FREQUENCY			2500  //Hz	//400	// 400us or 2.5kHz
#define CALC_PWM_DUTY_CYCLE(X)	((X * PWM_SOURCE_FREQUENCY) / PWM_FREQUENCY)

#define BRIGHTNESS_LOW			10
#define BRIGHTNESS_MEDIUM		45
#define BRIGHTNESS_HIGH			70
#define BRIGHTNESS_MAX			99

void init_pwm(void);

void enable_brightness_timer(void);

void disable_brightness_timer(void);

void toggle_display_output(void);

/* @param uint16_t duty_cycle: 0%-100% */
void change_pwm_duty_cycle(uint16_t duty_cycle);

uint8_t calc_new_brightness(uint32_t light_sample);

#endif /* PWM_H_ */
