/*
 * low_power.h
 *
 *  Created on: Jul 25, 2019
 *      Author: jack
 */

#ifndef LOW_POWER_H_
#define LOW_POWER_H_

#include "FreeRTOS.h"
#include "portmacro.h"

/*	L O W - P O W E R   */
void configure_pwr_for_sleep(void);
void configure_rtc_for_sleep(void);

/* this will prepares the device for DeepSleep/Standby Mode, which is a mode
 * that "stops all the clocks in the core supply domain and disables the PLL
 * and the HSI, HSI48, HSI14 and HSE oscillators" and "SRAM and register
 * contents are lost except for registers in the RTC domain and Standby circuitry". */
void configure_for_deepsleep(void);
void configure_for_stopmode(void);

/*	T I C K L E S S   I D L E   */
void vApplicationSleep(TickType_t xExpectedIdleTime);
unsigned long ulGetExternalTime(void);
void vSetWakeTimeInterrupt(TickType_t xExpectedIdleTime);
void prvSleep(void);
void prvStartTickInterruptTimer(void);
void prvStopTickInterruptTimer(void);

#endif /* LOW_POWER_H_ */
