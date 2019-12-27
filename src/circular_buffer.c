/*
 * circular_buffer.c
 *
 *  Created on: Jul 7, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "../inc/circular_buffer.h"

#include "stm32f091xc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"

/*	A P P L I C A T I O N   I N C L U D E S   */


// remember to use pvPortMalloc() instead of malloc()
// 		and vPortFree() instead of free()

CircBuf_t * create_CircBuf(uint16_t length) {
	if(length > 0) {
		CircBuf_t * temp = pvPortMalloc(sizeof(CircBuf_t));
		if(!temp)
			return NULL;

		temp->buffer = pvPortMalloc(sizeof(uint8_t)*length);
		if(!temp->buffer)
			return NULL;

		temp->length = length;
		reset_CircBuf(temp);
		return temp;
	}
	return NULL;
}

void delete_CircBuf(CircBuf_t * buf) {
    if(buf){
    	vPortFree(buf->buffer);
    	vPortFree(buf);
    }
}

void reset_CircBuf(CircBuf_t * buf){
    if(!buf)
        return;

    size_t i = 0;
    for(i=0; i < buf->length; i++)
        buf->buffer[i] = 0;
    buf->head = buf->buffer;
    buf->tail = buf->buffer;
    buf->num_items = 0;
}

// Return 1 = Buffer is full, Return 0 = Buffer is not full
int8_t is_full_CircBuf(CircBuf_t * buf) {
    if(!buf)
        return 0;
    else if (buf->num_items == buf->length)
        return 1;
    else
        return 0;
}

// add an item to the tail of the circBuf
void add_item_CircBuf(CircBuf_t * buf, char item) {
    if(!buf)
        return;
    else if(is_full_CircBuf(buf))
        return;

    *buf->tail = item;
    buf->tail = (buf->tail - (char *)buf->buffer + 1)%(buf->length) + (char *)buf->buffer;
    buf->num_items ++;
}

// check if the CircBuf has and items in it
uint8_t is_empty_CircBuf(CircBuf_t * buf){
    if(!buf)
        return 1;
    if(buf->num_items == 0)
        return 1;
    return 0;
}

// add a string of a certain length to the buffer
void load_str_to_CircBuf(CircBuf_t * buf, char * string, uint8_t length){
    if(!buf || !string ) {
        return;
    }
//    add_item_CircBuf(buf, strlen(string));

    volatile uint8_t i;
    for(i = 0; i<length; i++){
    	add_item_CircBuf(buf, string[i]);
    }

}

// remove and return a piece of data from the circBuf
uint8_t remove_item(CircBuf_t * buf) {
    if(!buf)
        return 0xFF;
    if(buf->num_items == 0)
        return 0xFF;

    uint8_t data = *buf->head;
    buf->head = (buf->head - (char *)buf->buffer + 1)%(buf->length) + (char *)buf->buffer;
    buf->num_items --;

    return data;
}
