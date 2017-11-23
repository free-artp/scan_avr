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

void i2c_init(void)							// Настройка режима мастера
{
	i2c_PORT |= 1<<i2c_SCL|1<<i2c_SDA;			// Включим подтяжку на ноги, вдруг юзер на резисторы пожмотился
	i2c_DDR &=~(1<<i2c_SCL|1<<i2c_SDA);

	TWBR = 0xFF;         						// Настроим битрейт
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

ISR(TWI_vect)								// Прерывание TWI Тут наше все.
{
	switch(TWSR & 0xF8){						// Отсекаем биты прескалера
		case 0x00:	// Bus Fail (автобус сломался)
			MACRO_i2c_WhatDo_ErrorOut
			break;
		case 0x08:	// Старт был, а затем мы:
			TWDR = i2c_addr & 0xFE;			// Addr+W				(TWDR = KEYBD_REG | 0x01;			// Addr+R)
			TWCR = 0<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;  	// Go!
			break;
		case 0x10:	// Повторный старт был, а затем мы
			break;
		case 0x18:	// Был послан SLA+W получили ACK, а затем:
			TWDR = i2c_byte;
			TWCR = 0<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;  // Go! 
			break;
		case 0x20:	// Был послан SLA+W получили NACK - слейв либо занят, либо его нет дома.
			TWCR = 0<<TWSTA|1<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;			// Шлем шине Stop
			MACRO_i2c_WhatDo_ErrorOut
			break;
		case 0x28: 	// Байт данных послали, получили ACK!  (если sawp - это был байт данных. если sawsarp - байт адреса страницы)
			TWCR = 0<<TWSTA|1<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;	// Шлем Stop
			ds_tick++;
			MACRO_i2c_WhatDo_MasterOut
			break;
		case 0x30:	//Байт ушел, но получили NACK причин две. 1я передача оборвана слейвом и так надо. 2я слейв сглючил.
			MACRO_i2c_WhatDo_MasterOut
			break;
		case 0x38:	//  Коллизия на шине. Нашелся кто то поглавней
			break;
		case 0x40: // Послали SLA+R получили АСК. А теперь будем получать байты
			break;
		case 0x48: // Послали SLA+R, но получили NACK. Видать slave занят или его нет дома.
			MACRO_i2c_WhatDo_ErrorOut
			break;
		case 0x50: // Приняли байт.
			break;
		case 0x58:	// Вот мы взяли последний байт, сказали NACK слейв обиделся и отпал.
			MACRO_i2c_WhatDo_MasterOut
			break;
		case 0x68:	// RCV SLA+W Low Priority							// Словили свой адрес во время передачи мастером
		case 0x78:	// RCV SLA+W Low Priority (Broadcast)				// Или это был широковещательный пакет. Не важно
			break;
		case 0x60: // RCV SLA+W  Incoming?								// Или просто получили свой адрес
		case 0x70: // RCV SLA+W  Incoming? (Broascast)					// Или широковещательный пакет
			break;
		case 0x80:	// RCV Data Byte									// И вот мы приняли этот байт. Наш или широковещательный. Не важно
		case 0x90:	// RCV Data Byte (Broadcast)
			break;
		case 0x88: // RCV Last Byte										// Приянли последний байт
		case 0x98: // RCV Last Byte (Broadcast)
			break;
		case 0xA0: // Ой, мы получили Повторный старт. Но чо нам с ним делать?
			break;
		case 0xB0:  // Поймали свой адрес на чтение во время передачи Мастером
			break;
		case 0xA8:	// // Либо просто словили свой адрес на чтение
			break;
		case 0xB8: // Послали байт, получили ACK
			break;
		case 0xC0: // Мы выслали последний байт, больше у нас нет, получили NACK
		case 0xC8: // или ACK. В данном случае нам пох. Т.к. больше байтов у нас нет.
			break;
		default:
			break;
	}
}

