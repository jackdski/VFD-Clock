/*
 * adc.h
 *
 *  Created on: Jun 18, 2019
 *      Author: jack
 */

#ifndef ADC_H_
#define ADC_H_

//#define DEMO
#define	MAX_ADC_READ			180
#define USE_BOTH_PHOTORESISTORS

#ifdef	DEMO
#define	PHOTORESISTOR_LEFT		6	// PC6 - AF0, CH6
#define PHOTORESISTOR_RIGHT		7	// PA7 - AF0, CH7
#else
#define	PHOTORESISTOR_LEFT		2	// PA2 - AF0, CH2
#define PHOTORESISTOR_RIGHT		9	// PB1 - AF0, CH9
#endif


#define RANGE_THRESHOLD_1	5
#define RANGE_THRESHOLD_2	20
#define RANGE_THRESHOLD_3	40
#define RANGE_THRESHOLD_4	70

void prvLight_Task(void *pvParameters);

void init_adc(void);
void calibrate_adc(void);
void enable_adc(void);
void disable_adc(void);
uint32_t sample_adc(void);

/* enables the VBAT channel
 * 	VBAT = 2 * conversion_value */
void enable_vbat_adc(void);


void disable_vbat_adc(void); /* disables the VBAT channel */

/* will take a sample of the VBAT channel */
uint32_t read_vbat_adc(void);

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

/* sets the ADC Watchdog threshold register with values */
void set_adc_watchdog_thresholds(uint32_t avg_light);

/* returns 1 if old and new values are in different range, 0 if in same range */
uint8_t in_diff_light_range(uint32_t old_value, uint32_t new_value);

/* checks if the ADC values given are close enough to trust */
uint8_t is_good_light_data(uint32_t sample_one, uint32_t sample_two);


#endif /* ADC_H_ */
