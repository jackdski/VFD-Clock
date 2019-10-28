/*
 * tmp35.c
 *
 *  Created on: Oct 21, 2019
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
#include "tmp36.h"
#include "vfd_typedefs.h"
#include "adc.h"


void init_tmp(void) {
	// ADC pin
	TMP36_OUT_PORT->MODER = (TMP36_OUT_PORT->MODER & ~(GPIO_MODER_MODER2)) | GPIO_MODER_MODER2_1;	// select alternate function mode
	TMP36_OUT_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL2); // // select AF0 on PB2, AFRL (Ports 0-7)

	// SHDW pin
	TMP36_SHDW_PORT->ODR &= ~(1 << TMP36_SHDW_PIN);	// set LOW
	TMP36_SHDW_PORT->MODER |= (GPIO_MODER_MODER2_0);	// set to Output
	TMP36_SHDW_PORT->OTYPER &= ~(1 << TMP36_SHDW_PIN);	// set to push-pull
	TMP36_SHDW_PORT->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR2_0;  // set to mid-speed
}


int8_t tmp_calculate_celsius_temperature(uint32_t mv) {
	return ((mv - 500)/10);
}

int8_t c_to_f(int8_t celsius) {
	return (((celsius * 9) / 5) + 32);
}

void tmp_enable_shutdown(void) {
	TMP36_SHDW_PORT->ODR |= (1 << TMP36_SHDW_PIN);
}

void tmp_disable_shutdown(void) {
	TMP36_SHDW_PORT->ODR &= ~(1 << TMP36_SHDW_PIN);
}
