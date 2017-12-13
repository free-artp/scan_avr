/*
 * menu.h
 *
 * Created: 04.12.2017 15:16:08
 *  Author: Artp
 */ 


#ifndef MENU_H_
#define MENU_H_

#include <avr/pgmspace.h>
#include "../RTOS/messages.h"


typedef struct PROGMEM{
	const void		*Next;
	const void		*Previous;
	const void		*Parent;
	const void		*Child;
	const uint8_t	Select;
	const char		Text[];
} menuItem;

#define MAKE_MENU(Name, Next, Previous, Parent, Child, Select, Text) \
	const extern menuItem Next;     \
	const extern menuItem Previous; \
	const extern menuItem Parent;   \
	const extern menuItem Child;  \
	const menuItem Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child, (uint8_t)Select, { Text }}

#define PREVIOUS   ((menuItem*)pgm_read_word(&selectedMenuItem->Previous))
#define NEXT       ((menuItem*)pgm_read_word(&selectedMenuItem->Next))
#define PARENT     ((menuItem*)pgm_read_word(&selectedMenuItem->Parent))
#define CHILD      ((menuItem*)pgm_read_word(&selectedMenuItem->Child))
#define SELECT		(pgm_read_byte(&selectedMenuItem->Select))


uint8_t startMenu();
void initMenu();
uint8_t selectMenu(msg_par par);




#endif /* MENU_H_ */