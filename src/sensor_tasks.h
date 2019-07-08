/*
 * sensor_tasks.h
 *
 *  Created on: Jun 17, 2019
 *      Author: jack
 */

#ifndef SENSOR_TASKS_H_
#define SENSOR_TASKS_H_

#include "timers.h"

void prvRTC_Task(void *pvParameters);

void prvTemperature_Task(void *pvParameters);

void prvLight_Task(void *pvParameters);

void prvChange_Brightness_Task(void *pvParameters);

void prvBLE_Send_Task(void *pvParameters);

void prvBLE_Receive_Task(void *pvParameters);

void five_sec_timer_callback(TimerHandle_t xTimer);

#endif /* SENSOR_TASKS_H_ */
