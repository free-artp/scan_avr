/*
 * Sortirovka.c
 *
 * Created: 13.11.2017 18:39:06
 *  Author: Artp
 */ 


#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stddef.h>

#include "../RTOS/HAL.h"
#include "../RTOS/EERTOS.h"
#include "../IIC_ultimate/IIC_ultimate.h"

#include "console_drv.h"
#include "opto22.h"
#include "comm2.h"

void new_req();
void new_packet();

volatile unsigned short current_dmts;
volatile unsigned short last_wood_index;


void d_test();
void d_test1();
void d_test2();
void d_test3();
void d_test4();
void d_testZ();

void d_testZ() {
	SetTask(d_test);
}

void d_test4() {
	d_setcursor(5,1);
	d_putstringP( PSTR("World!") );
	SetTimerTask(d_test1, 1000);
}

void d_test3() {
	d_putstring("Hello ");
	SetTimerTask(d_test4, 500);
}

void d_test2() {
	d_putchar('2');
	SetTimerTask(d_test3, 500);
}

void d_test1() {
	d_command(0x01);			// clear, cursor = 0, shift=0, i/d=1
	SetTimerTask(d_test2, 500);
}

void d_test() {
	PORTD &= ~(1<<PORTD5);	_delay_ms(0.2);	PORTD |= (1<<PORTD5);
	d_command(0x0E);			// display On, cursor On, blink Off
	SetTimerTask(d_test1, 500);
}


int main(void)
{
	wdt_enable(WDTO_4S);
	
//	display_init();
	
	InitAll();
	Init_i2c();
//	uart_init();

	d_init();


	InitRTOS();
	RunRTOS();	// разрешает прерывания

//	SetTask(new_req);
	SetTask(d_test);
	
	while(1)
	{
		wdt_reset();

		
		TaskManager();
	}

}

void new_packet(){
	o22_header_t *header = (o22_header_t *)rx_buff;

	switch (header->cmd) {
		case CMD_COMMON_INFO:
			header->address=1;
		break;
	}
	//....
//	SetTask(new_req);
}

void new_req(){
	scanner_mk_req(CMD_COMMON_INFO, NULL, 0);
	SetTimerTask(new_req, 1000);
}

