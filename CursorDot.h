// Курсор в виде точки для отображения поверх мыши
#pragma once
#include <Windows.h>
class CursorDot
{
public:
	static int Init();
	static void Show();
	static void Hide();
	static void UpdatePosition();
	static bool IsVisible() { return is_visible; }
protected:
	static HWND DotHwnd;
	static bool is_visible;
	static POINT last_mouse_pos;
	static DWORD last_update_time;
	static LRESULT CALLBACK DotWndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);
};