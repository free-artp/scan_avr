/*
 * i2c_dev.h
 *
 * Created: 18.11.2017 15:21:44
 *  Author: Artp
 */ 


#ifndef I2C_DEV_H_
#define I2C_DEV_H_


#include <avr/io.h>
#include <avr/interrupt.h>

#include "../RTOS/EERTOS.h"

#define i2c_PORT	PORTC				// Порт где сидит нога TWI
#define i2c_DDR		DDRC
#define i2c_SCL		5					// Биты соответствующих выводов
#define i2c_SDA		4

// x = R/W	R x=1, W x=0
// 0b1101000x
#define DS1307					0xD0
// 0b0100aaax
#define BASE_PCF8574			0x40
// 0b0111aaax
#define BASE_PCF8574A			0x70

#define PCF8574_A0(x)			(x<<1)
#define PCF8574_A1(x)			(x<<2)
#define PCF8574_A2(x)			(x<<3)

#define KEYBD_REG				( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(0) | PCF8574_A0(0) )
#define SENSOR_REG				( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(0) | PCF8574_A0(1) )

#define DISPLAY_CTRL_REG		( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(1) | PCF8574_A0(0) )
#define DISPLAY_DATA_REG		( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(1) | PCF8574_A0(1) )

#define DS_RS	6
#define DS_RW	5
#define DS_E	4

#define RBUFF_MASK		0x1F
#define RBUFF_LEN		RBUFF_MASK+1		

typedef struct struct_rbuff {
	u08 buffer[RBUFF_LEN];
	u08 ipop, ipush;
} struct_rbuff_t;

#define rbuff_empty(x)	((x)->ipop == (x)->ipush)
#define rbuff_full(x)	( (((x)->ipush+1) & RBUFF_MASK) == (x)->ipop)


#define MACRO_i2c_WhatDo_MasterOut 	(MasterOutFunc)();
#define MACRO_i2c_WhatDo_ErrorOut   (ErrorOutFunc)();


typedef void (*IIC_F)(void);

extern IIC_F MasterOutFunc;
extern IIC_F ErrorOutFunc;

#endif /* I2C_DEV_H_ */