/*
 * clocks.c
 *
 *  Created on: Jun 14, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "clocks.h"
#include "usart.h"

/*	G L O B A L   V A R I A B L E S   */
extern unsigned long ulHighFrequencyTimerTicks;

void init_sysclock(void) {
	/* Enable HSI48 and HSI14 */
	RCC->CR2 = 	( RCC_CR2_HSI48ON
				| RCC_CR2_HSI14ON);

	while(!(RCC->CR & RCC_CR_HSIRDY));

//	RCC->CIR |= RCC_CIR_HSI48RDYIE;
	RCC->CR |= RCC_CR_CSSON | RCC_CR_HSION;

	RCC->CFGR = ( // RCC_CFGR_SW_HSI48  	// use HSE as System Clock
				RCC_CFGR_HPRE_DIV1  		// do not divide System Clock
				| RCC_CFGR_PPRE_0  			// HCLK not divided for PCLK prescaler
				| RCC_CFGR_PLLSRC_HSI_DIV2  // PPL input clock source is HSI/2
				| RCC_CFGR_PLLMUL2 			// Mult. PPL input clock by 2 (is now 8MHz?)
				| RCC_CFGR_MCO_SYSCLK   	// output SYSCLK to MCO pin
				| RCC_CFGR_MCOPRE_DIV32		// MCO pin is divided by 32, output should be 1.5MHz
				| RCC_CFGR_PLLNODIV); 		// PPL is not divded by 2 for MCO

	/* APB Peripherals Reset */
//	RCC->APB1RSTR = 0x00000000;	 // p117 in Ref. Manual
//	RCC->APB2RSTR = 0x00000000;	 // p116 in Ref. Manual

	/* AHB Peripheral Clock Enable Registers */
	RCC->AHBENR =	( RCC_AHBENR_GPIOAEN
					| RCC_AHBENR_GPIOBEN
					| RCC_AHBENR_GPIOCEN
					| RCC_AHBENR_GPIOFEN
//					| RCC_AHBENR_TSCEN
					| RCC_AHBENR_TSEN);

	RCC->APB1ENR =  ( RCC_APB1ENR_TIM2EN
					| RCC_APB1ENR_TIM3EN
					| RCC_APB1ENR_TIM14EN
					| RCC_APB1ENR_I2C1EN
					| RCC_APB1ENR_I2C2EN
					| RCC_APB1ENR_WWDGEN
					| RCC_APB1ENR_PWREN);

	RCC->APB2ENR =  ( RCC_APB2ENR_ADCEN
					| RCC_APB2ENR_TIM1EN
					| RCC_APB2ENR_USART1EN
					| RCC_APB2ENR_SYSCFGEN);

//	/* Real-Time Clock */
//	RCC->BDCR =		( RCC_BDCR_LSEON	// LSE oscillator enable
//					| RCC_BDCR_LSEDRV_0		// not-bypass mode medium-high drive LSE
//					| RCC_BDCR_RTCSEL_LSI 	// LSI used for prototyping, use LSE otherwise
//					| RCC_BDCR_RTCEN		// enable RTC
//		);

	/* Do not divide PREDIV input clock */
	RCC->CFGR2 = 	RCC_CFGR2_PREDIV_DIV1;

	/* Select clocks sources for peripherals */
	RCC->CFGR3 = 	( RCC_CFGR3_USART1SW_HSI	// 8MHz
					| RCC_CFGR3_I2C1SW_HSI);	// 8MHz

	RCC->CIR = 0;  // clear Clock Interrupt Register

	// switch System Clock to HSI48 (48MHz)
//	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI48;
//	while(!(RCC->CFGR & RCC_CFGR_SWS));
}

void init_timing_stats_timer(void) {
	/* config TIM14 registers */
	TIM3->PSC = 79; 	// 8MHz / (79+1) = 100kHz
	TIM3->ARR = (uint16_t)100; // 0.001 seconds, 10x faster than SysTick tick
	TIM3->CCR1 = (uint16_t)100;

	TIM3->CR1 |= TIM_CR1_DIR | TIM_CR1_CEN;	// enable downcounter and counter
	TIM3->EGR |= TIM_EGR_UG;
	TIM3->DIER |= TIM_DIER_UIE;

	NVIC_SetPriority(TIM3_IRQn, 0);
	NVIC_EnableIRQ(TIM3_IRQn);
}

/*	W A T C H D O G   */
void init_wwdg(void) {
	/* Set prescaler to have a roll-over each about 5.5ms,
	set window value (about 2.25ms) */
	WWDG->CFR = 0x60;

	WWDG->SR |= WWDG_SR_EWIF;
	WWDG->CR |= WWDG_CR_WDGA; /* Activate WWDG */
	NVIC_EnableIRQ(WWDG_IRQn);
}

void WWDG_IRQHandler(void) {
	if(WWDG->CFR & WWDG_CFR_EWI) {
		char * error_str = "ERROR OCCURED";
		uart_send_bytes(error_str, 14);
	}
	WWDG->SR &= ~(WWDG_SR_EWIF);  // clear EWI flag
}

/*	P R O F I L I N G  */
void TIM3_IRQHandler(void) {
	NVIC_ClearPendingIRQ(TIM3_IRQn);
	TIM3->SR &= ~(TIM_SR_UIF);
	ulHighFrequencyTimerTicks++;
}
