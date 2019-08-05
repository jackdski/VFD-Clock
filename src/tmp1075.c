/*
 * tmp1075.c
 *
 *  Created on: Jul 26, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "i2c.h"
#include "vfd_typedefs.h"

#define TMP1075ADDR	0x48
// 16 bits of temp data, fist 8 MSB = integer, next 4 MSB = decimal
#define TEMP	0x00
#define CFGR		0x01
#define LLOW		0x02
#define HLIM		0x03
#define	ID			0x0F

/*	G L O B A L   V A R I A B L E S   */
extern System_State_E system_state;
extern int8_t temperature;	/* -128 - 127 */


void read_temperature(void) {
	i2c_change_sadd((uint8_t)TMP1075ADDR);

	// use one-shot mode
	// write BIT15 high to CONFIG
}
