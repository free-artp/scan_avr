#include <avr/io.h>
#include <util/atomic.h>
#include "messages.h"
#include "EERTOS.h"

volatile iHandler lHandler[maxHandlers]; // ������ ������������
volatile uint8_t numHandlers;

volatile iMessage lMessage[maxMessages]; // ����� ���������
volatile uint16_t lMesPointer, hMesPointer; // ��������� �� ������ � ����� ������

// ��������� ����������� �������
// ����������: setHandler(MSG_KEY_PRESS, &checkKey);
void setHandler(msg_num msg, handler hnd) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (numHandlers < maxHandlers) {
			lHandler[numHandlers].hnd = hnd; // � ����������� ����������
			lHandler[numHandlers].msg = msg;
			numHandlers++;
		}
	}
}

// ������ ����������� �������
// ����������: killHandler(MSG_KEY_PRESS, &checkKey);
// ������� ���� ��������� ����������.
void killHandler(msg_num msg, handler hnd) {
	if (numHandlers==0)
		return;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		int8_t i, j;
		j = 0;
		for (i = numHandlers-1; i >= 0; i--) {

			if ((lHandler[i].msg == msg) && (lHandler[i].hnd == hnd)) {

				// �������� ��� ������ � ������ ������, ����� ����� �� ����
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

// ������� ������� � �������
// ������ ������: sendMessage(MSG_KEY_PRESS, KEY_MENU)
void sendMessage(msg_num msg, msg_par par) {

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		hMesPointer = (hMesPointer + 1) & (maxMessages - 1); // �������� ��������� ������

		lMessage[hMesPointer].msg = msg; // ������� ������� � ��������
		lMessage[hMesPointer].par = par;
		if (hMesPointer == lMesPointer) { // ������� ������ �������, ������� �������������� ��������
			lMesPointer = (lMesPointer + 1) & (maxMessages - 1);
		}
	}
//	SetTask(dispatchMessage);
}


// ��������� �������
void dispatchMessage(void) {
	int8_t i;
	uint8_t res;
	msg_num msg;
	msg_par par;

	if (hMesPointer == lMesPointer) { // ���� ������ ������� - �������
		return;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		lMesPointer = (lMesPointer + 1) & (maxMessages - 1); // ������� ���������

		msg = lMessage[lMesPointer].msg;
		par = lMessage[lMesPointer].par;
	}

	if (msg != 0 && numHandlers > 0) {
		for (i = numHandlers - 1; i >= 0; i--) { // ������������� ����������� � �����
			if (lHandler[i].msg == msg) { // ��������� ���������� ����� ���������
				res = lHandler[i].hnd(par); // �������� ����������
				if (res) { // ���� ���������� ������ 1, �������� ��������� �������
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
