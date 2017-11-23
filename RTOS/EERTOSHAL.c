#include "EERTOSHAL.h"

//RTOS Запуск системного таймера
inline void RunRTOS (void)
{
/*
TCCR2 = 1<<WGM21|4<<CS20; 				// Freq = CK/64 - Установить режим и предделитель
										// Автосброс после достижения регистра сравнения
TCNT2 = 0;								// Установить начальное значение счётчиков
OCR2  = LO(TimerDivider); 				// Установить значение в регистр сравнения
TIMSK = 0<<TOIE0|1<<OCF2|0<<TOIE0;		// Разрешаем прерывание RTOS - запуск ОС
sei();
*/
/*
	TCCR2A = _BV(WGM21);				// Автосброс после достижения регистра сравнения
	TCCR2B = _BV(CS22); 				// Freq = CK/64 - Установить режим и предделитель
	TCNT2 = 0;							// Установить начальное значение счётчиков

	OCR2A  = LO(TimerDivider); 			// Установить значение в регистр сравнения

	TIMSK2 = _BV(OCIE2A);				// Разрешаем прерывание RTOS - запуск ОС
*/

	TCCR1B |=  _BV(WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);		// PRESCAler 64
	OCR1A = 250; /* Magical constant - 1 ms */
	TIMSK1 |= _BV(OCIE1A);
	
	Enable_Interrupt;
	
}
