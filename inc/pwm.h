/*
 * pwm.h
 *
 *  Created on: Jun 20, 2019
 *      Author: jack
 */

#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

#define STARTING_DISPLAY_BRIGHTNESS 	50	// initialize display to 50% brightness

void prvChange_Brightness_Task(void *pvParameters);
void set_target_brightness(uint32_t setting);
void init_pwm(void);
void enable_brightness_timer(void);
void disable_brightness_timer(void);
void toggle_display_output(void);
void change_pwm_duty_cycle(uint16_t duty_cycle);
uint16_t get_pwm_duty_cycle(void);
uint8_t calc_new_brightness(uint32_t light_sample);

#endif /* PWM_H_ */
