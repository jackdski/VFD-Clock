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
#include "main.h"
#include "gpio.h"
#include "vfd_typedefs.h"
#include "sensor_tasks.h"
#include "tubes.h"
#include "callbacks.h"

/*	T A S K   H A N D L E S   */
extern TaskHandle_t thRTC;
extern TaskHandle_t thOff;
extern TaskHandle_t thBLErx;
extern TaskHandle_t thBLEtx;
extern TaskHandle_t thTemperatureButton;

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
extern Light_Flash_E error_light_status;
extern Time_Change_Speed_E change_speed;
extern HC_10_Status_E ble_status;
extern Efuse_Status_E efuse_status;

extern uint8_t holds;

//#define		EFUSE_CURRENT_SENSE

//#define 	DEMO

/*	L E D   D E F I N E S   */
#ifdef		DEMO
#define		ERROR_LED			0	// PA0
#define		INDICATION_LED		5	// PA5
#else
#define		ERROR_LED			9	// PC9
#define		INDICATON_LED		8	// PC8
#endif


/*	B U T T O N  &  S W I T C H E S   D E F I N E S   */
#ifdef		DEMO
#define		ON_OFF_SWITCH		13	// PC13 - WKUP2
#define		PLUS_BUTTON_PIN		0	// PC0
#define		MINUS_BUTTON_PIN	1	// PC1
#define		TEMPERATURE_BUTTON	3	// PB3
#define		DATE_BUTTON			4	// PB4
#else
#define		ON_OFF_SWITCH		13	// PC13 - WKUP2
#define		PLUS_BUTTON_PIN		7	// PB7
#define		MINUS_BUTTON_PIN	8	// PB8
#define		TEMPERATURE_BUTTON	3	// PC3
#define		DATE_BUTTON			12	// PB12
#define		HC10_STATUS			8	// PA8 (EXTI-9)
#endif


/* E F U S E   D E F I N E S   */
#define 	EFUSE_FAULT			6	// PB6 - input.
#define		EFUSE_EN			7	// PB7 - output


/*	I N I T S   */

/* ERROR LED
 * 	Demo - PA0
 * 	Else - PC9
 * General Purpose LED
 */
void init_error_led(void) {
#ifdef DEMO
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;			/* enable peripheral clock */
	GPIOA->ODR &= ~(GPIO_ODR_0);				/* init LOW */
	GPIOA->MODER |= GPIO_MODER_MODER0_0;		/* set to output */
	GPIOA->OTYPER &=   ~(GPIO_OTYPER_OT_0);  	/* set to push-pull */
#else
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;			/* enable peripheral clock */
	GPIOC->ODR &= ~(GPIO_ODR_9);				/* init LOW */
	GPIOC->MODER |= GPIO_MODER_MODER9_0;		/* set to output */
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_9);		/* set to push-pull */
#endif
}

/* Indicator LED
 * 	Demo - PA5
 * 	Else - PC8
 */
void init_indication_led(void) {
#ifdef DEMO
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;			/* enable peripheral clock */
	GPIOA->ODR &= ~(GPIO_ODR_5);  				/* init LOW */
	GPIOA->MODER |= (GPIO_MODER_MODER5_0);		/* set to output */
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_5);		/* set to push-pull */
#else
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;			/* enable peripheral clock */
	GPIOC->ODR &= ~(GPIO_ODR_8);				/* init LOW */
	GPIOC->MODER |= GPIO_MODER_MODER8_0;		/* set to output */
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_8);		/* set to push-pull */
#endif
}

void init_power_switch(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER &= ~( GPIO_MODER_MODER13);
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_0;
}

/* configures buttons */
void init_buttons(void) {
#ifdef	DEMO
//  PLUS_BUTTON_PIN		0	// PC0
//	MINUS_BUTTON_PIN	1	// PC1
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
						| GPIO_PUPDR_PUPDR2_1);

	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_0;

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
//	EXTI->RTSR |= EXTI_RTSR_TR13;	// enable rising trigger
//	EXTI->FTSR &= ~EXTI_FTSR_TR13; 	// disable falling trigger
	EXTI->RTSR &= ~EXTI_RTSR_TR13;	// disable rising trigger
	EXTI->FTSR |= EXTI_FTSR_TR13; 	// enable falling trigger

	/* enable interrupts on EXTI Lines 2 & 3 */
	NVIC_EnableIRQ(EXTI2_3_IRQn);
	NVIC_SetPriority(EXTI2_3_IRQn, 1);

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

	/* enable interrupts on EXTI Lines */
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 1);

#else
	// 	PLUS_BUTTON_PIN		7	// PB7
	// 	MINUS_BUTTON_PIN	8	// PB8
	// 	TEMPERATURE_BUTTON	3	// PC3
	// 	DATE_BUTTON			12	// PB12
	//	ON_OFF_SWITCH		13 	// PC13

	/* make sure GPIOC is enabled */
	RCC->AHBENR |= (RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN);

	/* set to input */
	GPIOB->MODER &= ~(GPIO_MODER_MODER7 | GPIO_MODER_MODER8 | GPIO_MODER_MODER12);
	GPIOC->MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER13);

	/* configure to pull-down */
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR7_1 | GPIO_PUPDR_PUPDR8_1 | GPIO_PUPDR_PUPDR12_1);
	GPIOC->PUPDR |= ( GPIO_PUPDR_PUPDR3_1);

	/* configure to pull-up */
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_0;

	/* Configure PB7 ('+') button interrupt */
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PB;	// external interrupt on PB70]
	EXTI->IMR |= EXTI_IMR_MR7; 		// select line 0 for PB7;
	EXTI->RTSR |= EXTI_RTSR_TR7;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR7; 	// disable falling trigger

	/* Configure PB8 ('-') button interrupt */
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI8_PB;	// external interrupt on PB[8]
	EXTI->IMR |= EXTI_IMR_MR8; 		// select line 1 for PB8;
	EXTI->RTSR |= EXTI_RTSR_TR8;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR8; 	// disable falling trigger

	/* Configure PC3 (Temperature) Button interrupt */
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PC;	// external interrupt on PC[3]
	EXTI->IMR |= EXTI_IMR_MR3; 	// select line 3 for PC3;
	EXTI->RTSR |= EXTI_RTSR_TR3;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR3; 	// disable falling trigger

	/* Configure PB12 (Date) button interrupt */
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI12_PB;	// external interrupt on PB[12]
	EXTI->IMR |= EXTI_IMR_MR12; 		// select line 12 for PB12;
	EXTI->RTSR |= EXTI_RTSR_TR12;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR12; 	// disable falling trigger

	/* Configure PC13 (On/Off) switch interrupt */
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;	// external interrupt on PC[13]
	EXTI->IMR |= EXTI_IMR_MR13; 	// select line 13 for PC13;
	EXTI->RTSR &= ~EXTI_RTSR_TR13;	// disable rising trigger
	EXTI->FTSR |= EXTI_FTSR_TR13; 	// enable falling trigger


	/* enable interrupts on EXTI Lines 2 & 3 */
	NVIC_EnableIRQ(EXTI2_3_IRQn);
	NVIC_SetPriority(EXTI2_3_IRQn, 1);

	/* enable interrupts on EXTI Lines */
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 1);
#endif
}

void init_efuse_pins(void) {
	// EFUSE_nFAULT		PC5 - input.
	// EFUSE_EN			PC4 - output, EN/OVLO

	EFUSE_EN_PORT->MODER &= ~(GPIO_MODER_MODER4);			// output
	EFUSE_NFLT_PORT->MODER |= (GPIO_MODER_MODER5);			// input

	/* set output to low/enable */
	EFUSE_EN_PORT->ODR &= ~(GPIO_ODR_7);

	EFUSE_EN_PORT->PUPDR &=    ~(GPIO_PUPDR_PUPDR6_0);	// set to pull-up
	EFUSE_EN_PORT->OTYPER &=   ~(GPIO_OTYPER_OT_7);		// set to push-pull

#ifdef EFUSE_CURRENT_SENSE
	// set ADC channel to read voltage on ILM current limit resistor
	/* select alternate function mode */

	// DO NOT USE THIS PIN, copied from adc.c
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER7)) | GPIO_MODER_MODER7_1;
	// select AF0 on PA7
	GPIOA->AFR[0] &=  ~(GPIO_AFRL_AFRL7); // AFRL (Ports 0-7)
#endif

	/* Configure PC5 EFUSE Fault interrupt */
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PC;	// external interrupt on PB[5]
	EXTI->IMR |= EXTI_IMR_MR5; 		// select line 5 for PC5;
	EXTI->RTSR |= EXTI_RTSR_TR5;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR5; 	// disable falling trigger
}

/*	L O W - P O W E R   */
void configure_gpio_for_low_power(void) {
	GPIOA->MODER = 0x2800000;
	GPIOB->MODER = 0x00000000;
	GPIOC->MODER = 0x00000000;
	GPIOD->MODER = 0x00000000;
	GPIOE->MODER = 0x00000000;
	GPIOF->MODER = 0x00000000;

	GPIOA->OTYPER = 0x00000000;
	GPIOB->OTYPER = 0x00000000;
	GPIOC->OTYPER = 0x00000000;
	GPIOD->OTYPER = 0x00000000;
	GPIOE->OTYPER = 0x00000000;
	GPIOF->OTYPER = 0x00000000;
}

/*	O U T P U T S   */

/* toggle the output on the INDICATION LED */
void inline toggle_indication_led(void) {
#ifdef DEMO
	GPIOA->ODR ^= GPIO_ODR_5;
#else
	GPIOC->ODR ^= GPIO_ODR_8;
#endif
}

/* toggle the outputs on the ERROR LED */
void inline toggle_error_led(void) {
#ifdef DEMO
	GPIOA->ODR ^= GPIO_ODR_0;
#else
	GPIOC->ODR ^= GPIO_ODR_9;
#endif
}

void inline clear_error_led(void) {
#ifdef DEMO
	GPIOA->ODR &= ~GPIO_ODR_0;
#else
	GPIOC->ODR &= ~GPIO_ODR_9;
#endif
}

//void inline toggle_rtc_led(void) {
//	GPIOA->ODR ^= GPIO_ODR_1;
//}


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

/* 	Low = Unconnected output
* 	High = Connected output
* 	Suspends or resumes TX task based on ble_status */
void get_hc_10_status(void) {
	if(GPIOA->IDR & GPIO_IDR_8) {	// connected
		ble_status = Connected;
		vTaskResume(thBLEtx);
	}
	else {
		if(!(GPIOA->IDR & GPIO_IDR_8)) {
			ble_status = Disconnected;
			}
		else {
			ble_status = BLE_Error;
		}

		// suspend task if not already suspended
		if(eTaskGetState(thBLEtx) != eSuspended) {
			vTaskSuspend(thBLEtx);
		}
	}
}

/*   T A S K S   */

/* Toggles and flashes the Indication LED */
void prvIndication_LED(void *pvParameters) {
	const TickType_t delay_time = pdMS_TO_TICKS(100); // 0.1s period
	const TickType_t delay_time_long = pdMS_TO_TICKS(500); // 0.5s period
	static uint8_t response_counter = 0;
	for( ;; ) {
		if(indication_light_status == Flashing) {
			toggle_indication_led();
			response_counter += 1;
			if(response_counter == 6) {
				response_counter = 0;
				indication_light_status = Off;
			}
			vTaskDelay(delay_time);			// 0.25s
		}
		else {
			toggle_indication_led();
			vTaskDelay(delay_time_long);	// 0.5s
		}
	}
}

/* Flashes the Error LED according to error type */
void prvError_LED(void * pvParameters) {
	const TickType_t delay_time_ble = pdMS_TO_TICKS(3000); 	// 3s
	const TickType_t delay_time_efuse = pdMS_TO_TICKS(200);	// 0.2s

	for( ;; ) {
		toggle_error_led();
		vTaskDelay(delay_time_ble);
		if(error_light_status == Flashing) {
			if(efuse_status == Efuse_Error) {
				toggle_error_led();
				vTaskDelay(delay_time_efuse);
			}
			else if(ble_status == BLE_Error) {
				toggle_error_led();
				vTaskDelay(delay_time_ble);
			}
		}
		else {
			clear_error_led();
		}
	}
}

/*	G E N E R A L   */



/*	I N T E R R U P T S   */

/* '+' button -> PC0
 * '-' button -> PC1
 */
#ifdef DEMO
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
#endif

void EXTI2_3_IRQHandler(void) {
	//	TEMPERATURE_BUTTON	3	// PB3

	/* Temperature button was pressed */
	if(EXTI->PR & EXTI_PR_PR3) {
		if(GPIOB->IDR & GPIO_IDR_2) {
			/* use 5s timer to display the temperature, suspend the RTC update display task */
			vTaskSuspend(thRTC);
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			configASSERT(thTemperatureButton != NULL);
			vTaskNotifyGiveFromISR(thTemperatureButton, &xHigherPriorityTaskWoken );
			system_state = Button_Temperature;
			xTimerStartFromISR( five_sec_timer, &xHigherPriorityTaskWoken );
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}
		EXTI->PR |= EXTI_PR_PR3;
	}
	EXTI->PR |= (EXTI_PR_PR2 | EXTI_PR_PR3);
}

void EXTI4_15_IRQHandler(void) {
#ifdef DEMO
	//	ON_OFF_SWITCH		13	// PC13 - WKUP2
	//	PLUS_BUTTON_PIN		0	// PC0
	//	MINUS_BUTTON_PIN	1	// PC1
	//	DATE_BUTTON			4	// PB4
	//	HC10_STATUS			8	// PA8 (EXTI-9)
	// EFUSE_nFAULT		PC5 - input.
	// EFUSE_EN			PC4 - output, EN/OVLO


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

#else
	//	ON_OFF_SWITCH		13	// PC13 - WKUP2
	//	PLUS_BUTTON_PIN		7	// PB7
	// 	MINUS_BUTTON_PIN	8	// PB8
	//	DATE_BUTTON			12	// PB12
	//	HC10_STATUS			8	// PA8 (EXTI-9)
	// EFUSE_nFAULT			5	// PC5 - input

	if(EXTI->PR & EXTI_PR_PR5) {
		if(GPIOC->IDR & GPIO_IDR_5) {  // check if input is high
			// eFuse Error Routine
			efuse_status = Efuse_Error;				// prvError_LED will flash to indicate an error occured
			EFUSE_EN_PORT->ODR &= ~EFUSE_EN_PIN;	// disable eFuse
		}
	}

	/* '+' Button */
	if(EXTI->PR & EXTI_PR_PR7) {
		if(GPIOB->IDR & GPIO_IDR_7) {
			plus_button_status = Pressed;
			EXTI->RTSR &= ~EXTI_RTSR_TR0;	// disable rising trigger
			EXTI->FTSR |= EXTI_FTSR_TR0; 	// enable falling trigger
			if(system_state == Config) {
				/* +1 for every <3s hold,
				 * +5 for every >3s & <8s hold,
				 * +1hr every >8s hold */
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;

				/* disable 10s timer, increase minutes place */
				xTimerStop(ten_sec_timer, pdMS_TO_TICKS(500));	// stop timer, waiting max 500ms to execute
				holds = 0;
				change_speed = Slow;
				xTimerStartFromISR(button_timer, &xHigherPriorityTaskWoken);
			}
		}
		else if(!(GPIOB->IDR & GPIO_IDR_7)) {
			plus_button_status = Open;
			EXTI->FTSR &= ~EXTI_FTSR_TR0; 	// disable falling trigger
			EXTI->RTSR |= EXTI_RTSR_TR0;	// enable rising trigger
			if(system_state == Config) {
				// start/restart 10s timer
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xTimerStartFromISR(ten_sec_timer, &xHigherPriorityTaskWoken);
			}
		}
		EXTI->PR |= EXTI_PR_PR7;		// clear flag
		toggle_error_led();
	}

	/* '-' Button */
	if(EXTI->PR & EXTI_PR_PR8) {
		if(GPIOB->IDR & GPIO_IDR_8) {
			minus_button_status = Pressed;
			EXTI->RTSR &= ~EXTI_RTSR_TR1;	// disable rising trigger
			EXTI->FTSR |= EXTI_FTSR_TR1; 	// enable falling trigger
		}
		else if(!(GPIOB->IDR & GPIO_IDR_8)) {
			minus_button_status = Open;
			EXTI->FTSR &= ~EXTI_FTSR_TR1; 	// disable falling trigger
			EXTI->RTSR |= EXTI_RTSR_TR1;	// enable rising trigger
		}
		EXTI->PR |= EXTI_PR_PR8;
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

#endif

	/* EFUSE Fault Occurred */
	if(EXTI->PR & EXTI_PR_PR6) {
		efuse_status = Efuse_Error;
		error_light_status = Flashing;
		EXTI->PR |= EXTI_PR_PR6;
	}

	/* HC-10 Status change */
	if(EXTI->PR & EXTI_PR_PR9) {
		get_hc_10_status();
		EXTI->PR |= EXTI_PR_PR9;
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


