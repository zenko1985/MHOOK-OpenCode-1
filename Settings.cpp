#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <strsafe.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#include "Settings.h"
#include "RecentFiles.h"
#include "MHKeypad.h"
#include "CursorDot.h"
#include "MHRepErr.h"
#include "MVector.h"
#include "resource.h"
#include "Scancode.h"
#include "hh1.h"
#include "hh1a.h"
#include "hh2.h"
#include "hh3.h"
#include "hh4.h"
#include "hh5.h"
#include "hh6.h"
#include "hh7.h"
#include "MagicWindow.h"
#include "Localization.h"
#include "EmbeddedExe.h"
static char char_buf[4096];
static TCHAR tchar_buf[4096];
void ResetEytrackerBuffer(); // Определена в OnGazeData
extern HINSTANCE	MHInst;
extern HWND		MHhwnd;
extern HHOOK handle;
extern	bool flag_left_button_key;
extern bool flag_left_button_waits;
extern bool flag_right_button_waits;
extern	int top_position;
static TCHAR *filter_MHOOK=L"файлы MHOOK\0*.MHOOK\0\0";
static TCHAR tfilename[1258];
//static char tfiletitle[1258]={"default.MHOOK"};
static TCHAR tfiletitle[1258];
int MHSettings::num_positions=4;
int MHSettings::mouse_sensitivity=1;
DWORD MHSettings::time_between_pushes=100; // 100 миллисекунд между нажатиями на клавиши
DWORD MHSettings::timeout_after_move=100; // 100 миллисекунд после последнего известия о движении мыши
LONG MHSettings::minimal_mouse_speed=900; //(квадрат числа пикселов за 1/10 секунды)
LONG MHSettings::timeout_mouse_switch=1500; // полторы секунды на переключение
LONG MHSettings::timeout_mouse_click=143; // 1/7 секунды держим клавишу нажатой
LONG MHSettings::deadx=100, MHSettings::deady=100;
bool MHSettings::flag_enable_speed_button=false;
bool MHSettings::flag_2moves=false;
//bool MHSettings::flag_2moves_mode1=false;
bool MHSettings::flag_2moves_mode1=true;
bool MHSettings::flag_change_direction_ontheway=false;
bool MHSettings::flag_right_mb_iskey=false;
bool MHSettings::flag_no_move_right_mb=false; // Флаг запрещает двигать мышь, когда нажата правая кнопка
//bool MHSettings::flag_no_move_right_mb=true; // Флаг запрещает двигать мышь, когда нажата правая кнопка
bool MHSettings::flag_mode5autoclick=false;
bool MHSettings::flag_right_mb_doubleclick=false; // Стоп эмуляции по двойному щелчку
bool MHSettings::flag_left_mb_push_twice=false; // нажимать клавишу также при отпускании ЛК мыши
bool MHSettings::flag_right_mb_push_twice=false; // нажимать клавишу также при отпускании ПК мыши
int MHSettings::circle_scale_factor=0;
bool MHSettings::flag_downall=false; // вниз и вбок = просто вниз (1 режим)
bool MHSettings::flag_skip_fast=false; // Быстрое движение мыши игнорируется
bool MHSettings::flag_up_immediately=false; // Только что нажатая кнопка должна быть отжата (1 режим)
bool MHSettings::flag_autoclick_lmb=false; // Автокликер для левой кнопки мыши
int MHSettings::autoclick_speed_index=1; // Индекс скорости автокликера (по умолчанию "Fast")
bool MHSettings::flag_autoclick_ahk=false; // Запуск AHK скрипта при автоклике
bool MHSettings::flag_wheel_ahk=false; // Запуск AHK скрипта для колесика
bool MHSettings::flag_lmb_win_ahk=false; // Запуск AHK скрипта ЛКМ=Win
bool MHSettings::flag_lmb_esc=false; // ЛКМ 3 сек → Esc
bool MHSettings::flag_autoclick_ahk_loaded=false;
bool MHSettings::flag_wheel_ahk_loaded=false;
bool MHSettings::flag_lmb_win_ahk_loaded=false;
bool MHSettings::flag_cursor_visible=false; // Видимый курсор (красная точка)
int MHSettings::language=0; // 0=RU, 1=EN
bool MHSettings::flag_gamepad_enabled=false;
int MHSettings::gamepad_sensitivity=3;
WORD MHSettings::gamepad_mapping[14]={
	SC_LMOUSE, SC_RMOUSE, SC_MIDDLEMB, SC_WHEEL_UP,
	SC_NONE, SC_NONE, SC_NONE, SC_NONE,
	SC_NONE, SC_NONE,
	SC_WHEEL_UP, SC_LMOUSE, SC_MIDDLEMB, SC_RMOUSE
};
int MHSettings::gamepad_current_mapping[14]={103,104,110,111,0,0,0,0,0,0,111,103,110,104};
int MHSettings::mode=1;
int MHSettings::mode3axe=0;
// Интервалы автокликера в миллисекундах: 0: 20мс, 1: 50мс, 2: 100мс, 3: 500мс
static int autoclick_speeds[4]={20, 50, 100, 500};
MHookHandler *MHSettings::hh=NULL;
MHookHandler1 hh1;
MHookHandler1a hh1a;
MHookHandler2 hh2;
MHookHandler3 hh3;
MHookHandler4 hh4;
MHookHandler5 hh5;
MHookHandler6 hh6;
MHookHandler7 hh7;
//=== Массивы для параметров в диалоге ===/
// Чувствительность мыши
#define MH_NUM_SENSITIVITY 6
static MHIntChar dlg_sensitivity[MH_NUM_SENSITIVITY]={{L"1",1},{L"5",5},{L"10",10},{L"25",25},{L"50",50},{L"100",100}};
static int dlg_current_sensitivity=2;
// Здесь нет PrtScr,Pause
MHWORDChar dlg_scancodes[MH_NUM_SCANCODES_EXTRA]=
{
	{L"<ничего>",SC_NONE}, // 0
	{L"пробел",SC_SPACE},{L"вверх",SC_UP},{L"вправо",SC_RIGHT},{L"вниз",SC_DOWN},{L"влево",SC_LEFT}, // 1-5
	{L"A",SC_A},{L"B",SC_B},{L"C",SC_C},{L"D",SC_D},{L"E",SC_E}, // 6-10
	{L"F",SC_F},{L"G",SC_G},{L"H",SC_H},{L"I",SC_I},{L"J",SC_J}, // 14-18
	{L"K",SC_K},{L"L",SC_L},{L"M",SC_M},{L"N",SC_N},{L"O",SC_O}, // 19-23
	{L"P",SC_P},{L"Q",SC_Q},{L"R",SC_R},{L"S",SC_S},{L"T",SC_T}, // 24-28
	{L"U",SC_U},{L"V",SC_V},{L"W",SC_W},{L"X",SC_X},{L"Y",SC_Y}, // 29-33
	{L"Z",SC_Z},{L"0",SC_0},{L"1",SC_1},{L"2",SC_2},{L"3",SC_3}, // 34-38
	{L"4",SC_4},{L"5",SC_5},{L"6",SC_6},{L"7",SC_7},{L"8",SC_8}, // 39-43
	{L"9",SC_9},{L"~",SC_TILDE},{L"-",SC_MINUS},{L"=",SC_EQUALS},{L"\\",SC_BACKSLASH}, // 44-48
	{L"[",SC_LBRACKET},{L"]",SC_RBRACKET},{L";",SC_SEMICOLON},{L"'",SC_QUOTE},{L",",SC_COMMA}, // 49-53
	{L".",SC_PERIOD},{L"/",SC_SLASH},{L"Backspace",SC_BACKSPACE},{L"TAB",SC_TAB},{L"Caps Lock",SC_CAPSLOCK}, // 54-58
	{L"Левый Shift",SC_LSHIFT},{L"Левый Ctrl",SC_LCTRL},{L"Левый Alt",SC_LALT},{L"Левый Win",SC_LWIN},{L"Правый Shift",SC_RSHIFT}, // 59-63
	{L"Правый Ctrl",SC_RCTRL},{L"Правый Alt",SC_RALT},{L"Правый WIN",SC_RWIN},{L"Menu",SC_MENU},{L"Enter",SC_ENTER}, // 64-68
	{L"Esc",SC_ESC},{L"F1",SC_F1},{L"F2",SC_F2},{L"F3",SC_F3},{L"F4",SC_F4}, // 69-73
	{L"F5",SC_F5},{L"F6",SC_F6},{L"F7",SC_F7},{L"(F8 - запрещена) ",SC_F8},{L"F9",SC_F9}, // 74-78
	{L"F10",SC_F10},{L"F11",SC_F11},{L"F12",SC_F12},{L"Scroll Lock",SC_SCROLLLOCK},{L"Insert",SC_INSERT}, // 79-83
	{L"(Delete - запрещена)",SC_DELETE},{L"Home",SC_HOME},{L"End",SC_END},{L"PgUp",SC_PGUP},{L"PgDn",SC_PGDN}, // 84-88
	{L"Num Lock",SC_NUMLOCK},{L"Num /",SC_NUMSLASH},{L"Num *",SC_NUMASTER},{L"Num -",SC_NUMMINUS},{L"Num +",SC_NUMPLUS}, // 89-93
	{L"Num Enter",SC_NUMENTER},{L"(Num . - запрещена)",SC_NUMDOT},{L"Num 0",SC_NUM0},{L"Num 1",SC_NUM1},{L"Num 2",SC_NUM2}, // 94-98
	{L"Num 3",SC_NUM3},{L"Num 4",SC_NUM4},{L"Num 5",SC_NUM5},{L"Num 6",SC_NUM6},{L"Num 7",SC_NUM7}, // 99-103
	{L"Num 8",SC_NUM8},{L"Num 9",SC_NUM9}, // 104-105
	{L"ЛКМ+F12",SC_LMOUSE_F12},{L"Мышь влево",SC_AUTO_LEFT},{L"Мышь вправо",SC_AUTO_RIGHT},{L"Скролл туда",SC_SCROLL_THERE},{L"Скролл сюда",SC_SCROLL_HERE}, // 106-110
	{L"Колёсико вверх",SC_WHEEL_UP},{L"Колёсико вниз",SC_WHEEL_DOWN},{L"ЛКМ",SC_LMOUSE},{L"ПКМ",SC_RMOUSE},{L"Средняя кнопка",SC_MIDDLEMB} // 111-115
};
//static int dlg_current_scancodes[11]={0,1,2,3,12,11,4,5,6,7,11};
//static int dlg_current_scancodes[15]={1,2,3,4,67,53,27,8,23,5,0,9,10,11,12};
static int dlg_current_scancodes[17]={5,6,7,8,71,54,5,6,7,8,0,4,9,0,63,0,0};
static const int IDC_SCANCODES[17] = {
    IDC_UP, IDC_RIGHT, IDC_DOWN, IDC_LEFT,
    IDC_BUTTON5, IDC_BUTTON6, IDC_UP2, IDC_RIGHT2,
    IDC_DOWN2, IDC_LEFT2, IDC_BUTTON7,
    0, 0, 0, 0,
    IDC_BUTTON6_1, IDC_BUTTON7_1
};
// Таймаут после движения
#define MH_NUM_TIMEOUT 9
static MHIntChar dlg_timeout[MH_NUM_TIMEOUT]={{L"50 мс",50},{L"75 мс",75},{L"100 мс",100},{L"125 мс",125},{L"150 мс",150},
{L"200 мс",200},{L"250 мс",250},{L"0,5 секунды",500},{L"1 секунда",1000},};
static int dlg_current_timeout=0;
// Быстрая скорость движения мыши
#define MH_NUM_SPEED 7
static MHIntChar dlg_speed[MH_NUM_SPEED]={{L"100",100},{L"200",400},{L"300",900},{L"400",1600},{L"500",2500},{L"700",4900},{L"1000",10000}};
static int dlg_current_speed=3;
// Нужно для правильного сохранения и чтения конфигурации
#define MH_NUM_DIRECTIONS 2
static MHIntChar dlg_dirs[MH_NUM_DIRECTIONS]={{L"4",4},{L"8",8}};
static int dlg_current_direction=0;
// Таймаут переключения левой кнопки мыши
#define MH_NUM_SWITCH_TIMEOUT 6
static MHIntChar dlg_switch_timeout[MH_NUM_SWITCH_TIMEOUT]={{L"0,1",100},{L"0,5",500},{L"1",1000},{L"1,5",1000},{L"2",2000},{L"3",3000}};
static int dlg_current_switch_timeout=0;
// Количество пикселов в мертвой зоне
#define MH_DEAD_ZONES 4
static MHIntChar dlg_deadzones[MH_DEAD_ZONES]={{L"50",50},{L"100",100},{L"200",200},{L"25",25}};
static int dlg_current_deadzone_x=1,dlg_current_deadzone_y=1;
// Одна из осей в режиме 4 может работать в режиме 3
#define MH_NUM_MODE3AXE 3
static MHIntChar dlg_mode3axe[MH_NUM_MODE3AXE]={{L"не надо",-1},{L"на оси вправо-влево",0},{L"на оси вверх-вниз",1}};
static int dlg_current_mode3axe=0;
// В режиме 5 выбор колесом
#define MH_NUM_CIRCLE_SCALES 3
static MHIntChar dlg_circlescales[MH_NUM_CIRCLE_SCALES]={{L"не использовать",0},{L"50 пикселов",50},{L"100 пикселов",100}};
static int dlg_current_circlescale=0;
// static int res; // Selection result
// Флаг для предотвращения рекурсии при автозагрузке
static bool g_recentFileLoading = false;
static bool g_recentTyping = false;
// Прототип диалога номер два
BOOL CALLBACK DlgSettings2WndProc(HWND hdwnd,
						   UINT uMsg,
						   WPARAM wparam,
						   LPARAM lparam );
//===================================================================
// Диалог настроек
//===================================================================
// Ищет индекс клавиши в списке выбора скан-кодов. Номера в комментариях над
// таблицей dlg_scancodes не совпадают с реальными индексами, поэтому ищем
// по самому значению скан-кода
static int FindScancodeIndex(WORD scancode)
{
	for (int i=0;i<MH_NUM_SCANCODES_EXTRA;i++)
		if (dlg_scancodes[i].value==scancode) return i;
	return 0;
}
static bool wasd_shown=false; // Показаны ли кнопки WASD?
static BOOL CALLBACK DlgSettingsWndProc(HWND hdwnd,
						   UINT uMsg,
						   WPARAM wparam,
						   LPARAM lparam )
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wparam))
			{
			case IDC_BUTTON_DOPLNITELNO:
				MagicWindow::ShowEditable();
				DialogBox(MHInst,MAKEINTRESOURCE(IDD_DIALOG_SETTINGS2),hdwnd,(DLGPROC)DlgSettings2WndProc);
				MagicWindow::Hide();
				return 1;
			case IDC_BUTTON_WASD:
				// Кнопка переключает две раскладки: WASD и стрелки
				if(wasd_shown)
				{
					wasd_shown=false;
					SendDlgItemMessage(hdwnd,IDC_UP,   CB_SETCURSEL,FindScancodeIndex(SC_UP),   0L);
					SendDlgItemMessage(hdwnd,IDC_RIGHT,CB_SETCURSEL,FindScancodeIndex(SC_RIGHT),0L);
					SendDlgItemMessage(hdwnd,IDC_DOWN, CB_SETCURSEL,FindScancodeIndex(SC_DOWN), 0L);
					SendDlgItemMessage(hdwnd,IDC_LEFT, CB_SETCURSEL,FindScancodeIndex(SC_LEFT), 0L);
				}
				else
				{
					wasd_shown=true;
					SendDlgItemMessage(hdwnd,IDC_UP,   CB_SETCURSEL,FindScancodeIndex(SC_W),0L);
					SendDlgItemMessage(hdwnd,IDC_RIGHT,CB_SETCURSEL,FindScancodeIndex(SC_D),0L);
					SendDlgItemMessage(hdwnd,IDC_DOWN, CB_SETCURSEL,FindScancodeIndex(SC_S),0L);
					SendDlgItemMessage(hdwnd,IDC_LEFT, CB_SETCURSEL,FindScancodeIndex(SC_A),0L);
				}
				return 1;
			case IDC_BUTTON_LOAD: // Грузим файл
				MHSettings::OpenMHookConfig(hdwnd);
				MHSettings::AfterLoad(hdwnd);
				return 1;
			case IDC_BUTTON_SAVE: // Сохраняем файл
				MHSettings::BeforeSaveOrStart(hdwnd); // Текущие поля диалога копирует в переменные
				MHSettings::SaveMHookConfig(hdwnd);
				return 1;
			case IDC_CHECK_CURSOR_VISIBLE: // Видимый курсор - показываем/скрываем сразу
				if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_CURSOR_VISIBLE,BM_GETCHECK, 0, 0))
				{
					MHSettings::flag_cursor_visible=true;
					CursorDot::Show();
				}
				else
				{
					MHSettings::flag_cursor_visible=false;
					CursorDot::Hide();
				}
				return 1;
			case IDC_CHECK_WHEEL_AHK: // AHK Колесико - кнопки колесика сбрасываем на "ничего"
				if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_WHEEL_AHK,BM_GETCHECK, 0, 0))
				{
					MHSettings::flag_wheel_ahk=true;
					dlg_current_scancodes[10]=0;
					dlg_current_scancodes[16]=0;
					SendDlgItemMessage(hdwnd,IDC_BUTTON7, CB_SETCURSEL, 0, 0L);
					SendDlgItemMessage(hdwnd,IDC_BUTTON7_1, CB_SETCURSEL, 0, 0L);
				}
				else
				{
					MHSettings::flag_wheel_ahk=false;
				}
				return 1;
		case IDC_LIST_RECENT_FILES:
			if (HIWORD(wparam) == CBN_SELCHANGE) {
				if (!g_recentFileLoading && !g_recentTyping) {
					int sel = static_cast<int>(SendDlgItemMessage(hdwnd, IDC_LIST_RECENT_FILES, CB_GETCURSEL, 0, 0));
					if (sel != CB_ERR) {
						RecentFiles::OnDialogFileSelected(hdwnd, IDC_LIST_RECENT_FILES, sel);
					}
				}
			}
			else if (HIWORD(wparam) == CBN_EDITCHANGE) {
				if (!g_recentFileLoading) {
					g_recentTyping = true;
					KillTimer(hdwnd, 102);
					SetTimer(hdwnd, 102, 300, NULL);
				}
			}
			return 1;
			case IDC_BUTTON_LOAD_BY_WINDOW: {
				SetTimer(hdwnd, 100, 2000, NULL);
				SetWindowText(hdwnd, L(LOC_WND_CLICK_GAME));
				return 1;
			}
			case IDCANCEL: // Не случилось
				EndDialog(hdwnd,2);
				return 1;
			case IDC_BTN_LANG: {
				MHSettings::language = (MHSettings::language == MH_LANG_RU) ? MH_LANG_EN : MH_LANG_RU;
				MHLanguage = MHSettings::language;
				// Update button text
				SetWindowText(GetDlgItem(hdwnd, IDC_BTN_LANG), L(MHSettings::language == MH_LANG_RU ? LOC_BTN_LANG_RUS : LOC_BTN_LANG_ENG));
				// Update all dialog texts
				MHSettings::UpdateDialog1Texts(hdwnd);
				return 1;
			}
			case IDOK: 	//Хорошо!
				// 1. Чувствительность
				MHSettings::BeforeSaveOrStart(hdwnd);
				ResetEytrackerBuffer();
				MagicWindow::ForceTopMost();
				EndDialog(hdwnd,0);
				return 1;
			} // switch WM_COMMAND
		break; // if WM_COMMAND
	case WM_DROPFILES: {
		HDROP hDrop = (HDROP)wparam;
		TCHAR filename[MAX_PATH];
		if (DragQueryFile(hDrop, 0, filename, MAX_PATH)) {
			TCHAR* ext = PathFindExtension(filename);
			if (ext && _tcsicmp(ext, _T(".MHOOK")) == 0) {
				MHSettings::OpenMHookConfig(hdwnd, filename);
				MHSettings::AfterLoad(hdwnd);
			} else if (ext && _tcsicmp(ext, _T(".MHOO")) == 0) {
				size_t remaining = MAX_PATH - (ext - filename);
				if (remaining >= 7) {
					_tcscpy_s(ext, remaining, _T(".MHOOK"));
					MHSettings::OpenMHookConfig(hdwnd, filename);
					MHSettings::AfterLoad(hdwnd);
				}
			}
		}
		DragFinish(hDrop);
		return 1;
	}
	case WM_TIMER: {
		if (wparam == 100) {
			KillTimer(hdwnd, 100);
			SetWindowText(hdwnd, L(LOC_WND_CLICK_GAME));
			SetTimer(hdwnd, 101, 1500, NULL);
		}
		if (wparam == 101) {
			KillTimer(hdwnd, 101);
			HWND fgWnd = GetForegroundWindow();
			if (fgWnd && fgWnd != hdwnd) {
				DWORD pid;
				GetWindowThreadProcessId(fgWnd, &pid);
				if (pid != GetCurrentProcessId()) {
					TCHAR windowTitle[256];
					GetWindowText(fgWnd, windowTitle, 256);
					if (windowTitle[0]) {
			TCHAR exePath[MAX_PATH];
					TCHAR searchPattern[MAX_PATH];
					GetModuleFileName(NULL, exePath, MAX_PATH);
					PathRemoveFileSpec(exePath);
					PathAddBackslash(exePath);
					StringCchCopy(searchPattern, MAX_PATH, exePath);
					StringCchCat(searchPattern, MAX_PATH, _T("*.MHOOK"));
						TCHAR titleUpper[256];
						StringCchCopy(titleUpper, 256, windowTitle);
						TCHAR titleClean[256];
						int j = 0;
						for (int i = 0; titleUpper[i] && j < 255; i++) {
							if (titleUpper[i] != _T(' ') && titleUpper[i] != _T('-') && titleUpper[i] != _T('_') && titleUpper[i] != _T('(') && titleUpper[i] != _T(')') && titleUpper[i] != _T('[') && titleUpper[i] != _T(']')) {
								titleClean[j++] = titleUpper[i];
							}
						}
						titleClean[j] = _T('\0');
						CharUpperBuff(titleClean, static_cast<DWORD>(_tcslen(titleClean)));
						TCHAR msg[512];
							int bestMatchScore = 0;
						TCHAR bestMatchPath[MAX_PATH] = {0};
						bool bestMatchIsMHOOK = false;
						WIN32_FIND_DATA fd;
						HANDLE hFind = FindFirstFile(searchPattern, &fd);
						if (hFind != INVALID_HANDLE_VALUE) {
							do {
								TCHAR fileNameOrig[256];
								StringCchCopy(fileNameOrig, 256, fd.cFileName);
								TCHAR* dotPos = _tcsrchr(fd.cFileName, _T('.'));
								if (dotPos) *dotPos = _T('\0');
								TCHAR fileClean[256];
								j = 0;
								for (int i = 0; fd.cFileName[i] && j < 255; i++) {
									if (fd.cFileName[i] < 256 && _istalnum(fd.cFileName[i])) {
										fileClean[j++] = fd.cFileName[i];
									}
								}
								fileClean[j] = _T('\0');
								CharUpperBuff(fileClean, static_cast<DWORD>(_tcslen(fileClean)));
								int matchScore = 0;
								if (_tcslen(fileClean) >= 2 && _tcslen(titleClean) >= 2) {
									if (_tcsstr(fileClean, titleClean) != NULL) {
										matchScore = static_cast<int>(_tcslen(titleClean)) * 10;
									} else if (_tcsstr(titleClean, fileClean) != NULL) {
										matchScore = static_cast<int>(_tcslen(fileClean)) * 10;
									} else {
										int minLen = static_cast<int>(_tcslen(titleClean));
										if (_tcslen(fileClean) >= static_cast<size_t>(minLen) && minLen >= 3) {
											TCHAR filePrefix[256];
											_tcsncpy(filePrefix, fileClean, minLen);
											filePrefix[minLen] = _T('\0');
											if (_tcscmp(filePrefix, titleClean) == 0) {
												matchScore = static_cast<int>(_tcslen(titleClean)) * 8;
											}
										}
									}
								}
								TCHAR extCheck[256];
								StringCchCopy(extCheck, 256, fileNameOrig);
								bool endsWithMHOOK = false;
								TCHAR* dotInCheck = _tcsrchr(extCheck, _T('.'));
								if (dotInCheck && _tcsicmp(dotInCheck, _T(".MHOOK")) == 0) {
									endsWithMHOOK = true;
									dotInCheck[0] = _T('\0');
									TCHAR* prevDot = _tcsrchr(extCheck, _T('.'));
									if (prevDot && _tcsicmp(prevDot, _T(".MHOO")) == 0) {
										endsWithMHOOK = false;
									}
								}
								TCHAR fullPath[MAX_PATH];
								StringCchCopy(fullPath, MAX_PATH, exePath);
								StringCchCat(fullPath, MAX_PATH, fileNameOrig);
								if (matchScore > bestMatchScore || (matchScore == bestMatchScore && endsWithMHOOK && !bestMatchIsMHOOK)) {
									bestMatchScore = matchScore;
									StringCchCopy(bestMatchPath, MAX_PATH, fullPath);
									bestMatchIsMHOOK = endsWithMHOOK;
								}
							} while (FindNextFile(hFind, &fd));
							FindClose(hFind);
						}
						bool loaded = false;
						if (bestMatchScore > 0) {
							MHSettings::OpenMHookConfig(hdwnd, bestMatchPath);
							loaded = true;
						}
							if (!loaded) {
							StringCchPrintf(msg, 512, L(LOC_ERR_NOT_FOUND_FMT), windowTitle);
						MessageBox(hdwnd, msg, L(LOC_ERR_NOT_FOUND), MB_OK);
						}
						if (loaded) {
							RecentFiles::PopulateDialogList(hdwnd, IDC_LIST_RECENT_FILES);
							MHSettings::AfterLoad(hdwnd);
							MHSettings::BeforeSaveOrStart(hdwnd);
							ResetEytrackerBuffer();
							MagicWindow::ForceTopMost();
							EndDialog(hdwnd, 0);
						}
					}
				}
			}
			SetWindowText(hdwnd, L(LOC_WND_TITLE_SETTINGS));
		}
		if (wparam == 102) {
			KillTimer(hdwnd, 102);
			if (g_recentFileLoading) return 1;
			TCHAR typed[256] = {0};
			HWND hCombo = GetDlgItem(hdwnd, IDC_LIST_RECENT_FILES);
			if (hCombo) {
				GetWindowText(hCombo, typed, 256);
				if (typed[0]) {
					int idx = RecentFiles::FindByPrefix(typed);
					if (idx >= 0) {
						g_recentFileLoading = true;
						g_recentTyping = false;
						RecentFiles::OnDialogFileSelected(hdwnd, IDC_LIST_RECENT_FILES, idx);
						g_recentFileLoading = false;
					} else {
						g_recentTyping = false;
					}
				} else {
					g_recentTyping = false;
				}
			} else {
				g_recentTyping = false;
			}
		}
		return 1;
	}
	case WM_INITDIALOG:
		//SetWindowPos(hdwnd,NULL,50,50,0,0,SWP_NOSIZE);
		//SetWindowPos(hdwnd,HWND_TOPMOST,50,50,0,0,SWP_NOSIZE | SWP_NOREDRAW);
		SetWindowPos(hdwnd,HWND_TOP,50,50,0,0,SWP_NOSIZE | SWP_NOREDRAW);
		// Разрешаем drag and drop
		DragAcceptFiles(hdwnd, TRUE);
		// Здесь не работает.
		//SetWindowLong(hdwnd,GWL_STYLE,GetWindowLong(hdwnd,GWL_STYLE) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		MHSettings::FillDialogue(hdwnd); // Заполняет списки
		MHSettings::AfterLoad(hdwnd); // Показываем текущие значения
		// Заполняем список недавних файлов
		RecentFiles::PopulateDialogList(hdwnd, IDC_LIST_RECENT_FILES);
		return 1; // Да, ставь фокус куда надо
		break;
	} // switch uMsg
return 0;
}
// Эта функция определена в HookProc
LRESULT  CALLBACK HookProc(int disabled,WPARAM wParam,LPARAM lParam);
//====================================================================================
// Заполнить выпадающие списки диалога возможными значениями
//====================================================================================
void MHSettings::FillDialogue(HWND hdwnd)
{
	int i;
	// Имя файла показать в диалоге
	SendDlgItemMessage(hdwnd,IDC_EDIT1, WM_SETTEXT, 0L, (LPARAM)tfiletitle);
	// Заполнить выпадающие списки с текущими значениями!
		// 1. Чувствительность
		for(i=0;i<MH_NUM_SENSITIVITY;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_SENSITIVITY, CB_ADDSTRING, 0, (LPARAM)(dlg_sensitivity[i].stroka));
		}
		SendDlgItemMessage(hdwnd,IDC_SENSITIVITY, CB_SETCURSEL, dlg_current_sensitivity, 0L);
		for(i=0;i<MH_NUM_SCANCODES;i++)
		{
			for (int ci = 0; ci < 17; ci++)
				SendDlgItemMessage(hdwnd, IDC_SCANCODES[ci], CB_ADDSTRING, 0, (LPARAM)(dlg_scancodes[i].stroka));
		}
		// 2.1. Мёртвые зоны
		for(i=0;i<MH_DEAD_ZONES;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_DEADX, CB_ADDSTRING, 0, (LPARAM)(dlg_deadzones[i].stroka));
			SendDlgItemMessage(hdwnd,IDC_DEADY, CB_ADDSTRING, 0, (LPARAM)(dlg_deadzones[i].stroka));
		}
		// 2.2 Режим 3 для одной из осей в режиме 4
		for(i=0;i<MH_NUM_MODE3AXE;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_MODE3AXE, CB_ADDSTRING, 0, (LPARAM)(dlg_mode3axe[i].stroka));
		}
		// 2.5. Минимальная скорость мыши
		for(i=0;i<MH_NUM_SPEED;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_SPEED, CB_ADDSTRING, 0, (LPARAM)(dlg_speed[i].stroka));
		}
		// 2.6. Число направлений
		for(i=0;i<MH_NUM_DIRECTIONS;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_DIRECTIONS, CB_ADDSTRING, 0, (LPARAM)(dlg_dirs[i].stroka));
		}
		// 3. Радио-кнопка
		// 4. Таймаут
		for(i=0;i<MH_NUM_TIMEOUT;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_TIMEOUT, CB_ADDSTRING, 0, (LPARAM)(dlg_timeout[i].stroka));
		}
		// 4.5. Таймаут переключения левой кнопки мыши
		for(i=0;i<MH_NUM_SWITCH_TIMEOUT;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_SWITCH_TIMEOUT, CB_ADDSTRING, 0, (LPARAM)(dlg_switch_timeout[i].stroka));
		}
		// 4.6. Чувствительность мыши при прокрутке колеса
		for(i=0;i<MH_NUM_CIRCLE_SCALES;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_CIRCLE_SCALES, CB_ADDSTRING, 0, (LPARAM)(dlg_circlescales[i].stroka));
		}
		// 4.7. Скорость автокликера
		SendDlgItemMessage(hdwnd,IDC_AUTOCLICK_SPEED, CB_ADDSTRING, 0, (LPARAM)L(LOC_SPEED_20MS));
		SendDlgItemMessage(hdwnd,IDC_AUTOCLICK_SPEED, CB_ADDSTRING, 0, (LPARAM)L(LOC_SPEED_50MS));
		SendDlgItemMessage(hdwnd,IDC_AUTOCLICK_SPEED, CB_ADDSTRING, 0, (LPARAM)L(LOC_SPEED_100MS));
		SendDlgItemMessage(hdwnd,IDC_AUTOCLICK_SPEED, CB_ADDSTRING, 0, (LPARAM)L(LOC_SPEED_500MS));
		// Language button
		SetWindowText(GetDlgItem(hdwnd, IDC_BTN_LANG), L(MHSettings::language == MH_LANG_RU ? LOC_BTN_LANG_RUS : LOC_BTN_LANG_ENG));
}
//=======================================================================================
// Обновить тексты диалога 1 при смене языка
//=======================================================================================
void MHSettings::UpdateDialog1Texts(HWND hdwnd)
{
	// Caption
	SetWindowText(hdwnd, L(LOC_DIALOG1_TITLE));
	// Buttons
	SetWindowText(GetDlgItem(hdwnd, IDOK), L(LOC_BTN_START));
	SetWindowText(GetDlgItem(hdwnd, IDCANCEL), L(LOC_BTN_EXIT));
	SetWindowText(GetDlgItem(hdwnd, IDC_BUTTON_SAVE), L(LOC_BTN_SAVE));
	SetWindowText(GetDlgItem(hdwnd, IDC_BUTTON_LOAD), L(LOC_BTN_LOAD));
	SetWindowText(GetDlgItem(hdwnd, IDC_BUTTON_DOPLNITELNO), L(LOC_BTN_MAGIC_WINDOWS));
	SetWindowText(GetDlgItem(hdwnd, IDC_BUTTON_LOAD_BY_WINDOW), L(LOC_BTN_LOAD_BY_WINDOW));
	// Labels
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_SENSITIVITY), L(LOC_LBL_SENSITIVITY));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_UP), L(LOC_LBL_UP));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_LEFT), L(LOC_LBL_LEFT));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_RIGHT), L(LOC_LBL_RIGHT));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_DOWN), L(LOC_LBL_DOWN));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_DIRS), L(LOC_LBL_DIRS_COUNT));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_TIMEOUT_LABEL), L(LOC_LBL_KEY_TIMEOUT));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_FAST_SPEED), L(LOC_LBL_FAST_SPEED));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_LEFT_CORNER), L(LOC_LBL_LEFT_CORNER));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_SECONDS), L(LOC_LBL_SECONDS));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_LEFT_WORKS), L(LOC_LBL_LEFT_WORKS_AS));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_DEAD_LR), L(LOC_LBL_DEADZONE_LR));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_DEAD_UD), L(LOC_LBL_DEADZONE_UD));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_MODE3), L(LOC_LBL_KEEP_MODE3));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_WHEEL_SENS), L(LOC_LBL_WHEEL_SENSITIVITY));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_2), L(LOC_LBL_2));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_2B), L(LOC_LBL_2));
	// Radio buttons
	SetWindowText(GetDlgItem(hdwnd, IDC_RADIO1), L(LOC_RADIO_MODE1));
	SetWindowText(GetDlgItem(hdwnd, IDC_RADIO2), L(LOC_RADIO_MODE2));
	SetWindowText(GetDlgItem(hdwnd, IDC_RADIO3), L(LOC_RADIO_MODE3));
	SetWindowText(GetDlgItem(hdwnd, IDC_RADIO4), L(LOC_RADIO_MODE4));
	SetWindowText(GetDlgItem(hdwnd, IDC_RADIO5), L(LOC_RADIO_MODE5));
	SetWindowText(GetDlgItem(hdwnd, IDC_RADIO6), L(LOC_RADIO_MODE6));
	SetWindowText(GetDlgItem(hdwnd, IDC_RADIO7), L(LOC_RADIO_MODE7));
	// Checkboxes
	SetWindowText(GetDlgItem(hdwnd, IDC_FAST_PUSH), L(LOC_CHK_FAST_PUSH));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_2MOVES), L(LOC_CHK_2MOVES));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_2MOVES_MODE1), L(LOC_CHK_2MOVES_MODE1));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_CHANGE_DIRECTION_ONTHEWAY), L(LOC_CHK_CHANGE_DIR));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_RIGHT_MB_ISKEY), L(LOC_CHK_RIGHT_IS_KEY));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_AUTOCLICK), L(LOC_CHK_AUTOCLICK));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_LMB_AUTOCLICK), L(LOC_CHK_LMB_AUTOCLICK));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_AHK_AUTOCLICK), L(LOC_CHK_AHK_AUTOCLICK));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_LMB_WIN_AHK), L(LOC_CHK_LMB_WIN));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_LMB_ESC), L(LOC_CHK_LMB_ESC));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_WHEEL_AHK), L(LOC_CHK_AHK_WHEEL));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_CURSOR_VISIBLE), L(LOC_CHK_CURSOR_VISIBLE));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_RIGHT_DBLCLK), L(LOC_CHK_DBLCLK_PAUSE));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_LEFT_PUSH_TWICE), L(LOC_CHK_LEFT_PUSH_TWICE));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_RIGHT_PUSH_TWICE), L(LOC_CHK_RIGHT_PUSH_TWICE));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_DOWNALL), L(LOC_CHK_DOWNALL));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_SKIP_FAST), L(LOC_CHK_SKIP_FAST));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_UP_IMMEDIATELY), L(LOC_CHK_UP_IMMEDIATELY));
	// Groupboxes
	SetWindowText(GetDlgItem(hdwnd, IDC_GRP_MOUSE), L(LOC_GRP_MOUSE_BUTTONS));
	SetWindowText(GetDlgItem(hdwnd, IDC_GRP_RIGHT), L(LOC_GRP_RIGHT_MB_HOLD));
	// Mode5 description
	SetWindowText(GetDlgItem(hdwnd, IDC_MODE5_DESC), L(LOC_CHK_MODE5_DESC));
	// Update main window title
	SetWindowText(MHhwnd, L(LOC_APP_TITLE));
}
//=======================================================================================
// Актуализировать в полях диалога загруженнные значения переменных
//=======================================================================================
void MHSettings::AfterLoad(HWND hdwnd)
{
//	int i;
	// Заполнить выпадающие списки с текущими значениями!
		// 1. Чувствительность
		SendDlgItemMessage(hdwnd,IDC_SENSITIVITY, CB_SETCURSEL, dlg_current_sensitivity, 0L);
		for (int ci = 0; ci < 17; ci++)
			SendDlgItemMessage(hdwnd, IDC_SCANCODES[ci], CB_SETCURSEL, dlg_current_scancodes[ci], 0L);
		// 2.1. Мёртвые зоны
		SendDlgItemMessage(hdwnd,IDC_DEADX, CB_SETCURSEL, dlg_current_deadzone_x, 0L);
		SendDlgItemMessage(hdwnd,IDC_DEADY, CB_SETCURSEL, dlg_current_deadzone_y, 0L);
		// 2.2 Режим 3 для одной из осей в режиме 4
		SendDlgItemMessage(hdwnd,IDC_MODE3AXE, CB_SETCURSEL, dlg_current_mode3axe, 0L);
		// 2.5. Минимальная скорость мыши
		SendDlgItemMessage(hdwnd,IDC_SPEED, CB_SETCURSEL, dlg_current_speed, 0L);
		// 2.6. Число направлений
		// SendDlgItemMessage(hdwnd,IDC_DIRECTIONS, CB_SETCURSEL, MHSettings::GetNumPositions()/5, 0L);
		SendDlgItemMessage(hdwnd,IDC_DIRECTIONS, CB_SETCURSEL, dlg_current_direction, 0L);
		// 3. Радио-кнопка. Сначала все отпускаем (вот фигня-то!)
		SendDlgItemMessage(hdwnd, IDC_RADIO1, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, IDC_RADIO2, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, IDC_RADIO3, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, IDC_RADIO4, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, IDC_RADIO5, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, IDC_RADIO6, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, IDC_RADIO7, BM_SETCHECK, BST_UNCHECKED, 0);
		switch(MHSettings::mode)
		{
		case 1:
			SendDlgItemMessage(hdwnd, IDC_RADIO1, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 2:
			SendDlgItemMessage(hdwnd, IDC_RADIO2, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 3:
			SendDlgItemMessage(hdwnd, IDC_RADIO3, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 4:
			SendDlgItemMessage(hdwnd, IDC_RADIO4, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 5:
			SendDlgItemMessage(hdwnd, IDC_RADIO5, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 6:
			SendDlgItemMessage(hdwnd, IDC_RADIO6, BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 7:
			SendDlgItemMessage(hdwnd, IDC_RADIO7, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		// 4. Таймаут
		SendDlgItemMessage(hdwnd,IDC_TIMEOUT, CB_SETCURSEL, dlg_current_timeout, 0L);
		// 4.5. Таймаут переключения левой кнопки мыши
		SendDlgItemMessage(hdwnd,IDC_SWITCH_TIMEOUT, CB_SETCURSEL, dlg_current_switch_timeout, 0L);
		// 4.6. Чувствительность мыши при прокрутке колеса
		SendDlgItemMessage(hdwnd,IDC_CIRCLE_SCALES, CB_SETCURSEL, dlg_current_circlescale, 0L);
		// 6. Разрешить ли нажатие пятой кнопки при быстром движении мышью
		if(MHSettings::flag_enable_speed_button) SendDlgItemMessage(hdwnd, IDC_FAST_PUSH, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_FAST_PUSH, BM_SETCHECK, BST_UNCHECKED, 0);
		// 7. Использовать ли в 4 режиме движение в 2 шага
		if(MHSettings::flag_2moves) SendDlgItemMessage(hdwnd, IDC_CHECK_2MOVES, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_2MOVES, BM_SETCHECK, BST_UNCHECKED, 0);
		// 8. Использовать ли в 1 режиме движение в 2 шага
		if(MHSettings::flag_2moves_mode1) SendDlgItemMessage(hdwnd, IDC_CHECK_2MOVES_MODE1, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_2MOVES_MODE1, BM_SETCHECK, BST_UNCHECKED, 0);
		// 9. можно ли менять направление движения на ходу
		if(MHSettings::flag_change_direction_ontheway) SendDlgItemMessage(hdwnd, IDC_CHECK_CHANGE_DIRECTION_ONTHEWAY, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_CHANGE_DIRECTION_ONTHEWAY, BM_SETCHECK, BST_UNCHECKED, 0);
		// 10. правая кнопка мыши вместо обычного поведения ведёт себя, как клавиша
		if(MHSettings::flag_right_mb_iskey) SendDlgItemMessage(hdwnd, IDC_CHECK_RIGHT_MB_ISKEY, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_RIGHT_MB_ISKEY, BM_SETCHECK, BST_UNCHECKED, 0);
		// 12. автоклик в режиме 5
		if(MHSettings::flag_mode5autoclick) SendDlgItemMessage(hdwnd, IDC_CHECK_AUTOCLICK, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_AUTOCLICK, BM_SETCHECK, BST_UNCHECKED, 0);
		// 13. пауза по двойному щелчку
		if(MHSettings::flag_right_mb_doubleclick) SendDlgItemMessage(hdwnd, IDC_CHECK_RIGHT_DBLCLK, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_RIGHT_DBLCLK, BM_SETCHECK, BST_UNCHECKED, 0);
		// 14. // нажимать клавишу также при отпускании ЛК мыши
		if(MHSettings::flag_left_mb_push_twice) SendDlgItemMessage(hdwnd, IDC_CHECK_LEFT_PUSH_TWICE, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_LEFT_PUSH_TWICE, BM_SETCHECK, BST_UNCHECKED, 0);
		// 15. // нажимать клавишу также при отпускании ЛК мыши
		if(MHSettings::flag_right_mb_push_twice) SendDlgItemMessage(hdwnd, IDC_CHECK_RIGHT_PUSH_TWICE, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_RIGHT_PUSH_TWICE, BM_SETCHECK, BST_UNCHECKED, 0);
		// 16. вниз+вбок = простол вниз (режим 1)
		if(MHSettings::flag_downall) SendDlgItemMessage(hdwnd, IDC_CHECK_DOWNALL, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_DOWNALL, BM_SETCHECK, BST_UNCHECKED, 0);
		// 17. вниз+вбок сразу отпускать (режим 1)
		if(MHSettings::flag_up_immediately) SendDlgItemMessage(hdwnd, IDC_CHECK_UP_IMMEDIATELY, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_UP_IMMEDIATELY, BM_SETCHECK, BST_UNCHECKED, 0);
		// 18. игнорировать быстрое движение (режим 3)
		if(MHSettings::flag_skip_fast) SendDlgItemMessage(hdwnd, IDC_CHECK_SKIP_FAST, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_SKIP_FAST, BM_SETCHECK, BST_UNCHECKED, 0);
		// 19. автокликер для левой кнопки мыши (отдельный чекбокс)
		if(MHSettings::flag_autoclick_lmb) SendDlgItemMessage(hdwnd, IDC_CHECK_LMB_AUTOCLICK, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_LMB_AUTOCLICK, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd,IDC_AUTOCLICK_SPEED, CB_SETCURSEL, MHSettings::autoclick_speed_index, 0L);
		if(MHSettings::flag_autoclick_ahk) SendDlgItemMessage(hdwnd, IDC_CHECK_AHK_AUTOCLICK, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_AHK_AUTOCLICK, BM_SETCHECK, BST_UNCHECKED, 0);
		if(MHSettings::flag_wheel_ahk) SendDlgItemMessage(hdwnd, IDC_CHECK_WHEEL_AHK, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_WHEEL_AHK, BM_SETCHECK, BST_UNCHECKED, 0);
		if(MHSettings::flag_lmb_win_ahk) SendDlgItemMessage(hdwnd, IDC_CHECK_LMB_WIN_AHK, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_LMB_WIN_AHK, BM_SETCHECK, BST_UNCHECKED, 0);
		if(MHSettings::flag_lmb_esc) SendDlgItemMessage(hdwnd, IDC_CHECK_LMB_ESC, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_LMB_ESC, BM_SETCHECK, BST_UNCHECKED, 0);
		// 20. видимый курсор (красная точка)
		if(MHSettings::flag_cursor_visible) SendDlgItemMessage(hdwnd, IDC_CHECK_CURSOR_VISIBLE, BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_CURSOR_VISIBLE, BM_SETCHECK, BST_UNCHECKED, 0);
}
extern bool flag_stop_emulation;
extern bool flag_lmb_win_active;
extern bool flag_lmb_esc_active;
BOOL MHSettings::SettingsDialogue(HWND hwnd)
{
	bool restart_hook=false;
	BOOL return_code;
	if(MHSettings::hh)
	{
		// 0. Убрать окна
		MagicWindow::Hide();
		// 1. сначала остановить работающий хук
		restart_hook=true;
		UnhookWindowsHookEx(handle);
		// 2. Убить все таймеры
		KillTimer(hwnd,1);
		KillTimer(hwnd,2);
		KillTimer(hwnd,3);
		KillTimer(hwnd,4);
		KillTimer(hwnd,5);
		KillTimer(hwnd,7);
		KillTimer(hwnd,8);
		KillTimer(hwnd,9); // Watchdog таймер
		// Отпустить Win если был зажат галочкой ЛКМ=Win
		if(flag_lmb_win_active) {
			INPUT input = {0};
			input.type = INPUT_KEYBOARD;
			input.ki.wScan = 0xE05B; // SC_LWIN
			input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
			SendInput(1, &input, sizeof(INPUT));
			flag_lmb_win_active = false;
		}
		// Отпустить Esc если был зажат галочкой ЛКМ=Esc
		if(flag_lmb_esc_active) {
			INPUT input = {0};
			input.type = INPUT_KEYBOARD;
			input.ki.wScan = 0x01; // SC_ESC
			input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
			SendInput(1, &input, sizeof(INPUT));
			flag_lmb_esc_active = false;
		}
		// 3. Сбрасываем MVector и MHKeypad и чё там ещё
		MHSettings::hh->Halt();
		MHSettings::hh->HaltGeneral();
		MHVector::Reset();
		MHKeypad::Reset(); // !!! А вдруг надо было с шифтом??? Тогда он сбросился в хук-хендлере!!!
		// Почему-то Reset не включает перерисовку
		InvalidateRect(MHhwnd,NULL,TRUE);
		// 4. Текущий HookHandler обнуляем
		MHSettings::hh=NULL;
		flag_stop_emulation=false;
		flag_left_button_waits=false;
		flag_right_button_waits=false;
	}
	return_code = static_cast<BOOL>(DialogBox(MHInst, MAKEINTRESOURCE(IDD_DIALOG_SETTINGS), hwnd, (DLGPROC)DlgSettingsWndProc));
	if((!return_code)&&(restart_hook))
	{
		// Проинициализировать хук хендлер (пока не знаю, что его нужно проинициализировать)
		// Продолжаем работать, восстанавливаем хук с начальными параметрами
		flag_left_button_key=false;
		top_position=-1;
		handle = SetWindowsHookExW(WH_MOUSE_LL,
									HookProc,
                                 GetModuleHandle(NULL),
                                 NULL);
	}
	if(!return_code)
	{
		MagicWindow::ShowRuntime(); // Здесь же взводится пятый таймер
	}
	return return_code;
}
//=======================================================================================================
//  Считывать конфигурацию
//=======================================================================================================
typedef enum {save_empty,save_int,save_bool,save_WORD, save_MagicWindows} T_save_type;
typedef struct
{
	char *name; // Название в файле конфигурации
	T_save_type save_type;
	void *pointer;
	T_save_type check_type;
	void *check_pointer;
	int max_index;
} T_save_struct;
#define NUM_SAVE_LINES 64
static T_save_struct save_struct[NUM_SAVE_LINES]=
{
	{"Sensitivity",save_int,&dlg_current_sensitivity,save_int,&dlg_sensitivity, MH_NUM_SENSITIVITY},
	{"Button0",save_int,&(dlg_current_scancodes[0]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button1",save_int,&(dlg_current_scancodes[1]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button2",save_int,&(dlg_current_scancodes[2]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button3",save_int,&(dlg_current_scancodes[3]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button4",save_int,&(dlg_current_scancodes[4]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button5",save_int,&(dlg_current_scancodes[5]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button6",save_int,&(dlg_current_scancodes[6]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button7",save_int,&(dlg_current_scancodes[7]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button8",save_int,&(dlg_current_scancodes[8]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button9",save_int,&(dlg_current_scancodes[9]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button10",save_int,&(dlg_current_scancodes[10]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button11",save_int,&(dlg_current_scancodes[11]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button12",save_int,&(dlg_current_scancodes[12]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button13",save_int,&(dlg_current_scancodes[13]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button14",save_int,&(dlg_current_scancodes[14]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES}, //16
	{"Button15",save_int,&(dlg_current_scancodes[15]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"Button16",save_int,&(dlg_current_scancodes[16]),save_WORD,&dlg_scancodes, MH_NUM_SCANCODES},
	{"DeadzoneX",save_int,&dlg_current_deadzone_x,save_int,&dlg_deadzones, MH_DEAD_ZONES},
	{"DeadzoneY",save_int,&dlg_current_deadzone_y,save_int,&dlg_deadzones, MH_DEAD_ZONES}, //18
	// 2.2 Режим 3 для одной из осей в режиме 4
	{"Mode3Axe",save_int,&dlg_current_mode3axe,save_int,&dlg_mode3axe, MH_DEAD_ZONES},
	{"FastSpeed",save_int,&dlg_current_speed,save_int,&dlg_speed, MH_NUM_SPEED},
	{"Directions",save_int,&dlg_current_direction,save_int,&dlg_dirs, MH_NUM_DIRECTIONS},
	{"Mode",save_int,&MHSettings::mode,save_empty,NULL, 8}, // Количество режимов (на самом деле нулевого нет, то есть 7)
	// 4. Таймаут
	{"TimeoutMove",save_int,&dlg_current_timeout,save_int,dlg_timeout,MH_NUM_TIMEOUT}, //23
	// 4.5. Таймаут переключения левой кнопки мыши
	{"TimeoutSwitchLeftMB",save_int,&dlg_current_switch_timeout,save_int,dlg_switch_timeout,MH_NUM_SWITCH_TIMEOUT},
	// 6. Разрешить ли нажатие пятой кнопки при быстром движении мышью  и далее
	{"FastPush", save_bool, &MHSettings::flag_enable_speed_button,save_empty,0,0}, // 25
	{"2Moves", save_bool, &MHSettings::flag_2moves,save_empty,0,0},
	{"2MovesMode1", save_bool, &MHSettings::flag_2moves_mode1,save_empty,0,0},
	{"ChangeDirOnTheWay", save_bool, &MHSettings::flag_change_direction_ontheway,save_empty,0,0},
	{"RightMBisKey", save_bool, &MHSettings::flag_right_mb_iskey,save_empty,0,0},
	{"Autoclick", save_bool, &MHSettings::flag_mode5autoclick,save_empty,0,0},
	{"CircleScale", save_int, &dlg_current_circlescale,save_int,dlg_circlescales,MH_NUM_CIRCLE_SCALES},
	{"RightMBDoubleClick", save_bool, &MHSettings::flag_right_mb_doubleclick,save_empty,0,0},
	{"LeftMBPushTwice", save_bool, &MHSettings::flag_left_mb_push_twice,save_empty,0,0}, // 34
	{"RightMBPushTwice", save_bool, &MHSettings::flag_right_mb_push_twice,save_empty,0,0},
	{"DownAll", save_bool, &MHSettings::flag_downall,save_empty,0,0},
	{"SkipFast", save_bool, &MHSettings::flag_skip_fast,save_empty,0,0},
	{"UpImmediately", save_bool, &MHSettings::flag_up_immediately,save_empty,0,0},
	{"CursorVisible", save_bool, &MHSettings::flag_cursor_visible,save_empty,0,0},
	{"MagicWindows", save_MagicWindows, 0,save_empty,0,0},//39 - сохраняет ВСЕ MagicWindows одним махом
	{"NoMoveRightMB", save_bool, &MHSettings::flag_no_move_right_mb,save_empty,0,0},//40
	{"AutoclickLMB", save_bool, &MHSettings::flag_autoclick_lmb,save_empty,0,0},//41
	{"AutoclickSpeed", save_int, &MHSettings::autoclick_speed_index,save_empty,0,4},//42
	{"AutoclickAHK", save_bool, &MHSettings::flag_autoclick_ahk,save_empty,0,0},//43
	{"WheelAHK", save_bool, &MHSettings::flag_wheel_ahk,save_empty,0,0},//44
	{"LmbWinAHK", save_bool, &MHSettings::flag_lmb_win_ahk,save_empty,0,0},//45
	{"LmbEsc", save_bool, &MHSettings::flag_lmb_esc,save_empty,0,0},//46
	{"GamepadEnabled", save_bool, &MHSettings::flag_gamepad_enabled,save_empty,0,0},
	{"GamepadA", save_int, &(MHSettings::gamepad_current_mapping[0]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadB", save_int, &(MHSettings::gamepad_current_mapping[1]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadX", save_int, &(MHSettings::gamepad_current_mapping[2]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadY", save_int, &(MHSettings::gamepad_current_mapping[3]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadLB", save_int, &(MHSettings::gamepad_current_mapping[4]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadRB", save_int, &(MHSettings::gamepad_current_mapping[5]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadLThumb", save_int, &(MHSettings::gamepad_current_mapping[6]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadRThumb", save_int, &(MHSettings::gamepad_current_mapping[7]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadStart", save_int, &(MHSettings::gamepad_current_mapping[8]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadBack", save_int, &(MHSettings::gamepad_current_mapping[9]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadDPadUp", save_int, &(MHSettings::gamepad_current_mapping[10]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadDPadDown", save_int, &(MHSettings::gamepad_current_mapping[11]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadDPadLeft", save_int, &(MHSettings::gamepad_current_mapping[12]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"GamepadDPadRight", save_int, &(MHSettings::gamepad_current_mapping[13]),save_WORD,&dlg_scancodes,MH_NUM_SCANCODES_EXTRA},
	{"Language", save_int, &MHSettings::language,save_empty,0,2}
};
int MHSettings::OpenMHookConfig(HWND hwnd, TCHAR *default_filename)
{
	if(NULL==default_filename)
	{
		// выводим диалог
		OPENFILENAME ofn=
		{
			sizeof(OPENFILENAME),
			hwnd,
			NULL, // в данном конкретном случае игнорируется
			filter_MHOOK,
			NULL,
			0, // Не используем custom filter
			0, // -"-
			tfilename,
			256,
			tfiletitle,
			256,
			NULL,
			L(LOC_FILE_OPEN_TITLE),
			OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ,
			0,
			0,
			L(LOC_FILE_EXT),
			0,0,0
		};
		// Диалог запроса имени файла
		if(0==GetOpenFileName(&ofn))
		{
			return 1;
		}
		// Имя файла показать в диалоге
		SendDlgItemMessage(hwnd,IDC_EDIT1, WM_SETTEXT, 0L, (LPARAM)tfiletitle);
	}
	else // Имя файла получено в качестве параметра функции
	{
		StringCchCopy(tfilename, _countof(tfilename), default_filename);
		// Показываем имя файла в IDC_EDIT1
		TCHAR tmpPath[MAX_PATH];
		StringCchCopy(tmpPath, MAX_PATH, default_filename);
		TCHAR* fileName = wcsrchr(tmpPath, L'\\');
		if (fileName) fileName++;
		else fileName = tmpPath;
		TCHAR* dotPos = wcsrchr(fileName, L'.');
		if (dotPos) *dotPos = L'\0';
		SendDlgItemMessage(hwnd, IDC_EDIT1, WM_SETTEXT, 0, (LPARAM)fileName);
		StringCchCopy(tfiletitle, _countof(tfiletitle), fileName);
	}
	FILE *fin=NULL;
	_wfopen_s(&fin,tfilename,L"r");
	if(NULL==fin)
	{
		StringCchCopy(tchar_buf, _countof(tchar_buf), L(LOC_ERR_CANNOT_OPEN));
		StringCchCat(tchar_buf, _countof(tchar_buf), tfilename);
		StringCchCat(tchar_buf, _countof(tchar_buf), L"'");
		MHReportError(tchar_buf);
		return (-1);
	}
	int num_succeeded=0;
	int i;
	T_save_struct ss;
	bool found;
	// Сбрасываем флаги, которых может не быть в старых конфигах
	MHSettings::flag_enable_speed_button=false;
	MHSettings::flag_2moves=false;
	MHSettings::flag_2moves_mode1=true;
	MHSettings::flag_change_direction_ontheway=false;
	MHSettings::flag_right_mb_iskey=false;
	MHSettings::flag_no_move_right_mb=false;
	MHSettings::flag_mode5autoclick=false;
	MHSettings::flag_right_mb_doubleclick=false;
	MHSettings::flag_left_mb_push_twice=false;
	MHSettings::flag_right_mb_push_twice=false;
	MHSettings::flag_downall=false;
	MHSettings::flag_skip_fast=false;
	MHSettings::flag_up_immediately=false;
	MHSettings::flag_cursor_visible=false;
	MHSettings::flag_autoclick_lmb=false;
	MHSettings::flag_autoclick_ahk=false;
	MHSettings::flag_wheel_ahk=false;
	MHSettings::flag_lmb_win_ahk=false;
	MHSettings::flag_lmb_esc=false;
	MHSettings::flag_gamepad_enabled=false;
	// Сюда считываются числа
	int int_arg1, int_arg2;
	WORD WORD_arg;
	// Читаем все строки одну за другой
	while(1==fscanf_s(fin,"%s",char_buf, static_cast<unsigned int>(_countof(char_buf))))
	{
		found=false;
		for(i=0;i<NUM_SAVE_LINES;i++) // Перебираем все возможные параметры
		{
			if(0==strncmp(char_buf,save_struct[i].name,sizeof(char_buf)-1))
			{
				ss=save_struct[i];
				fgets(char_buf,sizeof(char_buf)-1,fin); // Остаток строки загоняем в буфер
				switch(ss.save_type)
				{
				case save_int:
					// Кроме индекса нужно считать ещё и значение одного из... N типов
					switch(ss.check_type)
					{
					case save_int:
						if(2!=sscanf_s(char_buf,"%d %d",&int_arg1,&int_arg2)) goto load_error;
						if((int_arg1<0)||(int_arg1>=ss.max_index)) goto load_error;
						// Проверка, что по указанному индексу лежит правлильное значение
				// Проверку значения пропускаем для совместимости со старыми конфигами
						*((int *)ss.pointer)=int_arg1; // Всё правильно, прописываем
						break;
					case save_WORD:
						if(2!=sscanf_s(char_buf,"%d %hx",&int_arg1,&WORD_arg)) goto load_error;
						if(WORD_arg == 0xFFFF) {
							*((int *)ss.pointer) = int_arg1;
						} else {
							int found_idx = 0;
							for(int si = 0; si < ss.max_index; si++) {
								if(((MHWORDChar *)(ss.check_pointer) + si)->value == WORD_arg) {
									found_idx = si;
									break;
								}
							}
							*((int *)ss.pointer) = found_idx;
						}
						break;
					case save_empty: // Берём не из списка значений, а прямо
						if(1!=sscanf_s(char_buf,"%d",&int_arg1)) goto load_error;
						if((int_arg1<0)||(int_arg1>=ss.max_index)) goto load_error;
						*((int *)ss.pointer)=int_arg1;
						break;
					default:
						goto load_error; // Не умеем обрабатывать
					} // switch check_type
					break;
				case save_bool:
					// Используем временную переменную типа int
					if(1!=sscanf_s(char_buf,"%d",&int_arg1)) goto load_error;
					*((bool *)ss.pointer)=(int_arg1 != 0);
					break;
			case save_MagicWindows:
				{
				if(1!=sscanf_s(char_buf,"%d",&int_arg1)) goto load_error;
				int to_load = int_arg1;
				int to_skip = 0;
				if(NUM_MAGIC_WINDOWS < to_load) {
					to_skip = to_load - NUM_MAGIC_WINDOWS;
					to_load = NUM_MAGIC_WINDOWS;
				}
				if(Load2(fin, to_load)) goto load_error;
				if(to_skip > 0) {
					char skip_buf[256];
					for(int k = to_skip; k > 0; k--) {
						if(NULL == fgets(skip_buf, 256, fin)) goto load_error;
						if(NULL == fgets(skip_buf, 256, fin)) goto load_error;
					}
				}
				}
				break;
				default:
					goto load_error; // Не умеем обрабатывать
				}
				num_succeeded++; // Количество успешно считанных параметров
found=true;
				break; // Не нужно больше сравнивать, выходим из цикла
			} // если найдена строка
		} // for
		if(!found) {
			fgets(char_buf,sizeof(char_buf)-1,fin); // Пропускаем строку с неизвестным параметром
		}
	}
	fclose(fin);
	MHLanguage = MHSettings::language;
	return 0;
load_error:
	fclose(fin);
	return -1;
}
///
int MHSettings::SaveMHookConfig(HWND hwnd)
{
	// Сначала выводим диалог
		OPENFILENAME ofn=
		{
			sizeof(OPENFILENAME),
			hwnd,
			NULL, // в данном конкретном случае игнорируется
			filter_MHOOK,
			NULL,
			0, // Не используем custom filter
			0, // -"-
			tfilename,
			256,
			tfiletitle,
			256,
			NULL,
			L(LOC_FILE_SAVE_TITLE),
			OFN_OVERWRITEPROMPT,
			0,
			0,
			L(LOC_FILE_EXT),
			0,0,0
		};
	// Диалог запроса имени файла
	if(0==GetSaveFileName(&ofn))
	{
		return 1;
	}
	// Имя файла показать в диалоге
	SendDlgItemMessage(hwnd,IDC_EDIT1, WM_SETTEXT, 0L, (LPARAM)tfiletitle);
	FILE *fout=NULL;
	_wfopen_s(&fout,tfilename,L"w+");
	if(NULL==fout)
	{
		StringCchCopy(tchar_buf, _countof(tchar_buf), L(LOC_ERR_CANNOT_CREATE));
		StringCchCat(tchar_buf, _countof(tchar_buf), tfilename);
		StringCchCat(tchar_buf, _countof(tchar_buf), L"'");
		MHReportError(tchar_buf);
		return (-1);
	}
	T_save_struct ss;
	for(int i=0;i<NUM_SAVE_LINES;i++)
	{
		ss=save_struct[i];
		// Сохраняем Имя вне зависимости от типа
		fprintf(fout,"%s ",ss.name);
		// Основное значение (индекс)
		switch(ss.save_type)
		{
		case save_int:
			fprintf(fout,"%d ",*((int *)ss.pointer));
			break;
		case save_bool:
			fprintf(fout,"%d ",*((bool *)ss.pointer));
			break;
		case save_MagicWindows:
			Save2(fout);
			break;
		}
		// Проверочное значение
		switch(ss.check_type)
		{
		case save_int:
			fprintf(fout,"%d ",((MHIntChar *)(ss.check_pointer) + *((int *)ss.pointer))->value);
			break;
		case save_WORD:
			fprintf(fout,"0x%hX ",((MHWORDChar *)(ss.check_pointer) + *((int *)ss.pointer))->value);
			break;
		}
		// Перевод строки
		fprintf(fout,"\n");
	}
	fclose(fout);
	return 0;
}
//===============================================================================================
// Копирует из полей диалога в реальные переменные
//===============================================================================================
void MHSettings::BeforeSaveOrStart(HWND hdwnd)
{
			dlg_current_sensitivity=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_SENSITIVITY, CB_GETCURSEL, 0, 0L));
			MHSettings::SetMouseSensitivity(dlg_sensitivity[dlg_current_sensitivity].value);
			// 2. Кнопки
			for (int ci = 0; ci < 17; ci++)
				if(IDC_SCANCODES[ci])
					dlg_current_scancodes[ci] = static_cast<int>(SendDlgItemMessage(hdwnd, IDC_SCANCODES[ci], CB_GETCURSEL, 0, 0L));
			WORD new_scancodes[17];
			for (int si = 0; si < 17; si++)
				new_scancodes[si] = dlg_scancodes[dlg_current_scancodes[si]].value;
			MHKeypad::Init(new_scancodes);
			// 2.1. Мёртвые зоны
			dlg_current_deadzone_x=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_DEADX, CB_GETCURSEL, 0, 0L));
			MHSettings::deadx=dlg_deadzones[dlg_current_deadzone_x].value;
			dlg_current_deadzone_y=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_DEADY, CB_GETCURSEL, 0, 0L));
			MHSettings::deady=dlg_deadzones[dlg_current_deadzone_y].value;
			// 2.2 Режим 3 для одной из осей в режиме 4
			dlg_current_mode3axe=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_MODE3AXE, CB_GETCURSEL, 0, 0L));
			MHSettings::mode3axe=dlg_mode3axe[dlg_current_mode3axe].value;
			// 2.5. Минимальная скорость мыши для нажатия на пятую кнопку
			dlg_current_speed=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_SPEED, CB_GETCURSEL, 0, 0L));
			MHSettings::minimal_mouse_speed=dlg_speed[dlg_current_speed].value;
			// 2.6. Число направлений
			dlg_current_direction=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_DIRECTIONS, CB_GETCURSEL, 0, 0L));
			MHSettings::SetNumPositions(dlg_dirs[dlg_current_direction].value);
			// 3. Радио-кнопка
			MHSettings::flag_no_move_right_mb=false; // Во всех режимах, кроме пятого
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_RADIO1,BM_GETCHECK, 0, 0))
			{
				MHSettings::mode=1;
				MHSettings::hh=&hh1;
			}
			else if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_RADIO2,BM_GETCHECK, 0, 0))
			{
				MHSettings::mode=2;
				MHSettings::hh=&hh2;
			}
			else if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_RADIO3,BM_GETCHECK, 0, 0))
			{
				MHSettings::mode=3;
				MHSettings::hh=&hh3;
			}
			else if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_RADIO4,BM_GETCHECK, 0, 0))
			{
				MHSettings::mode=4;
				MHSettings::hh=&hh4;
				// Важно!!! В 4 режиме принудительно выставить 8 позиций!!!
				MHSettings::SetNumPositions(8); dlg_current_direction=1;
			}
			else if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_RADIO5,BM_GETCHECK, 0, 0))
			{
				MHSettings::mode=5;
				MHSettings::hh=&hh5;
				// Важно!!! В 5 режиме принудительно запрещается движение курсора при правом нажатии!!
				MHSettings::flag_no_move_right_mb=true;
				MHSettings::SetNumPositions(8); dlg_current_direction=1; // и принудительно выставить 8 позиций!!!
			}
			else if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_RADIO6,BM_GETCHECK, 0, 0))
			{
				MHSettings::mode=6;
				MHSettings::hh=&hh6;
				// Важно!!! В 6 режиме принудительно выставить 4 позиций!!!
				MHSettings::SetNumPositions(4); dlg_current_direction=0;
			}
			else if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_RADIO7,BM_GETCHECK, 0, 0))
			{
				MHSettings::mode=7;
				MHSettings::hh=&hh7;
			}
			//else
			// 3.2. - кнопка не используется
			// 4. Таймаут
			dlg_current_timeout=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_TIMEOUT, CB_GETCURSEL, 0, 0L));
			MHSettings::timeout_after_move=dlg_timeout[dlg_current_timeout].value;
			// 4.5. Таймаут переключения режима левой кнопки мыши
			dlg_current_switch_timeout=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_SWITCH_TIMEOUT, CB_GETCURSEL, 0, 0L));
			MHSettings::timeout_mouse_switch=dlg_switch_timeout[dlg_current_switch_timeout].value;
			// 4.6.
			dlg_current_circlescale=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_CIRCLE_SCALES, CB_GETCURSEL, 0, 0L));
			MHSettings::circle_scale_factor=dlg_circlescales[dlg_current_circlescale].value;
			// 6. Разрешить ли нажатие пятой кнопки при быстром движении мышью
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_FAST_PUSH,BM_GETCHECK, 0, 0))
				MHSettings::flag_enable_speed_button=true;
			else MHSettings::flag_enable_speed_button=false;
			// 7. Использовать ли в 4 режиме движение в 2 шага
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_2MOVES,BM_GETCHECK, 0, 0))
				MHSettings::flag_2moves=true;
			else MHSettings::flag_2moves=false;
			// 8. Использовать ли в 1 режиме движение в 2 шага
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_2MOVES_MODE1,BM_GETCHECK, 0, 0))
				MHSettings::flag_2moves_mode1=true;
			else MHSettings::flag_2moves_mode1=false;
			// 9. можно ли менять направление движения на ходу
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_CHANGE_DIRECTION_ONTHEWAY,BM_GETCHECK, 0, 0))
				MHSettings::flag_change_direction_ontheway=true;
			else MHSettings::flag_change_direction_ontheway=false;
			// 10. правая кнопка мыши вместо обычного поведения ведёт себя, как клавиша
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_RIGHT_MB_ISKEY,BM_GETCHECK, 0, 0))
				MHSettings::flag_right_mb_iskey=true;
			else MHSettings::flag_right_mb_iskey=false;
			// 12. автоклик в режиме 5
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_AUTOCLICK,BM_GETCHECK, 0, 0))
				MHSettings::flag_mode5autoclick=true;
			else MHSettings::flag_mode5autoclick=false;
			// 13. Стоп эмуляции по двойному щелчку
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_RIGHT_DBLCLK,BM_GETCHECK, 0, 0))
				MHSettings::flag_right_mb_doubleclick=true;
			else MHSettings::flag_right_mb_doubleclick=false;
			// 14. нажимать клавишу также при отпускании ЛК мыши
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_LEFT_PUSH_TWICE,BM_GETCHECK, 0, 0))
				MHSettings::flag_left_mb_push_twice=true;
			else MHSettings::flag_left_mb_push_twice=false;
			// 15. нажимать клавишу также при отпускании ЛК мыши
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_RIGHT_PUSH_TWICE,BM_GETCHECK, 0, 0))
				MHSettings::flag_right_mb_push_twice=true;
			else MHSettings::flag_right_mb_push_twice=false;
			// 16. вниз+вбок = просто вниз (режим 1)
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_DOWNALL,BM_GETCHECK, 0, 0))
				MHSettings::flag_downall=true;
			else MHSettings::flag_downall=false;
			// 17. вниз+вбок сразу отпускать (режим 1)
			if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_UP_IMMEDIATELY,BM_GETCHECK, 0, 0))
				MHSettings::flag_up_immediately=true;
			else MHSettings::flag_up_immediately=false;
		// 18. игнорировать быстрое движение (режим 3)
		if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_SKIP_FAST,BM_GETCHECK, 0, 0))
			MHSettings::flag_skip_fast=true;
		else MHSettings::flag_skip_fast=false;
		// 19. автокликер для левой кнопки мыши (отдельный чекбокс)
		if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_LMB_AUTOCLICK,BM_GETCHECK, 0, 0))
			MHSettings::flag_autoclick_lmb=true;
		else MHSettings::flag_autoclick_lmb=false;
		MHSettings::autoclick_speed_index=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_AUTOCLICK_SPEED, CB_GETCURSEL, 0, 0L));
		if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_AHK_AUTOCLICK,BM_GETCHECK, 0, 0))
			MHSettings::flag_autoclick_ahk=true;
		else MHSettings::flag_autoclick_ahk=false;
		if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_WHEEL_AHK,BM_GETCHECK, 0, 0))
			MHSettings::flag_wheel_ahk=true;
		else MHSettings::flag_wheel_ahk=false;
		if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_LMB_WIN_AHK,BM_GETCHECK, 0, 0))
			MHSettings::flag_lmb_win_ahk=true;
		else MHSettings::flag_lmb_win_ahk=false;
		if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_LMB_ESC,BM_GETCHECK, 0, 0))
			MHSettings::flag_lmb_esc=true;
		else MHSettings::flag_lmb_esc=false;
		// Запуск AHK скриптов при активации галочек
		if(MHSettings::flag_autoclick_ahk) {
			TCHAR* scriptPath = EmbeddedExe_GetAutoClickPath();
			if(scriptPath) {
				TCHAR exeDir[MAX_PATH];
				StringCchCopy(exeDir, MAX_PATH, scriptPath);
				PathRemoveFileSpec(exeDir);
				ShellExecute(NULL, _T("open"), scriptPath, NULL, exeDir, SW_SHOW);
				MHSettings::flag_autoclick_ahk_loaded=true;
			}
		}
		if(MHSettings::flag_wheel_ahk) {
			TCHAR* scriptPath = EmbeddedExe_GetWheelPath();
			if(scriptPath) {
				TCHAR exeDir[MAX_PATH];
				StringCchCopy(exeDir, MAX_PATH, scriptPath);
				PathRemoveFileSpec(exeDir);
				ShellExecute(NULL, _T("open"), scriptPath, NULL, exeDir, SW_SHOW);
				MHSettings::flag_wheel_ahk_loaded=true;
			}
		}
		// 20. видимый курсор (красная точка)
		if(BST_CHECKED==SendDlgItemMessage(hdwnd,IDC_CHECK_CURSOR_VISIBLE,BM_GETCHECK, 0, 0))
			MHSettings::flag_cursor_visible=true;
		else MHSettings::flag_cursor_visible=false;
}