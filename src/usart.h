/*
 * usart.h
 *
 *  Created on: Jun 18, 2019
 *      Author: jack
 */

#ifndef USART_H_
#define USART_H_

#include "vfd_typedefs.h"

void init_usart(void);

void uart_send_byte(uint8_t byte);

void uart_send_bytes(uint8_t * str, uint8_t len);

void uart_send_ble_message(BLE_Message_t msg);

void uart_send_msgfail(void);



#endif /* USART_H_ */
