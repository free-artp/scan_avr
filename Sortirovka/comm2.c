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

#define BAUD 19200
#include <util/setbaud.h>

#include "../RTOS/EERTOS.h"

#include "pins.h"
#include "opto22.h"
#include "comm2.h"

uint8_t tx_buff[TXBUFF_LEN];
uint8_t rx_buff[RXBUFF_LEN];

volatile uint8_t tx_ptr, rx_ptr, tx_datalen, rx_datalen;

void new_packet();

#pragma region hardware_func

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
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | (1<<RXCIE0)| (0<<TXCIE0) | (0<<UDRIE0);   /* Разрешаем RX and TX, RXIE - байт принят, TXIE - байт отправлен, URDIE -буффер отправки пуст */
}

//Обработчик прерывания по окончанию приёма байта
ISR( USART_RX_vect )
{
	o22_header_t *header = (o22_header_t *)rx_buff;
	unsigned char rxbyte = UDR0;
	
	if( rx_ptr < rx_datalen-1 ) {
		rx_buff[rx_ptr++] = rxbyte;
		rx_datalen++;
	}
	
	if (rx_datalen >= header->packet_size +2 ) {
		SetTask(new_packet);
	}
}

// прерывание по опустошению сдвигового регистра передатчика
ISR( USART_TX_vect) {
	if (UCSR0A & _BV(UDRE0)) {						// есди буфер передатчика пусто, то передавать больше нечего
		PORTB |= _BV(pin_RS485_RW);					// переключаем драйвер RS485 на прием
		UCSR0B &= ~(_BV(UDRIE0) | _BV(TXCIE0) );	// запрещаем любые прерывания от передатчика
		rx_datalen = rx_ptr = 0;					// и устанавливаем на начало буфера приема (теперь должен прийти ответный пакет)
	}
}

// прерывание по опустошению буфера передатчика (байт ушел в сдвиговый регистр передатчика)
ISR( USART_UDRE_vect )
{
	unsigned char txbyte;
	if( tx_ptr < tx_datalen-1 ) {					// если буфер не пуст, то передаем следующий байт
		txbyte = tx_buff[tx_ptr++];
		UDR0 = txbyte;
	}
	else {
		UCSR0B &= ~_BV(UDRIE0);					//если данных в fifo больше нет то запрещаем это прерывание
	}
}

// разрешает прерывания по пустому буферу передатчика (начинает отсылку)
void uart_send_kick(){
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
// 110 		- 01-04-6e-04-c2d5
// 103(514) - 01 06 67 04 02 02 1a3a
//

BOOL scanner_mk_req( unsigned char cmd, unsigned char* dop, size_t ldop) {
	uint8_t* buffp = tx_buff;
	uint8_t packet_size=0;
	uint16_t tmp;

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
	}
	uart_send_kick();
	return TRUE;
}

BOOL check_packet_in_buffer( unsigned char cmd ){
o22_header_t *header = (o22_header_t *)tx_buff;

	if (rx_datalen >= sizeof(o22_header_t)){
		if (rx_datalen+2 >= header->packet_size)
			return TRUE;
	}
	return FALSE;
}

