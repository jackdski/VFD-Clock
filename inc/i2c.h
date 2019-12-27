/*
 * i2c.h
 *
 *  Created on: Jun 15, 2019
 *      Author: jack
 */

#ifndef I2C_H_
#define I2C_H_

/*	D E F I N E S */
#define I2C_TIMING			0X0010020A	// for 400kHz
#define I2C_ADDRESS 		0xCA

//#define	DEMO


/*	E N U M S   */
/* Enum that defines the possible states of the state machine */
typedef enum {
	Start,
	Reg_Send,
	Data_Send,
	Restart,
	Slave_Data,
	Stop
} I2C_State_Machine_E;

/* Enum to select in the Payload struct if the I2C transmission is reading or writing */
typedef enum {
	I2C_Read = 0,
	I2C_Write = 1
} I2C_Read_Write;


/*	S T R U C T S   */
/* Define the I2C operation and keeps state of the I2C operation */
typedef struct {
	I2C_TypeDef * I2Cx;
	I2C_State_Machine_E system_state;
	uint8_t device_address;
	uint8_t reg_address;
	I2C_Read_Write r_w;
	uint16_t * rx_buffer;
	//	uint8_t * tx_buffer;
	//	uint8_t n_bytes;
} I2C_Payload_t;


/*	F U N C T I O N S	*/
void init_i2c(I2C_TypeDef * I2Cx);
void i2c_disable_peripheral(void);
void i2c_write_reg(uint8_t device, uint8_t reg, uint8_t data);
uint8_t i2c_read_reg(uint8_t device, uint8_t reg);
void i2c_send_start(void);				/* Send a start byte on the I2C1 line */
void i2c_send_stop(void);				/* Send a stop byte on the I2C1 line */
void i2c_send_byte(uint8_t byte);  		/* Send a byte on the I2C1 line */
uint8_t i2c_read_byte(void);			/* Wait for a byte of data to be available then return that byte */
void i2c_change_sadd(uint8_t addr);		/* Changes the slave address */
void i2c_change_nbytes(uint16_t num);	/* Changes NYBYTES, number of bytes to send */
void i2c_set_tx_direction();			/* have master request a write transfer */
void i2c_set_rx_direction(void);		/* have master request a read transfer */

#endif /* I2C_H_ */
