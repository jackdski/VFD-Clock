/*
 * rtc.c
 *
 *  Created on: Jun 21, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "rtc.h"

void init_rtc(void) {
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);

	RTC->PRER = 0x007F0137;

	RTC->CR &= ~(RTC_CR_OSEL); // output disabled
	RTC->CR |= RTC_CR_BYPSHAD;

	RTC->WPR = 0xFE; /* (7) */
	RTC->WPR = 0x64; /* (7) */
}

/* Hours, minutes, and seconds read/write */
uint32_t read_rtc_time(void) {
	if((RTC->ISR & RTC_ISR_RSF) == RTC_ISR_RSF) {
		return RTC->TR; /* get time */
	}
}

uint8_t inline read_rtc_ampm(void) {
	return (uint8_t)(RTC->TR & RTC_TR_PM);
}

uint8_t inline read_rtc_hours(void) {
	return (uint8_t)((RTC->TR & RTC_TR_HT) * 10) + (RTC->TR & RTC_TR_HU);
}

uint8_t inline read_rtc_minutes(void) {
	return (uint8_t)((RTC->TR & RTC_TR_MNT) * 10) + (RTC->TR & RTC_TR_MNU);
}

uint8_t inline read_rtc_seconds(void) {
	return (uint8_t)((RTC->TR & RTC_TR_ST) * 10) + (RTC->TR & RTC_TR_SU);
}

void inline change_rtc_hours(uint8_t new_hours) {
	RTC->TR |= ((new_hours / 10) & RTC_TR_HT);
	RTC->TR |= ((new_hours % 10) & RTC_TR_HU);
}

void inline change_rtc_minutes(uint8_t new_minutes) {
	RTC->TR |= ((new_minutes / 10) & RTC_TR_MNT);
	RTC->TR |= ((new_minutes % 10) & RTC_TR_MNU);
}

void inline change_rtc_seconds(uint8_t new_seconds) {
	RTC->TR |= ((new_seconds / 10) & RTC_TR_ST);
	RTC->TR |= ((new_seconds % 10) & RTC_TR_SU);
}

/* Year, month, day, and weekday read/write */

uint32_t read_rtc_calender(void) {
	if((RTC->ISR & RTC_ISR_RSF) == RTC_ISR_RSF) {
		return RTC->DR;
	}
}

uint8_t inline read_rtc_year(void) {
	return (uint8_t)((RTC->DR & RTC_DR_YT) * 10) + (RTC->DR & RTC_DR_YU);
}

uint8_t inline read_rtc_month(void) {
	return (uint8_t)((RTC->DR & RTC_DR_MT) * 10) + (RTC->DR & RTC_DR_MU);
}

uint8_t inline read_rtc_day(void) {
	return (uint8_t)((RTC->DR & RTC_DR_DT) * 10) + (RTC->DR & RTC_DR_DU);
}

uint8_t inline read_rtc_day_of_week(void) {
	return (uint8_t)((RTC->DR & RTC_DR_WDU));
}

void inline change_rtc_year(uint8_t new_year) {
	RTC->DR |= ((new_year / 10) & RTC_DR_YT);
	RTC->DR |= ((new_year % 10) & RTC_DR_YU);
}

void inline change_rtc_month(uint8_t new_month) {
	RTC->DR |= ((new_month / 10) & RTC_DR_MT);
	RTC->DR |= ((new_month % 10) & RTC_DR_MU);
}

void inline change_rtc_day(uint8_t new_day) {
	RTC->DR |= ((new_day / 10) & RTC_DR_DT);
	RTC->DR |= ((new_day % 10) & RTC_DR_DU);
}

void inline change_rtc_day_of_week(uint8_t new_dow) {
	RTC->DR |= (new_dow & RTC_DR_WDU);
}






