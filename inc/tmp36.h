/*
 * tmp36.h
 *
 *  Created on: Oct 27, 2019
 *      Author: jack
 */

#ifndef TMP36_H_
#define TMP36_H_

#define TMP_ADC_CHANNEL 	8

void init_tmp(void);
int8_t tmp_calculate_celsius_temperature(uint32_t mv);
int8_t c_to_f(int8_t celsius);
void tmp_enable_shutdown(void);
void tmp_disable_shutdown(void);

#endif /* TMP36_H_ */
