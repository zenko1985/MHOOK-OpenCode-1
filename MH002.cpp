#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <tchar.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "Bitmap.h"
#include "Settings.h"
#include "MagicWindow.h"
#include "CursorDot.h"
extern HWND MHhwnd;
bool flag_inside_window=false;
extern HHOOK handle;
extern LONG screen_x, screen_y, screen_x_real, screen_y_real;
extern double screen_scale;
extern int top_position; // Это в HookProc определяет, в каком углу экрана мы задержались.
extern bool flag_left_button_waits;
extern bool flag_right_button_waits;
LONG quad_x=0,quad_y=0; // Координаты квадратика в окне
static TRACKMOUSEEVENT tme={sizeof(TRACKMOUSEEVENT),TME_LEAVE,0,HOVER_DEFAULT};
// для отладки (определены и назначаются в HookProc)
extern LONG debug_x, debug_y;
//====================================================================================
// Оконная процедура
//====================================================================================
LRESULT CALLBACK WndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
		// Пара событий, по которым мы определяем, находится ли мышь над окном
		case WM_MOUSEMOVE:
			if(!flag_inside_window)
			{
				flag_inside_window=true;
				TrackMouseEvent(&tme);
			}
			break;
		case WM_MOUSELEAVE:
			flag_inside_window=false;
			break;
		case WM_CREATE:
			// Инициализируем битмапы
			MHBitmap::Init(hwnd);
			// Инициализируем точку курсора
			CursorDot::Init();
			// Дополняем tme хендлером окна
			tme.hwndTrack=hwnd;
			// Разрешаем drag and drop
			DragAcceptFiles(hwnd, TRUE);
			// Содрано из интернета - так мы делаем окно прозрачным в белых его частях
			//SetLayeredWindowAttributes(hwnd,RGB(255,255,255),NULL,LWA_COLORKEY);
			// Нет, делаем вот так, а то мышь проваливается
			SetLayeredWindowAttributes(hwnd,NULL,255*70/100,LWA_ALPHA);
			// В третьем режиме инициализируем таймер (это делается в HookHandler(3) при необходимости)
			//if(3==MHSettings::mode) SetTimer(hwnd,1,MHSettings::timeout_after_move,NULL);
			// Показываем красную точку курсора если флаг включен
			if(MHSettings::flag_cursor_visible)
				CursorDot::Show();
			break;
		case WM_DESTROY:	// Завершение программы
			// Чистим за собой
			UnhookWindowsHookEx(handle);
			//if((3==MHSettings::mode)||(4==MHSettings::mode)||(1==MHSettings::mode))
			KillTimer(hwnd,1);
			KillTimer(hwnd,2);
			KillTimer(hwnd,3);
			KillTimer(hwnd,4);
			KillTimer(hwnd,5);
			MHKeypad::Reset();
			// Скрываем красную точку перед выходом
			CursorDot::Hide();
			PostQuitMessage(0);
			//DestroyWindow(hwnd);
			break;
		case WM_TIMER:
			switch(wparam)
			{
			case 1: // Таймер нажатых клавиш
				if(MHSettings::hh) MHSettings::hh->OnTimer(); // Это на случай, если меняем режим, а события в очереди остались
				break;
			case 2: // Таймер угла экрана
				KillTimer(hwnd,2); // Первым делом таймер прибить
				switch(top_position)
				{
				case 0:
					// Теперь смена позиция происходит только по выезду мыши из области!
					//top_position=-1;
					if(MHSettings::hh) MHSettings::hh->TopLeftCornerTimer();
					break;
				case 1:
					// Теперь смена позиция происходит только по выезду мыши из области!
					//top_position=-1;
					// Скрываем красную точку перед открытием диалога настроек
					CursorDot::Hide();
					//if(MHSettings::SettingsDialogue(hwnd))
				if(MHSettings::SettingsDialogue(MHhwnd))
				{
					// Чистим за собой - возможно, излишне
					if((3==MHSettings::mode)||(4==MHSettings::mode)||(1==MHSettings::mode)) KillTimer(hwnd,1);
					MHKeypad::Reset();
					UnhookWindowsHookEx(handle);
					PostQuitMessage(0);
				}
				// Показываем или скрываем красную точку курсора после закрытия диалога
				if(MHSettings::flag_cursor_visible)
					CursorDot::Show();
				else
					CursorDot::Hide();
				// Тут будет вывод диалога настроек
				break;
				} // Закрываем switch(top_position)
				//Beep(450,100);
				break;
			case 3:
				// Отпустить клавишу, которую нажала левая кнопка мыши
				KillTimer(hwnd,3);
				MHKeypad::Press4(5, false);
				flag_left_button_waits=false;
				break;
			case 4:
				// Отпустить клавишу, которую нажала правая кнопка мыши
				KillTimer(hwnd,4);
				MHKeypad::Press4(10,false);
				flag_right_button_waits=false;
				break;
		case 5:
			// Таймер волшебных окон, для имитации движения мыши
			MagicWindow::OnTimer5();
			// Обновление позиции красной точки курсора
			if(MHSettings::flag_cursor_visible)
				CursorDot::UpdatePosition();
			break;
		case 6:
			// Таймер автокликера - пульсация нажатия клавиши
			MHKeypad::Press4(5, false); // Отпустить
			MHKeypad::Press4(5, true);  // Нажать
			break;
		}
			break;
		case WM_DISPLAYCHANGE:
			//screen_x=(LONG)((SHORT)LOWORD(lparam));
			//screen_y=(LONG)((SHORT)HIWORD(lparam));
			screen_x=LOWORD(lparam);
			screen_y=HIWORD(lparam);
			// Козлиная система разрешений экрана в windows8.1...
			DEVMODE dm;
			ZeroMemory (&dm, sizeof (dm));
			EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dm);
			screen_x_real=dm.dmPelsWidth;
			screen_y_real=dm.dmPelsHeight;
			screen_scale=((double)screen_x)/dm.dmPelsWidth;
			break;
		case WM_PAINT:
			PAINTSTRUCT ps;
			HDC hdc;
			hdc=BeginPaint(hwnd,&ps);
			// Подсветить нажатые кнопки
			MHBitmap::OnDraw(hdc,MHSettings::GetPosition());
		if(MHSettings::hh) MHSettings::hh->OnDraw(hdc,200);
			//if((MHposition>-2)&&(MHposition<4)) MHBitmap::OnDraw(hdc,4,MHposition);
/*			RECT rect;
			// Квадратик с мышью
			//xpercent/100.0f*xsize
			//ypercent/100.0f*ysize
			rect.left=(LONG)(MH_WINDOW_SIZE/2+quad_x-10);
			rect.top=(LONG)(MH_WINDOW_SIZE/2+quad_y-10);
			rect.right=rect.left+20;
			rect.bottom=rect.top+20;
			FillRect(hdc,&rect,(HBRUSH)GetStockObject(GRAY_BRUSH));
			*/
			EndPaint(hwnd,&ps);
			break;
		case WM_DROPFILES: {
			HDROP hDrop = (HDROP)wparam;
			TCHAR filename[MAX_PATH];
			if (DragQueryFile(hDrop, 0, filename, MAX_PATH)) {
				TCHAR* ext = PathFindExtension(filename);
				bool isMhook = false;
				if (ext && _tcsicmp(ext, _T(".MHOOK")) == 0) {
					isMhook = true;
				} else if (ext && _tcsicmp(ext, _T(".MHOO")) == 0) {
					_tcscpy(ext, _T(".MHOOK"));
					isMhook = true;
				}
				if (isMhook) {
					MHSettings::OpenMHookConfig(hwnd, filename);
				} else {
					HWND targetWnd = GetForegroundWindow();
					if (targetWnd && targetWnd != hwnd) {
						TCHAR windowTitle[256];
						GetWindowText(targetWnd, windowTitle, 256);
						if (windowTitle[0]) {
							TCHAR mhookPath[MAX_PATH];
							GetModuleFileName(NULL, mhookPath, MAX_PATH);
							PathRemoveFileSpec(mhookPath);
							PathAppend(mhookPath, windowTitle);
							_tcscat(mhookPath, _T(".MHOOK"));
							if (GetFileAttributes(mhookPath) != INVALID_FILE_ATTRIBUTES) {
								MHSettings::OpenMHookConfig(hwnd, mhookPath);
							}
						}
					}
				}
			}
			DragFinish(hDrop);
			return 0;
		}
		default: // Сообщения обрабатываются системой
			return DefWindowProc(hwnd,message,wparam,lparam);
	}
return 0; // сами обработали
}