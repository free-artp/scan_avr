#include "EERTOSHAL.h"

//RTOS ������ ���������� �������
inline void RunRTOS (void)
{
/*
TCCR2 = 1<<WGM21|4<<CS20; 				// Freq = CK/64 - ���������� ����� � ������������
										// ��������� ����� ���������� �������� ���������
TCNT2 = 0;								// ���������� ��������� �������� ���������
OCR2  = LO(TimerDivider); 				// ���������� �������� � ������� ���������
TIMSK = 0<<TOIE0|1<<OCF2|0<<TOIE0;		// ��������� ���������� RTOS - ������ ��
sei();
*/
/*
	TCCR2A = _BV(WGM21);				// ��������� ����� ���������� �������� ���������
	TCCR2B = _BV(CS22); 				// Freq = CK/64 - ���������� ����� � ������������
	TCNT2 = 0;							// ���������� ��������� �������� ���������

	OCR2A  = LO(TimerDivider); 			// ���������� �������� � ������� ���������

	TIMSK2 = _BV(OCIE2A);				// ��������� ���������� RTOS - ������ ��
*/

	TCCR1B |=  _BV(WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);		// PRESCAler 64
	OCR1A = 250; /* Magical constant - 1 ms */
	TIMSK1 |= _BV(OCIE1A);
	
	Enable_Interrupt;
	
}
