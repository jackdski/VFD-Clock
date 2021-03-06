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
#include "usart.h"
#include "adc.h"
#include "rtc.h"
#include "tubes.h"
#include "pwm.h"
#include "MPL3115A2.h"
#include "tmp36.h"
#include "callbacks.h"
#include "low_power.h"

// globals
extern CircBuf_t * TX_Buffer;
extern CircBuf_t * RX_Buffer;

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

//extern uint8_t hours;		/* 1-12 */
//extern uint8_t minutes;		/* 0-59 */
//extern uint8_t seconds;		/* 0-59 */
extern uint8_t ampm;
extern int8_t temperature;	/* -128 - 127 */

extern uint16_t display_brightness;
static uint32_t target_brightness;

extern TimerHandle_t three_sec_timer;
extern TimerHandle_t five_sec_timer;
extern TimerHandle_t ten_sec_timer;

extern Button_Status_E plus_button_status;
extern Button_Status_E minus_button_status;
extern Light_Flash_E indication_light_status;
extern Light_Flash_E error_light_status;
extern HC_10_Status_E ble_status;

#define RTC_MAX_WAIT_TICKS		pdMS_TO_TICKS(100)


/*	T A S K S   */

/* if given the semaphore by the RTC Interrupt Handler, read the time and update the tube display */
void prvRTC_Task(void *pvParameters) {
	static uint32_t thread_notification;
	static uint8_t hours = 12;		/* 1-12*/
	static uint8_t minutes = 0;		/* 0-59 */
	static uint8_t seconds = 0;		/* 0-59 */
//	static uint8_t ampm = PM;

	/* if woken up from standby reset values */
	if((uint32_t)pvParameters == Standby_Wakeup) {
    	hours = read_rtc_hours();
    	minutes = read_rtc_minutes();
    	seconds = read_rtc_seconds();
//    	ampm = read_rtc_ampm();
	}

	for( ;; ) {
		thread_notification = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(thread_notification != 0) {
//			toggle_rtc_led();
			hours = read_rtc_hours();
			minutes = read_rtc_minutes();
			seconds = read_rtc_seconds();
			update_time(hours, minutes, seconds);	/* update tubes */
		}
	}
}

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
	static Temperature_Type_E temperature_units;
	temperature_units = (Temperature_Type_E)pvParameters;

	for( ;; ) {
#ifdef	USE_I2C
		check_whoami_mpl();  // make sure sensor is available
		trigger_sample_mpl();
		temperature = read_temp_c();
#else
		tmp_disable_shutdown();
		select_adc_channel(TMP_ADC_CHANNEL);
		tmp_out_measurement = sample_adc();
		if(temperature_units == Celsius) {
			temperature = tmp_calculate_celsius_temperature(tmp_out_measurement);
		}
		else if(temperature_units == Fahrenheit) {
			temperature = c_to_f(tmp_calculate_celsius_temperature(tmp_out_measurement));
		}
		tmp_enable_shutdown();
#endif
		vTaskDelay(delay_time);  // 3s
	}
}

void prvTemperature_Button_Task(void *pvParameters) {
	static uint32_t thread_notification;
	for( ;; ) {
		thread_notification = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(thread_notification != 0) {
			check_whoami_mpl();  // make sure sensor is available
			trigger_sample_mpl();
			temperature = read_temp_c();
		}
	}
}

/* updates the light_value private variable every 5 seconds
 * so that the prvChangePWM task changes the brightness */
void prvLight_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 2000 ); // 5s
	static uint32_t light_value = 200;
	static uint32_t light_sample;
	static uint32_t light_sample_two;
	for( ;; ) {
		vTaskDelay(delay_time);	// 1s
		select_adc_channel(PHOTORESISTOR_LEFT);
		light_sample = sample_adc();

#ifdef USE_BOTH_PHOTORESISTORS
		select_adc_channel(PHOTORESISTOR_RIGHT);
		light_sample_two = sample_adc();
		light_sample = (light_sample + light_sample_two) / 2;
#endif
		/* check if ADC conversions are close enough to trust */
		if(is_good_light_data(light_sample, light_sample_two)) {
			/* if in a new range now */
			if(in_diff_light_range(light_value, light_sample) == 1) {
				target_brightness = calc_new_brightness(light_sample);
				vTaskResume( thBrightness_Adj );
			}
			light_value = light_sample;
		}
	}
}

/* task is resumed in prvLight_Task after it is determined that the
 * display brightness should change. This task alters the duty cycle
 * by +/-1% every 50ms and should be configured as a low-priority task */
void prvChange_Brightness_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 50 );		// 50ms
	static uint16_t display_brightness = STARTING_DISPLAY_BRIGHTNESS;

	/* change display_brightness if parameter was passed into task function */
	if((uint32_t)pvParameters <= 100) {
		display_brightness = (uint16_t)(uint32_t)pvParameters;
	}

	for( ;; ) {
//		display_brightness = (TIM14->CCR1 * 100) / (uint16_t)PWM_FREQUENCY;
		display_brightness = get_pwm_duty_cycle();
		if(display_brightness == target_brightness) {
		     vTaskSuspend( NULL );	// suspend this task
		}
		if(display_brightness != target_brightness) {
			if(display_brightness > target_brightness)
				display_brightness -= 1;
			else if(display_brightness < target_brightness)
				display_brightness += 1;
			change_pwm_duty_cycle(display_brightness);
		}
		vTaskDelay(delay_time);
	}
}

/* sends that is in the TX message buffer every 1s */
void prvBLE_Send_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 1000 ); // 1s
	for( ;; ) {
		/* make sure that buffer is not empty */
		if(!is_empty_CircBuf(TX_Buffer)) {
			uart_send_bytes(TX_Buffer->head,TX_Buffer->num_items);
			reset_CircBuf(TX_Buffer);
		}
		vTaskDelay(delay_time);
	}
}

/* if there is a BLE connection, then this task will read the BLE RX message queue if it is not empty */
void prvBLE_Receive_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 1000 );  // 1ms

	/* strings to be sent */
	static uint8_t temp_msg[] = "TEMP";
	static uint8_t date_msg[] = "DATE";
	static uint8_t chg_date_msg[] = "DATE:";
	static uint8_t autobright_msg[] = "AUTOBRIGHT:";
	static uint8_t autobright_on_msg[] = "AUTOBRIGHT:ON";
	static uint8_t autobright_off_msg[] = "AUTOBRIGHT:OFF";
	static uint8_t time_msg[] = "TIME:";
	static uint8_t turnoff_msg[] = "TURNOFF";

	/* strings that should be received */
	static uint8_t wake_up_msg[] = "OK+WAKE";
	static uint8_t sleep_msg[] = "OK+SLEEP";
	static uint8_t name_msg[] = "OK+SetNAME";
//	static uint8_t baud_one[] = "OK+Set:9600";

	for( ;; ) {
	    get_hc_10_status();

	    /* if there is a message, format it */
	    if(RX_Buffer->num_items > 0) {
			size_t n = RX_Buffer->num_items;
			size_t i;
			uint8_t xRXMessage[n+1];
			for(i = 0; i < n; i++) {
				xRXMessage[i] = remove_item(RX_Buffer);
			}
			xRXMessage[n] = '\0';

			/* capitalize message */
			for(i = 0; i < n; i++) {
				xRXMessage[i] = toupper(xRXMessage[i]);
			}

			// if in naming mode make sure things are proper
			if(ble_status == Naming) {

			}

			/* if connected, determine see what to do with the rx'd message */
			if(ble_status == Connected) {
				// "TEMP"
				if(strcmp((const char *)xRXMessage, (const char *)temp_msg) == 0) {
					system_state = BLE_Temperature;
					char * msg = "TEMP:OK\0";
					load_str_to_CircBuf(TX_Buffer, msg, 8);
					toggle_error_led();
					vTaskSuspend(thRTC);
					display_temperature(temperature);
					xTimerStart(five_sec_timer, pdMS_TO_TICKS(100));
				}
				// "DATE"
				else if(strcmp((const char *)xRXMessage, (const char *)date_msg) == 0) {
					system_state = BLE_Date;
					char * msg = "DATE:OK\0";
					load_str_to_CircBuf(TX_Buffer, msg, 8);
					toggle_error_led();
					vTaskSuspend(thRTC);
					display_date();
					xTimerStart(five_sec_timer, pdMS_TO_TICKS(100));
				}
				// "DATE:XX:XX"
				else if(strncmp((const char *)xRXMessage, (const char *)chg_date_msg, 5) == 0) {
					uint8_t temp_day, temp_month = 0xFF;
					// change Month to xRXMessage[5:6]
					if((xRXMessage[5] >= '0') && (xRXMessage[5] < '2') && (xRXMessage[6] >= '0') && (xRXMessage[6] <= '9'))
						temp_month = ((xRXMessage[5] - 48) * 10) + (xRXMessage[6] - 48);
					if(xRXMessage[7] != ':')
						temp_month = 0xFF;
					// change Day to xRXMessage[8:9]
					if((xRXMessage[8] >= '0') && (xRXMessage[8] <= '3') && (xRXMessage[9] >= '0') && (xRXMessage[9] <= '9'))
						temp_day = ((xRXMessage[8] - 48) * 10) + (xRXMessage[9] - 48);
					// make sure that Month and Day values are valid, send "MSGFAIL" if not
					if(temp_month == 0xFF || temp_day == 0xFF) {
						uart_send_msgfail();
					}
					else {
						change_rtc_date(temp_month, temp_day);
						char * msg = "Date:OK\0";
						load_str_to_CircBuf(TX_Buffer, msg, 8);
					}
				}
				// "AUTOBRIGHT
				else if(strncmp((const char *)xRXMessage, (const char *)autobright_msg, 11) == 0) {
					// "AUTOBRIGHT:ON"
					if(strcmp((const char *)xRXMessage, (const char *)autobright_on_msg) == 0) {
						char * msg = "Brightness ON\0";
						load_str_to_CircBuf(TX_Buffer, msg, 14);
						vTaskResume( thAutoBrightAdj );		// resume task
					}
					// "AUTOBRIGHT:OFF"
					else if(strcmp((const char *)xRXMessage, (const char *)autobright_off_msg) == 0) {
						char * msg = "Brightness OFF\0";
						load_str_to_CircBuf(TX_Buffer, msg, 15);
						vTaskSuspend( thAutoBrightAdj );	// suspend task until On msg received
					}
					else {
						// "AUTOBRIGHT:XX"
						vTaskSuspend( thAutoBrightAdj );	// suspend task until On msg received
						// check if in correct range
						if((xRXMessage[11] >= '0') && (xRXMessage[11] <= '9') && (xRXMessage[12] >= '0') && (xRXMessage[12] <= '9')) {
							// change brightness to xRXMessage[11:12]
							char * msg = "BRIGHTNESS:OK\0";
							load_str_to_CircBuf(TX_Buffer, msg, 14);
							target_brightness = ((xRXMessage[11] - 48) * 10) + (xRXMessage[12] - 48);
							vTaskResume( thBrightness_Adj );		// resume task that changes brightness
						}
						else {  // not in range 0-99
							char * msg = "Use values between 0 and 99\0";
							load_str_to_CircBuf(TX_Buffer, msg, 29);
						}
					}
				}
				// TIME:XX:XX:X
				else if(strncmp((const char *)xRXMessage, (const char *)time_msg, 5) == 0) {
					uint8_t temp_hours, temp_mins, temp_ampm = 0xFF;
					// change hours to xRXMessage[5:6]
					if((xRXMessage[5] >= '0') && (xRXMessage[5] < '2') && (xRXMessage[6] >= '0') && (xRXMessage[6] <= '9')) {
						temp_hours = ((xRXMessage[5] - 48) * 10) + (xRXMessage[6] - 48);
					}

					if(xRXMessage[7] != ':') {
						temp_hours = 0xFF;
					}

					// change minutes to xRXMessage[8:9]
					if((xRXMessage[8] >= '0') && (xRXMessage[8] <= '5') && (xRXMessage[9] >= '0') && (xRXMessage[9] <= '9')) {
						temp_mins = ((xRXMessage[8] - 48) * 10) + (xRXMessage[9] - 48);
					}

					if(xRXMessage[10] != ':') {
							temp_mins = 0xFF;
					}

					// set am/pm value
					if(xRXMessage[11] == 'A') {
						temp_ampm = 0;	// set RTC to AM
					}
					else if(xRXMessage[11] == 'P') {
						temp_ampm = 1;	// set RTC to PM
					}

					// make sure no errors exist and update time/display
					if(temp_hours == 0xFF || temp_mins == 0xFF || temp_ampm == 0xFF) {
						uart_send_msgfail();
					}
					// change time on RTC and update
					else {
						change_rtc_time(temp_hours, temp_mins, 0, temp_ampm);
						update_time(temp_hours, temp_mins, 0);
					}
				}
				// "TURNOFF"
				else if(strcmp((const char *)xRXMessage, (const char *)turnoff_msg) == 0) {
					char * msg = "Remote Turn Off not available\0";
					load_str_to_CircBuf(TX_Buffer, msg, 30);
				}
				else {
					// send "MSGFAIL" back since an incorrect message was received
					uart_send_msgfail();
				}
				reset_CircBuf(RX_Buffer);
			}

			/* check if it is a response to an HC-10 command
			 * status will not be Connected since the device will either be
			 *   waking up or going to sleep */
			else if(ble_status != Connected) {
				if(strcmp((const char *)xRXMessage, (const char *)wake_up_msg) == 0) {
					get_hc_10_status();
				}
				else if(strcmp((const char *)xRXMessage, (const char *)sleep_msg) == 0) {
					ble_status = Sleep;
				}
				else {
					ble_status = BLE_Error;
					error_light_status = Flashing;
				}
			}
	    }
		vTaskDelay(delay_time);
	}
}

/* This task will effectively reset the device when it is turned back on.
 *  This mode "stops all the clocks in the core supply domain and disables
 *  the PLL and the HSI, HSI48, HSI14 and HSE oscillators" and "SRAM and
 *  register contents are lost except for registers in the RTC domain and
 *  Standby circuitry". */
void prvTurnOffTask(void *pvParameters) {
	static uint32_t thread_notification;
	for( ;; ) {
		thread_notification = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(thread_notification != 0) {
			// make sure VBAT is available/has enough charge left before going to sleep
			uint32_t vbat_measurement =  read_vbat_adc();
			if(vbat_measurement > VBAT_MINIMUM) {
				configure_for_deepsleep();
				__WFI();	// enter DeepSleep/Standby Mode
			}
			else {
				configure_for_stopmode();
				__WFI();	// enter Stop Mode

			}
		}
	}
}

