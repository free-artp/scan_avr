/*
 * circ.h
 *
 * Created: 13.11.2017 18:43:12
 *  Author: Artp
 */ 


#ifndef CIRC_H_
#define CIRC_H_

#include <inttypes.h>
// при инициализации:
// 	.txbuff = {
//    .start = u_txbuff,
//    .length = sizeof(u_txbuff),
//  },
// затем вызываетс€ circ_init
//

//
// push - увеличивает datalen. ƒанные складываютс€ за pointer'ом очередной байт не догонит pointer
// pop - отдает данные, начина€ с pointer. Pointer увеличиваетс€, а datalen уменьшаетс€.
// peek - отдает данные, но не измен€€ pointer и datalen
//

struct circ_buffer {
	uint8_t *start;					// указатель на начало буфера.  онстанта
	int length;						// 

	/* Private */
	volatile uint8_t *pointer;		// указывает на первый непрочтенный байт в буфере. ѕри чтении (pop) увеличиваетс€, а datalen уменьшаетс€. ѕри push'е не измен€етс€.
	volatile int datalen;			// говорит сколько содержательных байт лежит после pointer'a. ”величиваетс€, когда засовываем (push) в буфер
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