/*
 * circ.c
 *
 * Created: 13.11.2017 18:40:08
 *  Author: Artp
 */ 

#include <avr/io.h>

#include "circ.h"

// при инициализации:
// 	.txbuff = {
//    .start = u_txbuff,
//    .length = sizeof(u_txbuff),
//  },
// затем вызывается circ_init
//
void circ_init(struct circ_buffer *buff)
{
	buff->pointer = buff->start;
	buff->datalen = 0;
}

int circ_push(struct circ_buffer *buff, uint8_t *data, int len)
{
	uint8_t *bend = buff->start + buff->length - 1;
	uint8_t *dend = (buff->pointer - buff->start + buff->datalen) % buff->length + buff->start; /* This points to new byte */

	for (; len > 0; len--) {
		if (dend > bend) dend = buff->start;
		if (buff->datalen != 0 && dend == buff->pointer) break;
		*dend = *data;
		dend++;
		data++;
		buff->datalen++;
	}

	return len; /* Return amount of bytes left */
}

int circ_peek(struct circ_buffer *buff, uint8_t *data, int len)
{
	volatile uint8_t *ptr = buff->pointer;
	volatile uint8_t *bend = buff->start + buff->length - 1;
	int i;

	for (i = 0; i < len && i < buff->datalen; i++) {
		data[i] = ptr[i];
		if (ptr > bend) ptr = buff->start;
	}

	return i; /* Return the amount of bytes actually peeked */
}

int circ_pop(struct circ_buffer *buff, uint8_t *data, int len)
{
	uint8_t *bend = buff->start + buff->length - 1;
	int i;

	for (i = 0; i < len && buff->datalen > 0; i++) {
		data[i] = *buff->pointer;
		buff->pointer++;
		buff->datalen--;
		if (buff->pointer > bend) buff->pointer = buff->start;
	}

	return i; /* Return the amount of bytes actually popped */
}
