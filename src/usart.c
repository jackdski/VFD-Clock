/*
 * usart.c
 *
 *  Created on: Jun 18, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "usart.h"
#include "vfd_typedefs.h"

/*	G L O B A L   V A R I A B L E S   */
extern uint8_t usart_msg;
extern QueueHandle_t BLE_Queue;;

/*	U A R T   F U N C T I O N S   */
void init_usart(void) {
	/* disable USART */
	USART1->CR1 &=  ~USART_CR1_UE;	// disable USART

	/* configure GPIO */
	/* select alternate function mode */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10))
			| (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

	// select AF1 on PA9 and PA10
	GPIOA->AFR[1] |=  ((0x01 << GPIO_AFRH_AFRH1_Pos)
				  	  |(0x01 << GPIO_AFRH_AFRH2_Pos)); // AFRL (Ports 8-15)

	/* p.703 in reference manual, double if oversampling by 8 instead of 16 */
	USART1->BRR = 840;	// set to 9600 baud (actual is 9542 baud)

	/* Control Register 1 */
	USART1->CR1 |= 	( USART_CR1_TE			// enable Transmitter
					| USART_CR1_RE 			// enable Receiver
					| USART_CR1_RXNEIE		// enable RX interrupt
					| USART_CR1_UESM		// enable USART in STOP mode
					| (USART_CR1_M & 0) 	// 8-bit character length
					| USART_CR1_PS			// odd parity
					);

	USART1->CR1 &= ~( USART_CR1_OVER8);		// oversample 16


	/* Control Register 2 */
	USART1->CR2 |= (USART_CR2_STOP_Msk & 0);	// 1 stop bit

	USART1->CR2 &= ~( USART_CR2_RTOEN		// disable receiver timeout
					| USART_CR2_ABREN		// disable auto baud rate detection
					| USART_CR2_MSBFIRST 	// LSB first
					| USART_CR2_LINEN		// disable LIN mode
					| USART_CR2_CLKEN		// CK pin disabled
		);


	/* Control Register 3 */
	USART1->CR3 |=  ( USART_CR3_WUFIE
					| (USART_CR3_WUS & 0x3)	// Wake up from Stop mode interrupt trigger on RXNE
					| USART_CR3_OVRDIS 		// disable Overrun detection
		);

	USART1->CR3 &= ~( USART_CR3_CTSE	// disable CTS
					| USART_CR3_RTSE	// disable RTS
					| USART_CR3_DMAT	// disable DMA transmitter
					| USART_CR3_DMAR	// disable DMA receiver
					| USART_CR3_SCEN	// disable Smartcard mode
					| USART_CR3_HDSEL	// disable Half-Duplex selection
					| USART_CR3_IREN	// disable IrDA
					| USART_CR3_EIE		// disable Error interrupt
		);


	/* clear interrupt flags */
	USART1->ICR = 0x0000;

	/* enable USART */
	USART1->CR1 |=  USART_CR1_UE;

	/* enable USART1 interrupts */
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 2);
}

void inline uart_send_byte(uint8_t byte) {
	USART1->TDR = byte;
}

// TODO: add function to send a string
//void uart_send_bytes(uint8_t * bytes);


void USART1_IRQHandler() {
	/* if USART RX is not-empty, add to BLE/USART Queue */
	if(USART1->ISR & USART_ISR_RXNE) {
		// read data from RX register
		usart_msg = (uint8_t)(USART1->RDR);

		// add value of usart_msg to BLE_Queue without any blocking time
		xQueueSendToBackFromISR(BLE_Queue, &usart_msg, 0U);

		/* clear interrupt flag */
		USART1->ISR |= USART_ISR_RXNE;
	}
}


