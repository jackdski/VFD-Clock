/*
 * vfd_typedefs.h
 *
 *  Created on: Jun 25, 2019
 *      Author: jack
 */

#ifndef VFD_TYPEDEFS_H_
#define VFD_TYPEDEFS_H_

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
	Initialize_Msg = -3,
	Resend_Request = -2,
	Failure = -1,
	Time_On_Clock = 0,
	Time_Change_Cmd = 1,
	VFD_Brightness = 2,
	Brightness_Change_Cmd = 3,
	Temperature = 4,
	On_Off_Cmd = 5
} BLE_Message_Types_E;

typedef struct BLE_Message {
	BLE_Message_Types_E message_type;
	uint8_t	data_byte_one;
	uint8_t	data_byte_two;
}BLE_Message_t;

typedef struct ClockTime {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
}ClockTime_t;

#endif /* VFD_TYPEDEFS_H_ */
