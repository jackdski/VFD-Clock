/*
 * MPL3115A2.h
 *
 *  Created on: Jun 21, 2019
 *      Author: jack
 */

#ifndef MPL3115A2_H_
#define MPL3115A2_H_


void config_temperature_sensor_mpl();

void read_config_mpl(void);

void check_whoami_mpl(void);

void trigger_sample_mpl(void);

uint8_t read_temp_c(void);

uint8_t convert_to_fahrenheit(uint8_t temp_c);

uint8_t read_temp_f();

void set_mode_standby();

void set_mode_active();

void set_oversample_rate(uint8_t sample_rate);

void enable_event_flags();

#endif /* MPL3115A2_H_ */
