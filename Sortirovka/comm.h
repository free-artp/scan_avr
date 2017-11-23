/*
 * comm.h
 *
 * Created: 14.11.2017 13:25:29
 *  Author: Artp
 */ 


#ifndef COMM_H_
#define COMM_H_

#include <stddef.h>
#include "../circ/circ.h"

#define SCAN_ADDR 1

#define UARTRX_BUFF_SIZE 128
#define UARTTX_BUFF_SIZE 10

//struct circ_buffer txbuff, rxbuff;

void uart_init();

int scanner_mk_req( unsigned char cmd, unsigned char* dop, size_t ldop);

#endif /* COMM_H_ */