/*
 * low_power.c
 *
 *  Created on: Jul 25, 2019
 *      Author: jack
 */

#include "stm32f091xc.h"
#include <stdint.h>
#include "usart.h"
#include "rtc.h"
#include "tubes.h"

void configure_for_deepsleep(void) {
	set_sleep_mode_hc_10(); 	// put HC-10 in sleep mode

	update_time(0, 0, 0);	/* do not display anything on tubes */


	// configure RTC for DeepSleep/Standby mode
	RCC->BDCR |=	( RCC_BDCR_LSEON		// use LSE oscillator for RTC
					| RCC_BDCR_LSEDRV_0	);	// not-bypass mode medium-high drive LSE
	RCC->BDCR &= ~RCC_BDCR_RTCSEL_LSI; 	// disable LSI use LSE instead

	rtc_disable_alarm();	// make sure that standby mode is not exited by RTC Alarm A

	// DeepSleep/Standby mode entry
	SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
	PWR->CSR |= PWR_CSR_EWUP2;		// enable WKUP2 on PC13
	PWR->CR |= PWR_CR_PDDS;
	PWR->CSR |= PWR_CSR_WUF;
}
