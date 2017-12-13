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
#include <avr/eeprom.h>

#include "../include/avrlibtypes.h"
#include "../RTOS/EERTOS.h"

#include "console_drv.h"
#include "opto22.h"
#include "comm2.h"

#include "scaner.h"


typedef struct gate_t {
	uint8_t dmts_d;				// задержка от створа сканера до гейта. 1 dmts = 10 см.
	uint8_t	dir;
} gate;

typedef struct wood_t {
	uint16_t	index;
	uint16_t	dmts_s;			// timestamp
	uint8_t		gate;			// номер кармана
} wood;


/*
	Будем хранить в nvram:
	
	uint16_t current_dmts		- Счетчик перемещения транспортера [dmts]
	uint16_t last_wood_index	- Индекс последнего измеренного бревна в буфере бревен
	и список стволов на транспортере
	
	список стволов:
	запоминаем cur_dmts и задержку до сработки гейта
		
*/

gate EEMEM e_Gates[] = {
		{50,	0},
		{50,	1},
		{100,	0},
		{100,	1},
};
uint16_t EEMEM e_current_dmts, e_last_wood_index;


gate gates[4];
volatile uint16_t current_dmts = 0 ;
volatile uint16_t last_wood_index = 0;

volatile u08 noanswer_fl = 0;

//=====================================================================
void init_wood_system(){
	// прочтем из eprom
	eeprom_read_block( (void*)gates, (const void*)e_Gates,sizeof(eGates));
	eeprom_read_block( (void*)&current_dmts, (const void*)&e_current_dmts,sizeof(current_dmts));
	eeprom_read_block( (void*)&e_last_wood_index, (const void*)&e_last_wood_index,sizeof(last_wood_index));
	
}

void new_packet(){
	o22_header_t *header = (o22_header_t*)rx_buff;
	o22_common_t *cmn = (o22_common_t*)&rx_buff[4];
	u16 crc, crc_buff;

	noanswer_fl = 0;
	
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

void check_answer(){
	if (noanswer_fl) {
		d_setcursor(0,0);
		d_putstringP(PSTR("no answer\0"));
	}
	SetTimerTask(new_req, 1000);
}

void new_req(){

	SetTimerTask(check_answer,100);
	noanswer_fl = 1;
	
	scanner_mk_req(CMD_COMMON_INFO, NULL, 0);

	//	scanner_mk_req(CMD_SCANER_INFO, NULL, 0);
	//	SetTimerTask(printbuff, 250);
	//	SetTimerTask(new_req, 500);
}

