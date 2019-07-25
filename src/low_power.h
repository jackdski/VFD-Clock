/*
 * low_power.h
 *
 *  Created on: Jul 25, 2019
 *      Author: jack
 */

#ifndef LOW_POWER_H_
#define LOW_POWER_H_

/* this will prepares the device for DeepSleep/Standby Mode, which is a mode
 * that "stops all the clocks in the core supply domain and disables the PLL
 * and the HSI, HSI48, HSI14 and HSE oscillators" and "SRAM and register
 * contents are lost except for registers in the RTC domain and Standby circuitry". */
void configure_for_deepsleep(void);

#endif /* LOW_POWER_H_ */
