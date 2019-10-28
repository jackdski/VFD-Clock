/*
 * tmp36.h
 *
 *  Created on: Oct 27, 2019
 *      Author: jack
 */

#ifndef TMP36_H_
#define TMP36_H_

#define TMP_ADC_CHANNEL 	9

void init_tmp(void);
int8_t tmp_calculate_celsius_temperature(uint32_t mv);
void tmp_enable_shutdown(void);
void tmp_disable_shutdown(void);

#endif /* TMP36_H_ */
