#include <Windows.h>
//#include <stdio.h>
#include "GDIHelpers.h"
#include "Bitmap.h"
//#include "MVector.h"
#include "Settings.h"
#include "MHRepErr.h"
#include "MagicWindow.h"
#include "CircleWindow.h"
#include "TET.h"
#include "TobiiREX.h"
#include "Localization.h"
#define MH_WINDOW_SIZE 200
// Watchdog для WH_MOUSE_LL: Windows 11 24H2 может тихо снять хук при превышении таймаута
// Проверяем каждые 5 секунд и перевстанавливаем при необходимости
#define MH_HOOK_WATCHDOG_TIMER 9
#define MH_HOOK_WATCHDOG_INTERVAL 5000
//char debug_buf[4096];
// Глобальные переменные, которые могут потребоваться везде
TCHAR*		MHAppName=L"MHook V2";
HINSTANCE	MHInst;
HWND		MHhwnd=NULL;
GdiBrush g_green_brush(CreateSolidBrush(RGB(100,255,100)));
GdiBrush g_yellow_brush(CreateSolidBrush(RGB(227,198,2)));
GdiBrush g_red_brush(CreateSolidBrush(RGB(234,36,36)));
GdiBrush g_blue_brush(CreateSolidBrush(RGB(36,36,234)));
HBRUSH green_brush = g_green_brush.get(), yellow_brush = g_yellow_brush.get(),
       red_brush = g_red_brush.get(), blue_brush = g_blue_brush.get();
HBRUSH brushes[4] = {green_brush, yellow_brush, red_brush, blue_brush};
GdiPen g_green_pen(CreatePen(PS_SOLID,4,RGB(100,255,100)));
HPEN green_pen = g_green_pen.get();
GdiFont g_hfont(CreateFont( -32, 0, 0, 0, FW_BOLD, 0, 0, 0,
    RUSSIAN_CHARSET,
    0, 0, 0, 0, L"Arial"));
HFONT hfont = g_hfont.get();
short xsize,ysize; // Размер окна
LONG screen_x, screen_y, screen_x_real, screen_y_real;
double screen_scale=1.0;
HHOOK handle;
// Оконная процедура определена в MH002.cpp
LRESULT CALLBACK WndProc(HWND hwnd,	UINT message, WPARAM wparam, LPARAM lparam);
// А эта - в HookProc
LRESULT  CALLBACK HookProc(int disabled,WPARAM wParam,LPARAM lParam);
extern bool G_eytracker_is_working;
extern int G_eytracker_num; // какой из трекеров выбран
extern bool timer5_needed; // Из MagicWindow
//=======================================================================
// программа
//=======================================================================
int WINAPI WinMain(HINSTANCE hInst,HINSTANCE,LPSTR cline,INT)
// Командную строку не обрабатываем
{
	ATOM aresult; // Для всяких кодов возврата
	BOOL boolresult;
	MSG msg; // Сообщение
	TCHAR *MHWindowCName=L"MHook20";
	RECT rect={0,0,MH_WINDOW_SIZE,MH_WINDOW_SIZE};
	// Hardened DLL search order to prevent DLL hijacking (Windows 11 24H2)
	typedef BOOL (WINAPI* SetDefaultDllDirectoriesFunc)(DWORD);
	HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
	if(hKernel32) {
		SetDefaultDllDirectoriesFunc pSetDefaultDllDirs = (SetDefaultDllDirectoriesFunc)GetProcAddress(hKernel32, "SetDefaultDllDirectories");
		if(pSetDefaultDllDirs) {
			pSetDefaultDllDirs(LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
		}
	}
	// DPI Awareness: пробуем современный API (Win10 1607+), fallback на legacy
	// SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) — Win10 1703+
	typedef BOOL (WINAPI* SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
	SetProcessDpiAwarenessContextFunc pSetDpiCtx =
		(SetProcessDpiAwarenessContextFunc)GetProcAddress(hKernel32, "SetProcessDpiAwarenessContext");
	if(pSetDpiCtx) {
		// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = -4
		// Включает PerMonitorV2 +自動 scaling للôkна с мигрированными children
		if(!pSetDpiCtx((DPI_AWARENESS_CONTEXT)-4)) {
			// Если V2 не поддерживается — пробуем PerMonitor (V1)
			pSetDpiCtx((DPI_AWARENESS_CONTEXT)-3);
		}
	} else {
		// Fallback: legacy API для Win8.1/10 до 1607
		typedef HRESULT(WINAPI* SetProcessDpiAwarenessFunc)(int);
		HMODULE hShcore = LoadLibraryW(L"Shcore.dll");
		if (hShcore) {
			SetProcessDpiAwarenessFunc pSetProcessDpiAwareness =
				(SetProcessDpiAwarenessFunc)GetProcAddress(hShcore, "SetProcessDpiAwareness");
			if (pSetProcessDpiAwareness) {
				pSetProcessDpiAwareness(2); // PROCESS_PER_MONITOR_DPI_AWARE
			}
			FreeLibrary(hShcore);
		}
	}
	// Делаем hInst доступной для всех
	MHInst=hInst;
	// Найдём размер экрана
	screen_x=GetSystemMetrics(SM_CXSCREEN);
	screen_y=GetSystemMetrics(SM_CYSCREEN);
	// Козлиная система разрешений экрана в windows8.1...
	DEVMODE dm;
	ZeroMemory (&dm, sizeof (dm));
	EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dm);
	screen_x_real=dm.dmPelsWidth;
	screen_y_real=dm.dmPelsHeight;
	screen_scale = dm.dmPelsWidth > 0 ? ((double)screen_x)/dm.dmPelsWidth : 1.0;
	// Создаём окно с кружком
	CircleWindow::Init();
	// С самого начала пытаемся загрузить конфигурацию по умолчанию
	MHSettings::OpenMHookConfig(NULL,L"default.MHOOK");
	// Синхронизируем язык
	MHLanguage = MHSettings::language;
	// Note: AfterLoad is called from SettingsDialogue -> WM_INITDIALOG
	// Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInst,
                          //LoadIcon( hInst, MAKEINTRESOURCE(IDI_ICON1)),
						  LoadIcon( NULL, IDI_APPLICATION),
                          LoadCursor(NULL, IDC_ARROW),
                          //(HBRUSH)GetStockObject(WHITE_BRUSH),
						  NULL,
						  NULL,
						  //MAKEINTRESOURCE(IDR_MENU1),
						  MHWindowCName };
	aresult=RegisterClass(&wcl);
	if (aresult==0)
	{
		MHReportError(__WIDEFILE__,L"RegisterClass",__LINE__);
		return (1);
	}
	// Какой нам нужен размер окна для клиентской области 200x200?
	//AdjustWindowRect(&rect,	WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, false);
	AdjustWindowRect(&rect,	WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, false);
	//Создание главного окна
	MHhwnd=CreateWindowEx(WS_EX_LAYERED,
	//MHhwnd=CreateWindowEx(WS_EX_LAYERED|WS_EX_TOPMOST,
	//MHhwnd=CreateWindow(
		MHWindowCName,
		MHAppName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		//WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		//CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right-rect.left, rect.bottom-rect.top,
		0L, 0L,
		hInst, 0L );
	if (MHhwnd==NULL)
	{
		//BKBReportError(__FILE__,"CreateWindow",__LINE__);
		return (1);
	}
	// Показываем окно (перед диалогом настроек свёрнуто)
    ShowWindow(MHhwnd, SW_MINIMIZE );
	boolresult=UpdateWindow( MHhwnd );
	if(boolresult==0)
	{
		MHReportError(__WIDEFILE__,L"UpdateWindow",__LINE__);
		return (1);
	}
	// Для волшебных окон (и диалога настроек) нужно окно MHhwnd
	// Создаём волшебные окна (пока скрытые)
	MagicWindow::Init();
	// Диалог настроек
	if(MHSettings::SettingsDialogue(MHhwnd)) return -1;
	// Разворачиваем окно mhook
	ShowWindow( MHhwnd, SW_SHOWNORMAL );
	// Инициализируем работу хука (LL хук для Windows 10/11/24H2)
	handle = SetWindowsHookExW(WH_MOUSE_LL,
									HookProc,
                                  GetModuleHandle(NULL),
                                  NULL);
	// Запускаем watchdog для автовосстановления хука (Windows 11 24H2 может тихо снять его)
	SetTimer(MHhwnd, MH_HOOK_WATCHDOG_TIMER, MH_HOOK_WATCHDOG_INTERVAL, NULL);
	//Цикл обработки сообщений
	while(GetMessage(&msg,NULL,0,0))
    {
		TranslateMessage( &msg );
        DispatchMessage( &msg );
	}// while !WM_QUIT
	// Чистим за собой (на всякий случай ещё раз, если выходим не по WM_CLOSE)
	UnhookWindowsHookEx(handle);
	// Выключаем айтрекер
	if(G_eytracker_is_working) // выключаем
	{
		// здесь будет выключение
		if(0==G_eytracker_num) BKBTobiiREX::Halt(NULL);
		else BKBTET::Halt(NULL);
	}
	// битмапы
	MHBitmap::Halt();
	// Отпускаем нажатые кнопки волшебных окон
	MagicWindow::Hide();
	return 0;
}