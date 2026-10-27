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
// Запустить скрипт и запомнить его процесс, чтобы потом завершить именно его.
// scriptPath - путь к .ahk, exeDir - рабочий каталог для запуска.
// Возвращает true, если скрипт запущен (или уже был запущен нами).
bool EmbeddedExe_LaunchScript(int resourceId, TCHAR *scriptPath, TCHAR *exeDir);
// Завершить ранее запущенный нами скрипт
void EmbeddedExe_StopScript(int resourceId);
// Завершить все запущенные нами скрипты
void EmbeddedExe_StopAllScripts();
// Запущен ли сейчас скрипт
bool EmbeddedExe_IsScriptRunning(int resourceId);
#endif