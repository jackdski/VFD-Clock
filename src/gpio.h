/*
 * gpio.h
 *
 *  Created on: Jun 13, 2019
 *      Author: jack
 */

#ifndef GPIO_H_
#define GPIO_H_

/* Configure PA5 and PA6 as outputs to blink LEDs */
void init_error_led(void);

void init_indication_led(void);

void init_power_switch(void);

/* toggle the output on the INDICATION LED */
void toggle_indication_led(void);

/* toggle the outputs on the ERROR LED */
void toggle_error_led(void);

void clear_error_led(void);

/* toggle the outputs on the RTC LED */
//void toggle_rtc_led(void);

/* flashes an LED to indicate a response to user input */
void flash_indicator_led(void);

/* configures buttons */
void init_buttons(void);

/* returns current '+' button value */
uint8_t read_plus_button(void);

/* returns current '-' button value */
uint8_t read_minus_button(void);

/* reads current on/off switch value */
uint8_t read_power_switch(void);

/* reads the connectivity status output of the HC-10 module*/
void get_hc_10_status(void);

/* Toggles and flashes the Indication LED */
void prvIndication_LED(void *pvParameters);

/* toggles the Error LED according to the error that occured */
void prvError_LED(void * pvParameters);

#endif /* GPIO_H_ */
