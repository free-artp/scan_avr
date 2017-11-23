#ifndef _OPTO22_H
#define _OPTO22_H


#define CMD_SCANER_INFO	110
#define CMD_SET_DATEIME 111
#define CMD_COMMON_INFO	  0
#define CMD_WOOD_INFO	103

/*

SCANER_REQS = {
	CMD_SCANER_INFO: { 'size': 4, 'ext_size':None,	'info':'Scaner info' },
	CMD_SET_DATEIME: { 'size': 8, 'ext_size': 4,	'info':'Set datetime' },	// ext - Datetime
	CMD_COMMON_INFO: { 'size': 4, 'ext_size':None,	'info':'Common info' },
	CMD_WOOD_INFO:	 { 'size': 6, 'ext_size': 2,	'info':'Wood info' }		// wood_no
	}

*/

#pragma pack(push,1)

typedef struct o22_header {
	unsigned char address;		// адрес прибора
	unsigned char packet_size;	// размер пакета в байтах, не включая контрольную сумму
	unsigned char cmd;			// номер запроса
	unsigned char dop_byte_num;	// номер байта, с которго начинается доп.информация (всегда 4)
} o22_header_t;			// sizeof = 4 + 2(crc)

typedef struct o22_wood {
	unsigned short index;	// last wood
	unsigned long  time;
	unsigned short d1;
	unsigned short d2;
	unsigned short d3;
	unsigned short length;
	unsigned short sbeg;
	unsigned short sbegk;
	unsigned short volume;
	unsigned short nums;
	unsigned short flags;
	unsigned char  kr;
	unsigned char  ov;
} o22_wood_t;				// sizeof = 26

typedef struct o22_common {
	unsigned short state;
	
	o22_wood_t		last;
	
	unsigned char  reserve1;
	unsigned char  reserve2;
	unsigned char  reserve3;
	unsigned char  reserve4;
	unsigned short cur_low_beamA;
	unsigned short cur_upp_beamA;
	unsigned short cur_low_beamB;
	unsigned short cur_upp_beamB;
	unsigned short cur_length;
	unsigned short cur_dmts;
	unsigned short cur_speed;
	
} o22_common_t;			// sizeof 18

#pragma pack(pop)

#endif
