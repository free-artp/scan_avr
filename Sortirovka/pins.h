/*
 * pins.h
 *
 * Created: 15.11.2017 13:50:40
 *  Author: Artp
 */ 


#ifndef PINS_H_
#define PINS_H_


// PortB
// 0 - ~SD_Select, 1 - E_Reset, 2 - E_Select, 3 - MOSI, 4 - MISO, 5 - SCK
#define pin_SD_SELECT	PORTB0
#define pin_E_RESET		PORTB1
#define pin_E_SELECT	PORTB2
#define pin_MOSI		PORTB3
#define pin_MISO		PORTB4
#define pin_SCK			PORTB5

// PORTC
// 0-3 - выходные оптроны, 4 - SDA, 5 - SCL, 6 -SPI0 (~CS для spi на разъеме) , 7 - NC
#define pin_OUT1		PORTC0
#define pin_OUT2		PORTC1
#define pin_OUT3		PORTC2
#define pin_OUT4		PORTC3
#define pin_IIC_SDA		PORTC4
#define pin_IIC_SCL		PORTC5
#define pin_SPI_SELECT	PORTC6

// Port D
// 0 -RxD, 1 - TxD, 2 - INT0 (клавиатура), 3 - INT1 (входные датчики), 4 - 485_RW (~RE/DE), 5,6 -reserve, 7 - SD_DETECT

#define pin_UART_RxD	PORTD0
#define pin_UART_TxD	PORTD1
#define pin_INT_KBD		PORTD2
#define pin_INT_SENSOR	PORTD3
#define pin_RS485_RW	PORTD4
#define pin_SD_DETECT	PORTD7



#endif /* PINS_H_ */