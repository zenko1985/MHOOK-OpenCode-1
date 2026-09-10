#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"
#include "Settings.h"
#include "MagicWindow.h"
#include "TET.h"
#include "TobiiREX.h"
#include "CircleWindow.h"
#include "Localization.h"
#include "Scancode.h"
extern MHWORDChar dlg_scancodes[MH_NUM_SCANCODES_EXTRA];
bool G_eytracker_is_working=false; // От этого зависит поведение кнопки
int G_eytracker_num=0; // какой из трекеров выбран
bool G_show_circle=false;
static HBRUSH tracker_brush;
static RECT tracker_rect={300,15,330,40};
extern HBRUSH green_brush, yellow_brush, red_brush;
bool G_dialogue2_active=false; // чтобы окна могли в него писать на лету
HWND Settings2HWND; // Для тех же целей
// Списки полей диалога
int IDC_COMBO_WKEYS[NUM_MAGIC_WINDOWS] ={IDC_COMBO_WKEY1, IDC_COMBO_WKEY2, IDC_COMBO_WKEY3, IDC_COMBO_WKEY4,
	IDC_COMBO_WKEY5, IDC_COMBO_WKEY6, IDC_COMBO_WKEY7, IDC_COMBO_WKEY8,
	IDC_COMBO_WKEY9, IDC_COMBO_WKEY10, IDC_COMBO_WKEY11, IDC_COMBO_WKEY12,
	IDC_COMBO_WKEY13, IDC_COMBO_WKEY14, IDC_COMBO_WKEY15, IDC_COMBO_WKEY16,
	IDC_COMBO_WKEY17, IDC_COMBO_WKEY18};
int IDC_EDIT_WNAMES[NUM_MAGIC_WINDOWS] ={IDC_EDIT_WNAME1,IDC_EDIT_WNAME2,IDC_EDIT_WNAME3,IDC_EDIT_WNAME4,
	IDC_EDIT_WNAME5, IDC_EDIT_WNAME6, IDC_EDIT_WNAME7, IDC_EDIT_WNAME8,
	IDC_EDIT_WNAME9, IDC_EDIT_WNAME10, IDC_EDIT_WNAME11, IDC_EDIT_WNAME12,
	IDC_EDIT_WNAME13, IDC_EDIT_WNAME14, IDC_EDIT_WNAME15, IDC_EDIT_WNAME16,
	IDC_EDIT_WNAME17,IDC_EDIT_WNAME18};
int IDC_CHECK_WWISIBLES[NUM_MAGIC_WINDOWS] ={IDC_CHECK_WWISIBLE1, IDC_CHECK_WWISIBLE2,IDC_CHECK_WWISIBLE3,IDC_CHECK_WWISIBLE4,
	IDC_CHECK_WWISIBLE5, IDC_CHECK_WWISIBLE6, IDC_CHECK_WWISIBLE7, IDC_CHECK_WWISIBLE8,
	IDC_CHECK_WWISIBLE9, IDC_CHECK_WWISIBLE10, IDC_CHECK_WWISIBLE11, IDC_CHECK_WWISIBLE12,
	IDC_CHECK_WWISIBLE13, IDC_CHECK_WWISIBLE14, IDC_CHECK_WWISIBLE15, IDC_CHECK_WWISIBLE16,
	IDC_CHECK_WWISIBLE17,IDC_CHECK_WWISIBLE18};
int IDC_COMBO_WSENS[NUM_MAGIC_WINDOWS]={IDC_COMBO_WSENS1,IDC_COMBO_WSENS2,IDC_COMBO_WSENS3,IDC_COMBO_WSENS4,
	IDC_COMBO_WSENS5, IDC_COMBO_WSENS6, IDC_COMBO_WSENS7, IDC_COMBO_WSENS8,
	IDC_COMBO_WSENS9, IDC_COMBO_WSENS10, IDC_COMBO_WSENS11, IDC_COMBO_WSENS12,
	IDC_COMBO_WSENS13, IDC_COMBO_WSENS14, IDC_COMBO_WSENS15, IDC_COMBO_WSENS16,
	IDC_COMBO_WSENS17,IDC_COMBO_WSENS18};
int IDC_COMBO_WCOLORS[NUM_MAGIC_WINDOWS]={IDC_COMBO_WCOLOR1,IDC_COMBO_WCOLOR2,IDC_COMBO_WCOLOR3,IDC_COMBO_WCOLOR4,
	IDC_COMBO_WCOLOR5, IDC_COMBO_WCOLOR6, IDC_COMBO_WCOLOR7, IDC_COMBO_WCOLOR8,
	IDC_COMBO_WCOLOR9, IDC_COMBO_WCOLOR10, IDC_COMBO_WCOLOR11, IDC_COMBO_WCOLOR12,
	IDC_COMBO_WCOLOR13, IDC_COMBO_WCOLOR14, IDC_COMBO_WCOLOR15, IDC_COMBO_WCOLOR16,
	IDC_COMBO_WCOLOR17,IDC_COMBO_WCOLOR18};
int IDC_COMBO_WREPEATS[NUM_MAGIC_WINDOWS]={IDC_COMBO_WREPEAT1,IDC_COMBO_WREPEAT2,IDC_COMBO_WREPEAT3,IDC_COMBO_WREPEAT4,
	IDC_COMBO_WREPEAT5, IDC_COMBO_WREPEAT6, IDC_COMBO_WREPEAT7, IDC_COMBO_WREPEAT8,
	IDC_COMBO_WREPEAT9, IDC_COMBO_WREPEAT10, IDC_COMBO_WREPEAT11, IDC_COMBO_WREPEAT12,
	IDC_COMBO_WREPEAT13, IDC_COMBO_WREPEAT14, IDC_COMBO_WREPEAT15, IDC_COMBO_WREPEAT16,
	IDC_COMBO_WREPEAT17,IDC_COMBO_WREPEAT18};
int IDC_COMBO_WGROUPS[NUM_MAGIC_WINDOWS]={IDC_COMBO_WGROUP1,IDC_COMBO_WGROUP2,IDC_COMBO_WGROUP3,IDC_COMBO_WGROUP4,
	IDC_COMBO_WGROUP5, IDC_COMBO_WGROUP6, IDC_COMBO_WGROUP7, IDC_COMBO_WGROUP8,
	IDC_COMBO_WGROUP9, IDC_COMBO_WGROUP10, IDC_COMBO_WGROUP11, IDC_COMBO_WGROUP12,
	IDC_COMBO_WGROUP13, IDC_COMBO_WGROUP14, IDC_COMBO_WGROUP15, IDC_COMBO_WGROUP16,
	IDC_COMBO_WGROUP17,	IDC_COMBO_WGROUP18};
// Координаты окон
int IDC_EDIT_WXS[NUM_MAGIC_WINDOWS]={IDC_EDIT_WX1,IDC_EDIT_WX2,IDC_EDIT_WX3,IDC_EDIT_WX4,
	IDC_EDIT_WX5, IDC_EDIT_WX6, IDC_EDIT_WX7, IDC_EDIT_WX8,
	IDC_EDIT_WX9, IDC_EDIT_WX10, IDC_EDIT_WX11, IDC_EDIT_WX12,
	IDC_EDIT_WX13, IDC_EDIT_WX14, IDC_EDIT_WX15, IDC_EDIT_WX16,
	IDC_EDIT_WX17,IDC_EDIT_WX18};
int IDC_EDIT_WYS[NUM_MAGIC_WINDOWS]={IDC_EDIT_WY1,IDC_EDIT_WY2,IDC_EDIT_WY3,IDC_EDIT_WY4,
	IDC_EDIT_WY5, IDC_EDIT_WY6, IDC_EDIT_WY7, IDC_EDIT_WY8,
	IDC_EDIT_WY9, IDC_EDIT_WY10, IDC_EDIT_WY11, IDC_EDIT_WY12,
	IDC_EDIT_WY13, IDC_EDIT_WY14, IDC_EDIT_WY15, IDC_EDIT_WY16,
	IDC_EDIT_WY17,IDC_EDIT_WY18};
int IDC_EDIT_WWIDTHS[NUM_MAGIC_WINDOWS]={IDC_EDIT_WWIDTH1,IDC_EDIT_WWIDTH2,IDC_EDIT_WWIDTH3,IDC_EDIT_WWIDTH4,
	IDC_EDIT_WWIDTH5, IDC_EDIT_WWIDTH6, IDC_EDIT_WWIDTH7, IDC_EDIT_WWIDTH8,
	IDC_EDIT_WWIDTH9, IDC_EDIT_WWIDTH10, IDC_EDIT_WWIDTH11, IDC_EDIT_WWIDTH12,
	IDC_EDIT_WWIDTH13, IDC_EDIT_WWIDTH14, IDC_EDIT_WWIDTH15, IDC_EDIT_WWIDTH16,
	IDC_EDIT_WWIDTH17,IDC_EDIT_WWIDTH18};
int IDC_EDIT_WHEIGHTS[NUM_MAGIC_WINDOWS]={IDC_EDIT_WHEIGHT1, IDC_EDIT_WHEIGHT2,IDC_EDIT_WHEIGHT3,IDC_EDIT_WHEIGHT4,
	IDC_EDIT_WHEIGHT5, IDC_EDIT_WHEIGHT6, IDC_EDIT_WHEIGHT7, IDC_EDIT_WHEIGHT8,
	IDC_EDIT_WHEIGHT9, IDC_EDIT_WHEIGHT10, IDC_EDIT_WHEIGHT11, IDC_EDIT_WHEIGHT12,
	IDC_EDIT_WHEIGHT13, IDC_EDIT_WHEIGHT14, IDC_EDIT_WHEIGHT15, IDC_EDIT_WHEIGHT16,
	IDC_EDIT_WHEIGHT17,IDC_EDIT_WHEIGHT18};
#define MH_NUM_SENS 2
TCHAR *dlg_sens[MH_NUM_SENS];
#define MH_NUM_COLORS 4
TCHAR *dlg_colors[MH_NUM_COLORS];
#define MH_NUM_REPEAT 2
TCHAR *dlg_wrepeat[MH_NUM_REPEAT];
#define MH_NUM_GROUPS 5
TCHAR *dlg_wgroup[MH_NUM_GROUPS];
static void InitDialog2Strings()
{
	dlg_sens[0]=(TCHAR*)L(LOC_COMBO_MOUSE);
	dlg_sens[1]=(TCHAR*)L(LOC_COMBO_EYETRACKER);
	dlg_colors[0]=(TCHAR*)L(LOC_COMBO_GREEN);
	dlg_colors[1]=(TCHAR*)L(LOC_COMBO_YELLOW);
	dlg_colors[2]=(TCHAR*)L(LOC_COMBO_RED);
	dlg_colors[3]=(TCHAR*)L(LOC_COMBO_BLUE);
	dlg_wrepeat[0]=(TCHAR*)L(LOC_COMBO_BUTTON);
	dlg_wrepeat[1]=(TCHAR*)L(LOC_COMBO_SWITCH);
	dlg_wgroup[0]=(TCHAR*)L(LOC_COMBO_NO_GROUP);
	dlg_wgroup[1]=L"1";
	dlg_wgroup[2]=L"2";
	dlg_wgroup[3]=L"3";
	dlg_wgroup[4]=L"4";
}
//===================================================================================================
// Функция соединения/разъединения от айтреекера
//===================================================================================================
static void EytrackerConnect(HWND hwnd)
{
	int initresult; // результат включения/выключения
	// блокируем кнопки до конца операции
	EnableWindow( GetDlgItem( hwnd, IDC_EYETRACKER_CONNECT ), FALSE);
	EnableWindow( GetDlgItem( hwnd, IDC_COMBO_EYETRACKER ), FALSE);
	// Включаем или выключаем?
	if(G_eytracker_is_working) // выключаем
	{
		// здесь будет выключение
		if(1==G_eytracker_num) initresult=BKBTobiiREX::Halt(hwnd);
		else initresult=BKBTET::Halt(hwnd);
		// Результат игнорируем...
		//Sleep(1000);
		// Меняем надпись на кнопке
		SetWindowText(GetDlgItem( hwnd, IDC_EYETRACKER_CONNECT ), L(LOC_BTN_START_TRACKER));
		// Разблокируем список айтрекеров
		EnableWindow( GetDlgItem( hwnd, IDC_COMBO_EYETRACKER ), TRUE);
		// рисуем красный кружок
		tracker_brush=red_brush;
		InvalidateRect(hwnd,NULL,FALSE);
		G_eytracker_is_working=false;
		if(G_show_circle)
			CircleWindow::Hide();
	}
	else // включаем
	{
		// здесь будет включение
		if(1==G_eytracker_num) initresult=BKBTobiiREX::Init(hwnd);
		else initresult=BKBTET::Init(hwnd);
		//Sleep(1000);
		// Если включение удалось
		if(0==initresult)
		{
			// Меняем надпись на кнопке
			SetWindowText(GetDlgItem( hwnd, IDC_EYETRACKER_CONNECT ), L(LOC_BTN_STOP_TRACKER));
			// рисуем зелёный кружок
			tracker_brush=green_brush;
			InvalidateRect(hwnd,NULL,FALSE);
			G_eytracker_is_working=true;
			if(G_show_circle) CircleWindow::Show();
		}
		else // Если не удалось - оставляем красный кружок и разблокировать список
		{
			// Разблокируем список айтрекеров
			EnableWindow( GetDlgItem( hwnd, IDC_COMBO_EYETRACKER ), TRUE);
		}
	}
	// В конце операции разблокируем кнопку, но список разблокируется, только если айтрекер выключен
	EnableWindow( GetDlgItem( hwnd, IDC_EYETRACKER_CONNECT ), TRUE);
}
//===================================================================================================
// Оконная процедура второго диалога
//===================================================================================================
BOOL CALLBACK DlgSettings2WndProc(HWND hdwnd,
						   UINT uMsg,
						   WPARAM wparam,
						   LPARAM lparam )
{
	int i;
	WORD hi,lo;
	if (WM_COMMAND==uMsg)
	{
		// 1. IDOK
		switch (LOWORD(wparam))
		{
		case IDOK: 	//Хорошо!
		case IDCANCEL: // результат всё равно один
			G_dialogue2_active=false;
			MHSettings::BeforeSaveOrStart2(hdwnd);
			EndDialog(hdwnd,0);
			return 1;
		case IDC_EYETRACKER_CONNECT:
			EytrackerConnect(hdwnd);
			return 1;
		}
		// 2. Перебираем все controls
		hi=HIWORD(wparam);
		lo=LOWORD(wparam);
		for(i=0;i<NUM_MAGIC_WINDOWS;i++)
		{
			// 2.1. Имена окон
			if(IDC_EDIT_WNAMES[i]==lo)
			{
				if(hi==EN_CHANGE)
				{
					SendDlgItemMessage(Settings2HWND,IDC_EDIT_WNAMES[i],WM_GETTEXT, _countof(MagicWindow::magic_wnd[i].mw_name), (LPARAM)MagicWindow::magic_wnd[i].mw_name);
					SetWindowText(MagicWindow::magic_wnd[i].MWhwnd,MagicWindow::magic_wnd[i].mw_name);
					InvalidateRect(MagicWindow::magic_wnd[i].MWhwnd,NULL,FALSE);
					return 1;
				}
			}
			// 2.2. Цвета окон
			if(IDC_COMBO_WCOLORS[i]==lo)
			{
				if(hi==CBN_SELENDOK)
				{
					MagicWindow::magic_wnd[i].mw_color=static_cast<int>(SendDlgItemMessage(Settings2HWND,IDC_COMBO_WCOLORS[i], CB_GETCURSEL, 0, 0L));
					InvalidateRect(MagicWindow::magic_wnd[i].MWhwnd,NULL,FALSE);
					return 1;
				}
			}
			// 2.3. Видимость окон
			if(IDC_CHECK_WWISIBLES[i]==lo)
			{
				if(hi==BN_CLICKED )
				{
					if(BST_CHECKED==SendDlgItemMessage(Settings2HWND,IDC_CHECK_WWISIBLES[i],BM_GETCHECK, 0, 0))
					{
						MagicWindow::magic_wnd[i].active=1;
						ShowWindow(MagicWindow::magic_wnd[i].MWhwnd,SW_SHOWNORMAL);
					}
					else
					{
						MagicWindow::magic_wnd[i].active=0;
						ShowWindow(MagicWindow::magic_wnd[i].MWhwnd,SW_HIDE);
					}
					return 1;
				}
			}
		} // for
		// 2.4. Айтрекер
		if(IDC_COMBO_EYETRACKER==lo)
		{
			if(hi==CBN_SELENDOK)
			{
				G_eytracker_num=static_cast<int>(SendDlgItemMessage(Settings2HWND,IDC_COMBO_EYETRACKER, CB_GETCURSEL, 0, 0L));
				return 1;
			}
		}
		// 2.5. Показывать кружок взгляда
		if(IDC_CHECK_EYECIRCLE==lo)
		{
			if(hi==BN_CLICKED )
			{
				if(BST_CHECKED==SendDlgItemMessage(Settings2HWND,IDC_CHECK_EYECIRCLE,BM_GETCHECK, 0, 0))
				{
					G_show_circle=true;
					if(G_eytracker_is_working) CircleWindow::Show();
				}
				else
				{
					G_show_circle=false;
					CircleWindow::Hide();
				}
				return 1;
			}
		}
	} // switch WM_COMMAND
	else if(WM_INITDIALOG==uMsg)
	{
		Settings2HWND=hdwnd;
		G_dialogue2_active=true; // чтобы окна могли в него писать на лету
		MHSettings::FillDialogue2(hdwnd);
		// каким цветом рисовать состояние трекера?
		if(G_eytracker_is_working) tracker_brush=green_brush;
		else tracker_brush=red_brush;
		return 1;
	}
	else if(WM_PAINT==uMsg)
	{
		HDC hdc;
		PAINTSTRUCT ps;
		hdc=BeginPaint(hdwnd,&ps);
		FillRect(hdc,&tracker_rect,tracker_brush);
		EndPaint(hdwnd,&ps);
	}
return 0;
}
//====================================================================================
// Заполнить выпадающие списки диалога возможными значениями
//====================================================================================
void MHSettings::FillDialogue2(HWND hdwnd)
{
	int i,k;
	InitDialog2Strings();
	// Про ай-трекеры
	SendDlgItemMessage(hdwnd,IDC_COMBO_EYETRACKER, CB_ADDSTRING, 0, (LPARAM)L"TheEyeTribe");
	// Для Tobii раньше были только 32-разрядные библиотеки
	SendDlgItemMessage(hdwnd,IDC_COMBO_EYETRACKER, CB_ADDSTRING, 0, (LPARAM)L"Tobii 4C/EyeX");
	SendDlgItemMessage(hdwnd,IDC_COMBO_EYETRACKER, CB_SETCURSEL, G_eytracker_num, 0L);
	if(G_eytracker_is_working) 		SetWindowText(GetDlgItem( hdwnd, IDC_EYETRACKER_CONNECT ), L(LOC_BTN_STOP_TRACKER));
	else	SetWindowText(GetDlgItem( hdwnd, IDC_EYETRACKER_CONNECT ), L(LOC_BTN_START_TRACKER));
	if(G_show_circle) SendDlgItemMessage(hdwnd, IDC_CHECK_EYECIRCLE, BM_SETCHECK, BST_CHECKED, 0);
	else SendDlgItemMessage(hdwnd, IDC_CHECK_EYECIRCLE, BM_SETCHECK, BST_UNCHECKED, 0);
	// перебор всех строк
	for(i=0;i<NUM_MAGIC_WINDOWS;i++)
	{
		// 1.активно ли?
		if(MagicWindow::magic_wnd[i].active)  SendDlgItemMessage(hdwnd, IDC_CHECK_WWISIBLES[i], BM_SETCHECK, BST_CHECKED, 0);
		else SendDlgItemMessage(hdwnd, IDC_CHECK_WWISIBLES[i], BM_SETCHECK, BST_UNCHECKED, 0);
		// 2. имя окна
		SendDlgItemMessage(hdwnd,IDC_EDIT_WNAMES[i],WM_SETTEXT, 0, (LPARAM)MagicWindow::magic_wnd[i].mw_name);
		// 3.мышь или айтрекер
		for(k=0;k<MH_NUM_SENS;k++)
		{
			SendDlgItemMessage(hdwnd,IDC_COMBO_WSENS[i], CB_ADDSTRING, 0, (LPARAM)(dlg_sens[k]));
		}
		SendDlgItemMessage(hdwnd,IDC_COMBO_WSENS[i], CB_SETCURSEL, MagicWindow::magic_wnd[i].mouse_or_eytracker, 0L);
		// 4. цвета
		for(k=0;k<MH_NUM_COLORS;k++)
		{
			SendDlgItemMessage(hdwnd,IDC_COMBO_WCOLORS[i], CB_ADDSTRING, 0, (LPARAM)(dlg_colors[k]));
		}
		SendDlgItemMessage(hdwnd,IDC_COMBO_WCOLORS[i], CB_SETCURSEL, MagicWindow::magic_wnd[i].mw_color, 0L);
		// 5.клавиши
		for(k=0;k<MH_NUM_SCANCODES_EXTRA;k++)
		{
			SendDlgItemMessage(hdwnd,IDC_COMBO_WKEYS[i], CB_ADDSTRING, 0, (LPARAM)(dlg_scancodes[k].stroka));
		}
		SendDlgItemMessage(hdwnd,IDC_COMBO_WKEYS[i], CB_SETCURSEL, MagicWindow::magic_wnd[i].button_index, 0L);
		// 6.кнопка или переключатель
		for(k=0;k<MH_NUM_REPEAT;k++)
		{
			SendDlgItemMessage(hdwnd,IDC_COMBO_WREPEATS[i], CB_ADDSTRING, 0, (LPARAM)(dlg_wrepeat[k]));
		}
		SendDlgItemMessage(hdwnd,IDC_COMBO_WREPEATS[i], CB_SETCURSEL, MagicWindow::magic_wnd[i].button_or_switch, 0L);
		// 7.группа
		for(k=0;k<MH_NUM_GROUPS;k++)
		{
			SendDlgItemMessage(hdwnd,IDC_COMBO_WGROUPS[i], CB_ADDSTRING, 0, (LPARAM)(dlg_wgroup[k]));
		}
		SendDlgItemMessage(hdwnd,IDC_COMBO_WGROUPS[i], CB_SETCURSEL, MagicWindow::magic_wnd[i].mw_group, 0L);
		// 8. координаты и размеры (как в оконной процедурe MagicWindow)
		TCHAR unicode_buf[80];
		_itow_s(MagicWindow::magic_wnd[i].x,unicode_buf,10);
		SendDlgItemMessage(Settings2HWND,IDC_EDIT_WXS[i],WM_SETTEXT, 0, (LPARAM)unicode_buf);
		_itow_s(MagicWindow::magic_wnd[i].y,unicode_buf,10);
		SendDlgItemMessage(Settings2HWND,IDC_EDIT_WYS[i],WM_SETTEXT, 0, (LPARAM)unicode_buf);
		_itow_s(MagicWindow::magic_wnd[i].width,unicode_buf,10);
		SendDlgItemMessage(Settings2HWND,IDC_EDIT_WWIDTHS[i],WM_SETTEXT, 0, (LPARAM)unicode_buf);
		_itow_s(MagicWindow::magic_wnd[i].height,unicode_buf,10);
		SendDlgItemMessage(Settings2HWND,IDC_EDIT_WHEIGHTS[i],WM_SETTEXT, 0, (LPARAM)unicode_buf);
	}
}
//====================================================================================
// Получить характеристики окон, которые не подхватываются на лету
//====================================================================================
void MHSettings::BeforeSaveOrStart2(HWND hdwnd)
{
	int i;
	// перебор всех строк
	for(i=0;i<NUM_MAGIC_WINDOWS;i++)
	{
		// 3.мышь или айтрекер
		MagicWindow::magic_wnd[i].mouse_or_eytracker=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_COMBO_WSENS[i], CB_GETCURSEL, 0, 0L));
		// 5.клавиши
		MagicWindow::magic_wnd[i].button_index=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_COMBO_WKEYS[i], CB_GETCURSEL, 0, 0L));
		// 6.кнопка или переключатель
		MagicWindow::magic_wnd[i].button_or_switch=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_COMBO_WREPEATS[i], CB_GETCURSEL, 0, 0L));
		// 7.группа
		MagicWindow::magic_wnd[i].mw_group=static_cast<int>(SendDlgItemMessage(hdwnd,IDC_COMBO_WGROUPS[i], CB_GETCURSEL, 0, 0L));
	}
}
//====================================================================================
// Запись в файл конфигурации окон
//====================================================================================
void MHSettings::Save2(FILE *f)
{
	int i;
	MagicWindow *mw;
	char lcbuffer[1024];
	// Для проверки, что количество окон совпадает
	fprintf(f,"%d", NUM_MAGIC_WINDOWS);
	for(i=0;i<NUM_MAGIC_WINDOWS;i++)
	{
		mw=&MagicWindow::magic_wnd[i];
		// Параметры-циферки
		fprintf(f,"\n%d %d %d %d %d %d %d %d %d %d\n", mw->active, mw->mw_color, mw->x, mw->y, mw->width, mw->height,
			mw->button_or_switch, mw->mouse_or_eytracker, mw->button_index, mw->mw_group);
		// Параметр-строка
		int cbLen = WideCharToMultiByte(CP_UTF8,0,mw->mw_name,-1,lcbuffer,sizeof(lcbuffer),NULL,NULL);
		if(cbLen > 0 && cbLen <= sizeof(lcbuffer)) fputs(lcbuffer,f);
	}
}
//====================================================================================
// Считывание из файла конфигурации окон
//====================================================================================
// Таблица соответствия старого порядка индексов (для конфигов, созданных до reorder)
static const WORD old_scancode_order[MH_NUM_SCANCODES_EXTRA] = {
	SC_NONE, // 0
	SC_UP, SC_RIGHT, SC_DOWN, SC_LEFT, // 1-4
	SC_A, SC_B, SC_C, SC_D, SC_E, // 5-9
	SC_F, SC_G, SC_H, SC_I, SC_J, // 10-14
	SC_K, SC_L, SC_M, SC_N, SC_O, // 15-19
	SC_P, SC_Q, SC_R, SC_S, SC_T, // 20-24
	SC_U, SC_V, SC_W, SC_X, SC_Y, // 25-29
	SC_Z, SC_0, SC_1, SC_2, SC_3, // 30-34
	SC_4, SC_5, SC_6, SC_7, SC_8, // 35-39
	SC_9, SC_TILDE, SC_MINUS, SC_EQUALS, SC_BACKSLASH, // 40-44
	SC_LBRACKET, SC_RBRACKET, SC_SEMICOLON, SC_QUOTE, SC_COMMA, // 45-49
	SC_PERIOD, SC_SLASH, SC_BACKSPACE, SC_SPACE, SC_TAB, // 50-54
	SC_CAPSLOCK, SC_LSHIFT, SC_LCTRL, SC_LALT, SC_LWIN, // 55-59
	SC_RSHIFT, SC_RCTRL, SC_RALT, SC_RWIN, SC_MENU, // 60-64
	SC_ENTER, SC_ESC, SC_F1, SC_F2, SC_F3, // 65-69
	SC_F4, SC_F5, SC_F6, SC_F7, 0xFFFF, // 70-74 (F8 disabled)
	SC_F9, SC_F10, SC_F11, SC_F12, SC_SCROLLLOCK, // 75-79
	SC_INSERT, 0xE053, SC_HOME, SC_END, SC_PGUP, // 80-84 (Delete disabled)
	SC_PGDN, SC_NUMLOCK, SC_NUMSLASH, SC_NUMASTER, SC_NUMMINUS, // 85-89
	SC_NUMPLUS, SC_NUMENTER, 0xFFFF, SC_NUM0, SC_NUM1, // 90-94 (NumDot disabled)
	SC_NUM2, SC_NUM3, SC_NUM4, SC_NUM5, SC_NUM6, // 95-99
	SC_NUM7, SC_NUM8, SC_NUM9, // 100-102
	SC_LMOUSE, SC_RMOUSE, // 103-104
	SC_LMOUSE_F12, SC_AUTO_LEFT, SC_AUTO_RIGHT, SC_SCROLL_THERE, SC_SCROLL_HERE, // 105-109
	SC_MIDDLEMB, SC_WHEEL_UP, SC_WHEEL_DOWN // 110-112
};

int MHSettings::Load2(FILE *f,int wcount)
{
	int i,len;
	MagicWindow *mw;
	char lcbuffer[1024];
	for(i=0;i<wcount;i++)
	{
		mw=&MagicWindow::magic_wnd[i];
		// Параметры-циферки
		if(10!=fscanf_s(f,"%d %d %d %d %d %d %d %d %d %d\n", &mw->active, &mw->mw_color, &mw->x, &mw->y, &mw->width, &mw->height,
			&mw->button_or_switch, &mw->mouse_or_eytracker, &mw->button_index, &mw->mw_group))
			return 1;
		{
			WORD val = old_scancode_order[mw->button_index];
			if(val == 0xFFFF) {
				// Disabled key — keep original index as-is
			} else {
				for(int si = 0; si < MH_NUM_SCANCODES_EXTRA; si++) {
					if(dlg_scancodes[si].value == val) {
						mw->button_index = si;
						break;
					}
				}
			}
		}
		// Обязательно применить к окнам!
		// Корректируем размеры: в файле хранятся размеры клиентской области,
		// а MoveWindow ожидает размеры всего окна с учетом рамки и заголовка
		RECT rc = {0, 0, mw->width, mw->height};
		AdjustWindowRect(&rc, WS_CAPTION|WS_THICKFRAME, FALSE);
		int total_width = rc.right - rc.left;
		int total_height = rc.bottom - rc.top;
		MoveWindow(mw->MWhwnd, mw->x, mw->y, total_width, total_height, FALSE);
		// Параметр-строка
		if(NULL==fgets(lcbuffer,sizeof(lcbuffer),f)) return 1;
		len=static_cast<int>(strlen(lcbuffer));
		if(len>0)
		{
			if('\n'==lcbuffer[len-1]) lcbuffer[len-1]=0;
			if(len>1 && '\r'==lcbuffer[len-2]) lcbuffer[len-2]=0;
		}
		int wideLen = MultiByteToWideChar(CP_UTF8,0,lcbuffer,-1,mw->mw_name,_countof(mw->mw_name));
		if(wideLen == 0) mw->mw_name[0] = 0;
		// Обновляем заголовок окна сразу после загрузки имени
		if(mw->MWhwnd) SetWindowText(mw->MWhwnd, mw->mw_name);
	}
	return 0;
}
//====================================================================================
// Обновить тексты диалога 2 при смене языка
//====================================================================================
void MHSettings::UpdateDialog2Texts(HWND hdwnd)
{
	SetWindowText(hdwnd, L(LOC_DIALOG2_TITLE));
	SetWindowText(GetDlgItem(hdwnd, IDOK), L(LOC_BTN_OK));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_WTEXT), L(LOC_LBL_WINDOW_TEXT));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_SENSOR), L(LOC_LBL_SENSOR));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_COLOR), L(LOC_LBL_COLOR));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_COORDS), L(LOC_LBL_COORDINATES));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_SIZE), L(LOC_LBL_SIZE));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_KEY), L(LOC_LBL_KEY));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_MODE), L(LOC_LBL_MODE));
	SetWindowText(GetDlgItem(hdwnd, IDC_LBL_TRACKER), L(LOC_LBL_EYETRACKER));
	SetWindowText(GetDlgItem(hdwnd, IDC_CHECK_EYECIRCLE), L(LOC_CHK_EYE_CIRCLE));
	// Eyetracker connect button
	SetWindowText(GetDlgItem(hdwnd, IDC_EYETRACKER_CONNECT),
		G_eytracker_is_working ? L(LOC_BTN_STOP_TRACKER) : L(LOC_BTN_START_TRACKER));
}