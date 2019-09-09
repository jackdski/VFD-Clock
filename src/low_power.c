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

void configure_pwr_for_sleep(void) {
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CSR |= PWR_CSR_EWUP2;		// enable WKUP2 on PC13
	PWR->CR |= PWR_CR_CWUF; 		// clear the WUF flag
	PWR->CR |= PWR_CR_PDDS;
}

void configure_rtc_for_sleep(void) {
	RCC->BDCR |=	( RCC_BDCR_LSEON		// use LSE oscillator for RTC
					| RCC_BDCR_LSEDRV_0	);	// not-bypass mode medium-high drive LSE
	RCC->BDCR &= ~RCC_BDCR_RTCSEL_LSI; 	// disable LSI use LSE instead

	rtc_disable_alarm();	// make sure that standby mode is not exited by RTC Alarm A
}

/* Configures peripherals for entry to the DeepSleep Low-Power Mode,
 * 	ends FreeRTOS scheduler, and then sets the SCB->SCR bit to enter
 * 	the DeepSleep Low-Power Mode */
void configure_for_deepsleep(void) {
	set_sleep_mode_hc_10(); 	// put HC-10 in sleep mode
	uart_disable_peripheral();
	i2c_disable_peripheral();
	disable_adc();

	update_time(0, 0, 0);	/* do not display anything on tubes */
	// TODO: need to configure pins on Efuse?

	configure_rtc_for_sleep();  // configure RTC for DeepSleep/Standby mode
	vTaskEndScheduler();
	configure_pwr_for_sleep();

	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}
