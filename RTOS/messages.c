#include <avr/io.h>
#include <util/atomic.h>
#include "messages.h"
#include "EERTOS.h"

volatile iHandler lHandler[maxHandlers]; // список обработчиков
volatile uint8_t numHandlers;

volatile iMessage lMessage[maxMessages]; // буфер сообщений
volatile uint16_t lMesPointer, hMesPointer; // указатели на начало и конец буфера

// установка обработчика события
// вызывается: setHandler(MSG_KEY_PRESS, &checkKey);
void setHandler(msg_num msg, handler hnd) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (numHandlers < maxHandlers) {
			lHandler[numHandlers].hnd = hnd; // и регистрирем обработчик
			lHandler[numHandlers].msg = msg;
			numHandlers++;
		}
	}
}

// снятие обработчика события
// вызывается: killHandler(MSG_KEY_PRESS, &checkKey);
// удаляет один последний обработчик.
void killHandler(msg_num msg, handler hnd) {
	if (numHandlers==0)
		return;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		int8_t i, j;
		j = 0;
		for (i = numHandlers-1; i >= 0; i--) {

			if ((lHandler[i].msg == msg) && (lHandler[i].hnd == hnd)) {

				// сдвигаем все записи к началу списка, чтобы дырок не было
				for (j = i; j < numHandlers-1 ; j++) {
					lHandler[j].msg = lHandler[j + 1].msg;
					lHandler[j].hnd = lHandler[j + 1].hnd;
				}
				numHandlers--;
				break;
			}
		}
	}
}

// занести событие в очередь
// пример вызова: sendMessage(MSG_KEY_PRESS, KEY_MENU)
void sendMessage(msg_num msg, msg_par par) {

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		hMesPointer = (hMesPointer + 1) & (maxMessages - 1); // сдвигаем указатель головы

		lMessage[hMesPointer].msg = msg; // заносим событие и параметр
		lMessage[hMesPointer].par = par;
		if (hMesPointer == lMesPointer) { // догнали начало очереди, убиваем необработанное сообытие
			lMesPointer = (lMesPointer + 1) & (maxMessages - 1);
		}
	}
//	SetTask(dispatchMessage);
}


// обработка событий
void dispatchMessage(void) {
	int8_t i;
	uint8_t res;
	msg_num msg;
	msg_par par;

	if (hMesPointer == lMesPointer) { // если пустая очередь - возврат
		return;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		lMesPointer = (lMesPointer + 1) & (maxMessages - 1); // сдвинем указатель

		msg = lMessage[lMesPointer].msg;
		par = lMessage[lMesPointer].par;
	}

	if (msg != 0 && numHandlers > 0) {
		for (i = numHandlers - 1; i >= 0; i--) { // просматриваем обработчики с конца
			if (lHandler[i].msg == msg) { // последний занесенный имеет приоритет
				res = lHandler[i].hnd(par); // вызываем обработчик
				if (res) { // если обработчик вернул 1, перываем обработку события
					break;
				}
			}
		}
	}
}

void initMessages() {
	numHandlers = 0;
	lMesPointer = 0;
	hMesPointer = 0;
}
