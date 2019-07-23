/*
 * usart.c
 *
 *  Created on: Jun 18, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>
#include <string.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "usart.h"
#include "vfd_typedefs.h"
#include "circular_buffer.h"
#include "gpio.h"

/*	G L O B A L   V A R I A B L E S   */
extern uint8_t usart_msg;
extern CircBuf_t * TX_Buffer;
extern CircBuf_t * RX_Buffer;

#define 	USART1_TX	9	// PA9
#define		USART1_RX	10	// PA10

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
					| USART_CR1_PS);		// odd parity

	USART1->CR1 &= ~( USART_CR1_OVER8);		// oversample 16


	/* Control Register 2 */
	USART1->CR2 |= (USART_CR2_STOP_Msk & 0);	// 1 stop bit

	USART1->CR2 &= ~( USART_CR2_RTOEN		// disable receiver timeout
					| USART_CR2_ABREN		// disable auto baud rate detection
					| USART_CR2_MSBFIRST 	// LSB first
					| USART_CR2_LINEN		// disable LIN mode
					| USART_CR2_CLKEN);		// CK pin disabled


	/* Control Register 3 */
	USART1->CR3 |=  ( USART_CR3_WUFIE
					| (USART_CR3_WUS & 0x3)	// Wake up from Stop mode interrupt trigger on RXNE
					| USART_CR3_OVRDIS); 		// disable Overrun detection

	USART1->CR3 &= ~( USART_CR3_CTSE	// disable CTS
					| USART_CR3_RTSE	// disable RTS
					| USART_CR3_DMAT	// disable DMA transmitter
					| USART_CR3_DMAR	// disable DMA receiver
					| USART_CR3_SCEN	// disable Smartcard mode
					| USART_CR3_HDSEL	// disable Half-Duplex selection
					| USART_CR3_IREN	// disable IrDA
					| USART_CR3_EIE);	// disable Error interrupt

	USART1->ICR = 0x0000;			/* clear interrupt flags */
	USART1->CR1 |=  USART_CR1_UE;	/* enable USART */

	/* enable USART1 interrupts */
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 2);
}

void inline uart_send_byte(uint8_t byte) {
	USART1->TDR = byte;
	while(!(USART1->ISR & USART_ISR_TC));
}

void uart_send_bytes(uint8_t * str, uint8_t len) {
	uint8_t i;
	for(i = 0; i < len; i++)
		uart_send_byte(str[i]);
}

void uart_send_ble_message(BLE_Message_t msg) {
	uart_send_byte(msg.message_type);
	uart_send_byte(msg.data_byte_one);
	uart_send_byte(msg.data_byte_two);
}

/* loads "MSGFAIL" into the TX message buffer that sends every 1s */
void uart_send_msgfail(void) {
	uint8_t * msgfail = "MSGFAIL\0";
	load_str_to_CircBuf(TX_Buffer, msgfail, 11);
}

/* configures the baud rate of the HC-10 modules.
 * default is 9600 baud
 * should receive "OK+Set:@param"
 * 	@param baud: 9600, 57600, or 115200
 */
void set_hc_10_baud_rate(uint32_t baud) {
	uint8_t baud_str[8] = "AT+BAUD";
	switch(baud) {
		case 9600: baud_str[7] = '0'; break; //strcat(baud_str, "0"); break;
		case 57600: baud_str[7] = '3'; break;
		case 115200: baud_str[7] = '4'; break;
	}
	uart_send_bytes(baud_str, 8);
}

/* response should be "OK_Get:[baud]" */
void request_hc_10_baud_rate(void) {
	uint8_t baud_str[8] = "AT+BAUD?";
	uart_send_bytes(baud_str, 8);
}

/* wakes up the HC-10 Bluetooth Module */
void wake_up_hc_10(void) {
	uint8_t wake_up_text[81] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	uart_send_bytes(wake_up_text, 81);
	// should receive "OK+WAKE" back
}

void set_sleep_mode_hc_10(void) {
	uint8_t sleep_text[8] = "AT+SLEEP";
	uart_send_bytes(sleep_text, 8);
	// should receive "OK+SLEEP" back
}

void USART1_IRQHandler() {
	/* if USART RX is not-empty, add to BLE/USART Queue */
	if((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE){
		add_item_CircBuf(RX_Buffer, (uint8_t)(USART1->RDR));
	}
}


