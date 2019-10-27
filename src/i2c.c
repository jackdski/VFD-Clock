/*
 * i2c.c
 *
 *  Created on: Jun 15, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "main.h"
#include "i2c.h"
#include "MPL3115A2.h"

extern int8_t temperature;


void init_i2c(I2C_TypeDef * I2Cx) {
	I2Cx->CR1 &= ~I2C_CR1_PE;

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; /* Enable Peripheral clock on GPIOB */

	/* select alternate function mode */
	I2C_GPIO_PORT->MODER = (I2C_GPIO_PORT->MODER & ~(GPIO_MODER_MODER11 | GPIO_MODER_MODER12))
			| (GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1);

	// select AF5 on both PA11 and PA12
	I2C_GPIO_PORT->AFR[1] |=  ((0x05 << GPIO_AFRL_AFRL3_Pos)
					  |(0x05 << GPIO_AFRL_AFRL4_Pos)); // AFRH (Ports 8-15)

	I2C_GPIO_PORT->OTYPER |= (GPIO_OTYPER_OT_11 | GPIO_OTYPER_OT_12);

	I2C_GPIO_PORT->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR11_0 | GPIO_OSPEEDER_OSPEEDR12_0);

	I2C_GPIO_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR11 | GPIO_PUPDR_PUPDR12);

	RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C2RST);

	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // enable I2C clock

	I2Cx->TIMINGR = (uint32_t)I2C_TIMING;
	I2Cx->CR2 &= ~(I2C_CR2_ADD10); // select 7-bit address mode
	I2Cx->CR1 |= (I2C_CR1_RXIE | I2C_CR1_TXIE | I2C_CR1_NACKIE |
					I2C_CR1_TCIE | I2C_CR1_ERRIE);
	I2Cx->CR1 |= I2C_CR1_PE;

	/* Check readability */
	check_whoami_mpl();		// make sure sensor is available
	read_config_mpl();		//
	trigger_sample_mpl();
}


void i2c_write_reg(uint8_t device, uint8_t reg, uint8_t data) {
	i2c_change_sadd((uint8_t)device);	// set slave address

	while((SENSOR_I2C->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	i2c_set_tx_direction();			// set to TX direction
	i2c_change_nbytes(2);			// send 1 byte

	if ((SENSOR_I2C->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
		SENSOR_I2C->TXDR = (uint8_t)reg;
		SENSOR_I2C->CR2 |= I2C_CR2_START; /* Go */
	}

	while(!(SENSOR_I2C->ISR & I2C_ISR_TXE)) {
		if((SENSOR_I2C->TXDR & I2C_ISR_ARLO) == 1) {
			return;
		}
	}

	if ((SENSOR_I2C->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
		SENSOR_I2C->TXDR = (uint8_t)data;
	}

	while(!(SENSOR_I2C->ISR & I2C_ISR_TXE)) {
		if((SENSOR_I2C->TXDR & I2C_ISR_ARLO) == 1) {
			return;
		}
	}
	i2c_send_stop();
}


uint8_t i2c_read_reg(uint8_t device, uint8_t reg) {
	i2c_change_sadd((uint8_t)device);	// set slave address

	while((SENSOR_I2C->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	i2c_set_tx_direction();			// set to TX direction
	i2c_change_nbytes(1);			// send 1 byte

	if ((SENSOR_I2C->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
		SENSOR_I2C->TXDR = (uint8_t)reg;
		SENSOR_I2C->CR2 |= I2C_CR2_START; /* Go */
	}

//	while((I2CX->ISR & I2C_ISR_TC) == 0) {
	while(!(SENSOR_I2C->ISR & I2C_ISR_TXE)) {
		if((SENSOR_I2C->TXDR & I2C_ISR_ARLO) == 1) {
			return 0xFF;
		}
	}

	SENSOR_I2C->CR2 = 0x00000000;
	i2c_change_nbytes(1);
	i2c_change_sadd((uint8_t)device);	// set MPL3115A2 as slave
	i2c_set_rx_direction();			// set to RX direction
	i2c_send_start();				// send repeated start

	uint8_t data = 0xFF;
	while((SENSOR_I2C->ISR & I2C_ISR_TC) == 0) {
		if((SENSOR_I2C->ISR & I2C_ISR_RXNE)) {
			data = SENSOR_I2C->RXDR;
		}
	}

	i2c_send_stop();
	return data;
}


void i2c_disable_peripheral(void) {
	SENSOR_I2C->CR1 &= ~I2C_CR1_PE;
//	I2C2->CR1 &= ~I2C_CR1_PE;
}

/* Send a start byte on the I2C1 line */
void inline i2c_send_start(void) {
	SENSOR_I2C->CR2 |= I2C_CR2_START;
//	I2C2->CR2 |= I2C_CR2_START;
//	while((I2C2->CR2 & I2C_CR2_START));
}

/* Send a stop byte on the I2C1 line */
void inline i2c_send_stop(void) {
	SENSOR_I2C->CR2 |= I2C_CR2_STOP;
	while((SENSOR_I2C->CR2 & I2C_CR2_STOP));
	SENSOR_I2C->ICR |= (I2C_ICR_STOPCF);  // reset the ICR event flag
	while((SENSOR_I2C->ICR & I2C_ICR_STOPCF));
//	I2C2->CR2 |= I2C_CR2_STOP;
//	while((I2C2->CR2 & I2C_CR2_STOP));
//	I2C2->ICR |= (I2C_ICR_STOPCF);  // reset the ICR event flag
//	while((I2C2->ICR & I2C_ICR_STOPCF));
}

/* Send a byte on the I2C1 line */
void i2c_send_byte(uint8_t byte) {
	i2c_send_start();
	//Check Tx empty before writing to it
	if((SENSOR_I2C->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
		SENSOR_I2C->TXDR = byte;
	}
	else {
		//Wait for TX Register to clear
		while((SENSOR_I2C->ISR & I2C_ISR_TXE) == 0);
		SENSOR_I2C->TXDR = byte;
	}
	/* wait for TX interrupt status and Transfer Complete bits to be set low */
	while(!(SENSOR_I2C->ISR & (I2C_ISR_TXIS | I2C_ISR_TC)));
//	//Check Tx empty before writing to it
//	if((I2C2->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
//		I2C2->TXDR = byte;
//	}
//	else {
//		//Wait for TX Register to clear
//		while((I2C2->ISR & I2C_ISR_TXE) == 0);
//		I2C2->TXDR = byte;
//	}
//	/* wait for TX interrupt status and Transfer Complete bits to be set low */
// 	while(!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_TC))) {
// 		if(I2C2->ISR & I2C_ISR_ARLO) {
// 			break;
// 		}
// 	}
}

/* Wait for a byte of data to be available then return that byte */
uint8_t i2c_read_byte(void) {
	while(!(SENSOR_I2C->ISR & I2C_ISR_RXNE) == 0) {
		if(SENSOR_I2C->ISR & I2C_ISR_ARLO) {
			break;
		}
	}
	return (SENSOR_I2C->RXDR & 0xFF);
//	while((I2C2->ISR & I2C_ISR_RXNE) == 0) {
//		if(I2C2->ISR & I2C_ISR_ARLO) {
// 			break;
// 		}
//	}
//	return (I2C2->RXDR & 0xFF);
}

/* Changes the slave address */
void i2c_change_sadd(uint8_t addr) {
	// Clear slave address and use 7-bit addresses
	SENSOR_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_ADD10);
	SENSOR_I2C->CR2 |= (addr << 1);
//	I2C2->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_ADD10);
//	I2C2->CR2 |= (addr << 1);
}

/* Changes NYBYTES, number of bytes to send */
void i2c_change_nbytes(uint16_t num) {
	SENSOR_I2C->CR2 &= ~(I2C_CR2_NBYTES);
	SENSOR_I2C->CR2 = (I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (num << I2C_CR2_NBYTES_Pos);
//	I2C2->CR2 &= ~(I2C_CR2_NBYTES);
//	I2C2->CR2 = (I2C2->CR2 & ~(I2C_CR2_NBYTES)) | (num << I2C_CR2_NBYTES_Pos);
}

/* have master request a write transfer */
void inline i2c_set_tx_direction(void) {
	SENSOR_I2C->CR2 &= ~I2C_CR2_RD_WRN;
//	I2C2->CR2 &= ~I2C_CR2_RD_WRN;
}

/* have master request a read transfer */
void inline i2c_set_rx_direction(void) {
	SENSOR_I2C->CR2 |= I2C_CR2_RD_WRN;
//	I2C2->CR2 |= I2C_CR2_RD_WRN;
}

void I2C1_IRQHandler(void) {

}

