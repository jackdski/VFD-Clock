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
#include "vfd_typedefs.h"

/*	G L O B A L   V A R I A B L E S   */
extern System_State_E system_state;

extern uint8_t hours;
extern uint8_t minutes;
extern uint8_t seconds;
extern uint8_t temperature;


//#define		DEMO

/*	L E D   D E F I N E S   */
#define		ERROR_LED			0	// PA0
#define		RTC_LED				1	// PA1
#define		HEARTBEAT_LED		5	// PA5
//#define		LED_GEN_PURP		6	// PA6

/*	B U T T O N  &  S W I T C H E S   D E F I N E S   */
#define		ON_OFF_SWITCH		13	// PC13
#define		CONFIG_SWITCH		2	// PC2

#ifdef		DEMO
#define		PLUS_BUTTON_PIN		7	// PA7
#define		MINUS_BUTTON_PIN	8	// PA8
#else
#define		PLUS_BUTTON_PIN		0	// PC0
#define		MINUS_BUTTON_PIN	1	// PC1
#endif


/* ERROR LED
 * RTC LED
 * Heartbeat LED
 * General Purpose LED
 */
void init_led(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->ODR &= ~(GPIO_ODR_0 | GPIO_ODR_1 | GPIO_ODR_5 | GPIO_ODR_6);	// init LOW

	/* set to output */
	GPIOA->MODER |= 	( GPIO_MODER_MODER0_0
						| GPIO_MODER_MODER1_0
						| GPIO_MODER_MODER5_0
//						| GPIO_MODER_MODER6_0
		);

	/* set to push-pull */
	GPIOA->OTYPER &=   ~( GPIO_OTYPER_OT_0
						| GPIO_OTYPER_OT_1
						| GPIO_OTYPER_OT_5
//						| GPIO_OTYPER_OT_6
		);
}

/* toggle the outputs on PA5 and PA6 */
void inline toggle_led(void) {
	GPIOA->ODR ^= GPIO_ODR_5;
}

void inline toggle_error_led(void) {
	GPIOA->ODR ^= GPIO_ODR_0;
}

void inline toggle_rtc_led(void) {
	GPIOA->ODR ^= GPIO_ODR_1;
}

/* configures buttons */
void init_buttons(void) {
#ifdef DEMO
	/* make sure GPIOA is enabled */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		// make sure GPIOA is enabled

	/* set to input */
	GPIOA->MODER &=    ~( GPIO_MODER_MODER13
						| GPIO_MODER_MODER9
						| GPIO_MODER_MODER7
						| GPIO_MODER_MODER2
		);

	/* configure to pull-down */
	GPIOA->PUPDR |=     ( GPIO_PUPDR_PUPDR13_1
						| GPIO_PUPDR_PUPDR9_1
						| GPIO_PUPDR_PUPDR7_1
						| GPIO_PUPDR_PUPDR2_1
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

	/* enable interrupts on EXTI Lines 4-15*/
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 1);

#else
//#define		PLUS_BUTTON_PIN		0	// PC0
//#define		MINUS_BUTTON_PIN	1	// PC1
//#define		CONFIG_SWITCH		2	// PC2
//#define		ON_OFF_SWITCH		13 	// PC13

	/* make sure GPIOC is enabled */
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	/* set to input */
	GPIOC->MODER &=    ~( GPIO_MODER_MODER0
						| GPIO_MODER_MODER1
						| GPIO_MODER_MODER2
						| GPIO_MODER_MODER13
		);

	/* configure to pull-down */
	GPIOC->PUPDR |=     ( GPIO_PUPDR_PUPDR0_1
						| GPIO_PUPDR_PUPDR1_1
						| GPIO_PUPDR_PUPDR2_1
						| GPIO_PUPDR_PUPDR13_1
		);

	/* Configure PC0 ('+') button interrupt */
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PC;	// external interrupt on PC[0]
	EXTI->IMR |= EXTI_IMR_MR0; 		// select line 0 for PC0;
	EXTI->RTSR |= EXTI_RTSR_TR0;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR0; 	// disable falling trigger

	// clear pending interrupt flag
	EXTI->PR &= ~(EXTI_PR_PR0);

	/* Configure PC1 ('-') button interrupt */
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PC;	// external interrupt on PC[1]
	EXTI->IMR |= EXTI_IMR_MR1; 		// select line 1 for PC1;
	EXTI->RTSR |= EXTI_RTSR_TR1;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR1; 	// disable falling trigger

	// clear pending interrupt flag
	EXTI->PR &= ~(EXTI_PR_PR1);
	/* enable interrupts on EXTI Lines 0 & 1 */
	NVIC_EnableIRQ(EXTI0_1_IRQn);
	NVIC_SetPriority(EXTI0_1_IRQn, 1);
#endif

	/* Configure PC2 (Configure) switch interrupt */
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PC;	// external interrupt on PC[2]
	EXTI->IMR |= EXTI_IMR_MR2; 		// select line 2 for PC2;
	EXTI->RTSR |= EXTI_RTSR_TR2;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR2; 	// disable falling trigger

	/* Configure PC13 (On/Off) switch interrupt */
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;	// external interrupt on PC[13]
	EXTI->IMR |= EXTI_IMR_MR13; 	// select line 13 for PC13;
	EXTI->RTSR |= EXTI_RTSR_TR13;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR13; 	// disable falling trigger

	/* enable interrupts on EXTI Lines 2 & 3 */
	NVIC_EnableIRQ(EXTI2_3_IRQn);
	NVIC_SetPriority(EXTI2_3_IRQn, 1);
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
		vTaskDelay(delay_time);	// 0.5s

	}
}

/*	I N T E R R U P T S   */

#ifndef	DEMO
/* '+' button -> PC0
 * '-' button -> PC1
 */
void EXTI0_1_IRQHandler(void) {
	/* '+' Button */
	if(EXTI->PR & EXTI_PR_PR0) {
		EXTI->PR |= EXTI_PR_PR0;
		toggle_error_led();
	}
	/* '-' Button */
	if(EXTI->PR & EXTI_PR_PR1) {
		EXTI->PR |= EXTI_PR_PR1;
		toggle_error_led();
	}
	EXTI->PR |= (EXTI_PR_PR0 | EXTI_PR_PR1);
}
#endif

/* Config Switch -> PC2 */
void EXTI2_3_IRQHandler(void) {
	/* Config Switch */
	if(EXTI->PR & EXTI_PR_PR2) {
		// if Input is high, set to Switch_Config state and falling edge interrupt
		if(GPIOC->IDR & GPIO_IDR_2) {
			toggle_error_led();
			system_state = Switch_Config;	// TODO: use a function to select state
			EXTI->RTSR &= ~EXTI_RTSR_TR2;	// disable rising trigger
			EXTI->FTSR |= EXTI_FTSR_TR2; 	// enable falling trigger
		}
		else if(GPIOC->IDR & ~GPIO_IDR_2) {
			toggle_error_led();
			system_state = Clock;  			// TODO: use a function to select state
			EXTI->RTSR |= EXTI_RTSR_TR2;	// enable rising trigger
			EXTI->FTSR &= ~EXTI_FTSR_TR2; 	// disable falling trigger
		}
		EXTI->PR |= EXTI_PR_PR2;

	}
	EXTI->PR |= (EXTI_PR_PR2);
}

void EXTI4_15_IRQHandler(void) {
#ifdef DEMO
	/* '+' Button */
	if(EXTI->PR & EXTI_PR_PR7) {
		EXTI->PR |= EXTI_PR_PR7;
		/* increment if minutes < 60 and hours < 24 */
		if(minutes == 59) {
			minutes = 0;
			if(hours < 24)
				hours += 1;
		}
		else if(system_state == Switch_Config) {
			if(minutes < 60)
				minutes += 1;
		}
		toggle_error_led();
	}
	/* '-' Button */
	if(EXTI->PR & EXTI_PR_PR9) {
		EXTI->PR |= EXTI_PR_PR9;
		if(system_state == Switch_Config) {
			if(minutes == 0) {
				minutes = 59;
				if(hours > 0)
					hours -=1;
			}
			else if(minutes > 0)
				minutes -= 1;
		}
		else
			toggle_error_led();
	}
#endif
	/* on/off switch */
//	if(EXTI->PR & EXTI_PR_PR13) {
//		EXTI->PR |= EXTI_PR_PR13;
//		toggle_error_led();
//		system_state = Deep_Sleep;
//	}
}


