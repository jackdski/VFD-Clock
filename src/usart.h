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

void set_hc_10_baud_rate(uint32_t baud);

void request_hc_10_baud_rate(void);

void wake_up_hc_10(void);

void set_sleep_mode_hc_10(void);

#endif /* USART_H_ */
