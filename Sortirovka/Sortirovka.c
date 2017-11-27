/*
 * Sortirovka.c
 *
 * Created: 13.11.2017 18:39:06
 *  Author: Artp
 */ 


#include <avr/io.h>
#include <avr/wdt.h>
//#include <util/delay.h>
#include <stddef.h>

//#include <stdio.h>

#include "../RTOS/HAL.h"
#include "../RTOS/EERTOS.h"
#include "../IIC_ultimate/IIC_ultimate.h"

#include "console_drv.h"
//#include "opto22.h"
#include "comm2.h"
#include "scaner.h"

int main(void)
{
	wdt_enable(WDTO_4S);
	
	InitAll();
	Init_i2c();
	uart_init();
	d_init();		// инициализация буферов дисплея

	InitRTOS();
	RunRTOS();		// разрешает прерывания

	d_start();		// включение и очистка экрана

	SetTimerTask(new_req,100);

	while(1)
	{
		wdt_reset();
		TaskManager();
	}

}
