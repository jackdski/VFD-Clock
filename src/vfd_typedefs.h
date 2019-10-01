/*
 * vfd_typedefs.h
 *
 *  Created on: Jun 25, 2019
 *      Author: jack
 */

#ifndef VFD_TYPEDEFS_H_
#define VFD_TYPEDEFS_H_

/*	E N U M S   */

typedef enum {
	Error = -1,
	Clock = 0,
	Button_Date = 1,
	Button_Temperature = 2,
	Config = 3,
	Switch_Sleep = 4,
	BLE_Date = 5,
	BLE_Temperature = 6,
	BLE_Change_RTC = 7,
	BLE_Sleep = 8,
	Deep_Sleep = 9
} System_State_E;

typedef enum {
	Efuse_Error = -1,
	Normal = 0
} Efuse_Status_E;

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

typedef enum {
	BLE_Error = -1,
	Disconnected = 0,
	Connected = 1,
	Sleep = 2,
	Waking_Up = 3
} HC_10_Status_E;

typedef enum {
	Open = 0,
	Pressed = 1
} Button_Status_E;

typedef enum {
	Off = 0,
	Flashing = 1
} Light_Flash_E;

typedef enum {
	Slow = 0,
	Quick = 1,
	Fast = 2
} Time_Change_Speed_E;

typedef enum {
	Reset = 0,
	Standby_Wakeup = 1
} Time_Config_Options_E;

typedef enum {
	AM = 0,
	PM = 1
} Midday_Clock_E;

typedef struct BLE_Message {
	BLE_Message_Types_E message_type;
	uint8_t	data_byte_one;
	uint8_t	data_byte_two;
} BLE_Message_t;

typedef struct ClockTime {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} ClockTime_t;

#endif /* VFD_TYPEDEFS_H_ */
