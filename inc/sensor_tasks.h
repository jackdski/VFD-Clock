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
#define TMP_ADC_CHANNEL					8
#define PC3_ADC_CHANNEL		13	// pin PC3 is channel 13


void prvConfig_Task(void *pvParameters);
void prvTemperature_Task(void *pvParameters);
int8_t get_temperature(void);


#endif /* SENSOR_TASKS_H_ */
