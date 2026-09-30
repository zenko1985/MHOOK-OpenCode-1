#ifndef __MH_EMBEDDED_EXE
#define __MH_EMBEDDED_EXE
#include <Windows.h>
// Extract embedded AHK exe to temp dir and return path.
// Returns empty string on failure.
// Caller must free the returned buffer with HeapFree(GetProcessHeap(), 0, path).
TCHAR* EmbeddedExe_Extract(int resourceId);
// Get path to extracted autoclick exe (extracts if needed)
TCHAR* EmbeddedExe_GetAutoClickPath();
// Get path to extracted wheel exe (extracts if needed)
TCHAR* EmbeddedExe_GetWheelPath();
#endif