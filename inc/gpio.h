/*
 * gpio.h
 *
 *  Created on: Jun 13, 2019
 *      Author: jack
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "vfd_typedefs.h"

/* Configure PA5 and PA6 as outputs to blink LEDs */
void init_error_led(void);
void set_error_led_status(Light_Flash_E status);
void init_indication_led(void);
void set_indication_led_status(Light_Flash_E status);
void init_power_switch(void);
Power_Switch_status_E get_power_switch_status(void);

void init_efuse_pins(void);
void efuse_enable(void);
void efuse_disable(void);

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
void update_hc_10_status(void);

Button_Status_E get_plus_button_status(void);
Button_Status_E get_minus_button_status(void);

/* Toggles and flashes the Indication LED */
void prvIndication_LED(void *pvParameters);

/* toggles the Error LED according to the error that occured */
void prvError_LED(void * pvParameters);

#endif /* GPIO_H_ */
