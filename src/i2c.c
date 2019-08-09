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
#include "i2c.h"
#include "MPL3115A2.h"

extern int8_t temperature;

//#define I2C_TIMING			0x00201D2B 	// for 100kHz  // maybe 0x00281EAD?
//#define I2C_TIMING			0X0010020A	// for 400kHz
//#define I2C_ADDRESS 		0xCA

#define	DEMO

//#define	USE_I2C1
#ifdef	USE_I2C1
#define I2CX	I2C1
#else
#define	I2CX	I2C2
#endif

void init_i2c(void) {
#ifdef	USE_I2C1
	I2C1->CR1 &= ~I2C_CR1_PE;

	/* select alternate function mode */
	GPIOF->MODER = (GPIOF->MODER & ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1))
			| (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1);

	// select AF1 on PF0 and PF1
	GPIOF->AFR[0] |=  ((0x01 << GPIO_AFRL_AFRL0_Pos)
					  |(0x01 << GPIO_AFRL_AFRL1_Pos)); // AFRL (Ports 0-7)

	/* select alternate function mode */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10))
			| (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

	GPIOA->OTYPER |= (GPIO_OTYPER_OT_9 | GPIO_OTYPER_OT_10);

	GPIOA->PUPDR |=     ( GPIO_PUPDR_PUPDR9_0	// pull-up
						| GPIO_PUPDR_PUPDR10_0	// pull-up
		);

	// select AF4 on PA9 and PA10
	GPIOA->AFR[1] |=  ((0x04 << GPIO_AFRH_AFRH1_Pos)
					  |(0x04 << GPIO_AFRH_AFRH2_Pos)); // AFRL (Ports 0-7)

	RCC->AHBENR |= RCC_AHBENR_GPIOFEN; /* Enable Peripheral clock on GPIOF */
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // enable I2C clock

	RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C1RST);

	I2C1->TIMINGR = (uint32_t)I2C_TIMING;
	I2C1->CR1 |= I2C_CR1_PE;

#else
	I2C2->CR1 &= ~I2C_CR1_PE;

#ifdef	DEMO

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; /* Enable Peripheral clock on GPIOB */

	/* select alternate function mode */
	GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11))
			| (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);

	GPIOB->OTYPER |= (GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_11);

	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR10_0 | GPIO_OSPEEDER_OSPEEDR11_0);

	GPIOB->PUPDR &=    ~(GPIO_PUPDR_PUPDR10 | GPIO_PUPDR_PUPDR11);

	// select AF1 on both PB10 and PB11
	GPIOB->AFR[1] |=  ((0x01 << GPIO_AFRL_AFRL2_Pos)
					  |(0x01 << GPIO_AFRL_AFRL3_Pos)); // AFRH (Ports 8-15)

#else
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; /* Enable Peripheral clock on GPIOA */

	/* select alternate function mode */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER11 | GPIO_MODER_MODER12))
			| (GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1);

	GPIOA->OTYPER |= (GPIO_OTYPER_OT_11 | GPIO_OTYPER_OT_12);

	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR11_0 | GPIO_OSPEEDER_OSPEEDR12_0);

	GPIOA->PUPDR &=    ~(GPIO_PUPDR_PUPDR11 | GPIO_PUPDR_PUPDR12);

	// select AF5 on both PA11 and PA12
	GPIOB->AFR[1] |=  ((0x05 << GPIO_AFRL_AFRL3_Pos)
					  |(0x05 << GPIO_AFRL_AFRL4_Pos)); // AFRH (Ports 8-15)
#endif

	RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C2RST);

	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // enable I2C clock

	I2C2->TIMINGR = (uint32_t)I2C_TIMING;
	I2C2->CR2 &= ~(I2C_CR2_ADD10); // select 7-bit address mode
	I2C2->CR1 |= I2C_CR1_PE;
#endif

	check_whoami_mpl();		// make sure sensor is available
	read_config_mpl();		//
	trigger_sample_mpl();
}


void i2c_write_reg(uint8_t device, uint8_t reg, uint8_t data) {
	i2c_change_sadd((uint8_t)device);	// set slave address

//	while((I2C2->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
//	i2c_set_tx_direction();			// set to TX direction
//	i2c_change_nbytes(2);			// send 1 byte
//
//	if ((I2C2->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
//		I2C2->TXDR = (uint8_t)reg;
//		I2C2->CR2 |= I2C_CR2_START; /* Go */
//	}
//
//	while(!(I2C2->ISR & I2C_ISR_TXE)) {
//		if((I2C2->TXDR & I2C_ISR_ARLO) == 1) {
//			return;
//		}
//	}
//
//	if ((I2C2->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
//		I2C2->TXDR = (uint8_t)data;
//	}
//
//	while(!(I2C2->ISR & I2C_ISR_TXE)) {
//		if((I2C2->TXDR & I2C_ISR_ARLO) == 1) {
//			return;
//		}
//	}

	while((I2CX->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	i2c_set_tx_direction();			// set to TX direction
	i2c_change_nbytes(2);			// send 1 byte

	if ((I2CX->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
		I2CX->TXDR = (uint8_t)reg;
		I2CX->CR2 |= I2C_CR2_START; /* Go */
	}

	while(!(I2CX->ISR & I2C_ISR_TXE)) {
		if((I2CX->TXDR & I2C_ISR_ARLO) == 1) {
			return;
		}
	}

	if ((I2CX->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
		I2CX->TXDR = (uint8_t)data;
	}

	while(!(I2CX->ISR & I2C_ISR_TXE)) {
		if((I2CX->TXDR & I2C_ISR_ARLO) == 1) {
			return;
		}
	}
	i2c_send_stop();
}


uint8_t i2c_read_reg(uint8_t device, uint8_t reg) {
	i2c_change_sadd((uint8_t)device);	// set slave address

//	while((I2C2->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
//	i2c_set_tx_direction();			// set to TX direction
//	i2c_change_nbytes(1);			// send 1 byte
//
//	if ((I2C2->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
//		I2C2->TXDR = (uint8_t)reg;
//		I2C2->CR2 |= I2C_CR2_START; /* Go */
//	}
//
////	while((I2C2->ISR & I2C_ISR_TC) == 0) {
//	while(!(I2C2->ISR & I2C_ISR_TXE)) {
//		if((I2C2->TXDR & I2C_ISR_ARLO) == 1) {
//			return 0xFF;
//		}
//	}
//
//	I2C2->CR2 = 0x00000000;
//	i2c_change_nbytes(1);
//	i2c_change_sadd((uint8_t)device);	// set MPL3115A2 as slave
//	i2c_set_rx_direction();			// set to RX direction
//	i2c_send_start();				// send repeated start
//
//	uint8_t data = 0xFF;
//	while((I2C2->ISR & I2C_ISR_TC) == 0) {
//		if((I2C2->ISR & I2C_ISR_RXNE)) {
//			data = I2C2->RXDR;
//		}
//	}

	while((I2CX->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	i2c_set_tx_direction();			// set to TX direction
	i2c_change_nbytes(1);			// send 1 byte

	if ((I2CX->ISR & I2C_ISR_TXE) == I2C_ISR_TXE) {
		I2CX->TXDR = (uint8_t)reg;
		I2CX->CR2 |= I2C_CR2_START; /* Go */
	}

//	while((I2CX->ISR & I2C_ISR_TC) == 0) {
	while(!(I2CX->ISR & I2C_ISR_TXE)) {
		if((I2CX->TXDR & I2C_ISR_ARLO) == 1) {
			return 0xFF;
		}
	}

	I2CX->CR2 = 0x00000000;
	i2c_change_nbytes(1);
	i2c_change_sadd((uint8_t)device);	// set MPL3115A2 as slave
	i2c_set_rx_direction();			// set to RX direction
	i2c_send_start();				// send repeated start

	uint8_t data = 0xFF;
	while((I2CX->ISR & I2C_ISR_TC) == 0) {
		if((I2CX->ISR & I2C_ISR_RXNE)) {
			data = I2CX->RXDR;
		}
	}

	i2c_send_stop();
	return data;
}


void i2c_disable_peripheral(void) {
	I2CX->CR1 &= ~I2C_CR1_PE;
//	I2C2->CR1 &= ~I2C_CR1_PE;
}

/* Send a start byte on the I2C1 line */
void inline i2c_send_start(void) {
	I2CX->CR2 |= I2C_CR2_START;
//	I2C2->CR2 |= I2C_CR2_START;
//	while((I2C2->CR2 & I2C_CR2_START));
}

/* Send a stop byte on the I2C1 line */
void inline i2c_send_stop(void) {
	I2CX->CR2 |= I2C_CR2_STOP;
	while((I2CX->CR2 & I2C_CR2_STOP));
	I2CX->ICR |= (I2C_ICR_STOPCF);  // reset the ICR event flag
	while((I2CX->ICR & I2C_ICR_STOPCF));
//	I2C2->CR2 |= I2C_CR2_STOP;
//	while((I2C2->CR2 & I2C_CR2_STOP));
//	I2C2->ICR |= (I2C_ICR_STOPCF);  // reset the ICR event flag
//	while((I2C2->ICR & I2C_ICR_STOPCF));
}

/* Send a byte on the I2C1 line */
void i2c_send_byte(uint8_t byte) {
	i2c_send_start();
	//Check Tx empty before writing to it
	if((I2CX->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
		I2CX->TXDR = byte;
	}
	else {
		//Wait for TX Register to clear
		while((I2CX->ISR & I2C_ISR_TXE) == 0);
		I2CX->TXDR = byte;
	}
	/* wait for TX interrupt status and Transfer Complete bits to be set low */
	while(!(I2CX->ISR & (I2C_ISR_TXIS | I2C_ISR_TC)));
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
	while(!(I2CX->ISR & I2C_ISR_RXNE) == 0) {
		if(I2C2->ISR & I2C_ISR_ARLO) {
			break;
		}
	}
	return (I2CX->RXDR & 0xFF);
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
	I2CX->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_ADD10);
	I2CX->CR2 |= (addr << 1);
//	I2C2->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_ADD10);
//	I2C2->CR2 |= (addr << 1);
}

/* Changes NYBYTES, number of bytes to send */
void i2c_change_nbytes(uint16_t num) {
	I2CX->CR2 &= ~(I2C_CR2_NBYTES);
	I2CX->CR2 = (I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (num << I2C_CR2_NBYTES_Pos);
//	I2C2->CR2 &= ~(I2C_CR2_NBYTES);
//	I2C2->CR2 = (I2C2->CR2 & ~(I2C_CR2_NBYTES)) | (num << I2C_CR2_NBYTES_Pos);
}

/* have master request a write transfer */
void inline i2c_set_tx_direction(void) {
	I2CX->CR2 &= ~I2C_CR2_RD_WRN;
//	I2C2->CR2 &= ~I2C_CR2_RD_WRN;
}

/* have master request a read transfer */
void inline i2c_set_rx_direction(void) {
	I2CX->CR2 |= I2C_CR2_RD_WRN;
//	I2C2->CR2 |= I2C_CR2_RD_WRN;
}
