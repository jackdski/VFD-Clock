/*
 * vfd_typedefs.h
 *
 *  Created on: Jun 25, 2019
 *      Author: jack
 */

#ifndef VFD_TYPEDEFS_H_
#define VFD_TYPEDEFS_H_

#include <stdint.h>

typedef enum {
	Error = -1,
	Clock = 0,
	Button_Date = 1,
	Button_Temperature = 2,
	Switch_Config = 3,
	BLE_Date = 4,
	BLE_Temperature = 5,
	BLE_Change_RTC = 6,
	Deep_Sleep = 7
} System_State_E;


typedef enum {
	Failure = -1,
	Time = 0,
	Brightness = 1,
	OnOff = 2
} BLE_Message_Types_E;


typedef struct {
	BLE_Message_Types_E message_type;
	uint8_t	data_byte_one;
	uint8_t	data_byte_two;
} BLE_Message_t;


typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} ClockTime_t;

#endif /* VFD_TYPEDEFS_H_ */
