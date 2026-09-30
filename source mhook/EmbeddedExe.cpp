#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "resource.h"
#include "EmbeddedExe.h"
static TCHAR g_autoclickPath[MAX_PATH] = {0};
static TCHAR g_wheelPath[MAX_PATH] = {0};
TCHAR* EmbeddedExe_Extract(int resourceId)
{
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
	if(!hRes) return NULL;
	HGLOBAL hData = LoadResource(NULL, hRes);
	if(!hData) return NULL;
	void* pData = LockResource(hData);
	DWORD dataSize = SizeofResource(NULL, hRes);
	if(!pData || dataSize == 0) return NULL;
	// Determine output filename based on resource ID
	const TCHAR* filename = NULL;
	if(resourceId == IDR_AHK_AUTOCLICK) filename = _T("mhook_ac.ahk");
	else if(resourceId == IDR_AHK_WHEEL) filename = _T("mhook_wh.ahk");
	else return NULL;
	// Build path in temp directory
	TCHAR tempPath[MAX_PATH];
	GetTempPath(MAX_PATH, tempPath);
	TCHAR fullPath[MAX_PATH];
	StringCchCopy(fullPath, MAX_PATH, tempPath);
	PathAppend(fullPath, filename);
	// Check if already extracted and same size
	HANDLE hFile = CreateFile(fullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE) {
		DWORD existingSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		if(existingSize == dataSize) {
			// Already extracted, return path
			TCHAR* result = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, sizeof(fullPath));
			if(result) StringCchCopy(result, MAX_PATH, fullPath);
			return result;
		}
	}
	// Write to file
	hFile = CreateFile(fullPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) return NULL;
	DWORD written = 0;
	WriteFile(hFile, pData, dataSize, &written, NULL);
	CloseHandle(hFile);
	if(written != dataSize) {
		DeleteFile(fullPath);
		return NULL;
	}
	TCHAR* result = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, sizeof(fullPath));
	if(result) StringCchCopy(result, MAX_PATH, fullPath);
	return result;
}
TCHAR* EmbeddedExe_GetAutoClickPath()
{
	if(g_autoclickPath[0] == 0) {
		TCHAR* path = EmbeddedExe_Extract(IDR_AHK_AUTOCLICK);
		if(path) {
			StringCchCopy(g_autoclickPath, MAX_PATH, path);
			HeapFree(GetProcessHeap(), 0, path);
		}
	}
	return g_autoclickPath[0] ? g_autoclickPath : NULL;
}
TCHAR* EmbeddedExe_GetWheelPath()
{
	if(g_wheelPath[0] == 0) {
		TCHAR* path = EmbeddedExe_Extract(IDR_AHK_WHEEL);
		if(path) {
			StringCchCopy(g_wheelPath, MAX_PATH, path);
			HeapFree(GetProcessHeap(), 0, path);
		}
	}
	return g_wheelPath[0] ? g_wheelPath : NULL;
}