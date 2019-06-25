/*
 * gpio.c
 *
 *  Created on: Jun 13, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "gpio.h"

#define		LED_PIN_1			5
#define		LED_PIN_2			6
#define		PLUS_BUTTON_PIN		7
#define		MINUS_BUTTON_PIN	8

/*   I N I T   */

/* Configure PA5 and PA6 as outputs to blink LEDs */
void init_led(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->ODR &= ~GPIO_ODR_5;	// init PA5 to LOW

	GPIOA->MODER |= 		( GPIO_MODER_MODER5_0	// set PA5 to output
						| GPIO_MODER_MODER6_0	// set PA6 to output
		);

	GPIOA->OTYPER &=   ~( GPIO_OTYPER_OT_5  // set PA5 to push-pull
						| GPIO_OTYPER_OT_6  // set PA6 to push-pull
		);

//	GPIOA->OSPEEDR |= 	( GPIO_OSPEEDR_OSPEEDR5_0  // set PA5 to mid-speed
//						| GPIO_OSPEEDR_OSPEEDR6_0  // set PA6 to midspeed
//		);

//	GPIOA->PUPDR &=    ~( GPIO_PUPDR_PUPDR5 // no pull-up/pull-down
//						| GPIO_PUPDR_PUPDR6	// no pull-up/pull-down
//		);

	/* set outputs on pins */
//	GPIOA->ODR |= GPIO_ODR_5;	// init PA5 to HIGH
//	GPIOA->ODR |= GPIO_ODR_5;	// init PA5 to HIGH
}

/* toggle the outputs on PA5 and PA6 */
void inline toggle_led(void) {
	GPIOA->ODR ^= GPIO_ODR_5;
//	GPIOA->ODR ^= GPIO_ODR_6;
}

/* configures buttons */
void init_buttons(void) {
	/* PA7 and PA8 */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		// make sure GPIOA is enabled
	GPIOA->MODER &=    ~( GPIO_MODER_MODER7	// set to input
						| GPIO_MODER_MODER9	// set to input
		);

	GPIOA->PUPDR |=     ( GPIO_PUPDR_PUPDR7_1	// pull-down
						| GPIO_PUPDR_PUPDR9_1	// pull-down
		);

	/* Configure PA7 ('+') button interrupt */
	SYSCFG->EXTICR[2] = SYSCFG_EXTICR2_EXTI7_PA;	// external interrupt on PA[7]
	EXTI->IMR |= EXTI_IMR_MR7; 		// select line 7 for PA7;
	EXTI->RTSR |= EXTI_RTSR_TR7;	// enable rising trigger
	EXTI->FTSR &= EXTI_FTSR_TR7; 	// disable falling trigger

	// clear pending interrupt flag
	EXTI->PR &= ~(EXTI_PR_PR7);

	/* Configure PA9 ('-') button interrupt */
	SYSCFG->EXTICR[3] = SYSCFG_EXTICR3_EXTI9_PA;	// external interrupt on PA[7]
	EXTI->IMR |= EXTI_IMR_MR9; 		// select line 7 for PA9;
	EXTI->RTSR |= EXTI_RTSR_TR9;	// enable rising trigger
	EXTI->FTSR &= EXTI_FTSR_TR9; 	// disable falling trigger

	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 0);

}

/*   R E A D S   */

/* returns current '+' button value */
uint8_t inline read_plus_button(void) {
	return (GPIOA->IDR & GPIO_IDR_7);
}

/* returns current '-' button value */
uint8_t inline read_minus_button(void) {
	return (GPIOA->IDR & GPIO_IDR_9);
}

/*   T A S K S   */

/* Toggles the PA5 and PA6 LEDs */
void prvBlink_LED(void *pvParameters) {
	const TickType_t delay_time = pdMS_TO_TICKS(500); // 0.5s period
	for( ;; ) {
		toggle_led();
//		int i;
//		for(i=0; i < 200000; i++);
		vTaskDelay(delay_time);	// 0.5s

	}
}

/*	I N T E R R U P T S   */
void EXTI4_15_IRQHandler(void) {
	if(EXTI->PR & EXTI_PR_PR7) {
		EXTI->PR |= EXTI_PR_PR7;
	}
	if(EXTI->PR & EXTI_PR_PR9) {
		EXTI->PR |= EXTI_PR_PR9;
		GPIOA->ODR ^= GPIO_ODR_6;
	}
}


