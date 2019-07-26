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
#include "i2c.h"
#include "adc.h"

#include "FreeRTOS.h"
#include "task.h"

void configure_for_deepsleep(void) {
	set_sleep_mode_hc_10(); 	// put HC-10 in sleep mode
	uart_disable_peripheral();
	i2c_disable_peripheral();
	disable_adc();

	update_time(0, 0, 0);	/* do not display anything on tubes */
	// TODO: configure pins on Efuse

	// configure RTC for DeepSleep/Standby mode
	RCC->BDCR |=	( RCC_BDCR_LSEON		// use LSE oscillator for RTC
					| RCC_BDCR_LSEDRV_0	);	// not-bypass mode medium-high drive LSE
	RCC->BDCR &= ~RCC_BDCR_RTCSEL_LSI; 	// disable LSI use LSE instead

	rtc_disable_alarm();	// make sure that standby mode is not exited by RTC Alarm A

	// DeepSleep/Standby mode entry
	vTaskEndScheduler();
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CSR |= PWR_CSR_EWUP2;		// enable WKUP2 on PC13
	PWR->CR |= PWR_CR_CWUF; 		// clear the WUF flag
	PWR->CR |= PWR_CR_PDDS;
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}
