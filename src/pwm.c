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

/*	D E F I N E S   */

/* PWM */
#define	PWM_FREQUENCY	400		// 400us or 2.5kHz
//#define	PWM_FREQUENCY	1000		// 1000us or 1kHz

#define BRIGHTNESS_LOW		10
#define BRIGHTNESS_MEDIUM	45
#define BRIGHTNESS_HIGH		70
#define BRIGHTNESS_MAX		99

/* DIMMING */
#define DIMMING_ONE_SEC		10000	// 10k * 10kHz = 1s
#define DIMMING_50MS		500

//#define DIMMING_DEMO
#define BRIGHTNESS_DEMO

/*	G L O B A L   V A R I A B L E S   */
extern uint16_t display_brightness;
extern uint32_t light_value;

/* L O C A L   V A R I A B L E S   */
static int8_t dimming_step;
static uint32_t target_brightness;

/*	P W M   */
void init_pwm(void) {
	/* Info:
	 * 	freq = 2.5KHz (period is 400us)
	 * 	period = 25% - 100%
	 */

	/* config GPIO - PA4 to AF4 */
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
	uint16_t duty = ((uint16_t)PWM_FREQUENCY * duty_cycle) / 100;
	TIM14->CCR1 = duty;
}

uint16_t inline get_pwm_duty_cycle(void) {
	return (TIM14->CCR1 * 100) / (uint16_t)PWM_FREQUENCY;
}

uint8_t calc_new_brightness(uint32_t light_sample) {
	/* switch according to the value the ADC is reporting from the photoresistor circuit
	 *
	 * use dead bands to avoid switching back and forth often
	 */
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

/*	D I M M I N G   */
void init_dimming_timer(void) {
	/* config TIM14 registers */
	TIM2->PSC = 799; 	// 8MHz / (799+1) = 10kHz
	TIM2->ARR = (uint16_t)DIMMING_ONE_SEC; // 1 second
	TIM2->CCR1 = (uint16_t)TIM_SR_CC1OF;	// init to 0


	TIM2->CCER &= ~(TIM_CCER_CC1NP);	// OC1N active high

	/* Enable output (MOE = 1)*/
	TIM2->BDTR |= TIM_BDTR_MOE;

	TIM2->CR1 |= TIM_CR1_DIR; //| TIM_CR1_CEN;	// enable downcounter and counter

	TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S); 	// CC1 channel configured as output
}

void trigger_dimming_timer(uint32_t old_brightness, uint32_t new_brightness) {
	/* select dimming step direction */
	if(old_brightness > new_brightness)
		dimming_step = -1;
	if(new_brightness > old_brightness)
		dimming_step = 1;

	target_brightness = new_brightness;

	TIM2->ARR = (uint16_t)DIMMING_50MS;	// 1 second
	TIM2->CCR1 = (uint16_t)DIMMING_50MS;	// init to 0

	TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M);
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE;

	TIM2->DIER |= TIM_DIER_UIE | TIM_DIER_CC1IE;	// enable CC1 interrupt

	/* update generation */
	TIM2->EGR |= TIM_EGR_UG;
	TIM2->CR1 |= TIM_CR1_CEN;	// enable counter

	/* enable TIM2 interrupts */
	NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler() {
	if(TIM2->SR & TIM_SR_UIF) {
		toggle_error_led();
		// update current display brightness
		display_brightness = (TIM14->CCR1 * 100) / (uint16_t)PWM_FREQUENCY;

		// if display brightness if already target_brightness, disable timer
		if(display_brightness == target_brightness) {
			TIM2->CR1 &= ~TIM_CR1_CEN;		// disable timer/counter
			TIM2->DIER &= ~(TIM_DIER_UIE | TIM_DIER_CC1IE);  // disable interrupt
			TIM2->EGR &= TIM_EGR_UG;		// disable update generation
			NVIC_DisableIRQ(TIM2_IRQn);
		}

		// update PWM to affect brightness
		if(display_brightness != target_brightness) {
			display_brightness += dimming_step;
			change_pwm_duty_cycle(display_brightness);
		}
		TIM2->SR &= ~TIM_SR_UIF;
	}
}

/*	T A S K S   */

/* updates the value in the timer's */
void prvChangePWM(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 100 ); // 100ms
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
		else if(light_value >= 150) {
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
