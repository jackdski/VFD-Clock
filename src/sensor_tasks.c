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
#include "queue.h"
#include "semphr.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "sensor_tasks.h"
#include "vfd_typedefs.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"
#include "adc.h"
#include "rtc.h"
#include "tubes.h"
#include "pwm.h"
#include "mcp9808.h"
#include "MPL3115A2.h"

#define PC3_ADC_CHANNEL		13	// pin PC3 is channel 13

#define BLE_MSG_SIZE		sizeof(BLE_Message_t);

#define USE_BOTH_PHOTORESISTORS

/*	G L O B A L   V A R I A B L E S   */
extern uint8_t hours;
extern uint8_t minutes;
extern uint8_t seconds;
extern uint8_t temperature;

extern uint32_t light_value;
extern uint16_t display_brightness;
extern uint8_t usart_msg;
extern SemaphoreHandle_t sRTC;

#define RTC_MAX_WAIT_TICKS		5


/*	T A S K   D E C L A R A T I O N S   */

/* if given the semaphore by the RTC Interrupt Handler, read the time and update the tube display */
void prvRTC_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 1000 );
	const TickType_t delay_time_until = 100;  // 100 tick cycles

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for( ;; ) {
		vTaskDelayUntil( &xLastWakeTime, delay_time_until);
		if(xSemaphoreTake( sRTC, RTC_MAX_WAIT_TICKS ) == pdTRUE) {	// take the semaphore within 5 ticks
			hours = read_rtc_hours();
			minutes = read_rtc_minutes();
			seconds = read_rtc_seconds();
	//		if(old_seconds == seconds)
			toggle_rtc_led();
			/* update tubes */
			update_time(hours, minutes, seconds);
//			vTaskDelay(delay_time);
		}
	}
}

/* reads the temperature every 5 seconds over I2C and updates the tube display */
void prvTemperature_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 3000 ); // 3s
	for( ;; ) {
//		config_temperature_sensor();
		config_temperature_sensor_mpl();
		vTaskDelay(delay_time);		// 3s
		// TODO: take semaphore from interrupt
		// TODO:wake up temperature sensor();
		// TODO: disable_shutdown_mode();
//		read_config_register();
//		read_config_mpl();
		//sample_temperature();
	}
}

/* updates the light_value global variable every 5 seconds
 * so that the prvChangePWM task changes the brightness */
void prvLight_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 2000 ); // 5s
	static uint32_t light_sample;
	static uint32_t light_sample_two;
	for( ;; ) {
		vTaskDelay(delay_time);	// 1s
		select_adc_channel(PHOTORESISTOR_LEFT);
		light_sample = sample_adc();

#ifdef USE_BOTH_PHOTORESISTORS
		select_adc_channel(PHOTORESISTOR_RIGHT);
//		light_sample_two = (light_sample + sample_adc()) / 2;
		light_sample_two = sample_adc();
		light_sample = (light_sample + light_sample_two) / 2;
#endif
		/* check if ADC conversions are close enough to trust */
		if(is_good_light_data(light_sample, light_sample_two)) {
			/* if in a new range now */
			if(in_diff_light_range(light_value, light_sample) == 1) {
				trigger_dimming_timer(display_brightness, calc_new_brightness(light_sample));
				display_brightness = calc_new_brightness(light_sample);
			}
			light_value = light_sample;
		}
	}
}

/* acknowledges the BLE connection and sends updates every 1s that the devices are connected */
void prvBLE_Send_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 1000 ); // 1s
	for( ;; ) {
		uint8_t msg_size = (uint8_t)BLE_MSG_SIZE;
		vTaskDelay(delay_time);
		uart_send_byte(0x11);
	}
}

/* if there is a BLE connection, then this task will read the BLE RX message queue if it is not empty */
void prvBLE_Receive_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 500 );  // 500ms
	for( ;; ) {
		vTaskDelay(delay_time);
//		if();
	}

}
