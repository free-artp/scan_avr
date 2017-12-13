/*
 * menu.c
 *
 * Created: 04.12.2017 15:16:27
 *  Author: Artp
 */ 

#include "..\RTOS\EERTOS.h"
#include "menu.h"
#include "console_drv.h"

void nvram_write();
void nvram_read();

menuItem* selectedMenuItem; // текущий пункт меню

//menuItem* menuStack[10];
//volatile uint8_t menuStackTop;

const char strNULL[] PROGMEM = "";

#define NULL_ENTRY Null_Menu
const menuItem        Null_Menu = {(void*)0, (void*)0, (void*)0, (void*)0, 0, {0x00}};


enum {
	MENU_CANCEL=1,
	MENU_RESET,
	MENU_DATE,
	MENU_TIME,
	MENU_TEST1,
	MENU_TEST2
};

//                 NEXT,      PREVIOUS     PARENT,     CHILD
MAKE_MENU(m_s1i1,  m_s1i2,    NULL_ENTRY,  NULL_ENTRY, m_s2i1,       0,				"Test");
MAKE_MENU(m_s1i2,  m_s1i3,    m_s1i1,      NULL_ENTRY, m_s3i1,       0,				"Settings");
MAKE_MENU(m_s1i3,  NULL_ENTRY,m_s1i2,      NULL_ENTRY, NULL_ENTRY,   MENU_RESET,	"Reset");
//
// подменю ТЕСТ
MAKE_MENU(m_s2i1,	m_s2i2,		NULL_ENTRY,	m_s1i1,	NULL_ENTRY,	MENU_TEST1, "Write");
MAKE_MENU(m_s2i2,	NULL_ENTRY,	m_s2i1,		m_s1i1,	NULL_ENTRY,	MENU_TEST2, "Read");

// подменю Настройка
//                 NEXT,      PREVIOUS     PARENT,     CHILD
MAKE_MENU(m_s3i1,  m_s3i2,    NULL_ENTRY,  m_s1i2,     NULL_ENTRY,       MENU_DATE, "Date");
MAKE_MENU(m_s3i2,  NULL_ENTRY,m_s3i1,      m_s1i2,     NULL_ENTRY,       MENU_TIME, "Time");


void taskMenu(){
	startMenu();
}
uint8_t selectMenu(msg_par par) {
	switch (par) {
		case MENU_RESET: { // тут обработать выбор пункта Mode 1
			break;
		}
		case MENU_DATE: { // тут обработать выбор пункта Mode 2
			break;
		}
		case MENU_TIME: { // тут обработать выбор пункта Mode 3
			break;
		}
		case MENU_TEST1: {
			SetTask(nvram_write);
			break;
		}
		case MENU_TEST2: {
			SetTask(nvram_read);
			break;
		}
	}

	// после выбора возвращаемся в главное меню.
	SetTask(taskMenu);
	return 1;
}


void menuChange(menuItem* NewMenu)
{
	if ((void*)NewMenu == (void*)&NULL_ENTRY)
	return;

	selectedMenuItem = NewMenu;
}


unsigned char dispMenu(msg_par par) {
	menuItem* tempMenu;

	d_clear();
	d_setcursor(0,0);
	
	tempMenu = (menuItem*)pgm_read_word(&selectedMenuItem->Parent);
	if ((void*)tempMenu == (void*)&NULL_ENTRY) { // мы на верхнем уровне
		d_putstringP(PSTR("MENU:"));
	} else {
		d_putstringP(tempMenu->Text);
	}

	d_setcursor(0,1);
	d_putstringP(selectedMenuItem->Text);
	
	return (1);
}

uint8_t menuKey(msg_par par) {
	switch (par) {
		case 0: {
			return 1;
		}
		case KEY_LEFT: {
			menuChange(PREVIOUS);
			break;
		}
		case KEY_RIGHT: {
			menuChange(NEXT);
			break;
		}
		case KEY_SELECT:		 // выбор пункта
		{
			uint8_t sel;
			sel = SELECT;
			if (sel != 0) {
				sendMessage(MSG_MENU_SELECT, sel);

				killHandler(MSG_KEY_PRESS, &menuKey);
				killHandler(MSG_DISP_REFRESH, &dispMenu);

				return (1);
				} else {
				menuChange(CHILD);
			}
			break;
		}
		case KEY_MENU:			// отмена выбора (возврат)
		{
			menuChange(PARENT);
			break;
		}
	}
	dispMenu(0);
	return (1);
}

uint8_t startMenu() {
	selectedMenuItem = (menuItem*)&m_s1i2;

	dispMenu(0);
	setHandler(MSG_KEY_PRESS, &menuKey);
	setHandler(MSG_DISP_REFRESH, &dispMenu);
	return (0);
}

