/*
 * callbacks.h
 *
 *  Created on: Jul 22, 2019
 *      Author: jack
 */

#ifndef CALLBACKS_H_
#define CALLBACKS_H_

#include "timers.h"

/* checks if both +/- buttons are still pressed every 100ms and changes system_state
*	if 3s is reached */
void three_sec_timer_callback(TimerHandle_t xTimer);

/* call back function for the software timer created when "TEMP" or "DATE"
 * is received over UART */
void five_sec_timer_callback(TimerHandle_t xTimer);

/* call back function for the 50ms software timer created to count
 * 	how long the button has been held down for and how fast the
 * 	time should be changed */
void button_timer_callback(TimerHandle_t xTimer);

#endif /* CALLBACKS_H_ */
