/*
 * console_drv.h
 *
 * Created: 20.11.2017 15:08:37
 *  Author: Artp
 */ 


#ifndef CONSOLE_DRV_H_
#define CONSOLE_DRV_H_

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

#define KBD_REG					( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(0) | PCF8574_A0(0) )
#define SENSOR_REG				( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(0) | PCF8574_A0(1) )

#define DISPLAY_CTRL_REG		( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(1) | PCF8574_A0(0) )
#define DISPLAY_DATA_REG		( BASE_PCF8574 | PCF8574_A2(0) | PCF8574_A1(1) | PCF8574_A0(1) )

#define DS_RS	6
#define DS_RW	5
#define DS_E	4

// upper level buffer

#define DISP_NONE		0
#define DISP_CMD		0b00000010			// <DISP_CMD> <cmd>
#define DISP_CHR		0b00000100			// <DISP_CHR> <char>
#define DISP_STR		0b00001000			// <DISP_STR> <HI(*char)> <LO(*char)>
#define DISP_STR_N		0b00001001			// �������� �� ������
#define DISP_STRP		0b00010000			// <DISP_STRP> <HI(*char PROGMEM)> <LO(*char PROGMEM)>
#define DISP_STRP_N		0b00010001			// �������� �� ������

#endif /* CONSOLE_DRV_H_ */