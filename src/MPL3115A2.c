/*
 * MPL3115A2.c
 *
 *  Created on: Jun 21, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "i2c.h"

/* D E F I N E S */
/* MPL3115A2 Macros */
#define MPL3115A2           0x60 // 7-bit I2C address
//#define MPL3115A2           0xC0
#define STATUS              0x00
#define OUT_T_MSB           0x04
#define OUT_T_LSB           0x05
#define OUT_T_DELTA_MSB     0x0A
#define OUT_T_DELTA_LSB     0x0B
#define WHO_AM_I            0x0C
#define CTRL_REG1           0x26
#define PT_DATA_CFG         0x13
#define ACTIVE              0xB9  //value to write


void config_temperature_sensor_mpl() {
	i2c_change_sadd(MPL3115A2);	// set MCP9808 as slave
	i2c_set_tx_direction();			// set to TX direction
	i2c_change_nbytes(3);			// send 3 byte
	i2c_send_start();

	/* send CONFIG reg */
	i2c_send_byte((uint8_t)STATUS);
	i2c_send_byte(0x00);		// MSB data
	i2c_send_byte(0x01);		// LSB data

	i2c_send_stop();
}

void read_config_mpl() {
	i2c_change_sadd(MPL3115A2);	// set MCP9808 as slave
	i2c_set_tx_direction();			// set to TX direction
	I2C1->CR2 &= ~I2C_CR2_AUTOEND;
	i2c_change_nbytes(1);			// send 3 byte

	i2c_send_start();				// send start but and address
	i2c_send_byte((uint8_t)STATUS);	// want to read from CONFIG register
//	i2c_send_stop();
	i2c_change_nbytes(2);			// receive 2 bytes
	i2c_set_rx_direction();			// set to RX direction
	i2c_send_start();				// send repeated start
	uint8_t upper_byte = i2c_read_byte();
	uint8_t lower_byte = i2c_read_byte();
	i2c_send_stop();
}

void trigger_sample_mpl(void) {
    // get and set OST low
    uint8_t setting = i2c_read_reg((uint8_t)CTRL_REG1);
    setting &= ~(1 << 1);                   // clear OST bit
    i2c_write_reg(MPL3115A2, CTRL_REG1, setting);

    // set OST high again
    setting = i2c_read_reg(CTRL_REG1);
    setting |= (1 << 1);        // set OST bit
    i2c_write_reg(MPL3115A2, CTRL_REG1, setting);
}

uint8_t read_temp_c(void) {
	trigger_sample_mpl(); // trigger a new temp sample
    uint8_t status = 0;
    uint8_t temp_c;

    // wait for new temp data
    while((status & 0x08) == 0) {
        status = i2c_read_reg(OUT_T_MSB);
    }
    temp_c = i2c_read_reg(OUT_T_MSB);
    return temp_c;
}

uint8_t convert_to_fahrenheit(uint8_t temp_c) {
    return (temp_c * 9) / 5 + 32;
}


uint8_t read_temp_f() {
    uint8_t final_temp = convert_to_fahrenheit(read_temp_c());
    return final_temp;
}

void set_mode_standby() {
    uint8_t setting = i2c_read_reg(CTRL_REG1);  // read setting
    setting &= ~(1 << 0);                       // set standby bit
    i2c_write_reg(MPL3115A2, CTRL_REG1, setting);          // write to reg
}

void set_mode_active() {
    uint8_t setting = i2c_read_reg(CTRL_REG1);  // read setting
    setting |= (1 << 0);                        // set active bit
    i2c_write_reg(MPL3115A2, CTRL_REG1, setting);          // write to reg
}


void set_oversample_rate(uint8_t sample_rate) {
  if(sample_rate > 7) sample_rate = 7;
  sample_rate <<= 3;
  uint8_t setting = i2c_read_reg(CTRL_REG1);    // read setting
  setting &= 0b11000111;                        // clear out OS bits
  setting |= sample_rate;                       // use new OS bits
  i2c_write_reg(MPL3115A2, CTRL_REG1, setting);            // write to reg
}


void enable_event_flags() {
  i2c_write_reg(MPL3115A2, PT_DATA_CFG, 0x07); // Enable all three pressure and temp event flags
}


