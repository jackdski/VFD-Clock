/*
 * sensor_tasks.c
 *
 *  Created on: Jun 15, 2019
 *      Author: jack
 */
/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "main.h"
#include "sensor_tasks.h"
#include "vfd_typedefs.h"
#include "circular_buffer.h"
#include "gpio.h"
#include "i2c.h"
#include "adc.h"
#include "rtc.h"
#include "tubes.h"
#include "pwm.h"
#include "MPL3115A2.h"
#include "tmp36.h"
#include "callbacks.h"
#include "low_power.h"


// private variables
static int8_t temperature = 1;	/* -128 - 127 deg C */

/*	T A S K   H A N D L E S   */
extern TaskHandle_t thRTC;
extern TaskHandle_t thConfig;
extern TaskHandle_t thBrightness_Adj;
extern TaskHandle_t thAutoBrightAdj;
extern TaskHandle_t thOff;
extern TaskHandle_t thBLErx;
extern TaskHandle_t thBLEtx;

/*	G L O B A L   V A R I A B L E S   */
extern System_State_E system_state;
extern Settings_t settings;


/*	T A S K S   */

/* flashes the display with the time at 2Hz
 * 	- have this task suspend itself after either of the +/- buttons have
 *		not been pressed for over 10s */
void prvConfig_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS(500);
	static Light_Flash_E config_display_flashing = Flashing;
	static uint16_t config_display_brightness = 75;

	for( ;; ) {
		uint8_t hours = read_rtc_hours();
		uint8_t minutes = read_rtc_minutes();
		uint8_t seconds = read_rtc_seconds();
		uint8_t ampm = read_rtc_ampm();

		if(system_state != Config) {
			vTaskSuspend( NULL );	// suspend this task if outside of Config mode
			/* set RTC values to hours, minutes, seconds variable values */
			change_rtc_time(hours, minutes, seconds, ampm);
			update_time(hours, minutes, seconds);
		}
		/* if currently off, update display and turn it on */
		if(config_display_flashing == Off) {
			update_time(hours, minutes, seconds);	/* update tubes */
			change_pwm_duty_cycle(config_display_brightness);
			config_display_flashing = Flashing;
		}

		/* if currently flashing, turn display off */
		else if(config_display_flashing == Flashing) {
			change_pwm_duty_cycle(0);
			config_display_flashing = Off;
		}
		vTaskDelay(delay_time);
	}
}

/* reads the temperature every 5 seconds over I2C and updates the tube display */
void prvTemperature_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 3000 ); // 3s
	static uint32_t tmp_out_measurement = 0;

	for( ;; ) {
#ifdef	USE_I2C
		check_whoami_mpl();  // make sure sensor is available
		trigger_sample_mpl();
		temperature = read_temp_c();
#else
		tmp_disable_shutdown();
		select_adc_channel(TMP_ADC_CHANNEL);
		tmp_out_measurement = sample_adc();
		if(settings.temperature_units == Celsius) {
			temperature = tmp_calculate_celsius_temperature(tmp_out_measurement);
		}
		else if(settings.temperature_units == Fahrenheit) {
			temperature = c_to_f(tmp_calculate_celsius_temperature(tmp_out_measurement));
		}
		tmp_enable_shutdown();
#endif
		vTaskDelay(delay_time);  // 3s
	}
}

int8_t get_temperature(void) {
	return temperature;
}

