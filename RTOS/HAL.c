#include "HAL.h"

inline void InitAll(void)
{

	// 1- output, 0-input
	// Port B
	// 0 - ~SD_Select, 1 - E_Reset, 2 - E_Select, 3 - MOSI, 4 - MISO, 5 - SCK
	PORTB = _BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3) | _BV(PORTB5);
	DDRB = _BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3) | _BV(PORTB5);

	SPCR = (1<<SPE)|(1<<MSTR);								// enable SPI, master mode 0
	SPSR |= (1<<SPI2X);										// set the clock rate fck/2

	//
	// PORT C
	// 0-3 - выходные оптроны, 4 - SDA, 5 - SCL, 6 -SPI0 (~CS для spi на разъеме) , 7 - NC
	//
	PORTC = _BV(PORTC4) | _BV(PORTC5) | _BV(PORTC6);
	DDRC = _BV(PORTC0) |_BV(PORTC1) |_BV(PORTC2) |_BV(PORTC3) |_BV(PORTC4) | _BV(PORTC5) | _BV(PORTC6);

	// Port D
	// 0 -RxD, 1 - TxD, 2 - INT0 (клавиатура), 3 - INT1 (входные датчики), 4 - 485_RW (~RE/DE), 5,6 -reserve, 7 - SD_DETECT
	//
	PORTD = _BV(PORTD1) | _BV(PORTD4);
	DDRD = _BV(PORTD0) | _BV(PORTD4) | _BV(PORTD5) | _BV(PORTD6);
}



