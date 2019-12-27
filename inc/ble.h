/*
 * ble.h
 *
 *  Created on: Dec 26, 2019
 *      Author: jack
 */

#ifndef BLE_H_
#define BLE_H_

#include "vfd_typedefs.h"

typedef enum {
	Initializing,
	Idle,
	Transmitting,
	Stop_Transmitting,
	Receiving,
} eUSART_State_Machine;

/* USART */
void init_usart(USART_TypeDef * USARTx);
void usart_disable_peripheral(void);

/* T A S K S */
void prvBLE_Receive_Task(void *pvParameters);

/* B L E */
void ble_write(char * str);
void ble_send_byte(uint8_t byte);
void ble_send_msgfail(void);
void ble_set_sleep_mode_hc_10(void);
void ble_wake_up_hc_10(void);
void ble_get_name_hc_10(void);
void ble_set_name_hc_10(void);
void ble_set_hc_10_status(HC_10_Status_E status);
HC_10_Status_E ble_get_hc_10_status(void);

#endif /* BLE_H_ */
