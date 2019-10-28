/*
 * sensor_tasks.h
 *
 *  Created on: Jun 17, 2019
 *      Author: jack
 */

#ifndef SENSOR_TASKS_H_
#define SENSOR_TASKS_H_

#include "timers.h"

/*	D E F I N E S   */
#define STARTING_DISPLAY_BRIGHTNESS 	50	// initialize display to 50% brightness
#define TMP_ADC_CHANNEL					9
#define PC3_ADC_CHANNEL		13	// pin PC3 is channel 13
#define BLE_MSG_SIZE		sizeof(BLE_Message_t);
#define USE_BOTH_PHOTORESISTORS


void prvRTC_Task(void *pvParameters);

void prvConfig_Task(void *pvParameters);

void prvTemperature_Task(void *pvParameters);

void prvLight_Task(void *pvParameters);

void prvChange_Brightness_Task(void *pvParameters);

void prvBLE_Send_Task(void *pvParameters);

void prvBLE_Receive_Task(void *pvParameters);

void five_sec_timer_callback(TimerHandle_t xTimer);

void prvTurnOffTask(void *pvParameters);

void prvTurnOnTask(void *pvParameters);

#endif /* SENSOR_TASKS_H_ */
