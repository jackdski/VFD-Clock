/*
 * gpio.h
 *
 *  Created on: Jun 13, 2019
 *      Author: jack
 */

#ifndef GPIO_H_
#define GPIO_H_

/* Configure PA5 and PA6 as outputs to blink LEDs */
void init_led(void);

void init_power_switch(void);

/* toggle the outputs on PA5 */
void toggle_led(void);

/* toggle the outputs on PA0 */
void toggle_error_led(void);

/* toggle the outputs on PA1 */
void toggle_rtc_led(void);

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

/* LED response to user interaction */
void prvBlink_LED(void *pvParameters);

/* toggles the Error LED according to the error that occured */
void prvError_LED(void * pvParameters);

void increase_minutes(uint8_t mins);

void decrease_minutes(uint8_t mins);

void increase_hours(void);

void decrease_hours(void);

#endif /* GPIO_H_ */
