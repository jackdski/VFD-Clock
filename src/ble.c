/*
 * ble.c
 *
 *  Created on: Dec 26, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "main.h"
#include "ble.h"
#include "gpio.h"
#include "callbacks.h"
#include "vfd_typedefs.h"
#include "circular_buffer.h"
#include "tubes.h"
#include "rtc.h"
#include "pwm.h"

/*	G L O B A L   V A R I A B L E S   */
extern TaskHandle_t thBrightness_Adj;
extern TaskHandle_t thAutoBrightAdj;
extern TaskHandle_t thRTC;
extern System_State_E system_state;
extern TimerHandle_t five_sec_timer;


/*	P R I V A T E   V A R I A B L E S   */
static char * ble_name = "VFD-Clock";
static HC_10_Status_E ble_status = Disconnected;
static eUSART_State_Machine usart_state_machine = Idle;
static uint8_t tx_n;
static uint8_t tx_counter;

/* C I R C U L A R   B U F F E R S   */
CircBuf_t * TX_Buffer;
CircBuf_t * RX_Buffer;

/*	T A S K S   */

/* if there is a BLE connection, then this task will read the BLE RX message queue if it is not empty */
void prvBLE_Receive_Task(void *pvParameters) {
	static TickType_t delay_time = pdMS_TO_TICKS( 1000 );  // 1ms

	/* strings to be sent */
	static char temp_msg[] = "TEMP";
	static char autobright_msg[] = "AUTOBRIGHT:";
	static char autobright_on_msg[] = "AUTOBRIGHT:ON";
	static char autobright_off_msg[] = "AUTOBRIGHT:OFF";
	static char time_msg[] = "TIME:";
	static char turnoff_msg[] = "TURNOFF";

	/* strings that should be received */
	static char wake_up_msg[] = "OK+WAKE";
	static char sleep_msg[] = "OK+SLEEP";
	static char reset_msg[] = "OK+RESET";
	static char name_msg[25] = "OK+Set:";
	strcat(name_msg, ble_name);
//	static uint8_t baud_one[] = "OK+Set:9600";

	for( ;; ) {
	    update_hc_10_status();

	    /* if there is a message, format it */
	    if(RX_Buffer->num_items > 0) {
			size_t n = RX_Buffer->num_items;
			size_t i;
			char c;
			char xRXMessage[n+1];
			for(i = 0; i < n; i++) {
				c = remove_item(RX_Buffer);
				if(c != '\r' || c != '\n') {
					xRXMessage[i] = c;	// add to string
				}
				else {
					break;
				}
			}
			xRXMessage[i] = '\0';

			/* capitalize message */
			for(i = 0; i <= strlen(xRXMessage); i++) {
				xRXMessage[i] = toupper(xRXMessage[i]);
			}

			/* Done formatting */
			if(ble_status == Get_Name) {	// if name is not correct, set it to ble_name
				if(strcmp(xRXMessage, name_msg) != 0) {
					ble_set_name_hc_10();
				}
			}
			if(ble_status == Set_Name) {	// write the string received from HC10
				char name_set_str[80];
				strcpy(name_set_str, xRXMessage);
				name_set_str[strlen(xRXMessage)] = '\n';
				ble_write(name_set_str);
			}

			/* if connected, determine see what to do with the rx'd message */
			if(ble_status == Connected) {
				// "TEMP"
				if(strcmp((const char *)xRXMessage, (const char *)temp_msg) == 0) {
					system_state = BLE_Temperature;
					ble_write("TEMP:OK\n");
					toggle_error_led();
					vTaskSuspend(thRTC);
					display_temperature();
					// instead send prvTemperature_Task a notification here
					xTimerStart(five_sec_timer, pdMS_TO_TICKS(100));
				}
				// "AUTOBRIGHT
				else if(strncmp((const char *)xRXMessage, (const char *)autobright_msg, 11) == 0) {
					// "AUTOBRIGHT:ON"
					if(strcmp((const char *)xRXMessage, (const char *)autobright_on_msg) == 0) {
						ble_write("Brightness ON\n");
						vTaskResume( thAutoBrightAdj );		// resume task
					}
					// "AUTOBRIGHT:OFF"
					else if(strcmp((const char *)xRXMessage, (const char *)autobright_off_msg) == 0) {
						ble_write("Brightness OFF\n");
						vTaskSuspend(thAutoBrightAdj);	// suspend task until On msg received
					}
					else {
						// "AUTOBRIGHT:XX"
						vTaskSuspend( thAutoBrightAdj );	// suspend task until On msg received
						// check if in correct range
						if((xRXMessage[11] >= '0') && (xRXMessage[11] <= '9') && (xRXMessage[12] >= '0') && (xRXMessage[12] <= '9')) {
							// change brightness to xRXMessage[11:12]
							ble_write("BRIGHTNESS:OK\n");
							set_target_brightness(((xRXMessage[11] - 48) * 10) + (xRXMessage[12] - 48));
							vTaskResume( thBrightness_Adj );		// resume task that changes brightness
						}
						else {  // not in range 0-99
							ble_write("Use values between 0 and 99\n");
						}
					}
				}
				// TIME:XX:XX:X
				else if(strncmp((const char *)xRXMessage, (const char *)time_msg, 5) == 0) {
					uint8_t temp_hours, temp_mins, temp_ampm = 0xFF;
					// change hours to xRXMessage[5:6]
					if((xRXMessage[5] >= '0') && (xRXMessage[5] < '2') && (xRXMessage[6] >= '0') && (xRXMessage[6] <= '9')) {
						temp_hours = ((xRXMessage[5] - 48) * 10) + (xRXMessage[6] - 48);
					}

					if(xRXMessage[7] != ':') {
						temp_hours = 0xFF;
					}

					// change minutes to xRXMessage[8:9]
					if((xRXMessage[8] >= '0') && (xRXMessage[8] <= '5') && (xRXMessage[9] >= '0') && (xRXMessage[9] <= '9')) {
						temp_mins = ((xRXMessage[8] - 48) * 10) + (xRXMessage[9] - 48);
					}

					if(xRXMessage[10] != ':') {
							temp_mins = 0xFF;
					}

					// set am/pm value
					if(xRXMessage[11] == 'A') {
						temp_ampm = 0;	// set RTC to AM
					}
					else if(xRXMessage[11] == 'P') {
						temp_ampm = 1;	// set RTC to PM
					}

					// make sure no errors exist and update time/display
					if(temp_hours == 0xFF || temp_mins == 0xFF || temp_ampm == 0xFF) {
						ble_send_msgfail();
					}
					// change time on RTC and update
					else {
						change_rtc_time(temp_hours, temp_mins, 0, temp_ampm);
						update_time(temp_hours, temp_mins, 0);
					}
				}
				// "TURNOFF"
				else if(strcmp((const char *)xRXMessage, (const char *)turnoff_msg) == 0) {
					ble_write("Remote Turn Off not available\n");
				}
				else {
					// send "MSGFAIL" back since an incorrect message was received
					ble_send_msgfail();
				}
				reset_CircBuf(RX_Buffer);
			}

			/* check if it is a response to an HC-10 command
			 * status will not be Connected since the device will either be
			 *   waking up or going to sleep */
			else if(ble_status != Connected) {
				if(strcmp((const char *)xRXMessage, (const char *)wake_up_msg) == 0) {
					update_hc_10_status();
				}
				else if(strcmp((const char *)xRXMessage, (const char *)sleep_msg) == 0) {
					ble_status = Sleep;
				}
				else {
					ble_status = BLE_Error;
					set_error_led_status(Flashing);
				}
			}
	    }
		vTaskDelay(delay_time);
	}
}

void ble_write(char * str) {
	uint8_t tx_n = strlen(str);
	tx_counter = 0;
	load_str_to_CircBuf(TX_Buffer, str, tx_n);
	usart_state_machine = Transmitting;
	// enable TXE interrupt, starting transmission state machine
}

void inline ble_send_byte(uint8_t byte) {
	BLE_USART->TDR = (uint16_t)byte;
}

/* loads "MSGFAIL" into the TX message buffer that sends every 1s */
void ble_send_msgfail(void) {
	ble_write("MSGFAIL\n");
}

void ble_set_sleep_mode_hc_10(void) {
	ble_write("AT+SLEEP");
	// should receive "OK+SLEEP" back
	// use eBLE_Expected_RX here
}

/* wakes up the HC-10 Bluetooth Module */
void ble_wake_up_hc_10(void) {
	ble_status = Waking_Up;
	ble_write("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	// should receive "OK+WAKE" back
}

void ble_get_name_hc_10(void) {
	ble_status = Get_Name;
	ble_write("AT+NAME?");
}

void ble_set_name_hc_10(void) {
	ble_status = Set_Name;
	char * name_string = "AT+NAME";
	strcat(name_string, ble_name);
	ble_write(name_string);
}

void ble_set_hc_10_status(HC_10_Status_E status) {
	ble_status = status;
}

HC_10_Status_E ble_get_hc_10_status(void) {
	return ble_status;
}

void USART1_IRQHandler() {
	taskENTER_CRITICAL();
	uint32_t usart_isr_status = USART1->ISR;
	USART1->ICR = usart_isr_status;	// clear flags

/*	RX	*/
	/* if USART RX is not-empty, add to BLE/USART Circular Buffer */
	if((usart_isr_status & USART_ISR_RXNE) == USART_ISR_RXNE){
		usart_state_machine = Receiving;
		add_item_CircBuf(RX_Buffer, (uint8_t)(USART1->RDR));
		usart_state_machine = Idle;
	}

//	if(usart_isr_status & USART_ISR_RTOF) {		// RX time out
//		// task notification
//	}

/*	TX	*/
	/* Transmit Buffer Empty */
	if(usart_isr_status & USART_ISR_TXE) {
		USART1->CR1 &= ~USART_CR1_TXEIE; // disable TXE interrupt

		if(tx_counter <= tx_n) {	// more characters need to be sent
			ble_send_byte(remove_item(TX_Buffer));
			tx_counter++;
		}
		if(tx_counter == tx_n) {	// if all characters have been sent
			USART1->CR1 |= USART_CR1_TCIE; // enable TXC interrupt
			USART1->CR1 &= ~USART_CR1_TXEIE; // disable TXE interrupt
		}
	}

	/* Transmission Complete */
	if(usart_isr_status & USART_ISR_TC) {
		if(tx_counter == tx_n) {
			USART1->CR1 &= ~USART_CR1_TXEIE; // disable TXE interrupt
			tx_counter = 0;
			tx_n = 0;
		}
	}

	taskEXIT_CRITICAL();
}

/*	U A R T   F U N C T I O N S   */
void init_usart(USART_TypeDef * USARTx) {
	usart_state_machine = Initializing;

	/* create circular buffers for BLE messages */
	TX_Buffer = create_CircBuf(100);
	RX_Buffer = create_CircBuf(100);

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

	ble_wake_up_hc_10();	// make sure HC-10 BLE module is awake

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

void usart_disable_peripheral(void) {
	BLE_USART->CR1 &= ~USART_CR1_UE;	// disable USART
	RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;	// disable USART clock source
}

