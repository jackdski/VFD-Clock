/*
 * rtc.c
 *
 *  Created on: Jun 21, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "rtc.h"
#include "gpio.h"

/*	S E M A P H O R E S   */
extern SemaphoreHandle_t sRTC;
extern TaskHandle_t thRTC;


void init_rtc(void) {
	PWR->CR |= PWR_CR_DBP;		// set bit for write-protection

	RCC->BDCR |= RCC_BDCR_BDRST;

	RCC->BDCR =		( RCC_BDCR_LSEON		// LSE oscillator enable
					| RCC_BDCR_LSEDRV_0		// not-bypass mode medium-high drive LSE
					| RCC_BDCR_RTCSEL_0 	// LSE used for prototyping, use LSE otherwise
					| RCC_BDCR_RTCEN);		// enable RTC

	/* unlock RTC registers */
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	RTC->ISR |= RTC_ISR_INIT;
	while (!(RTC->ISR & RTC_ISR_INITF));

/* NOTE: The following value given in the AN3371 document is wrong,
 * 	This value assumes a 40kHz quartz instead of the 32kHz clock that
 * 	is provided on the Nucleo Development board
 *	RTC->PRER = 0x007F0137; */
	RTC->PRER = 0x007F00FF;		// Correct value to output exactly 1s on RTC with 32kHz quartz

	RTC->CR &= ~(RTC_CR_OSEL); 	// output disabled
	RTC->CR |= (RTC_CR_BYPSHAD | RTC_CR_ALRAIE);

	/* initialize time to 12:00:00 */
	RTC->TR |= RTC_TR_PM;  // set to PM
	RTC->TR = (1 & RTC_TR_HT) | (2 & RTC_TR_HU);

	/* configure alarm */
	RTC->CR &= ~RTC_CR_ALRAE;	// disable alarm
	while ((RTC->ISR & RTC_ISR_ALRAWF) != RTC_ISR_ALRAWF);

	RTC->ALRMAR = 	( RTC_ALRMAR_MSK4 | RTC_ALRMAR_MSK3
					| RTC_ALRMAR_MSK2 | RTC_ALRMAR_MSK1);

	RTC->CR |= RTC_CR_ALRAIE | RTC_CR_ALRAE;	// enable Alarm A and Alarm A interrupt

	RTC->ISR &= ~RTC_ISR_INIT;	// clear init bit

	/* lock RTC registers */
	RTC->WPR = 0xFE;
	RTC->WPR = 0x64;

	/* Configure RTC Alarm A interrupt */
	SYSCFG->IT_LINE_SR[2] = SYSCFG_ITLINE2_SR_RTC_ALRA;
	EXTI->IMR |= EXTI_IMR_MR17;
	EXTI->RTSR |= EXTI_RTSR_TR17;	// enable rising trigger
	EXTI->SWIER |= EXTI_SWIER_SWIER17;

	EXTI->PR &= ~(EXTI_PR_PR17);

	/* enable RTC interrupts */
	NVIC_EnableIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);
}

/* handles interrupts from RTC's Alarm A and
 * notifies the RTC task to read the TR registers and update the display */
void RTC_IRQHandler(void) {
	if(RTC->ISR & RTC_ISR_ALRAF) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;	// default value is pdFALSES
		configASSERT(thRTC != NULL);
		vTaskNotifyGiveFromISR(thRTC, &xHigherPriorityTaskWoken );
		RTC->ISR &= ~RTC_ISR_ALRAF;		// clear RTC->ISR flag;
		EXTI->PR |= (EXTI_PR_PR17);		// clear Pending External Interrupt Flag
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

/* Hours, minutes, and seconds read/write */
uint32_t read_rtc_time(void) {
	if((RTC->ISR & RTC_ISR_RSF) == RTC_ISR_RSF) {
		return RTC->TR; /* get time */
	}
	return RTC->TR; /* get time */
}

uint8_t inline read_rtc_ampm(void) {
	return (uint8_t)(RTC->TR & RTC_TR_PM);
}

uint8_t inline read_rtc_hours(void) {
	return (uint8_t)((((RTC->TR & RTC_TR_HT) >> RTC_TR_HT_Pos) * 10) + ((RTC->TR & RTC_TR_HU) >> RTC_TR_HU_Pos));
}

uint8_t inline read_rtc_minutes(void) {
	return (uint8_t)((((RTC->TR & RTC_TR_MNT) >> RTC_TR_MNT_Pos) * 10) + ((RTC->TR & RTC_TR_MNU) >> RTC_TR_MNU_Pos));
}

uint8_t inline read_rtc_seconds(void) {
	return  (uint8_t)((((RTC->TR & RTC_TR_ST) >> RTC_TR_ST_Pos) * 10) + ((RTC->TR & RTC_TR_SU) >> RTC_TR_SU_Pos));
}

void inline change_rtc_hours(uint8_t new_hours) {
	RTC->TR &= ~(RTC_TR_HT | RTC_TR_HU);
	RTC->TR |= ((new_hours / 10) << RTC_TR_HT_Pos);
	RTC->TR |= ((new_hours % 10) << RTC_TR_HU_Pos); // ((new_hours % 10) << RTC_TR_HU_Pos);
}

void inline change_rtc_minutes(uint8_t new_minutes) {
	RTC->TR &= ~(RTC_TR_MNT | RTC_TR_MNU);
	RTC->TR |= ((new_minutes / 10) << RTC_TR_MNT_Pos);
	RTC->TR |= ((new_minutes % 10) << RTC_TR_MNU_Pos);
}

void inline change_rtc_seconds(uint8_t new_seconds) {
	RTC->TR &= ~(RTC_TR_ST | RTC_TR_SU);
	RTC->TR |= ((new_seconds / 10) & RTC_TR_ST_Pos);
	RTC->TR |= ((new_seconds % 10) & RTC_TR_SU_Pos);
}

void inline change_rtc_ampm(uint8_t new_ampm) {
	if(new_ampm == 1) // PM
		RTC->TR |= (RTC_TR_PM);
	else
		RTC->TR &= ~(RTC_TR_PM);
}

void change_rtc_time(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t ampm) {
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);

	// write new values
	change_rtc_hours(hours);
	change_rtc_minutes(minutes);
	change_rtc_seconds(seconds);
	change_rtc_ampm(ampm);

	RTC->ISR &=~ RTC_ISR_INIT;
	RTC->WPR = 0xFE;
	RTC->WPR = 0x64;
}

/* Year, month, day, and weekday read/write */

uint32_t read_rtc_calender(void) {
	if((RTC->ISR & RTC_ISR_RSF) == RTC_ISR_RSF) {
		return RTC->DR;
	}
	return RTC->DR;
}

uint8_t inline read_rtc_year(void) {
	return (uint8_t)((((RTC->DR & RTC_DR_YT) >> RTC_DR_YT_Pos) * 10) + ((RTC->DR & RTC_DR_YU) >> RTC_DR_YU_Pos));
}

uint8_t inline read_rtc_month(void) {
	return (uint8_t)((((RTC->DR & RTC_DR_MT) >> RTC_DR_MT_Pos ) * 10) + ((RTC->DR & RTC_DR_MU) >> RTC_DR_MU_Pos));
}

uint8_t inline read_rtc_day(void) {
	return (uint8_t)((((RTC->DR & RTC_DR_DT) >> RTC_DR_DT_Pos ) * 10) + ((RTC->DR & RTC_DR_DU) >> RTC_DR_DU_Pos));
}

uint8_t inline read_rtc_day_of_week(void) {
	return (uint8_t)((RTC->DR & RTC_DR_WDU) >> RTC_DR_WDU_Pos);
}

void inline change_rtc_year(uint8_t new_year) {
	RTC->DR &= ~(RTC_DR_YT | RTC_DR_YU);
	RTC->DR |= ((new_year / 10) << RTC_DR_YT_Pos);
	RTC->DR |= ((new_year % 10) << RTC_DR_YU_Pos);
}

void inline change_rtc_month(uint8_t new_month) {
	RTC->DR &= ~(RTC_DR_MT | RTC_DR_MU);
	RTC->DR |= ((new_month / 10) << RTC_DR_MT_Pos);
	RTC->DR |= ((new_month % 10) << RTC_DR_MU_Pos);
}

void inline change_rtc_day(uint8_t new_day) {
	RTC->DR &= ~(RTC_DR_DT | RTC_DR_DU);
	RTC->DR |= ((new_day / 10) << RTC_DR_DT_Pos);
	RTC->DR |= ((new_day % 10) << RTC_DR_DU_Pos);
}

void inline change_rtc_day_of_week(uint8_t new_dow) {
	RTC->DR |= (new_dow & RTC_DR_WDU);
}

void change_rtc_date(uint8_t month, uint8_t day) {
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);

	// write new values
	change_rtc_day(day);
	change_rtc_month(month);

	RTC->ISR &=~ RTC_ISR_INIT;
	RTC->WPR = 0xFE;
	RTC->WPR = 0x64;
}
