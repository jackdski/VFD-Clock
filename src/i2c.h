/*
 * i2c.h
 *
 *  Created on: Jun 15, 2019
 *      Author: jack
 */

#ifndef I2C_H_
#define I2C_H_

//#define I2C_TIMING			0x00201D2B 	// for 100kHz  // maybe 0x00281EAD?
#define I2C_TIMING			0X0010020A	// for 400kHz
#define I2C_ADDRESS 		0xCA

void init_i2c(void);

void i2c_disable_peripheral(void);

void i2c_write_reg(uint8_t device, uint8_t reg, uint8_t data);

uint8_t i2c_read_reg(uint8_t device, uint8_t reg);

/* Send a start byte on the I2C1 line */
void i2c_send_start(void);

/* Send a stop byte on the I2C1 line */
void i2c_send_stop(void);

/* Send a byte on the I2C1 line */
void i2c_send_byte(uint8_t byte);

/* Wait for a byte of data to be available then return that byte */
uint8_t i2c_read_byte(void);

/* Changes the slave address */
void i2c_change_sadd(uint8_t addr);

/* Changes NYBYTES, number of bytes to send */
void i2c_change_nbytes(uint16_t num);

/* have master request a write transfer */
void i2c_set_tx_direction();

/* have master request a read transfer */
void i2c_set_rx_direction(void);

#endif /* I2C_H_ */
