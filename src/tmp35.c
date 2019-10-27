/*
 * tmp35.c
 *
 *  Created on: Oct 21, 2019
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
#include "sensor_tasks.h"
#include "vfd_typedefs.h"
#include "circular_buffer.h"
#include "gpio.h"
#include "adc.h"



uint8_t tmp_temperature = 0; // [Celsius]

uint8_t calculate_celsius_temperature(uint16_t mv) {
	return ((mv - 500)/10);
}
