/*
 * circ.h
 *
 * Created: 13.11.2017 18:43:12
 *  Author: Artp
 */ 


#ifndef CIRC_H_
#define CIRC_H_

#include <inttypes.h>
// ��� �������������:
// 	.txbuff = {
//    .start = u_txbuff,
//    .length = sizeof(u_txbuff),
//  },
// ����� ���������� circ_init
//

//
// push - ����������� datalen. ������ ������������ �� pointer'�� ��������� ���� �� ������� pointer
// pop - ������ ������, ������� � pointer. Pointer �������������, � datalen �����������.
// peek - ������ ������, �� �� ������� pointer � datalen
//

struct circ_buffer {
	uint8_t *start;					// ��������� �� ������ ������. ���������
	int length;						// 

	/* Private */
	volatile uint8_t *pointer;		// ��������� �� ������ ������������ ���� � ������. ��� ������ (pop) �������������, � datalen �����������. ��� push'� �� ����������.
	volatile int datalen;			// ������� ������� �������������� ���� ����� ����� pointer'a. �������������, ����� ���������� (push) � �����
};

#define circ_datalen(buff) \
((buff)->datalen)

#define circ_is_full(buff) \
((buff)->length == (buff)->datalen)

#define circ_is_empty(buff) \
(circ_datalen(buff) == 0)

void circ_init(struct circ_buffer *buff);

/* Return the amount of bytes left */
int circ_push(struct circ_buffer *buff, uint8_t *data, int len);
int circ_push_byte(struct circ_buffer *buff, uint8_t data);

/* Returns amount of bytes popped/peeked */
int circ_pop(struct circ_buffer *buff, uint8_t *data, int len);
int circ_peek(struct circ_buffer *buff, uint8_t *data, int len);



#endif /* CIRC_H_ */