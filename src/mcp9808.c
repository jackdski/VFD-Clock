/*
 * mcp9808.c
 *
 *  Created on: Jun 13, 2019
 *      Author: jack
 */
/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "i2c.h"

#define MCP9808_ADDR	0x18		// MCP9808 device ID
//#define MCP9808_ADDR	0x1F		// MCP9808 device ID


#define	CONFIG			0x01		// Configuration Register
#define CONFIG_HYST		0x06	// Tupper and Tlower limit hysteresis, bits 9-10
#define CONFIG_SHDN		0x01		// Shutdown Mode mask - BIT8
#define CONFIG_CRT_LOCK 0x80		// Tcrit lock bit, 1 = locked, TCRIT reg cannot be written to
#define CONFIG_WIN_LOCK	0x40		// Tup/Tlow window lock, 1= locked, Tup/low cannot be written
#define CONFIG_INT_CLR	0x20		// interrupt clear bit, 1= clear interrupt output
#define CONFIG_ALERT_ST	0x10		// alert output status, 1= output is asserted
#define CONFIG_ALERT_EN 0x08		// alert output control, 1 = enabled
#define CONFIG_ALERT_SL	0x04		// alert output select
#define CONFIG_ALERT_PL 0x02		// alert output polarity, 1 = active-high
#define CONFIG_ALERT_MD 0x01		// alert output mode, 1 = interrupt output

#define UPPER_BOUND		0x02		// Upper bound temp threshold register
#define LOWER_BOUND		0x03		// Lower bound temp threshold register
#define CRITICAL_BOUND	0x04		// Critical Boundary register
#define BOUND_SIGN		(1 << 12)	// sign bit (0=TA>=0C, 1=TA<0C)
#define BOUND_TEMP_MASK	0x0FFC		// mask to set temp bounds in 2's complement

#define TEMP_AMBIENT	0x05		// Ambient Temperature Register
#define IS_TCRIT		0x80		// 0 = TA < Tcrit, 0x8000
#define TA_VS_TUPPER	0x40		// 0 = TA <= Tupper, 0x4000
#define TA_VS_TLOWER	0x20		// 0 = TA >= Tlower, 0x2000
#define TA_SIGN			0x10		// 0 = TA >= 0C, 1 = TA < 0C, 0x1000
#define TEMPERATURE		0xFFF		// 12 bit mask for Ambient Temp. Bits

#define RESOLUTION		0x80		// Sensor Resolution register
#define LOW_RES			0x00		// +0.5C, tconv = 30ms
#define MEDIUM_RES		0x01		// +0.25C, tconv = 65ms
#define HIGH_RES		0x02		// +0.125C, tconv = 130ms
#define DEFAULT_RES		0x03		// +0.0625C, power-up default, tconv = 250ms


void config_temperature_sensor() {
//	init_i2c();
	// switch slave address to MCP9808
	i2c_change_sadd(MCP9808_ADDR);	// set MCP9808 as slave
	i2c_set_tx_direction();			// set to TX direction
	i2c_change_nbytes(3);			// send 3 byte
	i2c_send_start();

	/* send CONFIG reg */
	i2c_send_byte((uint8_t)CONFIG);
	i2c_send_byte(0x00);		// MSB data
	i2c_send_byte(0x00);		// LSB data

	i2c_send_stop();

//	i2c_write_reg((uintn8_t)MCP9808_ADDR, (uint8_t)CONFIG, 0x00);
}

void read_config_register() {
	// switch slave address to MCP9808
	i2c_change_sadd(MCP9808_ADDR);	// set MCP9808 as slave
	i2c_set_tx_direction();			// set to TX direction
	I2C1->CR2 &= ~I2C_CR2_AUTOEND;
	i2c_change_nbytes(3);			// send 3 byte

	i2c_send_start();				// send start but and address
	i2c_send_byte((uint8_t)CONFIG);	// want to read from CONFIG register
//	i2c_send_start();				// send repeated start
	i2c_send_stop();
	i2c_change_nbytes(2);			// receive 1 byte
	i2c_set_rx_direction();			// set to RX direction
	i2c_send_start();
//	i2c_send_byte((uint8_t)CONFIG);	// read from this register
	uint8_t upper_byte = i2c_read_byte();
	uint8_t lower_byte = i2c_read_byte();
	i2c_send_stop();
}

void enable_shutdown_mode() {
	i2c_send_start();					// START command
	i2c_send_byte(MCP9808_ADDR & 0xFE);	// WRITE command
	i2c_send_byte(0x01);				// write CONFIG register
	i2c_send_byte(0xFF & CONFIG_SHDN);	// data
	i2c_send_stop();				// STOP command
}

void disable_shutdown_mode() {
	i2c_send_start();					// START command
	i2c_send_byte(MCP9808_ADDR & 0xFE);	// WRITE command
	i2c_send_byte(0x01);				// write CONFIG register
	i2c_send_byte(0xFF & ~CONFIG_SHDN);		// data
	i2c_send_stop();				// STOP command
}

void write_mcp9808(uint32_t data) {
	// switch slave address to MCP9808
	i2c_change_sadd(MCP9808_ADDR);	// set MCP9808 as slave
	i2c_set_tx_direction();			// set to TX direction
	i2c_change_nbytes(3);			// send 3 byte
	i2c_send_start();

	/* send CONFIG reg */
	i2c_send_byte((uint8_t)CONFIG);


	i2c_send_stop();
}

uint8_t sample_temperature() {


	/* get data over I2C */
	i2c_send_start();
	i2c_send_byte(MCP9808_ADDR & 0xFE);
	i2c_send_byte(0x05);
	i2c_send_start();
	i2c_send_byte(MCP9808_ADDR | CONFIG);
	uint8_t upper_byte = i2c_read_byte();
	uint8_t lower_byte = i2c_read_byte();
	i2c_send_stop();

	/* check TA vs TCRIT */
	if((upper_byte & IS_TCRIT)) {

	}

	/* check TA vs TUPPER */
	if(upper_byte & TA_VS_TUPPER) {
	}

	/* check TA vs TLOWER */
	if(upper_byte & TA_VS_TLOWER) {
	}

	/* clear flag bits */
	upper_byte &= 0x1F;

	/* convert temperature data */
	/* TA < 0C */
	if((upper_byte & (TA_SIGN >> 8)) == 0x10) {
		upper_byte &= 0x0F;		// clear SIGN bit
		return (256 - (upper_byte * 16 + lower_byte /16));
	}
	/* TA > 0C */
	else {
		return (upper_byte * 16 + lower_byte / 16);
	}
}

