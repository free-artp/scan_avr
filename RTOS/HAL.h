#ifndef HAL_H
#define HAL_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include "avrlibtypes.h"
//#include "avrlibdefs.h"
#include "avr/pgmspace.h"
#include <avr/wdt.h>
/*
//Clock Config
#ifndef F_CPU
#define F_CPU 8000000L
#endif
*/
//System Timer Config
#define Prescaler	  		64
#define	TimerDivider  		(F_CPU/Prescaler/1000)		// 1 mS

#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)

#define i2c_PORT	PORTC				// Порт где сидит нога TWI
#define i2c_DDR		DDRC
#define i2c_SCL		5					// Биты соответствующих выводов
#define i2c_SDA		4

extern void InitAll(void);



#endif
