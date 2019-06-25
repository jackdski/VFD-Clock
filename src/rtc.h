/*
 * rtc.h
 *
 *  Created on: Jun 21, 2019
 *      Author: jack
 */

#ifndef RTC_H_
#define RTC_H_

void init_rtc(void);

/* Hours, minutes, and seconds read/write */
uint32_t read_rtc_time(void);

uint8_t read_rtc_ampm(void);

uint8_t read_rtc_hours(void);

uint8_t read_rtc_minutes(void);

uint8_t read_rtc_hours(void);

void change_rtc_hours(uint8_t new_hours);

void change_rtc_minutes(uint8_t new_minutes);

void change_rtc_seconds(uint8_t new_seconds);

/* Year, month, day, and weekday read/write */

uint32_t read_rtc_calender(void);

uint8_t read_rtc_year(void);

uint8_t read_rtc_month(void);

uint8_t read_rtc_day(void);

uint8_t read_rtc_day_of_week(void);

void change_rtc_year(uint8_t new_year);

void change_rtc_month(uint8_t new_month);

void change_rtc_day(uint8_t new_day);

void change_rtc_day_of_week(uint8_t new_dow);

#endif /* RTC_H_ */
