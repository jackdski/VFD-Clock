/*
 * pwm.c
 *
 *  Created on: Jun 20, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "pwm.h"
#include "gpio.h"
#include "tubes.h"
#include "adc.h"

/*	P W M   */
void init_pwm(void) {
	/* Info:
	 * 	freq = 2.5KHz (period is 400us)
	 * 	period = 25% - 100%
	 */

	/* PA4 */
	GPIOA->MODER |=     (GPIO_MODER_MODER4);	// PA4 to output
	GPIOA->ODR &= ~(GPIO_ODR_4);				// set output to low
	GPIOA->OTYPER &=   ~(GPIO_OTYPER_OT_7);		// set to push-pull

	/* config GPIO - PA4 to AF4 */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER4)) | GPIO_MODER_MODER4_1;
	GPIOA->AFR[0] |=  (0x04 << GPIO_AFRL_AFRL4_Pos); // select AF4 on PA4

	/* config TIM14 registers */
	TIM14->PSC = 7; 	// 8MHz / (7+1) = 1MHz
	TIM14->ARR = (uint16_t)PWM_FREQUENCY; // determines PWM frequency
	TIM14->CCR1 = (uint16_t)CALC_PWM_DUTY_CYCLE(50); // determines duty cycle, init to 50%

	/* Select PWM mode 1 on OC1 (OC1M = 110),
	 	 enable preload register on OC1 (OC1PE = 1) */
	TIM14->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;

	/* Select active high polarity on OC1 (CC1P = 0, reset value),
		enable the output on OC1 (CC1E = 1) */
	TIM14->CCER |= TIM_CCER_CC1E;

//	TIM14->CCER &= ~(TIM_CCER_CC1NP);	// OC1N active high
	TIM14->CCER |= TIM_CCER_CC1NP;	// OC1N active low

	/* Enable output (MOE = 1)*/
	TIM14->BDTR |= TIM_BDTR_MOE;

	/* Enable counter (CEN = 1)
		select edge aligned mode (CMS = 00, reset value)
		select direction as upcounter (DIR = 0, reset value) */
	TIM14->CR1 |= TIM_CR1_CEN; /* (7) */

	TIM14->CCMR1 &= ~(TIM_CCMR1_CC1S); 	// CC1 channel configured as output

	/* Force update generation (UG = 1) */
	TIM14->EGR |= TIM_EGR_UG;
}

void enable_brightness_timer(void) {
	TIM14->BDTR |= TIM_BDTR_MOE;		// Enable output (MOE = 1)*/

	/* config GPIO - PA4 to AF4 */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER4)) | GPIO_MODER_MODER4_1;
	GPIOA->AFR[0] |=  (0x04 << GPIO_AFRL_AFRL4_Pos); // select AF4 on PA4
}

void disable_brightness_timer(void) {
	TIM14->BDTR &= ~(TIM_BDTR_MOE);		// disable timer output

	/* config GPIO - PA4 to AF0 */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER4));
	GPIOA->AFR[0] &=  ~(0x04 << GPIO_AFRL_AFRL4_Pos); // select AF0 on PA4
}

void inline toggle_display_output(void) {
	GPIOA->ODR ^= GPIO_ODR_4;
}

/* @param uint16_t duty_cycle: 0%-100% */
void change_pwm_duty_cycle(uint16_t duty_cycle) {
	if(duty_cycle > 100)
		TIM14->CCR1 = ((uint16_t)CALC_PWM_DUTY_CYCLE(100));
	else {
		uint16_t duty = ((uint16_t)CALC_PWM_DUTY_CYCLE(duty_cycle));
		TIM14->CCR1 = duty;
	}
}

uint16_t inline get_pwm_duty_cycle(void) {
	return (TIM14->CCR1 * 100) / (uint16_t)PWM_FREQUENCY;
}

uint8_t calc_new_brightness(uint32_t light_sample) {
	/* - switch according to the value the ADC is reporting from the photoresistor circuit
	 * - use dead bands to avoid switching back and forth often */
	if(light_sample < RANGE_THRESHOLD_1) {
		return BRIGHTNESS_LOW;
	}
	else if((light_sample < RANGE_THRESHOLD_2) && (light_sample >= RANGE_THRESHOLD_1)) {
		return BRIGHTNESS_MEDIUM;
	}
	else if((light_sample < RANGE_THRESHOLD_3) && (light_sample >= RANGE_THRESHOLD_2)) {
		return BRIGHTNESS_HIGH;
	}
	else if(light_sample >= RANGE_THRESHOLD_3) {
		return BRIGHTNESS_MAX;
	}
	/* default */
	return BRIGHTNESS_HIGH;
}
