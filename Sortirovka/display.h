/*
 * display.h
 *
 * Created: 17.11.2017 15:35:30
 *  Author: Artp
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#define PCF8574_A0(x)			(x<<1)
#define PCF8574_A1(x)			(x<<2)
#define PCF8574_A2(x)			(x<<3)

#define BASE_PCF8574			0x40
#define BASE_PCF8574A			0x70

#define DISPLAY_DATA_REG		( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(1) | PCF8574_A0(1) )
#define DISPLAY_CONTROL_REG		( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(1) | PCF8574_A0(0) )

#define KEYBD_REG				( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(0) | PCF8574_A0(0) )


void display_init();

#endif /* DISPLAY_H_ */