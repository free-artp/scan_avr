/*
 * console_drv.c
 *
 * Created: 20.11.2017 15:08:22
 *  Author: Artp
 *
 * ������� ������� � ����������
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "avrlibtypes.h"
#include "..\circ\circ.h"
#include "..\IIC_ultimate\IIC_ultimate.h"

#include "console_drv.h"

#define DRV_BUFF	15
u08 buff_inp[DRV_BUFF];
u08 buff_out[DRV_BUFF];

struct circ_buffer drv_queue_inp{
	.start = buff_inp,
	.length = sizeof(buff_inp),
};

struct circ_buffer drv_queue_out{
	.start = buff_out,
	.length = sizeof(buff_out),
};

#define MEDIUM_BUFF	32
u08 m_Buffer[MEDIUM_BUFF];
u08 m_index = 0, m_ByteCount = 0;
u08 m_cmd = DISP_NONE;
u08 *m_ptr = NULL;

void init_parsers() {
	circ_init( &drv_queue_inp);
	circ_init( &drv_queue_out);
}

// �������� ���������� �� TaskManager �� ����������:
//	- ������� ������ �� �������. ��� ������ � ��������� ����� � ������ ���� �� ���������� parser_d
//	- ������� ���������� ������, �������� ����������� ��������� �����
//
// ������ ����� �� ���������� �������� ������ (drv_queue_inp) ��������� ����������� ���� <CMD><����> ��� <CMD><���������>.
//  ( CMD - �� ����� ��������� � �������� �������. ��� ������� ����: ������� ����, ������� ������ (DISP_xxx) )
// � ������ � ������������� �������� ����� (m_Buffer) ����� ����: <����� ������><���-�� ����><����-1><����-2>...<����-N>.
// ������ ����� ����� ������ ���������� ��� ������������ �������� i2c ���� (IIC_ultimate), �.�. ����� �� ����� ���������� (�� ������ ������ ������).
// ��� �����, ��������, ����� ����� � ����� ������ �������������� ���������� ������� ����� ����������� ������� � ��������.
// ������������� ����� ������ � ������ � ������, ����� ���� ������� ���� ������� ���������� ������.
//
// ������ ���������� ������ (parser_m) ����������� ������� i2c, ������������� ���� ��� ����� ������ �� ��������, ����������� �� m_Buffer ��������� �������� (�� ���� ����� ������)
// � ���������� �� � �������� ������� IIC_ultimate, ������� m_index.
// ����� parcer_m ����� ���� ����� ������������� ��� ���� � m_Buffer ������, �� ��������� ���� i2c � ����� ��������� parcer_d, ����� �������� ��������� ������ ������.
//
// � ��������, ����� �� ������� ������� ���� ������� ������ ������ (�.�. ��������� �� zero-ended ������ � RAM ��� PGM_SPACE, �� �� ���������� ������� � m_cmd � ��������� � m_ptr
// � ��� ��������� ����� � parcer_d ��������������� ���, � �� ������� �� drv_queue_inp
//

void parser_d(){
	u08 cmd;
	u08 RS_bit;
	u08 data;

	if  (m_index < m_ByteCount) {							// ��� ���� ������� �������. m_Buffer �� ���� � parser_m ��� �� ��� �� ���� ����� ��� ��������
		SetTimerTask(parser_d, 10);
		return;
	}
	if ( (m_cmd==0) && (circ_is_empty(&drv_queue_inp)) ) {	//
		return;
	}
	if (m_cmd) {
		cmd = m_cmd;
	} else {
		circ_pop( &drv_queue_inp, &cmd, 1) );
	}
		
	RS_bit = _BV(DS_RS);		// �� ������� - ������
		
	switch(cmd) {

		case DISP_STR:
			circ_pop(&drv_queue_inp, (u08 *)&m_ptr, 2);
			m_cmd = DISP_STR_N;
		case DISP_STR_N:
			data = *m_ptr;
			m_ptr++;
			if(data == 0) m_cmd = cmd = DISP_NONE;
			break;

		case DISP_STRP:
			circ_pop(&drv_queue_inp, &m_ptr, 2);
			m_cmd = DISP_STRP_N;
		case DISP_STRP_N:
			data = pgm_read_byte(m_ptr);				//��������� ���� �� PGM_SPACE
			m_ptr++;
			if(data == 0) m_cmd = cmd = DISP_NONE;
			break;

		case DISP_CMD:	// �������� !!!!
			RS_bit = 0;
		case DISP_CHR:
			circ_pop( &drv_queue_inp, &data, 1))
			break;
	}
	// ���� ������� ������� �����, �� ������ �� ��������
	if (cmd) {
		m_index = 0;
		m_ByteCount = 0;

		m_Buffer[m_index++] = DISPLAY_CTRL_REG;
		m_Buffer[m_index++] = 2;
		m_Buffer[m_index++] = (0<<DS_E) | (0<<DS_RW) | RS_bit;		// �����, ������, �������
		m_Buffer[m_index++] = (1<<DS_E) | (0<<DS_RW) | RS_bit;		// �����, ������, �������

		m_Buffer[m_index++] = DISPLAY_DATA_REG;
		m_Buffer[m_index++] = 1;
		m_Buffer[m_index++] = data;									// ������� ��� ������

		m_Buffer[m_index++] = DISPLAY_CTRL_REG;
		m_Buffer[m_index++] = 2;
		m_Buffer[m_index++] = (0<<DS_E) | (0<<DS_RW) | RS_bit;		// �����, ������, �������
		m_Buffer[m_index++] = (0<<DS_E) | (1<<DS_RW) | (1<<DS_RS);	// �����, ������, �������

		m_ByteCount = m_index;
		m_index = 0;
		SetTask(parser_m);
	}
}

//
// ���������� �������� �� TasManager'� ��� �������� IIC_ultimate
// ����������� i2c-����, ��������� m_Buffer, ������ ������ ����� IIC_ultimate
//
void parser_m(){
	u08 cnt;

	// �������� ����� �������� �, ��������, ������������� ���� � ������� � �����


// �����������	
	if ((m_index == 0) && (i2c_Do & i2c_Busy)) {		// ���� ��� ������ ����� � �����, �� �������� ������������ ����. ����� - ���� ��������� ����
		SetTimerTask(parser_m, 50);
	} else {
		//
		// ���� �������� ���� �� ������� ������� (�� ����������� ��������� ������� ��� ����� �������, �� � ������ ��������): strob - strob - data - strob,
		// ���� ����� ��������� ������� � ���� �� ���������
		// ��� ��� �� ���� ���� ���� ������ �� ������� - if(i2c_Do & (i2c_ERR_NA|i2c_ERR_BF))
		// ��-��������, ����� ����������� ������� �������, ����� �� ������� ��������� � ����� ������ �  ������ ����������� ����� �������
		//
		if (m_index < m_ByteCount) {
			// m_Buffer : <SlaveAddr>, <N>, <byte-1>, <byte-2> ... <byte-N>
			i2c_Do = i2c_sawp | i2c_Busy;
			i2c_index = 0;
			i2c_SlaveAddress = m_Buffer[m_index++];
			cnt = m_Buffer[m_index++];
			while (cnt--) {
				i2c_Buffer[i2c_index++] = m_Buffer[m_index++];
			}
			i2c_ByteCount = i2c_index;
			i2c_index = 0;

			MasterOutFunc = parser_m;
			ErrorOutFunc =  parser_m;

			TWCR = 1<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;

		} else {
			i2c_Do &= i2c_Free;
			// ���� ����� ��������, �� ���������� ���� ����� � SetTimerTask ��� ������ �����
			SetTask(parser_d);	// �������� ��� ������ !!!
		}
	}
}

// �-�� ������ � �������� �������� ������

void d_init() {
	
}

void d_clear() {
	
}

void d_setcursor (u08 column, row) {
	
}

void d_putchar(char chr){
	
}

void d_putstring(char *str){
	char pp="aaaaaaa";
	circ_push(drv_queue_inp,(void *)pp, 2);
	
}

void d_putstringP(char *str){
	
}
