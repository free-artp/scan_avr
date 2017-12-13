/*
 * rtc.h
 *
 * Created: 03.12.2017 20:37:19
 *  Author: Artp
 */ 


#ifndef RTC_H_
#define RTC_H_


#define RTC_ADDR	(0x68<<1)


extern uint8_t year, month, day, hour, minute, second;

//extern void rtc_init();
extern void rtc_setdate();
extern void rtc_getdate();


#endif /* RTC_H_ */