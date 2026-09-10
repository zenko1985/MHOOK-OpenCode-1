#include <Windows.h>
#include "MHKeypad.h"
#include "Settings.h"
#include "MHRepErr.h"
#include "Scancode.h"
extern HWND		MHhwnd;
int MHKeypad::keypad_position=-1;
WORD MHKeypad::scancode[17]={SC_UP, SC_RIGHT, SC_DOWN, SC_LEFT, SC_F1, SC_F1};
// Для поиска глюков
#ifdef _DEBUG
#endif
static int button_pressed[15]={0};
// Таблица кодирования 8 направлений четырьмя клавишами
int key8[8][4]=
{
	{1,0,0,0}, // 0-е направление = нулевая кнопка
	{1,1,0,0}, // 1-е направление = кнопки 0 и 1
	{0,1,0,0}, // 2
	{0,1,1,0}, // 3
	{0,0,1,0}, // 4
	{0,0,1,1}, // 5
	{0,0,0,1}, // 6
	{1,0,0,1} // 7
};
void MHKeypad::Init(const WORD (&scancodes)[17])
{
	if(-1!=keypad_position) Reset();
	for (int i = 0; i < 17; i++)
		scancode[i] = scancodes[i];
}
//=========================================================================
// Преобразует нажатие клавиши в структуру INPUT (без SendInput)
// Возвращает количество записанных INPUT (0, 1 или 2)
//=========================================================================
int MHKeypad::PressKeyToInput(int position, bool down, int shift, INPUT *out)
{
	if (position + shift < 0 || position + shift >= 17) return 0;
	int count = 0;
	WORD sc = scancode[position + shift];
	if (SC_NONE == sc) return 0;
	if (sc == SC_LMOUSE || sc == SC_RMOUSE || sc == SC_MIDDLEMB)
	{
		out[count].type = INPUT_MOUSE;
		out[count].mi.dx = 0;
		out[count].mi.dy = 0;
		out[count].mi.mouseData = 0;
		out[count].mi.dwExtraInfo = 0;
		out[count].mi.time = 0;
		if (sc == SC_MIDDLEMB)
			out[count].mi.dwFlags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
		else
			out[count].mi.dwFlags = down
				? (sc == SC_LMOUSE ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN)
				: (sc == SC_LMOUSE ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP);
		count++;
	}
	else if (sc == SC_WHEEL_UP || sc == SC_WHEEL_DOWN)
	{
		if (down)
		{
			out[count].type = INPUT_MOUSE;
			out[count].mi.dx = 0;
			out[count].mi.dy = 0;
			out[count].mi.mouseData = (sc == SC_WHEEL_UP) ? WHEEL_DELTA : -WHEEL_DELTA;
			out[count].mi.dwFlags = MOUSEEVENTF_WHEEL;
			out[count].mi.dwExtraInfo = 0;
			out[count].mi.time = 0;
			count++;
		}
	}
	else
	{
		out[count].type = INPUT_KEYBOARD;
		out[count].ki.dwFlags = KEYEVENTF_SCANCODE;
		if (!down) out[count].ki.dwFlags |= KEYEVENTF_KEYUP;
		if (sc > 0xFF)
			out[count].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
		out[count].ki.wScan = sc;
		count++;
	}
	// Поддержка второй клавиши для ЛКМ и ПКМ
	if (position == 5 || position == 10) {
		int second_key_index = position == 5 ? 15 : 16;
		WORD sc2 = scancode[second_key_index];
		if (SC_NONE != sc2) {
			if (sc2 == SC_LMOUSE || sc2 == SC_RMOUSE || sc2 == SC_MIDDLEMB)
			{
				out[count].type = INPUT_MOUSE;
				out[count].mi.dx = 0;
				out[count].mi.dy = 0;
				out[count].mi.mouseData = 0;
				out[count].mi.dwExtraInfo = 0;
				out[count].mi.time = 0;
				if (sc2 == SC_MIDDLEMB)
					out[count].mi.dwFlags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
				else
					out[count].mi.dwFlags = down
						? (sc2 == SC_LMOUSE ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN)
						: (sc2 == SC_LMOUSE ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP);
				count++;
			}
			else if (sc2 == SC_WHEEL_UP || sc2 == SC_WHEEL_DOWN)
			{
				if (down)
				{
					out[count].type = INPUT_MOUSE;
					out[count].mi.dx = 0;
					out[count].mi.dy = 0;
					out[count].mi.mouseData = (sc2 == SC_WHEEL_UP) ? WHEEL_DELTA : -WHEEL_DELTA;
					out[count].mi.dwFlags = MOUSEEVENTF_WHEEL;
					out[count].mi.dwExtraInfo = 0;
					out[count].mi.time = 0;
					count++;
				}
			}
			else
			{
				out[count].type = INPUT_KEYBOARD;
				out[count].ki.dwFlags = KEYEVENTF_SCANCODE;
				if (!down) out[count].ki.dwFlags |= KEYEVENTF_KEYUP;
				if (sc2 > 0xFF)
					out[count].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
				out[count].ki.wScan = sc2;
				count++;
			}
		}
	}
	return count;
}
bool MHKeypad::Press4(int position, bool down, int shift)
{
	if (position + shift < 0 || position + shift >= 17) return false;
#ifdef _DEBUG
	if((position<0)||(position>10))
  		MHReportError(L"Неверный аргумент у Press4");
	button_pressed[position+shift] = down ? 1 : 0;
#endif
	int num_of_keys = 0;
	INPUT inputs[2] = { 0 };
	INPUT input1 = { 0 };
	INPUT input2 = { 0 };
	WORD sc = scancode[position + shift];
	if (SC_NONE != sc) {
		if (sc == SC_LMOUSE || sc == SC_RMOUSE || sc == SC_MIDDLEMB)
		{
			input1.type = INPUT_MOUSE;
			if (sc == SC_MIDDLEMB)
				input1.mi.dwFlags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
			else
				input1.mi.dwFlags = down
					? (sc == SC_LMOUSE ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN)
					: (sc == SC_LMOUSE ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP);
			input1.mi.dwExtraInfo = 0;
			input1.mi.mouseData = 0;
			input1.mi.time = 0;
			input1.mi.dx = 0;
			input1.mi.dy = 0;
		}
		else if (sc == SC_WHEEL_UP || sc == SC_WHEEL_DOWN)
		{
			if (down)
			{
				input1.type = INPUT_MOUSE;
				input1.mi.dwFlags = MOUSEEVENTF_WHEEL;
				input1.mi.mouseData = (sc == SC_WHEEL_UP) ? WHEEL_DELTA : -WHEEL_DELTA;
				input1.mi.dwExtraInfo = 0;
				input1.mi.time = 0;
				input1.mi.dx = 0;
				input1.mi.dy = 0;
			}
		}
		else
		{
			input1.type = INPUT_KEYBOARD;
			input1.ki.dwFlags = KEYEVENTF_SCANCODE;
			if (!down) input1.ki.dwFlags |= KEYEVENTF_KEYUP;
			if (sc > 0xFF)
				input1.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
			input1.ki.wScan = sc;
		}
		inputs[num_of_keys] = input1;
		++num_of_keys;
	}
	// поддержка второй клавиши для ЛКМ и ПКМ
	bool only_second_key = false;
	if (position == 5 || position == 10) {
		int second_key_index = position == 5 ? 15 : 16;
		WORD sc2 = scancode[second_key_index];
		if (SC_NONE != sc2) {
			if (sc2 == SC_LMOUSE || sc2 == SC_RMOUSE || sc2 == SC_MIDDLEMB)
			{
				input2.type = INPUT_MOUSE;
				if (sc2 == SC_MIDDLEMB)
					input2.mi.dwFlags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
				else
					input2.mi.dwFlags = down
						? (sc2 == SC_LMOUSE ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN)
						: (sc2 == SC_LMOUSE ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP);
				input2.mi.dwExtraInfo = 0;
				input2.mi.mouseData = 0;
				input2.mi.time = 0;
				input2.mi.dx = 0;
				input2.mi.dy = 0;
			}
			else if (sc2 == SC_WHEEL_UP || sc2 == SC_WHEEL_DOWN)
			{
				if (down)
				{
					input2.type = INPUT_MOUSE;
					input2.mi.dwFlags = MOUSEEVENTF_WHEEL;
					input2.mi.mouseData = (sc2 == SC_WHEEL_UP) ? WHEEL_DELTA : -WHEEL_DELTA;
					input2.mi.dwExtraInfo = 0;
					input2.mi.time = 0;
					input2.mi.dx = 0;
					input2.mi.dy = 0;
				}
			}
			else
			{
				input2.type = INPUT_KEYBOARD;
				input2.ki.dwFlags = KEYEVENTF_SCANCODE;
				if (!down) input2.ki.dwFlags |= KEYEVENTF_KEYUP;
				if (sc2 > 0xFF)
					input2.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
				input2.ki.wScan = sc2;
			}
			inputs[num_of_keys] = input2;
			++num_of_keys;
			only_second_key = num_of_keys == 1;
		}
	}
	if (num_of_keys == 0) {
		return false;
	}
	SendInput(num_of_keys, inputs, sizeof(INPUT));
	// if only second key pressed return false to also process the mouse button
	if (only_second_key) {
		return false;
	}
#ifdef _DEBUG
	TCHAR debug_buf[4096];
	swprintf_s(debug_buf,_countof(debug_buf),L"position: %d, button_pressed: %d %d %d %d %d %d  %d %d %d %d  %d  %d %d %d %d\r\n",
		position, button_pressed[0],button_pressed[1],button_pressed[2],button_pressed[3],button_pressed[4],
		button_pressed[5],
		button_pressed[6],button_pressed[7],button_pressed[8],button_pressed[9],button_pressed[10],
		button_pressed[11],button_pressed[12],button_pressed[13],button_pressed[14]);
	OutputDebugString(debug_buf);
#endif
	return true;
}
void MHKeypad::Press(int position, bool down, int shift)
{
	if (position < 0 || position > 7) return;
	if(down && (position==keypad_position))
	{
#ifdef _DEBUG
		OutputDebugString(L"Нажали уже нажатую кнопку");
#endif
		return;
	}
	if(!down && (position!=keypad_position))
	{
#ifdef _DEBUG
		OutputDebugString(L"Отжали ненажатую кнопку");
#endif
		return;
	}
	if(-1==position)
	{
#ifdef _DEBUG
		OutputDebugString(L"Нажали кнопку -1");
#endif
		return;
	}
	if(4==MHSettings::GetNumPositions())
	{
		if(true==down)
		{
			Reset(shift);
			keypad_position=position;
		}
		else
		{
			keypad_position=-1;
		}
		Press4(position,down,shift);
	}
	else
	{
		if(true==down)
		{
			// Батчинг: собираем все нажатия/отпускания в один SendInput
			INPUT all_inputs[8]; // макс 4 отжатия + 4 нажатия
			int total = 0;
			for(int i=0;i<4;i++)
			{
				if(-1!=keypad_position)
				{
					if (key8[keypad_position][i] == key8[position][i]) {
						continue;
					}
					if (1 == key8[keypad_position][i]) {
						total += PressKeyToInput(i, false, shift, &all_inputs[total]);
					}
					else {
						total += PressKeyToInput(i, true, shift, &all_inputs[total]);
					}
				}
				else
				{
					if (1 == key8[position][i]) {
						total += PressKeyToInput(i, true, shift, &all_inputs[total]);
					}
				}
			}
			if(total > 0) SendInput(total, all_inputs, sizeof(INPUT));
			keypad_position=position;
		}
		else
		{
			Reset(shift);
			keypad_position=-1;
		}
	}
}
//====================================================================================
// Сбросить все нажатые кнопки
//====================================================================================
void MHKeypad::Reset(int shift)
{
	if(4==MHSettings::GetNumPositions())
	{
		// Отжимаем одну кнопку
		if(-1!=keypad_position) Press4(keypad_position, false, shift);
	}
	else // когда 8 направлений движения
	{
		// Батчинг: собираем все отпускания в один SendInput
		INPUT all_inputs[8]; // макс 4 отпускания * 2 (вторая клавиша)
		int total = 0;
		switch(keypad_position)
		{
			// Отжимаем одну кнопку
		case 0:
		case 2:
		case 4:
		case 6:
			total += PressKeyToInput(keypad_position/2, false, shift, &all_inputs[total]);
			break;
			// Отжимаем две кнопки
		case 1: // нулевую и первую
			total += PressKeyToInput(0, false, shift, &all_inputs[total]);
			total += PressKeyToInput(1, false, shift, &all_inputs[total]);
			break;
		case 3: // первую и вторую
			total += PressKeyToInput(1, false, shift, &all_inputs[total]);
			total += PressKeyToInput(2, false, shift, &all_inputs[total]);
			break;
		case 5: // вторую и третью
			total += PressKeyToInput(2, false, shift, &all_inputs[total]);
			total += PressKeyToInput(3, false, shift, &all_inputs[total]);
			break;
		case 7: // нулевую и третью
			total += PressKeyToInput(0, false, shift, &all_inputs[total]);
			total += PressKeyToInput(3, false, shift, &all_inputs[total]);
			break;
		}
		if(total > 0) SendInput(total, all_inputs, sizeof(INPUT));
	}
	keypad_position=-1;
}
//=========================================================================
// Для 8 разных клавиш (8 умений)
//=========================================================================
void MHKeypad::Press8(int position, bool down)
{
	if (position < 0 || position > 7) return;
	if(true==down)
	{
		if(keypad_position!=-1)
		{
			if(keypad_position>3) Press4(keypad_position-4,false,6);
			else Press4(keypad_position,false,0);
		}
		keypad_position=position;
	}
	else
	{
		keypad_position=-1;
	}
	if(position>3) 	Press4(position-4,down,6);
	else Press4(position,down,0);
}
//=========================================================================
// Тап: нажать и сразу отпустить (для hh1a и подобных)
//=========================================================================
void MHKeypad::Tap(int position, int shift)
{
	Press(position, true, shift);
	Press(position, false, shift);
}