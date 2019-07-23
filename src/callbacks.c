/*
 * callbacks.c
 *
 *  Created on: Jul 22, 2019
 *      Author: jack
 */

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "callbacks.h"
#include "vfd_typedefs.h"
#include "gpio.h"
#include "rtc.h"
#include "tubes.h"
#include "pwm.h"

/*	T A S K   N O T I F I C A T I O N S   */
extern TaskHandle_t thRTC;
extern TaskHandle_t thConfig;

/*	G L O B A L   V A R I A B L E S   */
extern System_State_E system_state;

extern uint8_t hours;		/* 1-12 */
extern uint8_t minutes;		/* 0-59 */
extern uint8_t seconds;		/* 0-59 */

extern uint16_t display_brightness;
extern uint8_t config_timer_callback_count;

extern TimerHandle_t three_sec_timer;
extern TimerHandle_t five_sec_timer;
extern TimerHandle_t ten_sec_timer;

extern Button_Status_E plus_button_status;
extern Button_Status_E minus_button_status;
extern Light_Flash_E indication_light_status;
extern uint8_t holds;
extern Time_Change_Speed_E change_speed;


/*	T I M E R   C A L L B A C K S   */

/* checks if both +/- buttons are still pressed every 100ms and changes system_state
*	if 3s is reached */
void three_sec_timer_callback(TimerHandle_t xTimer) {
	if(system_state != Config) {
		// break since buttons weren't pressed long enough
		if((plus_button_status != Pressed) || (minus_button_status != Pressed)) {
			xTimerStop(three_sec_timer, pdMS_TO_TICKS(500));	// stop timer, waiting max 500ms to execute
			config_timer_callback_count = 0;					// reset counter
		}
		// button was pressed long enough, so stop timer and got to Config
		else {
			config_timer_callback_count += 1;
			if(config_timer_callback_count == 30) {	 // 3s threshold is reached
				xTimerStop(three_sec_timer, pdMS_TO_TICKS(500));	// stop timer, waiting max 500ms to execute
				config_timer_callback_count = 0;
				system_state = Config;
				indication_light_status = Flashing;
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xTimerStartFromISR(ten_sec_timer, &xHigherPriorityTaskWoken);
				vTaskSuspend(thRTC);
				vTaskResume(thConfig);	// resume task
			}
		}
	}
}

/* call back function for the software timer created when "TEMP" or "DATE" is received
 * 	over UART */
void five_sec_timer_callback(TimerHandle_t xTimer) {
	toggle_error_led();
	system_state = Clock;
	hours = read_rtc_hours();
	minutes = read_rtc_minutes();
	seconds = read_rtc_seconds();
	update_time(hours, minutes, seconds);	// show time on display again
	vTaskResume(thRTC);
	indication_light_status = Flashing;
}

/* call back function for the software timer created to leave Config mode
*	 without touching the buttons */
void ten_sec_timer_callback(TimerHandle_t xTimer) {
	system_state = Clock;
	// disable 2Hz display flashing and resume normal clock operations
	vTaskSuspend(thConfig);
	vTaskResume(thRTC);
	change_pwm_duty_cycle(display_brightness);
	indication_light_status = Flashing;
}

/* call back function for the 50ms software timer created to count
 * 	how long the button has been held down for and how fast the
 * 	time should be changed
 * 	+1min for every <3s hold, +5mins for every >3s & <8s hold, +1hr every >8s hold */
 void button_timer_callback(TimerHandle_t xTimer) {
	// make sure one of the buttons is still pressed
	if(!(read_plus_button()) || !(read_plus_button())) {
		holds = 0;
		change_speed = Slow;
		return;
	}
	// if a button is pressed, increase time accordingly
	else {
		holds++;
		switch(holds) {
			case 60: change_speed = Quick; break;
			case 160: change_speed = Fast; break;
			default: break;
		}

		if((change_speed == Slow) && (holds % 20 == 0))			// increase every (20 * 50ms) = 1s
			increase_minutes(1);
		else if((change_speed == Quick) && (holds % 7 == 0)) 	// increase every (7 * 50ms) = 0.35s
			increase_minutes(10);
		else if(change_speed == Fast && (holds % 20 == 0))		// increase every (20 * 50ms) = 1s
			increase_hours();
	}
}
