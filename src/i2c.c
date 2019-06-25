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

//#define I2C_TIMING			0x00201D2B 	// for 100kHz  // maybe 0x00281EAD?
#define I2C_TIMING			0X0010020A	// for 400kHz
#define I2C_ADDRESS 		0xCA

//#define	USE_I2C1


//void init_i2c(void) {
//	/*  setup GPIO, AF1 on PF0 and PF1
//	 *
//	 * 	PF0 -> SDA
//	 * 	PF1 -> SCL
//	 */
//	RCC->AHBENR |= RCC_AHBENR_GPIOFEN; /* Enable Peripheral clock on GPIOF */
//	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // enable I2C clock
//
//	/* make sure that peripheral is first disabled */
//	I2C1->CR1 &= ~(I2C_CR1_PE);
//
//	/* select alternate function mode */
//	GPIOF->MODER = (GPIOF->MODER & ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1))
//			| (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1);
//
//	// select AF1 on PF0 and PF1
//	GPIOF->AFR[0] |=  ((0x01 << GPIO_AFRL_AFRL0_Pos)
//				  	  |(0x01 << GPIO_AFRL_AFRL1_Pos)); // AFRL (Ports 0-7)
//
//	/* Control Register 1 */
//	I2C1->CR1 &=   ~( I2C_CR1_DNF		// disable Digital Noise Filter
////					| I2C_CR1_PECEN 	// disable PEC calculation
////					| I2C_CR1_ALERTEN	// disable SMBus alert
//					| I2C_CR1_SMBDEN	// disable SMBus Device Default address enable
//					| I2C_CR1_SMBHEN	// disable SMBus Host address enable
////					| I2C_CR1_SBC		// disable Slave Byte Control
////					| I2C_CR1_RXDMAEN	// disable DMA reception requests
////					| I2C_CR1_TXDMAEN	// disable DMA transmission requests
//					| I2C_CR1_ANFOFF	// enable Analog noise filter
////					| I2C_CR1_STOPIE	// disable STOP interrupt
////					| I2C_CR1_NACKIE	// disable NACK interrupt
////					| I2C_CR1_ADDRIE	// disable Address Match interrupt
//				);
//
//	I2C1->CR1 |= (I2C_CR1_DNF & 0b0000);  // disable digital noise filter for WUPEN
//
////	I2C1->CR1 |= 	( I2C_CR1_WUPEN			// enable wake up from Stop mode
////					| I2C_CR1_NOSTRETCH		// disable clock stretching
////					| I2C_CR1_ERRIE			// enable Errors interrupt
////					| I2C_CR1_TCIE			// enable Transfer Complete interrupt
////					| I2C_CR1_RXIE			// enable RX interrupt
////					| I2C_CR1_TXIE			// enable TX interrupt
////				);
//
//
//	/* Conrol Register 2 */
//	I2C1->CR2 &= 	~( I2C_CR2_AUTOEND	// TC flag is set when NBYTES data is transferred
//					| I2C_CR2_RELOAD	// transfer is completed after the NBYTES data transfer
//					| I2C_CR2_NACK		// send an ACK after current received byte
//					| I2C_CR2_RD_WRN	// init to requesting a write transfer
//					| I2C_CR2_ADD10		// operate in 7-bit addressing mode
//				);
//
//	I2C1->CR2 &= ~(I2C_CR2_SADD);		// clear slave address
//	// I2C1->CR2 |= ADDRESS;		// write a new slave address
//
////	I2C1->CR2 |= ( I2C_CR2_HEAD10R);	// master only sends 1st 7 bits of 10 bit address, followed by READ direction
//
//
//	/* OAR1 - Own Address 1 */
////	I2C1->OAR1 = (I2C_OAR1_OA1 & 0x12);	// set own address 1 to 0x12 (0b001_0010)
////	I2C1->OAR1 &= ~(I2C_OAR1_OA1MODE);	// set to 7-bit address
////	I2C1->OAR1 |= I2C_OAR1_OA1EN;		// enable Own Address 1
//
//
//	/* Timing */
//	I2C1->TIMINGR &= (0x00000000);		// Reset
//	I2C1->TIMINGR |= 0x00201D2B;		// 100kHz? Need to check with STM32CubeMX tool
//
//	/* Enable I2C1 peripheral */
//	I2C1->CR1 |= I2C_CR1_PE;
//}

void init_i2c(void) {
#ifdef	USE_I2C1
//	I2C1->CR1 &= ~I2C_CR1_PE;

//	/* select alternate function mode */
//	GPIOF->MODER = (GPIOF->MODER & ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1))
//			| (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1);
//
//	// select AF1 on PF0 and PF1
//	GPIOF->AFR[0] |=  ((0x01 << GPIO_AFRL_AFRL0_Pos)
//					  |(0x01 << GPIO_AFRL_AFRL1_Pos)); // AFRL (Ports 0-7)


	/* alt */

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

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; /* Enable Peripheral clock on GPIOB */

	/* select alternate function mode */
	GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11))
			| (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);

	GPIOB->OTYPER |= (GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_11);

	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR10_0 | GPIO_OSPEEDER_OSPEEDR11_0);

//	GPIOB->PUPDR |=     ( GPIO_PUPDR_PUPDR10_0	// pull-up
//						| GPIO_PUPDR_PUPDR11_0	// pull-up
//		);

	GPIOB->PUPDR &=    ~( GPIO_PUPDR_PUPDR10 | GPIO_PUPDR_PUPDR11 );

	// select AF1 on PB10 and PB11
	GPIOB->AFR[1] |=  ((0x01 << GPIO_AFRL_AFRL2_Pos)
					  |(0x01 << GPIO_AFRL_AFRL3_Pos)); // AFRH (Ports 8-15)


	RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C2RST);

	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // enable I2C clock

	I2C2->TIMINGR = (uint32_t)I2C_TIMING;
	I2C2->CR2 &= ~(I2C_CR2_ADD10); // select 7-bit address mode
	I2C2->CR1 |= I2C_CR1_PE;

//	I2C2->CR2 |= (0x18 << 1);
//	I2C2->CR2 &= ~(I2C_CR2_RD_WRN);
//	I2C2->CR2 |= (1 << 16);

//	I2C2->CR2 |= I2C_CR2_START;
//	while(I2C2->CR2 & I2C_CR2_START);
//	I2C2->TXDR = 0xAA;
//	while(!(I2C2->ISR & I2C_ISR_TXE));

//	I2C2->CR2 |= I2C_CR2_STOP;
//	while(I2C2->CR2 & I2C_CR2_STOP);


#endif
}

void i2c_write_reg(uint8_t device, uint8_t reg, uint8_t data) {
	I2C1->CR2 = 0x00000000;

#ifdef USE_I2C1
	while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
#else
	while((I2C2->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
#endif

	i2c_change_sadd(device);
	i2c_change_nbytes(1);
//	i2c_send_start();
	i2c_send_byte(data);
	i2c_send_stop();

}

/* Send a start byte on the I2C1 line */
void inline i2c_send_start(void) {
#ifdef	USE_I2C1
	I2C1->CR2 |= I2C_CR2_START;
	while((I2C1->CR2 & I2C_CR2_START));
#else
	I2C2->CR2 |= I2C_CR2_START;
	while((I2C2->CR2 & I2C_CR2_START));
#endif
}

/* Send a stop byte on the I2C1 line */
void inline i2c_send_stop(void) {
#ifdef	USE_I2C1
	I2C1->CR2 |= I2C_CR2_STOP;
	while((I2C1->CR2 & I2C_CR2_STOP));
	I2C1->ICR |= (I2C_ICR_STOPCF);  // reset the ICR event flag
	while((I2C1->ICR & I2C_ICR_STOPCF));
#else
	I2C2->CR2 |= I2C_CR2_STOP;
	while((I2C2->CR2 & I2C_CR2_STOP));
	I2C2->ICR |= (I2C_ICR_STOPCF);  // reset the ICR event flag
	while((I2C2->ICR & I2C_ICR_STOPCF));
#endif
}

/* Send a byte on the I2C1 line */
void i2c_send_byte(uint8_t byte) {
#ifdef	USE_I2C1
	i2c_send_start();
	//Check Tx empty before writing to it
	if((I2C1->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
		I2C1->TXDR = byte;
	}
	else {
		//Wait for TX Register to clear
		while((I2C1->ISR & I2C_ISR_TXE) == 0);
		I2C1->TXDR = byte;
	}
	/* wait for TX interrupt status and Transfer Complete bits to be set low */
	while(!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_TC)));
#else
	//Check Tx empty before writing to it
	if((I2C2->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
		I2C2->TXDR = byte;
	}
	else {
		//Wait for TX Register to clear
		while((I2C2->ISR & I2C_ISR_TXE) == 0);
		I2C2->TXDR = byte;
	}
	/* wait for TX interrupt status and Transfer Complete bits to be set low */
 	while(!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_TC)));
#endif
}

/* Wait for a byte of data to be available then return that byte */
uint8_t i2c_read_byte(void) {
#ifdef	USE_I2C1
	while(!(I2C1->ISR & I2C_ISR_RXNE));
	return (I2C1->RXDR & 0xFF);
#else
	while(!(I2C2->ISR & I2C_ISR_RXNE));
	return (I2C2->RXDR & 0xFF);
#endif
}

/* Changes the slave address */
void i2c_change_sadd(uint8_t addr) {
	// Clear slave address and use 7-bit addresses
#ifdef	USE_I2C1
	I2C1->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_ADD10);
	I2C1->CR2 |= (addr << 1);
#else
	I2C2->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_ADD10);
	I2C2->CR2 |= (addr << 1);
#endif
}

/* Changes NYBYTES, number of bytes to send */
void i2c_change_nbytes(uint16_t num) {
#ifdef	USE_I2C1
	I2C1->CR2 &= ~(I2C_CR2_NBYTES);
	I2C1->CR2 = (I2C1->CR2 & ~(I2C_CR2_NBYTES)) | (num << I2C_CR2_NBYTES_Pos);
#else
	I2C2->CR2 &= ~(I2C_CR2_NBYTES);
	I2C2->CR2 = (I2C2->CR2 & ~(I2C_CR2_NBYTES)) | (num << I2C_CR2_NBYTES_Pos);
#endif
}

/* have master request a write transfer */
void inline i2c_set_tx_direction(void) {
#ifdef	USE_I2C1
	I2C1->CR2 &= ~I2C_CR2_RD_WRN;
#else
	I2C2->CR2 &= ~I2C_CR2_RD_WRN;
#endif
}

/* have master request a read transfer */
void inline i2c_set_rx_direction(void) {
#ifdef	USE_I2C1
	I2C1->CR2 |= I2C_CR2_RD_WRN;
#else
	I2C2->CR2 |= I2C_CR2_RD_WRN;
#endif
}
