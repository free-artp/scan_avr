/*
 * comm.c
 *
 * Created: 14.11.2017 10:14:36
 *  Author: Artp
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include <stddef.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "comm.h"
#include "opto22.h"
#include "pins.h"

static uint8_t u_txbuff[UARTTX_BUFF_SIZE];
uint8_t u_rxbuff[UARTRX_BUFF_SIZE];

static struct circ_buffer txbuff ={
	.start = u_txbuff,
	.length = sizeof(u_txbuff),

};
static struct circ_buffer rxbuff = {
	.start = u_rxbuff,
	.length = sizeof(u_rxbuff),
};

#pragma region hardware_func

void uart_init() {

	circ_init(&rxbuff);
	circ_init(&txbuff);

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
//	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | (1<<RXCIE0)| (1<<TXCIE0) | (1<<UDRIE0);   /* Разрешаем RX and TX, RXIE - байт принят, TXIE - байт отправлен, URDIE -буффер отправки пуст */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | (1<<RXCIE0)| (0<<TXCIE0) | (0<<UDRIE0);   /* Разрешаем RX and TX, RXIE - байт принят, TXIE - байт отправлен, URDIE -буффер отправки пуст */
}

//Обработчик прерывания по окончанию приёма байта
ISR( USART_RX_vect )
{
	unsigned char rxbyte = UDR0;
	if( !circ_is_full( &rxbuff ) ) {
		circ_push( &rxbuff, &rxbyte, 1 );
	}
}

// прерывание по опустошению сдвигового регистра передатчика
ISR( USART_TX_vect) {
	if (UCSR0A & _BV(UDRE0)) {						// есди буфер передатчика пусто, то передавать больше нечего
		PORTB |= _BV(pin_RS485_RW);					// переключаем драйвер RS485 на прием
		UCSR0B &= ~(_BV(UDRIE0) | _BV(TXCIE0) );	// и запрещаем любые прерывания от передатчика
	}
}

// прерывание по опустошению буфера передатчика (байт ушел в сдвиговый регистр передатчика)
ISR( USART_UDRE_vect )
{
	unsigned char txbyte;
	if( circ_is_empty( &txbuff ) ) {		//если данных в fifo больше нет то запрещаем это прерывание
		UCSR0B &= ~_BV(UDRIE0);
	}
	else {											//иначе передаем следующий байт
		circ_pop( &txbuff, &txbyte, 1 );
		UDR0 = txbyte;
	}
}

// разрешает прерывания по пустому буферу передатчика (начинает отсылку)
void uart_send_push(){
		if (PINB & _BV(pin_RS485_RW))										// переключаем драйвер RS485 на передачу
			PORTB &= ~_BV(pin_RS485_RW);
		if ( !(UCSR0B & _BV(UDRIE0)) || !(UCSR0B & _BV(TXCIE0)) )			// и разрешаем прерывания по опустошению буфера передатчика.
			UCSR0B |= _BV(UDRIE0) |_BV(TXCIE0);
}
#pragma endregion hard

/*
*********************************************************************************************************************
*/
unsigned short crc16(const unsigned char* data_p, unsigned char length){
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
// Возвращает количество байт запроса, не поместившиеся в кольцевой буфер передачи
// 0 если все байты туда поместились
//
//
// 110 		- 01-04-6e-04-c2d5
// 103(514) - 01 06 67 04 02 02 1a3a
//

int scanner_mk_req( unsigned char cmd, unsigned char* dop, size_t ldop) {
	unsigned char buffer[10];	// hardcoded!!! 4 - header, 2-4 - ext info, 2 - crc
	unsigned char* buffp;
	uint16_t tmp;
	unsigned char packet_size=0;

	buffp = buffer;
	
	*buffp++ = SCAN_ADDR;
	switch (cmd) {
		case CMD_SCANER_INFO:	packet_size = 4;		break;
		case CMD_SET_DATEIME:	packet_size = 8;		break;
		case CMD_COMMON_INFO:	packet_size = 4;		break;
		case CMD_WOOD_INFO:		packet_size = 6;		break;
	}
	*buffp++ = packet_size;
	*buffp++ = cmd;
	*buffp++ = 4;
	while (ldop>0) {
		*buffp++ = *dop++;
		ldop--;
	}
	tmp = crc16( buffer, packet_size );
	
	*buffp++ = (unsigned char) ((tmp >>8) & 0xFF);
	*buffp++ = (unsigned char) (tmp & 0xFF);
	packet_size += 2;	//для CRC

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		tmp = circ_push( &txbuff, buffer, packet_size );
		if ( tmp > 0 ) 
			txbuff.datalen -= (packet_size - tmp);			 // не поместилось - откатить буфер назад. TODO: не структурненько работать с приватным свойством
	}

	if ( tmp > 0 )
		return 0;

	uart_send_push();
	
	return tmp;
}

int check_packet_in_buffer( unsigned char cmd ){
o22_header_t header;

	if (circ_datalen(&rxbuff) >= sizeof(o22_header_t)){
		circ_peek( &rxbuff, (uint8_t *)&header, sizeof(o22_header_t) );
		if (circ_datalen(&rxbuff)+2 >= header.packet_size)
			return 1;
	}
	return 0;
}

