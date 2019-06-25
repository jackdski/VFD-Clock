/*
 * sensor_tasks.h
 *
 *  Created on: Jun 17, 2019
 *      Author: jack
 */

#ifndef SENSOR_TASKS_H_
#define SENSOR_TASKS_H_

void prvTemperature_Task(void *pvParameters);

void prvLight_Task(void *pvParameters);

void prvBLE_Update_Task(void *pvParameters);

#endif /* SENSOR_TASKS_H_ */
