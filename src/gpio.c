/*
 * gpio.c
 *
 *  Created on: Jun 13, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "gpio.h"
#include "vfd_typedefs.h"
#include "sensor_tasks.h"
#include "tubes.h"
#include "callbacks.h"

/*	T A S K   H A N D L E S   */
extern TaskHandle_t thRTC;
extern TaskHandle_t thOff;

/*	G L O B A L   V A R I A B L E S   */
extern System_State_E system_state;

extern uint8_t hours;
extern uint8_t minutes;
extern uint8_t seconds;
extern uint8_t ampm;
extern int8_t temperature;	/* -128 - 127 */

extern TimerHandle_t three_sec_timer;
extern TimerHandle_t five_sec_timer;
extern TimerHandle_t ten_sec_timer;
extern TimerHandle_t button_timer;

extern Button_Status_E plus_button_status;
extern Button_Status_E minus_button_status;
extern Light_Flash_E indication_light_status;
extern Time_Change_Speed_E change_speed;

extern uint8_t holds;

#define		NO_TSC
//#define		EFUSE_CURRENT_SENSE

/*	L E D   D E F I N E S   */
#define		ERROR_LED			0	// PA0
#define		RTC_LED				1	// PA1
#define		INDICATOR_LED		5	// PA5
//#define		LED_GEN_PURP		6	// PA6

/*	B U T T O N  &  S W I T C H E S   D E F I N E S   */
 #define		ON_OFF_SWITCH		13	// PC13 - WKUP2
//#define		CONFIG_SWITCH		2	// PC2

#ifdef		DEMO
#define		PLUS_BUTTON_PIN		7	// PA7
#define		MINUS_BUTTON_PIN	8	// PA8
#else
#define		PLUS_BUTTON_PIN		0	// PC0
#define		MINUS_BUTTON_PIN	1	// PC1
#endif

#ifdef		NO_TSC
#define		TEMPERATURE_BUTTON	3	// PB3
#define		DATE_BUTTON			4	// PB4
#endif

/* E F U S E   D E F I N E S   */
#define 	EFUSE_FAULT			6	// PB6 - input.
#define		EFUSE_EN			7	// PB7 - output


/*	I N I T S   */

/* ERROR LED
 * RTC LED
 * Indicator LED
 * General Purpose LED
 */
void init_led(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->ODR &= ~(GPIO_ODR_0 | GPIO_ODR_1 | GPIO_ODR_5 | GPIO_ODR_6);	// init LOW

	/* set to output */
	GPIOA->MODER |= 	( GPIO_MODER_MODER0_0
						| GPIO_MODER_MODER1_0
						| GPIO_MODER_MODER5_0
					/*	| GPIO_MODER_MODER6_0 */ );

	/* set to push-pull */
	GPIOA->OTYPER &=   ~( GPIO_OTYPER_OT_0
						| GPIO_OTYPER_OT_1
						| GPIO_OTYPER_OT_5
						/* | GPIO_OTYPER_OT_6 */ );
}

/* configures buttons */
void init_buttons(void) {
//  PLUS_BUTTON_PIN		0	// PC0
//	MINUS_BUTTON_PIN	1	// PC1
//	CONFIG_SWITCH		2	// PC2
//	ON_OFF_SWITCH		13 	// PC13

	/* make sure GPIOC is enabled */
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	/* set to input */
	GPIOC->MODER &=    ~( GPIO_MODER_MODER0
						| GPIO_MODER_MODER1
						| GPIO_MODER_MODER2
						| GPIO_MODER_MODER13);

	/* configure to pull-down */
	GPIOC->PUPDR |=     ( GPIO_PUPDR_PUPDR0_1
						| GPIO_PUPDR_PUPDR1_1
						| GPIO_PUPDR_PUPDR2_1
						| GPIO_PUPDR_PUPDR13_1);

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
//	EXTI->PR &= ~(EXTI_PR_PR1);
	/* enable interrupts on EXTI Lines 0 & 1 */
	NVIC_EnableIRQ(EXTI0_1_IRQn);
	NVIC_SetPriority(EXTI0_1_IRQn, 1);

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

#ifdef NO_TSC
	/* set to input */
	GPIOB->MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER4);

	/* configure to pull-down */
	GPIOB->PUPDR |= ( GPIO_PUPDR_PUPDR3_1 | GPIO_PUPDR_PUPDR4_1);

	/* Configure PB3 TEMPERATURE button interrupt */
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB;	// external interrupt on PB[3]
	EXTI->IMR |= EXTI_IMR_MR3; 		// select line 3 for PB3;
	EXTI->RTSR |= EXTI_RTSR_TR3;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR3; 	// disable falling trigger

	/* Configure PB4 DATE button interrupt */
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PB;	// external interrupt on PB[4]
	EXTI->IMR |= EXTI_IMR_MR4; 		// select line 4 for PB4;
	EXTI->RTSR |= EXTI_RTSR_TR4;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR4; 	// disable falling trigger

	/* enable interrupts on EXTI Lines 2 & 3 */
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 1);
#endif
}

void init_efuse_pins(void) {
	// EFUSE_FAULT			6	// PB6 - input.
	// EFUSE_EN				7	// PB7 - output, active low, ~EN/OVLO

	GPIOB->MODER &=    ~(GPIO_MODER_MODER6);	// PB6 to input
	GPIOB->MODER |=     (GPIO_MODER_MODER7);	// PB7 to output

	/* set output to low/enable */
	GPIOB->ODR &= ~(GPIO_ODR_7);

	GPIOB->PUPDR &=    ~(GPIO_PUPDR_PUPDR6_0);	// set to pull-up
	GPIOB->OTYPER &=   ~(GPIO_OTYPER_OT_7);		// set to push-pull

#ifdef EFUSE_CURRENT_SENSE
	// set ADC channel to read voltage on ILM current limit resistor
	/* select alternate function mode */

	// DO NOT USE THIS PIN, copied from adc.c
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER7)) | GPIO_MODER_MODER7_1;
	// select AF0 on PA7
	GPIOA->AFR[0] &=  ~(GPIO_AFRL_AFRL7); // AFRL (Ports 0-7)
#endif

}

/*	O U T P U T S   */

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


/*   R E A D S   */

/* returns current '+' button value */
uint8_t inline read_plus_button(void) {
	return (GPIOA->IDR & GPIO_IDR_7);
}

/* returns current '-' button value */
uint8_t inline read_minus_button(void) {
	return (GPIOA->IDR & GPIO_IDR_9);
}

/* reads current on/off switch value */
uint8_t inline read_power_switch(void) {
	return (GPIOC->IDR & GPIO_IDR_13);
}

/*   T A S K S   */

/* Toggles the PA5 and PA6 LEDs */
void prvBlink_LED(void *pvParameters) {
	const TickType_t delay_time = pdMS_TO_TICKS(100); // 0.1s period
	const TickType_t delay_time_long = pdMS_TO_TICKS(500); // 0.5s period
	static uint8_t response_counter = 0;
	for( ;; ) {
		if(indication_light_status == Flashing) {
			toggle_led();
			response_counter += 1;
			if(response_counter == 6) {
				response_counter = 0;
				indication_light_status = Off;
			}
			vTaskDelay(delay_time);			// 0.25s
		}
		else {
			toggle_led();
			vTaskDelay(delay_time_long);	// 0.5s
		}
	}
}


/*	G E N E R A L   */
void increase_minutes(uint8_t mins) {
	minutes += mins;
	if(minutes >= 60) {
		minutes -= 60;
		if(hours == 12) {
			hours = 1;
			ampm ^= 1;	// change am/pm
		}
		else
			hours += 1;
	}
}

void decrease_minutes(uint8_t mins) {
	minutes -= mins;
	if(minutes < 0) {
		minutes += 60;
		if(hours == 1) {
			hours = 12;
			ampm ^= 1;	// change am/pm
		}
		else
			hours -= 1;
	}
}

void increase_hours(void) {
	if(hours == 12) {
		hours = 1;
		ampm ^= 1;	// change am/pm
	}
	else
		hours += 1;
}

void decrease_hours(void) {
	if(hours == 1) {
		hours = 12;
		ampm ^= 1;	// change am/pm
	}
	else
		hours -= 1;
}


/*	I N T E R R U P T S   */

/* '+' button -> PC0
 * '-' button -> PC1
 */
void EXTI0_1_IRQHandler(void) {
	/* '+' Button */
	if(EXTI->PR & EXTI_PR_PR0) {
		if(GPIOC->IDR & GPIO_IDR_0) {
			plus_button_status = Pressed;
			EXTI->RTSR &= ~EXTI_RTSR_TR0;	// disable rising trigger
			EXTI->FTSR |= EXTI_FTSR_TR0; 	// enable falling trigger
			if(system_state == Config) {
				/* disable 10s timer, increase minutes place */
				/*+1 for every <3s hold, +5 for every >3s & <8s hold, +1hr every >8s hold */
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xTimerStop(ten_sec_timer, pdMS_TO_TICKS(500));	// stop timer, waiting max 500ms to execute
				holds = 0;
				change_speed = Slow;
				xTimerStartFromISR(button_timer, &xHigherPriorityTaskWoken);
			}
		}
		else if(!(GPIOC->IDR & GPIO_IDR_0)) {
			plus_button_status = Open;
			EXTI->FTSR &= ~EXTI_FTSR_TR0; 	// disable falling trigger
			EXTI->RTSR |= EXTI_RTSR_TR0;	// enable rising trigger
			if(system_state == Config) {
				// start/restart 10s timer
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xTimerStartFromISR(ten_sec_timer, &xHigherPriorityTaskWoken);
			}
		}
		EXTI->PR |= EXTI_PR_PR0;		// clear flag
		toggle_error_led();
	}

	/* '-' Button */
	if(EXTI->PR & EXTI_PR_PR1) {
		if(GPIOC->IDR & GPIO_IDR_1) {
			minus_button_status = Pressed;
			EXTI->RTSR &= ~EXTI_RTSR_TR1;	// disable rising trigger
			EXTI->FTSR |= EXTI_FTSR_TR1; 	// enable falling trigger
		}
		else if(!(GPIOC->IDR & GPIO_IDR_1)) {
			minus_button_status = Open;
			EXTI->FTSR &= ~EXTI_FTSR_TR1; 	// disable falling trigger
			EXTI->RTSR |= EXTI_RTSR_TR1;	// enable rising trigger
		}
		EXTI->PR |= EXTI_PR_PR1;
		toggle_error_led();
	}

	/* if both buttons are pressed start timer/counter */
	if((plus_button_status == Pressed) && (minus_button_status == Pressed)) {
		if(system_state != Config) {
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			xTimerStartFromISR(three_sec_timer, &xHigherPriorityTaskWoken);
		}
		else if(system_state == Config) {
			system_state = Clock;
			indication_light_status = Flashing;
		}
	}

	EXTI->PR |= (EXTI_PR_PR0 | EXTI_PR_PR1);
}

void EXTI2_3_IRQHandler(void) {
#ifdef	CONFIG_SWITCH
	/* Config Switch -> PC2 */
	if(EXTI->PR & EXTI_PR_PR2) {
		// if Input is high, set to Config state and falling edge interrupt
		if(GPIOC->IDR & GPIO_IDR_2) {
			toggle_error_led();
			system_state = Config;	// hardware takes priority over software in this state change
			EXTI->RTSR &= ~EXTI_RTSR_TR2;	// disable rising trigger
			EXTI->FTSR |= EXTI_FTSR_TR2; 	// enable falling trigger
		}
		else if(GPIOC->IDR & ~GPIO_IDR_2) {
			toggle_error_led();
			system_state = Clock;  			// hardware takes priority over software in thise state change
			EXTI->RTSR |= EXTI_RTSR_TR2;	// enable rising trigger
			EXTI->FTSR &= ~EXTI_FTSR_TR2; 	// disable falling trigger
		}
		EXTI->PR |= EXTI_PR_PR2;
	}
#endif
	/* Temperature button was pressed */
	if(EXTI->PR & EXTI_PR_PR3) {
		if(GPIOB->IDR & GPIO_IDR_2) {
			/* use 5s timer to display the temperature, suspend the RTC update display task */
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			vTaskSuspend(thRTC);
			display_temperature(temperature);	// do this in a task
			system_state = Button_Temperature;
			xTimerStartFromISR( five_sec_timer, &xHigherPriorityTaskWoken );
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}
		EXTI->PR |= EXTI_PR_PR3;
	}
	EXTI->PR |= (EXTI_PR_PR2 | EXTI_PR_PR3);
}

void EXTI4_15_IRQHandler(void) {
	/* Date button was pressed */
	if(EXTI->PR & EXTI_PR_PR4) {
		if(GPIOB->IDR & GPIO_IDR_4) {
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			/* suspend the RTC update display task, create 5s time to display the temperature */
			vTaskSuspend(thRTC);
			display_date();
			system_state = Button_Date;
			xTimerStartFromISR( five_sec_timer, &xHigherPriorityTaskWoken );
		}
		EXTI->PR |= EXTI_PR_PR4;
	}

	/* on/off switch */
	if(EXTI->PR & EXTI_PR_PR13) {
		/* on/off switch needs to be active high */
		if(GPIOC->IDR & GPIO_IDR_13) {
			EXTI->PR |= EXTI_PR_PR13;		// clear interrupt
			EXTI->RTSR |= EXTI_RTSR_TR13;	// enable rising trigger
			EXTI->FTSR &= ~EXTI_FTSR_TR13; 	// disable falling trigger
			system_state = Clock;
		}
		if(!(GPIOC->IDR & GPIO_IDR_13)) {
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			EXTI->RTSR &= ~EXTI_RTSR_TR13;	// disable rising trigger
			EXTI->FTSR |= EXTI_FTSR_TR13; 	// enable falling trigger
			configASSERT(thOff != NULL);
			vTaskNotifyGiveFromISR(thOff, &xHigherPriorityTaskWoken );
			system_state = Deep_Sleep;
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}
	}
}


