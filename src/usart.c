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
#include "main.h"
#include "usart.h"
#include "vfd_typedefs.h"
#include "circular_buffer.h"
#include "gpio.h"

/*	G L O B A L   V A R I A B L E S   */
extern CircBuf_t * TX_Buffer;
extern CircBuf_t * RX_Buffer;
extern HC_10_Status_E ble_status;

/*	P R I V A T E   V A R I A B L E S   */
static eUSART_State_Machine usart_state_machine = Idle;

//#define 	USART1_TX	9	// PA9
//#define		USART1_RX	10	// PA10
//#define		HC10_STATUS	8	// PA8

/*	U A R T   F U N C T I O N S   */
void init_usart(USART_TypeDef * USARTx) {
	usart_state_machine = Initializing;

	/* disable USART */
	USARTx->CR1 &=  ~(USART_CR1_UE);	// disable USART

	/* configure GPIO */
	/* select alternate function mode */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10))
				   | (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

	// select AF1 on PA9 and PA10
	GPIOA->AFR[1] |=  ((0x01 << GPIO_AFRH_AFRH1_Pos)
				  	  |(0x01 << GPIO_AFRH_AFRH2_Pos)); // AFRL (Ports 8-15)

	/* p.703 in reference manual, double if oversampling by 8 instead of 16 */
	USARTx->BRR = 840;	// set to 9600 baud (actual is 9542 baud)

	/* Control Register 1 */
	USARTx->CR1 |= 	( USART_CR1_TE			// enable Transmitter
						| USART_CR1_RE 			// enable Receiver
						| USART_CR1_RXNEIE		// enable RX interrupt
						| USART_CR1_UESM		// enable USART in STOP mode
						| (USART_CR1_M & 0) 	// 8-bit character length
						| USART_CR1_PS);		// odd parity

	USARTx->CR1 &= ~(USART_CR1_OVER8);			// oversample


	/* Control Register 2 */
	USARTx->CR2 |= (USART_CR2_STOP_Msk & 0);	// 1 stop bit

	USARTx->CR2 &=  ~(  USART_CR2_ABREN			// disable auto baud rate detection
						| USART_CR2_MSBFIRST 	// LSB first
						| USART_CR2_LINEN		// disable LIN mode
						| USART_CR2_CLKEN);		// CK pin disabled


	/* Control Register 3 */
	USARTx->CR3 |=   ( USART_CR3_WUFIE
						| (USART_CR3_WUS & 0x3)	// Wake up from Stop mode interrupt trigger on RXNE
						| USART_CR3_OVRDIS); 		// disable Overrun detection

	USARTx->CR3 &=  ~( USART_CR3_CTSE		// disable CTS
						| USART_CR3_RTSE	// disable RTS
						| USART_CR3_DMAT	// disable DMA transmitter
						| USART_CR3_DMAR	// disable DMA receiver
						| USART_CR3_SCEN	// disable Smartcard mode
						| USART_CR3_HDSEL	// disable Half-Duplex selection
						| USART_CR3_IREN	// disable IrDA
						| USART_CR3_EIE);	// disable Error interrupt

	/* Receiver Timeout */
	USARTx->RTOR |= (11 * 2);  				// timeout = length of 2 messages
	USARTx->CR2 |= USART_CR2_RTOEN;			// enable receiver timeout

	USARTx->ICR = 0x0000;			/* clear interrupt flags */
	BLE_USART->CR1 |=  USART_CR1_UE;	/* enable USART */

	/* enable USART1 interrupts */
	if(USARTx == USART1) {
		NVIC_EnableIRQ(USART1_IRQn);
		NVIC_SetPriority(USART1_IRQn, 2);
	}
	else if(USARTx == USART2) {
		NVIC_EnableIRQ(USART2_IRQn);
		NVIC_SetPriority(USART2_IRQn, 2);
	}

	wake_up_hc_10();	// make sure HC-10 BLE module is awake

#ifdef HC10_STATUS
	set_hc_10_status_pin();		// set the HC-10 status pin to PIO11

	/* make sure GPIOA is enabled */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	GPIOA->MODER &= ~(GPIO_MODER_MODER8);	// set to input
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR8_1);  // configure to pull-down

	/* Configure PA8 (HC-10 Status) interrupt */
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR3_EXTI8_PA;	// external interrupt on PA[8]
	EXTI->IMR |= EXTI_IMR_MR9; 		// select line 9 for PA8;
	EXTI->RTSR |= EXTI_RTSR_TR9;	// enable rising trigger
	EXTI->FTSR &= ~EXTI_FTSR_TR9; 	// disable falling trigger

	NVIC_SetPriority(EXTI4_15_IRQn, 1);
#endif
	usart_state_machine = Initializing;
}

void inline uart_disable_peripheral(void) {
	BLE_USART->CR1 &=  ~USART_CR1_UE;	/* disable USART */
}

void inline uart_send_byte(uint8_t byte) {
	BLE_USART->TDR = byte;
	while(!(BLE_USART->ISR & USART_ISR_TC));
}

void uart_send_bytes(char * str, uint8_t len) {
	uint8_t i;
	for(i = 0; i < len; i++)
		uart_send_byte(str[i]);
}

/* loads "MSGFAIL" into the TX message buffer that sends every 1s */
void uart_send_msgfail(void) {
	char * msgfail = "MSGFAIL\0";
	load_str_to_CircBuf(TX_Buffer, msgfail, 11);
}

/* configures the baud rate of the HC-10 modules.
 * default is 9600 baud
 * should receive "OK+Set:@param"
 * 	@param baud: 9600, 57600, or 115200
 */
void set_hc_10_baud_rate(uint32_t baud) {
	char baud_str[8] = "AT+BAUD";
	switch(baud) {
		case 9600: baud_str[7] = '0'; break; //strcat(baud_str, "0"); break;
		case 57600: baud_str[7] = '3'; break;
		case 115200: baud_str[7] = '4'; break;
	}
	uart_send_bytes(baud_str, 8);
}

/* response should be "OK_Get:[baud]" */
void request_hc_10_baud_rate(void) {
	char baud_str[8] = "AT+BAUD?";
	uart_send_bytes(baud_str, 8);
}


/* Will set PIO11 to the connected status pin */
void set_hc_10_status_pin(void) {
	char sys_led_str[8] = "AT+PIO11";
	uart_send_bytes(sys_led_str, 8);
}

/* wakes up the HC-10 Bluetooth Module */
void wake_up_hc_10(void) {
	char wake_up_text[81] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	uart_send_bytes(wake_up_text, 81);
	ble_status = Waking_Up;
	// should receive "OK+WAKE" back

	// get name of the device
	char name_string = "AT+NAME?";
	ble_status = Naming;
}

void set_sleep_mode_hc_10(void) {
	char sleep_text[8] = "AT+SLEEP";
	uart_send_bytes(sleep_text, 8);
	// should receive "OK+SLEEP" back
}

void set_name_hc_10(char * name) {
	char name_text[20] = "AT+NAME";
	strcat(name_text, name);

}

void USART1_IRQHandler() {
	/* if USART RX is not-empty, add to BLE/USART Circular Buffer */
	if((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE){
		add_item_CircBuf(RX_Buffer, (uint8_t)(USART1->RDR));
	}

	if(USART1->ISR & USART_ISR_RTOF) {
		// task notification
	}
	/* if Transmit Buffer Empty */
	if(USART1->ISR & USART_ISR_TXE) {

	}
	/* if Transmit Complete */
	if(USART1->ISR & USART_ISR_TXC) {

	}
}


