/*
 * CFile1.c
 *
 * Created: 18.11.2017 15:20:53
 *  Author: Artp
 */ 

#include "..\include\avrlibtypes.h"

#include "i2c_dev.h"

struct_rbuff_t i2c_wbuffer;

void init_IIC(){
	i2c_wbuffer.ipop = i2c_wbuffer.ipush = 0;
}

BOOL rbuff_push(struct_rbuff_t *buf, u08 num, ...){
	u08 *ptr = &num;
	if ( ((buf->ipush + num) & RBUFF_MASK) == buf->ipop ) {
		return FALSE;
	} else {
		while (num--) {
			buf->ipush = (buf->ipush+1) & RBUFF_MASK;
			buf->buffer[buf->ipush] = *ptr++;
		}
	}
	return TRUE;
}

u08 rbuff_pop(struct_rbuff_t * buf){
	if ( !rbuff_empty(&i2c_wbuffer) ) {
		u08 ch = buf->buffer[buf->ipop];
		buf->ipop = (buf->ipop+1)&RBUFF_MASK;
		return ch;
	}
	return (u08)-1;
}

//----------------------

void i2c_init(void)							// ��������� ������ �������
{
	i2c_PORT |= 1<<i2c_SCL|1<<i2c_SDA;			// ������� �������� �� ����, ����� ���� �� ��������� ����������
	i2c_DDR &=~(1<<i2c_SCL|1<<i2c_SDA);

	TWBR = 0xFF;         						// �������� �������
	TWSR = 0x03;
}


IIC_F MasterOutFunc;
IIC_F ErrorOutFunc;

#define i2c_start	TWCR = 1<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE
	
static u08 i2c_addr, i2c_byte = 0;
static u08 ds_tick, ds_byte;
static u08 ds_rs_flag = _BV(DS_RS); // 1 - simbol, 0 - command

void display_init();

void ds_write_byte() {
	switch (ds_tick) {
		case 0:		// RS=0/1, wr=0
			i2c_addr = DISPLAY_CTRL_REG;
			i2c_byte = ds_rs_flag | (0<<DS_RW) | (0<<DS_E);
			i2c_start;
			break;
		case 1:		// E=1
			i2c_byte |= ds_rs_flag | (0<<DS_RW) | (1<<DS_E);
			i2c_start;
			break;
		case 3:		// byte
			i2c_addr = DISPLAY_DATA_REG;
			i2c_byte = ds_byte;
			i2c_start;
			break;
		case 4:		// E=0
			i2c_addr = DISPLAY_CTRL_REG;
			i2c_byte = ds_rs_flag | (0<<DS_RW) | (0<<DS_E);;
			i2c_start;
			break;
		case 5:	// RS=1, wr=1
			i2c_addr = DISPLAY_CTRL_REG;
			i2c_byte = (1<<DS_RW) | (1<<DS_RW) | (0<<DS_E);;
			i2c_start;
			break;
		case 6:		// clr data
			i2c_addr = DISPLAY_DATA_REG;
			i2c_byte = 0;
			i2c_start;
			break;
		case 7:
			SetTask(display_init);
	}
}

void ds_write_cmd(u08 cmd){
	ds_tick = 0;
	ds_rs_flag = (0<<DS_RS);
	ds_byte = cmd;
	ds_write_byte();
}

void display_init(){
	ds_write_cmd(0x0E);
}


void i2c_test2();
void i2c_test(){
	MasterOutFunc = i2c_test2;
	ErrorOutFunc  = i2c_test2;
	
	i2c_addr = DISPLAY_DATA_REG;
	i2c_byte = (i2c_byte == 0)? 1 : ((i2c_byte<<1) & 0xFF);
	i2c_start;
}

void i2c_test2(){
	SetTimerTask(i2c_test, 200)	;
}

ISR(TWI_vect)								// ���������� TWI ��� ���� ���.
{
	switch(TWSR & 0xF8){						// �������� ���� ����������
		case 0x00:	// Bus Fail (������� ��������)
			MACRO_i2c_WhatDo_ErrorOut
			break;
		case 0x08:	// ����� ���, � ����� ��:
			TWDR = i2c_addr & 0xFE;			// Addr+W				(TWDR = KEYBD_REG | 0x01;			// Addr+R)
			TWCR = 0<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;  	// Go!
			break;
		case 0x10:	// ��������� ����� ���, � ����� ��
			break;
		case 0x18:	// ��� ������ SLA+W �������� ACK, � �����:
			TWDR = i2c_byte;
			TWCR = 0<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;  // Go! 
			break;
		case 0x20:	// ��� ������ SLA+W �������� NACK - ����� ���� �����, ���� ��� ��� ����.
			TWCR = 0<<TWSTA|1<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;			// ���� ���� Stop
			MACRO_i2c_WhatDo_ErrorOut
			break;
		case 0x28: 	// ���� ������ �������, �������� ACK!  (���� sawp - ��� ��� ���� ������. ���� sawsarp - ���� ������ ��������)
			TWCR = 0<<TWSTA|1<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;	// ���� Stop
			ds_tick++;
			MACRO_i2c_WhatDo_MasterOut
			break;
		case 0x30:	//���� ����, �� �������� NACK ������ ���. 1� �������� �������� ������� � ��� ����. 2� ����� �������.
			MACRO_i2c_WhatDo_MasterOut
			break;
		case 0x38:	//  �������� �� ����. ������� ��� �� ���������
			break;
		case 0x40: // ������� SLA+R �������� ���. � ������ ����� �������� �����
			break;
		case 0x48: // ������� SLA+R, �� �������� NACK. ������ slave ����� ��� ��� ��� ����.
			MACRO_i2c_WhatDo_ErrorOut
			break;
		case 0x50: // ������� ����.
			break;
		case 0x58:	// ��� �� ����� ��������� ����, ������� NACK ����� �������� � �����.
			MACRO_i2c_WhatDo_MasterOut
			break;
		case 0x68:	// RCV SLA+W Low Priority							// ������� ���� ����� �� ����� �������� ��������
		case 0x78:	// RCV SLA+W Low Priority (Broadcast)				// ��� ��� ��� ����������������� �����. �� �����
			break;
		case 0x60: // RCV SLA+W  Incoming?								// ��� ������ �������� ���� �����
		case 0x70: // RCV SLA+W  Incoming? (Broascast)					// ��� ����������������� �����
			break;
		case 0x80:	// RCV Data Byte									// � ��� �� ������� ���� ����. ��� ��� �����������������. �� �����
		case 0x90:	// RCV Data Byte (Broadcast)
			break;
		case 0x88: // RCV Last Byte										// ������� ��������� ����
		case 0x98: // RCV Last Byte (Broadcast)
			break;
		case 0xA0: // ��, �� �������� ��������� �����. �� �� ��� � ��� ������?
			break;
		case 0xB0:  // ������� ���� ����� �� ������ �� ����� �������� ��������
			break;
		case 0xA8:	// // ���� ������ ������� ���� ����� �� ������
			break;
		case 0xB8: // ������� ����, �������� ACK
			break;
		case 0xC0: // �� ������� ��������� ����, ������ � ��� ���, �������� NACK
		case 0xC8: // ��� ACK. � ������ ������ ��� ���. �.�. ������ ������ � ��� ���.
			break;
		default:
			break;
	}
}

