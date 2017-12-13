/*
 * rtc.c
 *
 * Created: 03.12.2017 20:37:05
 *  Author: Artp
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "error.h"
#include "../IIC_ultimate/IIC_ultimate.h"
#include "../RTOS/EERTOS.h"
#include "rtc.h"

#include "console_drv.h"
#include <stdio.h>

uint8_t year, month, day, hour, minute, second;
const uint8_t ds1307_daysinmonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	
/*
 * transform decimal value to bcd
 */
uint8_t ds1307_dec2bcd(uint8_t val) {
	return val + 6 * (val / 10);
}

/*
 * transform bcd value to deciaml
 */
static uint8_t ds1307_bcd2dec(uint8_t val) {
	return val - 6 * (val >> 4);
}

/*
 * get number of days since 2000/01/01 (valid for 2001..2099)
 */
static uint16_t ds1307_date2days(uint8_t y, uint8_t m, uint8_t d) {
	uint16_t days = d;
	for (uint8_t i = 1; i < m; ++i)
		days += pgm_read_byte(ds1307_daysinmonth + i - 1);
	if (m > 2 && y % 4 == 0)
		++days;
	return days + 365 * y + (y + 3) / 4 - 1;
}


/*
 * get day of a week
 */
uint8_t ds1307_getdayofweek(uint8_t y, uint8_t m, uint8_t d) {
	uint16_t day = ds1307_date2days(y, m, d);
	return (day + 6) % 7;
}

/*
 * set date
 */

void rtc_setdate2(){
	
}

void rtc_setdate() {
	//sanitize data
	if (second < 0 || second > 59 ||
		minute < 0 || minute > 59 ||
		hour < 0 || hour > 23 ||
		day < 1 || day > 31 ||
		month < 1 || month > 12 ||
		year < 0 || year > 99) {
			ErrorCode = ERR_WRONG_DATE;
			return;
		}

	//sanitize day based on month
	if(day > pgm_read_byte(ds1307_daysinmonth + month - 1)) {
		ErrorCode = ERR_WRONG_DATE;
		return;
	}
	//get day of week
	uint8_t dayofweek = ds1307_getdayofweek(year, month, day);

	//write date
	if ( (i2c_Do & i2c_Busy) ) {
		SetTimerTask(rtc_setdate, i2c_Retrain);
		} else {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			i2c_Do = i2c_sawp | i2c_Busy;
			i2c_SlaveAddress = RTC_ADDR;
			i2c_index = 0;
			i2c_Buffer[i2c_index++] = 0x00;
			i2c_Buffer[i2c_index++] = ds1307_dec2bcd(second);
			i2c_Buffer[i2c_index++] = ds1307_dec2bcd(minute);
			i2c_Buffer[i2c_index++] = ds1307_dec2bcd(hour);
			i2c_Buffer[i2c_index++] = ds1307_dec2bcd(dayofweek);
			i2c_Buffer[i2c_index++] = ds1307_dec2bcd(day);
			i2c_Buffer[i2c_index++] = ds1307_dec2bcd(month);
			i2c_Buffer[i2c_index++] = ds1307_dec2bcd(year);
			i2c_ByteCount = i2c_index;
			i2c_index = 0;

			MasterOutFunc = rtc_setdate2;
			ErrorOutFunc =  rtc_setdate2;
			
			TWCR = 1<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;
		}
	}
	ErrorCode = ERR_NONE;
}

/*
 * get date
 */
void rtc_getdate() {
/*
	i2c_start_wait(DS1307_ADDR | I2C_WRITE);
	i2c_write(0x00);//stop oscillator
	i2c_stop();

	i2c_rep_start(DS1307_ADDR | I2C_READ);
	*second = ds1307_bcd2dec(i2c_readAck() & 0x7F);
	*minute = ds1307_bcd2dec(i2c_readAck());
	*hour = ds1307_bcd2dec(i2c_readAck());
	i2c_readAck();
	*day = ds1307_bcd2dec(i2c_readAck());
	*month = ds1307_bcd2dec(i2c_readAck());
	*year = ds1307_bcd2dec(i2c_readNak());
	i2c_stop();
*/
}


static uint16_t	nv_cnt = 1;

void nvram_write_end(){
	i2c_Do &= i2c_Free;											// Освобождаем шину
	d_setcursor(7,1);
	if(i2c_Do & (i2c_ERR_NA|i2c_ERR_BF)) {
		//
	}
	
}

void nvram_write( ) {
	
	if ( (i2c_Do & i2c_Busy) ) {
		SetTimerTask(nvram_write, i2c_Retrain);
		} else {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			i2c_Do = i2c_sawp | i2c_Busy;
			i2c_SlaveAddress = RTC_ADDR;
			i2c_index = 0;
			i2c_Buffer[i2c_index++] = 0x08;							// адрес
			i2c_Buffer[i2c_index++] = ((nv_cnt)>>8) & 0xFF;
			i2c_Buffer[i2c_index++] = nv_cnt & 0xFF;
			i2c_ByteCount = i2c_index;
			i2c_index = 0;

			MasterOutFunc = nvram_write_end;
			ErrorOutFunc =  nvram_write_end;
			
			TWCR = 1<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;
		}
	}
	
}
uint8_t nvram_write_block( void *ptr_ram, void *ptr_nvram, size_t len) {
	if (len>sizeof(i2c_Buffer)) return 1;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if ( (i2c_Do & i2c_Busy) ) {
			sys_wait_message(SYS_I2C_FREE);
		}
	}
	
	
}
//=============================================================================
void nvram_read2(){
	
	nv_cnt = (i2c_Buffer[0]<<8) | i2c_Buffer[1];
	i2c_Do &= i2c_Free;											// Освобождаем шину
	d_setcursor(7,1);
	if(i2c_Do & (i2c_ERR_NA|i2c_ERR_BF)) {
		d_setcursor(7,1);
		sprintf( str1, "E ");
		d_putstring(str1);
	}
	sprintf( str1, "%d", nv_cnt);
	d_putstring(str1);
	
}
void nvram_read() {
	if ( (i2c_Do & i2c_Busy) ) {
		SetTimerTask(nvram_read, i2c_Retrain);
		} else {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			i2c_index = 0;
			i2c_ByteCount = 2;
			i2c_SlaveAddress = RTC_ADDR;
			i2c_PageAddress[0] = 0x08;
			i2c_PageAddrIndex = 0;
			i2c_PageAddrCount = 1;

			MasterOutFunc = nvram_read2;
			ErrorOutFunc =  nvram_read2;

			i2c_Do = i2c_sawsarp | i2c_Busy;
			TWCR = 1<<TWSTA|0<<TWSTO|1<<TWINT|0<<TWEA|1<<TWEN|1<<TWIE;
		}
	}
}
