/*
 * display.c
 *
 * Created: 17.11.2017 15:09:32
 *  Author: Artp
 */ 


#include "../IIC_ultimate/IIC_ultimate.h"
#include "../RTOS/EERTOS.h"



u08 i2c_display_init( IIC_F WhatDo){
	if (i2c_Do & i2c_Busy) return 0;
	i2c_index = 0;
	i2c_ByteCount = 1;
	i2c_SlaveAddress = DISPLAY_DATA_REG;
	i2c_Buffer[0] = 0;

	i2c_Do = i2c_sawp;

	MasterOutFunc = WhatDo;
	ErrorOutFunc = WhatDo;

	TWCR = 1<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;

	i2c_Do |= i2c_Busy;

	return 1;

}

void display_inited(){
	i2c_Do &= i2c_Free;											// ����������� ����

	if(i2c_Do & (i2c_ERR_NA|i2c_ERR_BF)) {						// ���� ������ �� �������
		SetTimerTask(display_init_proto, 20);					// ��������� �������
	} else {
		SetTask(SendAddrToSlave);								// ���� ��� ��, �� ���� �� ���������
	}														// ����� ������� - �������� ������ ������ 2

}

//--------------------------- ����� �� ���� � ��������
/*
	M68-Type

	E=0		- �������, � �������� ������ �����
	R/S=0	- 0-�������, 1-������
	R/W=0	- 1-������ 0-������
	
	init - �������� 0 � E, R/S, R/W

	������ 0 � E, R/W. � R/S 0-�������/1-������
	�������� 1 � E
	�������� � DATA_REG - dataB, dataE
	�����
	�������� 0 � E
	�����
	
*/


//---------------------------

void display_init_proto(){
	if ( !i2c_display_init( display_inited ) ) {				// ���� ���� �����������
		SetTimerTask(display_init_proto, 50);					// ��������� ������� ����� 50��
	}
}

void display_init(){
	Init_i2c();
	SetTask(display_init_proto);
}