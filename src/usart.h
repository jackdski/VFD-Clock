/*
 * usart.h
 *
 *  Created on: Jun 18, 2019
 *      Author: jack
 */

#ifndef USART_H_
#define USART_H_

#include "vfd_typedefs.h"

typedef enum {
	Initializing,
	Idle,
	Transmitting,
	Stop_Transmitting,
	Receiving,
} eUSART_State_Machine;

typedef struct {
	bool tx_ready;
	uint8_t n_characters;
} USART_Payload_t;

void init_usart(USART_TypeDef * USARTx);
void uart_disable_peripheral(void);
void uart_send_byte(uint8_t byte);
void uart_send_bytes(char * str, uint8_t len);
void uart_send_ble_message(BLE_Message_t msg);
void uart_send_msgfail(void);
void set_hc_10_baud_rate(uint32_t baud);
void request_hc_10_baud_rate(void);
void set_hc_10_status_pin(void);
void wake_up_hc_10(void);
void set_sleep_mode_hc_10(void);

#endif /* USART_H_ */
