/*
 * console_drv.c
 *
 * Created: 20.11.2017 15:08:22
 *  Author: Artp
 *
 * Драйвер дисплея и клавиатуры
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include <stddef.h>

#include "../RTOS/EERTOS.h"

#include "../include/avrlibtypes.h"
#include "../circ/circ.h"
#include "../IIC_ultimate/IIC_ultimate.h"

#include "console_drv.h"

#include<util/delay.h>

u08 buff_inp[DRV_BUFF];
u08 buff_out[DRV_BUFF];

char str0[17]="0123456789ABCDEF\0";
char str1[17]="0123456789ABCDEF\0";


struct circ_buffer drv_queue_inp = {
	.start = buff_inp,
	.length = sizeof(buff_inp),
};

struct circ_buffer drv_queue_out = {
	.start = buff_out,
	.length = sizeof(buff_out),
};


u08 m_Buffer[MEDIUM_BUFF];
u08 m_index = 0, m_ByteCount = 0;
u08 m_cmd = DISP_NONE;
u08 *m_ptr = NULL;
u16 m_delay = 0;	// пауза, которую надо выждать после посылки всего пакета (несколько записей в разные слейвы)
u08 m_delay_fl = 0;

void parser_m();


// получает управление из TaskManager по инициативе:
//	- функции вывода на дисплей. Они кладут в кольцевой буфер и ставят таск на выполнение parser_d
//	- парсера следующего уровня, которому понадобился очередной пакет
//
// Парсер берет из кольцевого входного буфера (drv_queue_inp) очередную конструкцию типа <CMD><байт> или <CMD><указатель>.
//  ( CMD - не имеет отношения к командам дисплея. Это команды вида: вывести байт, вывести строку (DISP_xxx) )
// и кладет в промежуточный линейный буфер (m_Buffer) блоки вида: <адрес слейва><кол-во байт><байт-1><байт-2>...<байт-N>.
// Каждый такой набор блоков передается без освобождения драйвера i2c шины (IIC_ultimate), т.е. никто не может вклиниться (ну только другой мастер).
// Это нужно, например, когда вывод в некий девайс осуществляется генерацией стробов путем выставления битиков в регистре.
// Устанавливает длину данных в буфере и индекс, после чего шедулит таск парсера следующего уровня.
//
// Парсер следующего уровня (parser_m) захватывает драйвер i2c, устанавливает себя как адрес выхода из автомата, вытаскивает из m_Buffer атомарные отправки (на один адрес слейва)
// и засовывает их в конечный автомат IIC_ultimate, сдвигая m_index.
// Когда parcer_m вынет весь набор заготовленных для него в m_Buffer блоков, он освободит шину i2c и снова зашедулит parcer_d, чтобы получить очередную порцию блоков.
//
// В ситуации, когда во входной очереди была команда вывода строки (т.е. указатель на zero-ended массив в RAM или PGM_SPACE, то мы запоминаем команду в m_cmd и указатель в m_ptr
// и при очередном входе в parcer_d руководствуемся ими, а не данными из drv_queue_inp
//

void parser_d(){
	u08 cmd;
	u08 RS_bit;
	u08 data;

//	PORTD &= ~(1<<PORTD6);	_delay_ms(0.1);	PORTD |= (1<<PORTD6);	_delay_ms(0.1);

	if  (m_ByteCount > 0) {							// Уже идет процесс общения. m_Buffer не пуст и parser_m еще не все из него вынул для отправки
		SetTimerTask(parser_d, 50);
		return;
	}
	if ( (m_cmd == DISP_NONE) && (circ_is_empty(&drv_queue_inp)) ) {
		return;
	}

	if (m_cmd) {
		cmd = m_cmd;
	} else {
		circ_pop( &drv_queue_inp, &cmd, 1);
	}
		
	RS_bit = _BV(DS_RS);		// по дефолту - символ
	m_delay = 0;				// и отсутствие паузы в конце посылки. Нужна для отработки комманд дисплеем
	m_delay_fl = 0;
		
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
			circ_pop(&drv_queue_inp, (u08 *)&m_ptr, 2);
			m_cmd = DISP_STRP_N;
		case DISP_STRP_N:
			data = pgm_read_byte(m_ptr);				//загрузить байт из PGM_SPACE
			m_ptr++;
			if(data == 0) m_cmd = cmd = DISP_NONE;
			break;

		case DISP_CMD:	
			m_delay = DS_CMD_DELAY_DEFAULT;				// задержка для выполнения команды. Отрабатывает ПОСЛЕ посылки команды.
			RS_bit = 0;
		case DISP_CHR:
			circ_pop( &drv_queue_inp, &data, 1);
			break;
	}
	// если команда требует паузы, то задать ее величину
	if (cmd) {
		m_index = 0;
		m_ByteCount = 0;

		m_Buffer[m_index++] = DISPLAY_CTRL_REG;
		m_Buffer[m_index++] = 2;
		m_Buffer[m_index++] = (0<<DS_E) | (0<<DS_RW) | RS_bit;		// строб, запись, команда
		m_Buffer[m_index++] = (1<<DS_E) | (0<<DS_RW) | RS_bit;		// строб, запись, команда

		m_Buffer[m_index++] = DISPLAY_DATA_REG;
		m_Buffer[m_index++] = 1;
		m_Buffer[m_index++] = data;									// команда или символ

		m_Buffer[m_index++] = DISPLAY_CTRL_REG;
		m_Buffer[m_index++] = 2;
		m_Buffer[m_index++] = (0<<DS_E) | (0<<DS_RW) | RS_bit;		// строб, запись, команда
		m_Buffer[m_index++] = (0<<DS_E) | (1<<DS_RW) | (1<<DS_RS);	// строб, запись, команда

		m_ByteCount = m_index;
		m_index = 0;
		SetTask(parser_m);
	}
}


void parser_m_afterpause(){
	m_delay_fl = 0;
	SetTask(parser_d);
}


//
// сюда приходим либо из очереди дисплея (он сформировал несколько посылок для одной команды, но в разные регистры): strob - strob - data - strob,
// либо после окончания посылки в один из регистров
// тут как то надо быть если запись не удалась - if(i2c_Do & (i2c_ERR_NA|i2c_ERR_BF))
// по-видимому, нужно отсигналить парсеру дисплея, чтобы он откатил указатель в своем буфере и  заново сформировал пачку посылок
//
// m_Buffer : <SlaveAddr>, <N>, <byte-1>, <byte-2> ... <byte-N>
//
void parser_m(){
	u08 cnt = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (!m_delay_fl)			// проверка семафора(?) задержки. Можно бы, перепоставить себя в очередь, но это произойдет в parser_m_afterpause
		{
			if ((m_index == 0) && (i2c_Do & i2c_Busy)) {		// если это первый заход в пачке (т.е. нас вызвал parser_d), то подождем освобождения шины. Иначе - шина уже захвачена нами
				SetTimerTask(parser_m, 5);
			} else {
				if (m_index < m_ByteCount) {			// m_Buffer : <SlaveAddr>, <N>, <byte-1>, <byte-2> ... <byte-N>
					i2c_Do = i2c_sawp | i2c_Busy;
					MasterOutFunc = parser_m;
					ErrorOutFunc =  parser_m;
					i2c_index = 0;
					i2c_SlaveAddress = m_Buffer[m_index++];
					cnt = m_Buffer[m_index++];
					while (cnt--) {
						i2c_Buffer[i2c_index++] = m_Buffer[m_index++];
					}
					i2c_ByteCount = i2c_index;
					i2c_index = 0;
					TWCR = 1<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;
				} else {
					i2c_Do &= i2c_Free;
					m_index = m_ByteCount = 0;
					if (m_delay) {	// если нужна задержка для выполнения команды, то  флаг паузы - m_pause_fl и SetTimerTask для сброса флага
						SetTimerTask(parser_m_afterpause, m_delay);
						m_delay_fl = 1;
						m_delay = 0;
					}
					SetTask(parser_d);
				}
			}
		}
	}
} // И уходим обратно в IIC_ultimate, где выходим из прерывания, ну или в TaskManager


// ф-ии работы с дисплеем верхнего уровня

void d_init() {
	circ_init( &drv_queue_inp);
	circ_init( &drv_queue_out);
}

void d_on(){
	d_command(0x0E);			// display On, cursor On, blink Off
	SetTask(parser_d);
}

void d_clear() {
	d_command(0x01);			// clear, cursor = 0, shift=0, i/d=1
	SetTask(parser_d);
}

void d_setcursor (u08 column, u08 row) {
	u08 cmd;
	row %= 2;
	column %= 0x40;
	cmd = 0x80 | (row * 0x40 + column);
	d_command(cmd);
}

void d_command(u08 cmd){
	circ_push_byte( &drv_queue_inp, DISP_CMD);
	circ_push_byte( &drv_queue_inp, cmd);
	SetTask(parser_d);
}

void d_putchar(char chr){
	circ_push_byte( &drv_queue_inp, DISP_CHR);
	circ_push_byte( &drv_queue_inp, chr);
	SetTask(parser_d);
}

void d_putstring(char *str){
	circ_push_byte( &drv_queue_inp, DISP_STR);
	circ_push( &drv_queue_inp, (u08*)&str, 2);
	SetTask(parser_d);
}

void d_putstringP(const char *str){
	circ_push_byte( &drv_queue_inp, DISP_STRP);
	circ_push( &drv_queue_inp, (u08*)&str, 2);
	SetTask(parser_d);
}

void d_start() {
	circ_push_byte( &drv_queue_inp, DISP_CMD);
	circ_push_byte( &drv_queue_inp, 0x01);		// clear, cursor = 0, shift=0, i/d=1
	circ_push_byte( &drv_queue_inp, DISP_CMD);
	circ_push_byte( &drv_queue_inp, 0x0E);		// display On, cursor On, blink Off
	SetTask(parser_d);
}


//============================

uint8_t lastKey,prevKey;
uint8_t kf1,kf2,kf3;

void kbd_read();

void kbd_readed(){
	u08 kf;
	kf = i2c_Buffer[0];
	
	i2c_Do &= i2c_Free;											// Освобождаем шину
	if(i2c_Do & (i2c_ERR_NA|i2c_ERR_BF))						// Если запись не удалась
	{
		SetTimerTask(kbd_read,20);								// повторяем попытку
	}
	else														// Если все ок, то ...
	{
		kf3=kf2;
		kf2=kf1;
		kf1 = kf;
	}
}

void kbd_read(){
	if ( (i2c_Do & i2c_Busy) ) {
		SetTimerTask(kbd_read, 5);
		} else {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			i2c_Do = i2c_sarp | i2c_Busy;
			i2c_SlaveAddress = KBD_REG;
			i2c_index = 0;
			i2c_ByteCount = 1;
			
			MasterOutFunc = kbd_readed;
			ErrorOutFunc =  kbd_readed;
			
			TWCR = 1<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;
		}
	}
}

// прерывание по изменению состояния входных пинов PCF8574


// включить подтяжку
// настроить на HI-to-LOW

// TODO надо проверить  как себя ведет PCF при отпускании клавиши. Может если в бит записать 0, то он даст прерывание на отпускание ? Тогда просто с автоповтором.
//
// An interrupt is generated by any rising or falling edge of the port inputs in the input mode.
// ...
// Resetting and reactivating the interrupt circuit is achieved when data on the port is changed to the original setting
// or data is read from, or written to, the port that generated the interrupt.
// ...
// The I/Os should be high before being used as inputs
//
// получается, что:
// записали единички
// притянули бит в 0 - прерывание
// прочитали - прерывание снялось
// отпустили бит в 1 - прерывание
// прочитали - прерывание снялось
//
ISR(INT0_vect) {
	SetTask(kbd_read);
}

