/*
 * comm2.c
 *
 * Created: 16.11.2017 19:11:02
 *  Author: Artp
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stddef.h>
#include <util/delay.h>

#include <stdio.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "../RTOS/EERTOS.h"

#include "console_drv.h"

#include "pins.h"
#include "opto22.h"
#include "comm2.h"


uint8_t tx_buff[TXBUFF_LEN];
uint8_t rx_buff[RXBUFF_LEN];

volatile uint8_t tx_ptr, rx_ptr, tx_datalen, rx_datalen;


void new_packet();

void printbuff(){
		tohex(rx_buff, (size_t)8, str0, sizeof(str0) );
		d_clear();

//		d_command(0x01);
//		d_setcursor(0,0);
		d_putstring(str0);

		sprintf(str1, "%d", (INT)rx_datalen);
		d_setcursor( 0, 1);
		d_putstring(str1);
}

//#pragma region hardware_func

void uart_init() {

	tx_ptr = rx_ptr = 0;
	tx_datalen = rx_datalen = 0;

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */

	//	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | (1<<RXCIE0)| (1<<TXCIE0) | (1<<UDRIE0);   /* Разрешаем RX and TX, RXIE - байт принят, TXIE - байт отправлен, URDIE -буффер отправки пуст */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | (1<<RXCIE0)| (0<<TXCIE0) | (0<<UDRIE0);		/* Разрешаем RX and TX, RXIE - байт принят, TXIE - байт отправлен, URDIE -буффер отправки пуст */
}



//Обработчик прерывания по окончанию приёма байта
ISR( USART_RX_vect )
{
	unsigned char rxbyte = UDR0;
//	if (PIND & _BV(PORTD6)) PORTD &= ~(1<<PORTD6); else PORTD |= (1<<PORTD6);
	
	if( rx_ptr < RXBUFF_LEN ) {
		rx_buff[rx_ptr++] = rxbyte;
		rx_datalen++;
	}
	if ( ((o22_header_t *)rx_buff)->packet_size == rx_datalen) {
		SetTask(new_packet);
	}
}


// прерывание по опустошению сдвигового регистра передатчика
ISR( USART_TX_vect) {

//	if (PIND & _BV(PORTD6)) PORTD &= ~(1<<PORTD6); else PORTD |= (1<<PORTD6);

	if (UCSR0A & _BV(UDRE0)) {						// есди буфер передатчика пуст, то передавать больше нечего
		rx_datalen = rx_ptr = 0;					// и устанавливаем на начало буфера приема (теперь должен прийти ответный пакет)
		PORTD &= ~_BV(pin_RS485_RW);				// переключаем драйвер RS485 на прием
		UCSR0B &= ~(_BV(UDRIE0) | _BV(TXCIE0) );	// запрещаем любые прерывания от передатчика
	}
}

// прерывание по опустошению буфера передатчика (байт ушел в сдвиговый регистр передатчика)
ISR( USART_UDRE_vect )
{
	unsigned char txbyte;

//	if (PIND & _BV(PORTD6)) PORTD &= ~(1<<PORTD6); else PORTD |= (1<<PORTD6);

	if( tx_ptr < tx_datalen ) {					// если буфер не пуст, то передаем следующий байт. ptr уже указывает на след. байт. Поэтому < datalen а не datalen-1
		txbyte = tx_buff[tx_ptr++];
		UDR0 = txbyte;
	}
	else {
		UCSR0B &= ~_BV(UDRIE0);					//если данных в fifo больше нет то запрещаем это прерывание
	}
}

// разрешает прерывания по пустому буферу передатчика (начинает отсылку)
void uart_send_kick(){
//	PORTD &= ~(1<<PORTD5);	_delay_ms(0.2);	PORTD |= (1<<PORTD5);
//	if ( (PIND & _BV(pin_RS485_RW)) == 0)							// можно и проверять
	PORTD |= _BV(pin_RS485_RW);										// переключаем драйвер RS485 на передачу
		
	if ( !(UCSR0B & _BV(UDRIE0)) || !(UCSR0B & _BV(TXCIE0)) )			// и разрешаем прерывания по опустошению буфера передатчика.
	UCSR0B |= _BV(UDRIE0) |_BV(TXCIE0);
}
//#pragma endregion hard

/*
*********************************************************************************************************************
*/
u16 crc16(const unsigned char* data_p, unsigned char length){
	unsigned char x,i;
	unsigned short crc = 0;		// CRC-16 XMODEM. Otherwise 0xFFFF

	i = length;
	while (i--){
		x = crc >> 8 ^ *data_p++;
		x ^= x>>4;
		crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
	}
	return crc;
}
/*

typedef struct o22_header {
	unsigned char address;		// адрес прибора
	unsigned char packet_size;	// размер пакета в байтах, не включая контрольную сумму
	unsigned char cmd;			// номер запроса
	unsigned char dop_byte_num;	// номер байта, с которго начинается доп.информация (всегда 4)
} o22_header_t;

*/

//
// 110 		- 01-04-6e-04-c2d5
// 103(514) - 01 06 67 04 02 02 1a3a
//


BOOL scanner_mk_req( unsigned char cmd, unsigned char* dop, size_t ldop) {
	uint8_t* buffp = tx_buff;
	uint8_t packet_size=0;
	uint16_t tmp;

//		tohex(rx_buff, (size_t)8, str, sizeof(str) );

	switch (cmd) {
		case CMD_SCANER_INFO:	packet_size = 4;		break;
		case CMD_SET_DATEIME:	packet_size = 8;		break;
		case CMD_COMMON_INFO:	packet_size = 4;		break;
		case CMD_WOOD_INFO:		packet_size = 6;		break;
	}

	if( ( packet_size - ldop - sizeof(o22_header_t) != 0 )
		|| (packet_size +2 > TXBUFF_LEN) )
			return FALSE;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//		d_command(0x01);
//		d_putstring(str);

		*buffp++ = SCANNER_ADDR;
		*buffp++ = packet_size;
		*buffp++ = cmd;
		*buffp++ = 4;
		while (ldop>0) {
			*buffp++ = *dop++;
			ldop--;
		}
		tmp = crc16( tx_buff, packet_size );
	
		*buffp++ = (unsigned char) ((tmp >>8) & 0xFF);
		*buffp++ = (unsigned char) (tmp & 0xFF);
		tx_ptr = 0;
		tx_datalen = packet_size+2;
		
//		tohex(tx_buff, (size_t)tx_datalen, tmp_str, sizeof(tmp_str) );
//		d_command(0x01);
//		d_putstring(tmp_str);

	}

//	d_putchar('+');

	uart_send_kick();
	return TRUE;
}

//void tohex(unsigned char * in, u08 insz, char * out, u08 outsz)
void tohex(unsigned char * in, size_t insz, char * out, size_t outsz)
{
	unsigned char * pin = in;
	const char * hex = "0123456789ABCDEF";
	char * pout = out;
	for(; pin < in+insz; pout +=2, pin++){
		pout[0] = hex[(*pin>>4) & 0xF];
		pout[1] = hex[ *pin     & 0xF];
		if (pout + 2 - out > outsz){
			/* Better to truncate output string than overflow buffer */
			/* it would be still better to either return a status */
			/* or ensure the target buffer is large enough and it never happen */
			break;
		}
	}
	*pout = 0;
}
