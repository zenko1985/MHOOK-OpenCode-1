// Курсор в виде точки для отображения поверх мыши
#include <Windows.h>
#include "CursorDot.h"
#include "Settings.h"
HWND CursorDot::DotHwnd = NULL;
bool CursorDot::is_visible = false;
POINT CursorDot::last_mouse_pos = {0, 0};
DWORD CursorDot::last_update_time = 0;
HBRUSH CursorDot::hDotBrush = NULL;
HPEN CursorDot::hDotPen = NULL;
static bool class_registered = false;
extern HINSTANCE MHInst;
// Размер окна (маленький)
#define DOT_SIZE 6
// Цвет точки (красный с прозрачностью)
#define DOT_COLOR RGB(255, 50, 50)
// Прозрачность окна (0-255)
#define DOT_ALPHA 200
int CursorDot::Init()
{
	// Кэшируем GDI-объекты один раз
	if(!hDotBrush) hDotBrush = CreateSolidBrush(DOT_COLOR);
	if(!hDotPen) hDotPen = CreatePen(PS_SOLID, 1, DOT_COLOR);
	if (!class_registered)
	{
		WNDCLASS wc = {0};
		wc.lpfnWndProc = DotWndProc;
		wc.hInstance = MHInst;
		wc.lpszClassName = TEXT("CursorDotClass2");
		wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.hCursor = NULL;
		if (!RegisterClass(&wc))
		{
			if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
				return -1;
		}
		class_registered = true;
	}
	if (DotHwnd == NULL)
	{
		GetCursorPos(&last_mouse_pos);
		DotHwnd = CreateWindowEx(
			WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT,
			TEXT("CursorDotClass2"),
			TEXT("CursorDot"),
			WS_POPUP,
			last_mouse_pos.x - DOT_SIZE/2,
			last_mouse_pos.y - DOT_SIZE/2,
			DOT_SIZE, DOT_SIZE,
			NULL, NULL, MHInst, NULL
		);
		if (DotHwnd == NULL)
		{
			is_visible = false;
			return -1;
		}
		SetLayeredWindowAttributes(DotHwnd, 0, DOT_ALPHA, LWA_ALPHA);
	}
	return 0;
}
void CursorDot::Show()
{
	if (DotHwnd != NULL)
	{
		ShowWindow(DotHwnd, SW_SHOWNA);
		UpdateWindow(DotHwnd);
		is_visible = true;
		InvalidateRect(DotHwnd, NULL, TRUE);
		POINT pt;
		if(GetCursorPos(&pt)) UpdatePosition(pt.x, pt.y);
		return;
	}
	if (Init() == 0)
	{
		ShowWindow(DotHwnd, SW_SHOWNA);
		UpdateWindow(DotHwnd);
		InvalidateRect(DotHwnd, NULL, TRUE);
		is_visible = true;
	}
}
void CursorDot::Hide()
{
	if (DotHwnd != NULL)
	{
		ShowWindow(DotHwnd, SW_HIDE);
		is_visible = false;
	}
}
void CursorDot::UpdatePosition(LONG x, LONG y)
{
	if (!is_visible || DotHwnd == NULL) return;
	// Throttle: не обновлять чаще 16мс (~60fps)
	DWORD now = timeGetTime();
	if (now - last_update_time < 16) return;
	last_update_time = now;
	if (x == last_mouse_pos.x && y == last_mouse_pos.y) return;
	last_mouse_pos.x = x;
	last_mouse_pos.y = y;
	SetWindowPos(
		DotHwnd,
		HWND_TOPMOST,
		x - DOT_SIZE/2,
		y - DOT_SIZE/2,
		0, 0,
		SWP_NOACTIVATE | SWP_NOSIZE
	);
}
LRESULT CALLBACK CursorDot::DotWndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	switch (uMsg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			// Используем кэшированные GDI-объекты
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hDotBrush);
			HPEN oldPen = (HPEN)SelectObject(hdc, hDotPen);
			Ellipse(hdc, 1, 1, DOT_SIZE-1, DOT_SIZE-1);
			SelectObject(hdc, oldBrush);
			SelectObject(hdc, oldPen);
			EndPaint(hwnd, &ps);
			return 0;
		}
		// Все сообщения мыши проходят сквозь окно
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
			// Передаем событие следующему окну
			return DefWindowProc(hwnd, uMsg, wparam, lparam);
		case WM_NCHITTEST:
			// Окно прозрачно для кликов
			return HTTRANSPARENT;
		case WM_DESTROY:
			return 0;
		default:
			return DefWindowProc(hwnd, uMsg, wparam, lparam);
	}
}