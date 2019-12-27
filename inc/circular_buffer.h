/*
 * circular_buffer.h
 *
 *  Created on: Jul 8, 2019
 *      Author: jack
 */

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

#include <stdint.h>

typedef struct {
	char * buffer;
	char * head;
	char * tail;
	uint16_t num_items;
	uint16_t length;
}CircBuf_t;

CircBuf_t * create_CircBuf(uint16_t length);

void delete_CircBuf(CircBuf_t * buf);

void reset_CircBuf(CircBuf_t * buf);

// Return 1 = Buffer is full, Return 0 = Buffer is not full
int8_t is_full_CircBuf(CircBuf_t * buf);

// add an item to the tail of the circBuf
void add_item_CircBuf(CircBuf_t * buf, char item);

// check if the CircBuf has and items in it
uint8_t is_empty_CircBuf(CircBuf_t * buf);

// add a string of a certain length to the buffer
void load_str_to_CircBuf(CircBuf_t * buf, char * string, uint8_t length);

// remove and return a piece of data from the circBuf
uint8_t remove_item(CircBuf_t * buf);

#endif /* CIRCULAR_BUFFER_H_ */
