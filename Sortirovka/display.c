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
	i2c_Do &= i2c_Free;											// Освобождаем шину

	if(i2c_Do & (i2c_ERR_NA|i2c_ERR_BF)) {						// Если запись не удалась
		SetTimerTask(display_init_proto, 20);					// повторяем попытку
	} else {
		SetTask(SendAddrToSlave);								// Если все ок, то идем на следующий
	}														// Пункт задания - передача данных слейву 2

}

//--------------------------- Обмен по шине с дисплеем
/*
	M68-Type

	E=0		- исходно, а активный задний фронт
	R/S=0	- 0-команды, 1-данные
	R/W=0	- 1-чтение 0-запись
	
	init - записать 0 в E, R/S, R/W

	запись 0 в E, R/W. В R/S 0-команды/1-данные
	записать 1 в E
	записать в DATA_REG - dataB, dataE
	пауза
	записать 0 в E
	пауза
	
*/


//---------------------------

void display_init_proto(){
	if ( !i2c_display_init( display_inited ) ) {				// Если байт незаписался
		SetTimerTask(display_init_proto, 50);					// Повторить попытку через 50мс
	}
}

void display_init(){
	Init_i2c();
	SetTask(display_init_proto);
}