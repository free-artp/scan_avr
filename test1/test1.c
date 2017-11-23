/*
 * test1.c
 *
 * Created: 19.11.2017 13:39:23
 *  Author: Artp
*/

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	// Port D initialization
	// Function: Bit7=In Bit6=Out Bit5=Out Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
	DDRD=(0<<DDD7) | (1<<DDD6) | (1<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
	// State: Bit7=T Bit6=0 Bit5=0 Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
	PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);

    while(1)
    {
        PORTD = (1<<PORTD5);
        _delay_ms(500);
        PORTD = 0;
        _delay_ms(500);
        PORTD = (1<<PORTD6);
        _delay_ms(500);
        PORTD = 0;
        _delay_ms(500);
    }
}