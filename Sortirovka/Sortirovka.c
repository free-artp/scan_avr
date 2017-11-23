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

#include "opto22.h"
#include "comm2.h"
#include "display.h"

void new_req();
void new_packet();

volatile unsigned short current_dmts;
volatile unsigned short last_wood_index;

//void i2c_init();
//void i2c_test();



int main(void)
{
	wdt_enable(WDTO_4S);
	
//	display_init();
	
	InitAll();
//	uart_init();
//	i2c_init();


	InitRTOS();
	RunRTOS();	// разрешает прерывания

//	SetTask(new_req);
//	SetTask(display_init);
//	SetTask(i2c_test);

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

