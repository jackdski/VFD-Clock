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

void init_sysclock(void) {
	/* Enable HSI48 and HSI14 */
	RCC->CR2 = 	( RCC_CR2_HSI48ON
				| RCC_CR2_HSI14ON
		);


//	NVIC_EnableIRQ(RCC_CRS_IRQn);
//	NVIC_SetPriority(RCC_CRS_IRQn,0);

//	RCC->CIR |= RCC_CIR_HSI48RDYIE;
	RCC->CR |= RCC_CR_CSSON | RCC_CR_HSION;

	RCC->CFGR = ( //RCC_CFGR_SW_HSI48  		// use HSE as System Clock
				RCC_CFGR_HPRE_DIV1  		// do not divide System Clock
				| RCC_CFGR_PPRE_0  			// HCLK not divided for PCLK prescaler
				| RCC_CFGR_PLLSRC_HSI_DIV2  // PPL input clock source is HSI/2
				| RCC_CFGR_PLLMUL2 			// Mult. PPL input clock by 2 (is now 8MHz?)
				| RCC_CFGR_MCO_SYSCLK   	// output SYSCLK to MCO pin
				| RCC_CFGR_MCOPRE_DIV32		// MCO pin is divided by 32, output should be 1.5MHz
				| RCC_CFGR_PLLNODIV 		// PPL is not divded by 2 for MCO
			);

	/* APB Peripherals Reset */
//	RCC->APB1RSTR = 0x00000000;	 // p117 in Ref. Manual
//	RCC->APB2RSTR = 0x00000000;	 // p116 in Ref. Manual

	/* AHB Peripheral Clock Enable Registers */
	RCC->AHBENR =	( RCC_AHBENR_GPIOAEN
					| RCC_AHBENR_GPIOBEN
					| RCC_AHBENR_GPIOCEN
					| RCC_AHBENR_GPIODEN
					| RCC_AHBENR_GPIOEEN
					| RCC_AHBENR_GPIOFEN
					| RCC_AHBENR_TSCEN
		);

	RCC->APB1ENR =  ( RCC_APB1ENR_TIM2EN
					| RCC_APB1ENR_TIM14EN
					| RCC_APB1ENR_USART2EN
					| RCC_APB1ENR_USART3EN
					| RCC_APB1ENR_I2C1EN
					| RCC_APB1ENR_I2C2EN
					| RCC_APB1ENR_PWREN
		);

	RCC->APB2ENR =  ( RCC_APB2ENR_ADCEN
					| RCC_APB2ENR_TIM1EN
					| RCC_APB2ENR_USART1EN
					| RCC_APB2ENR_TIM15EN
					| RCC_APB2ENR_TIM16EN
					| RCC_APB2ENR_SYSCFGEN
		);

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
					| RCC_CFGR3_I2C1SW_HSI		// 8MHz
					| RCC_CFGR3_USART2SW_HSI	// 8MHz
					| RCC_CFGR3_USART3SW_HSI	// 8MHz
		);

	RCC->CIR = 0;  // clear Clock Interrupt Register

	// switch System Clock to HSI48 (48MHz)
//	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI48;
//	while(!(RCC->CFGR & RCC_CFGR_SWS));
}

//void RCC_CRS_IRQHandler(void) {
//	if((RCC->CIR & RCC_CIR_HSI48RDYF) != 0) {
//		RCC->CIR |= RCC_CIR_HSI48RDYC;
//		RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI48;
//	}
//}

