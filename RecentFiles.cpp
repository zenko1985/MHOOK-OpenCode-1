#include "RecentFiles.h"
#include "Settings.h"
#include "resource.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <algorithm>
#include <tchar.h>
HWND RecentFiles::hListBox = NULL;
std::vector<RecentFileInfo> RecentFiles::files;
std::vector<RecentFileInfo> RecentFiles::embeddedFiles;
bool RecentFiles::initialized = false;
void RecentFiles::LoadEmbeddedFiles() {
	std::basic_string<TCHAR> dir = GetExecutableDirectory();
	std::basic_string<TCHAR> searchPattern = dir + _T("*.mhook");
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchPattern.c_str(), &fd);
	embeddedFiles.clear();
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			std::basic_string<TCHAR> fname = fd.cFileName;
			std::basic_string<TCHAR> fullPath = dir + fname;
			RecentFileInfo info;
			info.filename = fname;
			info.fullpath = fullPath;
			info.lastWriteTime = fd.ftLastWriteTime;
			info.isEmbedded = false;
			embeddedFiles.push_back(info);
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	initialized = true;
}
std::basic_string<TCHAR> RecentFiles::GetExecutableDirectory() {
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
	PathAddBackslash(path);
	return std::basic_string<TCHAR>(path);
}
void RecentFiles::PopulateDialogList(HWND hDlg, int comboId) {
	LoadEmbeddedFiles();
	files.clear();
	files = embeddedFiles;
	std::sort(files.begin(), files.end(),
		[](const RecentFileInfo& a, const RecentFileInfo& b) {
			return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) > 0;
		});
	HWND hCombo = GetDlgItem(hDlg, comboId);
	if (!hCombo) return;
	SendMessage(hCombo, WM_SETREDRAW, FALSE, 0);
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	size_t count = files.size() > 50 ? 50 : files.size();
	SendMessage(hCombo, CB_INITSTORAGE, count, count * 64);
	for (size_t i = 0; i < count; i++) {
		std::basic_string<TCHAR> displayName = files[i].filename;
		size_t dotPos = displayName.find_last_of(_T('.'));
		if (dotPos != std::basic_string<TCHAR>::npos) {
			displayName = displayName.substr(0, dotPos);
		}
		LRESULT idx = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)displayName.c_str());
		SendMessage(hCombo, CB_SETITEMDATA, idx, i);
	}
	SendMessage(hCombo, WM_SETREDRAW, TRUE, 0);
}
void RecentFiles::OnDialogFileSelected(HWND hDlg, int comboId, int index) {
	if (index >= 0 && index < (int)files.size()) {
		const RecentFileInfo& file = files[index];
		std::basic_string<TCHAR> fullPath = file.fullpath;
		if (GetFileAttributes(fullPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
			MHSettings::OpenMHookConfig(hDlg, (TCHAR*)fullPath.c_str());
			MHSettings::AfterLoad(hDlg);
		}
	}
}
void RecentFiles::Shutdown() {
	if (hListBox) {
		DestroyWindow(hListBox);
		hListBox = NULL;
	}
	files.clear();
	embeddedFiles.clear();
	initialized = false;
}