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

//#pragma pack(push,1)

typedef struct o22_header {
	u08 address;		// адрес прибора
	u08 packet_size;	// размер пакета в байтах, не включая контрольную сумму
	u08 cmd;			// номер запроса
	u08 dop_byte_num;	// номер байта, с которго начинается доп.информация (всегда 4)
} o22_header_t;			// sizeof = 4 + 2(crc)

typedef struct o22_wood {
	u16 index;	// last wood
//	u08  time[4];
	uint32_t time;
	u16 d1;
	u16 d2;
	u16 d3;
	u16 length;
	u16 sbeg;
	u16 sbegk;
	u16 volume;
	u16 nums;
	u16 flags;
	u08  kr;
	u08  ov;
} o22_wood_t;				// sizeof = 26

typedef struct o22_common {
	u16 state;
	
	o22_wood_t		last;
	
	u08  reserve1;
	u08  reserve2;
	u08  reserve3;
	u08  reserve4;
	u16 cur_low_beamA;
	u16 cur_upp_beamA;
	u16 cur_low_beamB;
	u16 cur_upp_beamB;
	u16 cur_length;
	u16 cur_dmts;
	u16 cur_speed;
	
} o22_common_t;			// sizeof 18

//#pragma pack(pop)

#endif
