/*
 * comm2.h
 *
 * Created: 16.11.2017 19:10:48
 *  Author: Artp
 */ 


#ifndef COMM2_H_
#define COMM2_H_

#include "opto22.h"


#define SCANNER_ADDR 1

#define TXBUFF_LEN (sizeof(o22_header_t) + 4 + 2)
#define RXBUFF_LEN (sizeof(o22_header_t) + sizeof(o22_wood_t) + sizeof(o22_common_t) + 2)

extern uint8_t tx_buff[TXBUFF_LEN];
extern uint8_t rx_buff[RXBUFF_LEN];

extern volatile uint8_t tx_ptr, rx_ptr, tx_datalen, rx_datalen;

void uart_init();
BOOL scanner_mk_req( unsigned char cmd, unsigned char* dop, size_t ldop);
void tohex(unsigned char * in, size_t insz, char * out, size_t outsz);
u16 crc16(const unsigned char* data_p, unsigned char length);

#endif /* COMM2_H_ */