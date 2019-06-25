/*
 * usart.h
 *
 *  Created on: Jun 18, 2019
 *      Author: jack
 */

#ifndef USART_H_
#define USART_H_

void init_usart(void);

void uart_send_byte(uint8_t byte);

/* receive a byte */
void uart_read_byte(void);

#endif /* USART_H_ */
