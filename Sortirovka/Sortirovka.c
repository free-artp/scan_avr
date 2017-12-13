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
#include "../RTOS/messages.h"

#include "../IIC_ultimate/IIC_ultimate.h"

#include "console_drv.h"
#include "comm2.h"
#include "scaner.h"
#include "menu.h"

int main(void)
{
	wdt_enable(WDTO_4S);
	
	InitAll();
	Init_i2c();
	uart_init();
	d_init();		// инициализация буферов дисплея

	initMessages();
	InitRTOS();
	
	RunRTOS();		// разрешает прерывания

	d_start();		// включение и очистка экрана
	kbd_init();		// единички в регистр (чтобы было, что притягивать к 0), разрешаем INT0
	
	SetTimerTask(new_req,100);

	setHandler(MSG_MENU_SELECT, &selectMenu);
	startMenu(0);
	
	while(1)
	{
		wdt_reset();
//		dispatchMessage();		// вызывает (не шедулет!!!) обработчики с параметром
		TaskManager();			// выполняет таски зашедуленные "просто" и через "time-сервис"
	}

}
