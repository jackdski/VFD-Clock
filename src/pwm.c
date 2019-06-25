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

/*	D E F I N E S   */
#define	PWM_FREQUENCY	400		// 400us or 2.5kHz
//#define	PWM_FREQUENCY	1000		// 1000us or 1kHz

#define BRIGHTNESS_LOW		25
#define BRIGHTNESS_MEDIUM	50
#define BRIGHTNESS_HIGH		75
#define BRIGHTNESS_MAX		95

//#define DIMMING_DEMO
#define BRIGHTNESS_DEMO

/*	G L O B A L   V A R I A B L E S   */
extern uint32_t light_value;;

void init_pwm(void) {
	/* Info:
	 * 	freq = 2.5KHz (period is 400us)
	 * 	period = 25% - 100%
	 */

	/* config GPIO - PA8 to AF2 */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER8)) | GPIO_MODER_MODER8_1;
	GPIOA->AFR[1] |=  (0x02 << GPIO_AFRH_AFRH0_Pos); // select AF2 on PA8

	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER7)) | GPIO_MODER_MODER7_1;
	GPIOA->AFR[0] |=  (0x02 << GPIO_AFRL_AFRL7_Pos); // select AF1 on PA7

	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER4)) | GPIO_MODER_MODER4_1;
	GPIOA->AFR[0] |=  (0x04 << GPIO_AFRL_AFRL4_Pos); // select AF4 on PA4



	/* config TIM14 registers */
	TIM14->PSC = 7; 	// 8MHz / (7+1) = 1MHz
	TIM14->ARR = (uint16_t)PWM_FREQUENCY; // determines PWM frequency
	TIM14->CCR1 = (uint16_t)PWM_FREQUENCY / 2; // determines duty cycle, init to 50%

	/* Select PWM mode 1 on OC1 (OC1M = 110),
	 	 enable preload register on OC1 (OC1PE = 1) */
	TIM14->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;

	/* Select active high polarity on OC1 (CC1P = 0, reset value),
		enable the output on OC1 (CC1E = 1) */
	TIM14->CCER |= TIM_CCER_CC1E;

	TIM14->CCER &= ~(TIM_CCER_CC1NP);	// OC1N active high

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

/* @param uint16_t duty_cycle: 25%-100% */
void change_pwm_duty_cycle(uint16_t duty_cycle) {
//	if(duty_cycle >= 25 && duty_cycle <= 25) {
	uint16_t duty = ((uint16_t)PWM_FREQUENCY * duty_cycle) / 100;
	TIM14->CCR1 = duty;
//	}
}

void prvChangePWM(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 100 ); // 1s
	uint8_t i = 1;
	uint8_t dir = 0;
	for( ;; ) {
		vTaskDelay(delay_time);
#ifdef	ROTATION_DEMO
		if(i == 1) {
			change_pwm_duty_cycle(5);
			dir = 0;
		}
		else if(i == 2)
			change_pwm_duty_cycle(25);
		else if(i == 3)
			change_pwm_duty_cycle(50);
		else if(i == 4)
			change_pwm_duty_cycle(75);
		else if(i == 5) {
			change_pwm_duty_cycle(100);
			dir = 1;
		}
#endif

#ifdef DIMMING_DEMO
		change_pwm_duty_cycle(i);

		if(i == 100)
			dir = 1;
		else if(i == 0)
			dir = 0;

		if(dir == 0)
			i++;
		else
			i--;
#endif
#ifdef BRIGHTNESS_DEMO
		/* switch according to the value the ADC is reporting from the photoresistor circuit
		 *
		 * use dead bands to avoid switching back and forth often
		 */
		if(light_value < 40) {
			while(i != BRIGHTNESS_LOW) {
				if(i > BRIGHTNESS_LOW)
					i--;
				else if(i < BRIGHTNESS_LOW)
					i++;
				change_pwm_duty_cycle(i);
			}
		}
		/* 45 <= light_value < 70 */
		else if((light_value < 80) && (light_value >= 40)) {
			while(i != BRIGHTNESS_MEDIUM) {
				if(i > BRIGHTNESS_MEDIUM)
					i--;
				else if(i < BRIGHTNESS_MEDIUM)
					i++;
				change_pwm_duty_cycle(i);
			}
		}
		/* 80 <= light_value < 120 */
		else if((light_value < 150) && (light_value >= 80)) {
			while(i != BRIGHTNESS_HIGH) {
				if(i > BRIGHTNESS_HIGH)
					i--;
				else if(i < BRIGHTNESS_HIGH)
					i++;
				change_pwm_duty_cycle(i);
			}
		}
		if(light_value >= 150) {
			while(i != BRIGHTNESS_MAX) {
				if(i > BRIGHTNESS_MAX)
					i--;
				else if(i < BRIGHTNESS_MAX)
					i++;
				change_pwm_duty_cycle(i);
			}
		}
#endif
	}
}
