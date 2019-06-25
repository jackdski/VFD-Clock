/*
 * sensor_tasks.c
 *
 *  Created on: Jun 15, 2019
 *      Author: jack
 */
/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "sensor_tasks.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"
#include "adc.h"
#include "mcp9808.h"
#include "MPL3115A2.h"

#define PC3_ADC_CHANNEL		13	// pin PC3 is channel 13

/*	G L O B A L   V A R I A B L E S   */
extern uint32_t light_value;;

void prvRTC_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 980 );
	for( ;; ) {

	}
}

void prvTemperature_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 3000 ); // 3s
	for( ;; ) {
		config_temperature_sensor();
		config_temperature_sensor_mpl();
		vTaskDelay(delay_time);		// 3s
		// TODO: take semaphore from interrupt
		// TODO:wake up temperature sensor();
		// TODO: disable_shutdown_mode();
//		read_config_register();
		read_config_mpl();
		//sample_temperature();
	}
}

void prvLight_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 5000 ); // 5s
	for( ;; ) {
		vTaskDelay(delay_time);	// 1s
		select_adc_channel(13);
		light_value = sample_adc();
	}
}

void prvBLE_Update_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 1000 ); // 1s
	for( ;; ) {
		vTaskDelay(delay_time);
		uart_send_byte(0x11);
	}
}
