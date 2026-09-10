#ifndef __ARC_WM_USER
#define __ARC_WM_USER
#define WM_USER_MOVEWINDOW (WM_USER + 102)
#include <Windows.h>
extern HWND MHhwnd;
extern DWORD last_invalidate_time;
#define MH_MIN_INVALIDATE_MS 33
__forceinline void ThrottledInvalidate() {
	DWORD now = timeGetTime();
	if (now - last_invalidate_time >= MH_MIN_INVALIDATE_MS) {
		InvalidateRect(MHhwnd, NULL, FALSE);
		last_invalidate_time = now;
	}
}
#endif