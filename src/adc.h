/*
 * adc.h
 *
 *  Created on: Jun 18, 2019
 *      Author: jack
 */

#ifndef ADC_H_
#define ADC_H_

void init_adc(void);

void calibrate_adc(void);

void enable_adc(void);

void disable_adc(void);

uint32_t sample_adc(void);

/* enables the VBAT channel
 * 	VBAT = 2 * conversion_value */
void enable_vbat_adc(void);

/* disables the VBAT channel */
void disable_vbat_adc(void);

/* will take a sample of the VBAT channel */
uint16_t read_vbat_adc(void);

/* enables the internal Voltage Reference */
void enable_vrefint_adc(void);

/* disables the internal Voltage Reference */
void disable_vrefint_adc(void);

void set_analog_watchdog_adc(uint8_t channel);

/* Input channel specified is selected for conversion
 *  @param uint8_t channel: 0-18 */
void select_adc_channel(uint8_t channel);

/* inline function that returns the 16 bit value currently in
 * the ADC Data Register (ADC_DR) */
uint32_t read_adc(void);

#endif /* ADC_H_ */
