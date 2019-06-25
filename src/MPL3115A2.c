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
