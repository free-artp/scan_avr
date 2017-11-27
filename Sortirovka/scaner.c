/*
 * scaner.c
 *
 * Created: 28.11.2017 0:57:19
 *  Author: Artp
 */ 


#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stddef.h>
#include <stdio.h>

#include "../include/avrlibtypes.h"
#include "../RTOS/EERTOS.h"

#include "console_drv.h"
#include "opto22.h"
#include "comm2.h"

#include "scaner.h"

volatile unsigned short current_dmts = 0 ;
volatile unsigned short last_wood_index = 0;

void new_packet(){
	o22_header_t *header = (o22_header_t*)rx_buff;
	o22_common_t *cmn = (o22_common_t*)&rx_buff[4];
	u16 crc, crc_buff;

	d_command(0x01);
	
	crc = crc16(rx_buff, header->packet_size);
	crc_buff = rx_buff[header->packet_size] <<8 | rx_buff[header->packet_size+1];
	if (crc == crc_buff) {
		
		current_dmts = cmn->cur_dmts;
		
		sprintf(str0,"%d %ud",cmn->last.index, current_dmts);
		d_putstring(str0);

		sprintf(str1,"%3d %3d %3d",cmn->last.d1, cmn->last.d2, cmn->last.d3);
		d_setcursor( 0, 1);
		d_putstring(str1);
		} else {
		d_putstringP(PSTR("CRC error\0"));
	}

	SetTimerTask(new_req, 1000);

}

void new_req(){
	scanner_mk_req(CMD_COMMON_INFO, NULL, 0);
	//	scanner_mk_req(CMD_SCANER_INFO, NULL, 0);
	//	SetTimerTask(printbuff, 250);
	//	SetTimerTask(new_req, 500);

}

