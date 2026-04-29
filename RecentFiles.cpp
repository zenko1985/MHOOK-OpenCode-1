#include "RecentFiles.h"
#include "Settings.h"
#include "resource.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <algorithm>
#include <set>
#include <tchar.h>
#include <fstream>
HWND RecentFiles::hListBox = NULL;
std::vector<RecentFileInfo> RecentFiles::files;
std::vector<RecentFileInfo> RecentFiles::embeddedFiles;
bool RecentFiles::initialized = false;
std::basic_string<TCHAR> RecentFiles::lastLoadedName;
void RecentFiles::LoadEmbeddedFiles() {
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_EMBEDDEDSETTINGS), RT_RCDATA);
	if (!hRes) return;
	HGLOBAL hData = LoadResource(NULL, hRes);
	if (!hData) return;
	DWORD size = SizeofResource(NULL, hRes);
	const BYTE* pData = (const BYTE*)LockResource(hData);
	if (!pData || size < 4) return;
	DWORD numFiles = *(DWORD*)pData;
	const BYTE* ptr = pData + 4;
	embeddedFiles.clear();
	for (DWORD i = 0; i < numFiles; i++) {
		if (ptr + 4 > pData + size) break;
		DWORD nameLen = *(DWORD*)ptr;
		ptr += 4;
		if (ptr + nameLen * 2 > pData + size) break;
		std::basic_string<TCHAR> name;
		name.resize(nameLen);
		memcpy(&name[0], ptr, nameLen * 2);
		ptr += nameLen * 2;
		if (ptr + 8 > pData + size) break;
		ULONGLONG filetime = *(ULONGLONG*)ptr;
		ptr += 8;
		if (ptr + 4 > pData + size) break;
		DWORD contentLen = *(DWORD*)ptr;
		ptr += 4;
		if (ptr + contentLen > pData + size) break;
		RecentFileInfo info;
		info.filename = name;
		info.fullpath = name;
		info.lastWriteTime.dwHighDateTime = (DWORD)(filetime >> 32);
		info.lastWriteTime.dwLowDateTime = (DWORD)(filetime & 0xFFFFFFFF);
		info.isEmbedded = true;
		info.content.resize(contentLen);
		memcpy(info.content.data(), ptr, contentLen);
		ptr += contentLen;
		embeddedFiles.push_back(info);
	}
	initialized = true;
}
std::basic_string<TCHAR> RecentFiles::GetExecutableDirectory() {
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
	PathAddBackslash(path);
	std::basic_string<TCHAR> result(path);
	return result;
}
bool RecentFiles::LoadEmbeddedConfig(HWND hwnd, const RecentFileInfo& file) {
	if (file.content.empty()) return false;
	std::basic_string<TCHAR> displayName = file.filename;
	size_t dotPos = displayName.find_last_of(_T('.'));
	if (dotPos != std::basic_string<TCHAR>::npos) {
		displayName = displayName.substr(0, dotPos);
	}
	std::basic_string<TCHAR> safeName;
	for (size_t i = 0; i < displayName.length(); i++) {
		TCHAR c = displayName[i];
		if (c == _T('\\') || c == _T('/') || c == _T(':') || c == _T('*') ||
			c == _T('?') || c == _T('"') || c == _T('<') || c == _T('>') || c == _T('|')) {
			safeName += _T('_');
		} else {
			safeName += c;
		}
	}
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	PathRemoveFileSpec(exePath);
	TCHAR tempFile[MAX_PATH];
	wcscpy_s(tempFile, exePath);
	PathAppend(tempFile, safeName.c_str());
	wcscat_s(tempFile, _T(".mhook"));
	std::ofstream fout;
	fout.open(tempFile, std::ios::binary);
	if (!fout.is_open()) return false;
	fout.write(file.content.data(), file.content.size());
	fout.close();
	lastLoadedName = displayName;
	MHSettings::OpenMHookConfig(hwnd, tempFile);
	SendDlgItemMessage(hwnd, IDC_EDIT1, WM_SETTEXT, 0, (LPARAM)displayName.c_str());
	MHSettings::AfterLoad(hwnd);
	DeleteFile(tempFile);
	return true;
}
void RecentFiles::PopulateDialogList(HWND hDlg, int comboId) {
	LoadEmbeddedFiles();
	files.clear();
	std::basic_string<TCHAR> dir = GetExecutableDirectory();
	std::vector<RecentFileInfo> allFiles;
	std::set<std::basic_string<TCHAR>> addedNames;
	std::basic_string<TCHAR> searchPattern1 = dir + _T("*.mhook");
	std::basic_string<TCHAR> searchPattern2 = dir + _T("*.MHOOK");
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchPattern1.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			std::basic_string<TCHAR> fname = fd.cFileName;
			if (addedNames.find(fname) == addedNames.end()) {
				addedNames.insert(fname);
				RecentFileInfo info;
				info.filename = fname;
				info.fullpath = dir + fname;
				info.lastWriteTime = fd.ftLastWriteTime;
				info.isEmbedded = false;
				allFiles.push_back(info);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	hFind = FindFirstFile(searchPattern2.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			std::basic_string<TCHAR> fname = fd.cFileName;
			if (addedNames.find(fname) == addedNames.end()) {
				addedNames.insert(fname);
				RecentFileInfo info;
				info.filename = fname;
				info.fullpath = dir + fname;
				info.lastWriteTime = fd.ftLastWriteTime;
				info.isEmbedded = false;
				allFiles.push_back(info);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	allFiles.insert(allFiles.end(), embeddedFiles.begin(), embeddedFiles.end());
	std::vector<RecentFileInfo> diskFiles;
	std::vector<RecentFileInfo> embFiles;
	for (auto& f : allFiles) {
		if (!f.isEmbedded) diskFiles.push_back(f);
		else embFiles.push_back(f);
	}
	// Фильтруем встроенные файлы - убираем если есть на диске
	std::set<std::basic_string<TCHAR>> diskNames;
	for (auto& f : diskFiles) {
		std::basic_string<TCHAR> nameLower = f.filename;
		for (size_t i = 0; i < nameLower.length(); i++) {
			if (nameLower[i] >= _T('A') && nameLower[i] <= _T('Z')) {
				nameLower[i] = nameLower[i] - _T('A') + _T('a');
			}
		}
		diskNames.insert(nameLower);
	}
	std::vector<RecentFileInfo> filteredEmbFiles;
	for (auto& f : embFiles) {
		std::basic_string<TCHAR> nameLower = f.filename;
		for (size_t i = 0; i < nameLower.length(); i++) {
			if (nameLower[i] >= _T('A') && nameLower[i] <= _T('Z')) {
				nameLower[i] = nameLower[i] - _T('A') + _T('a');
			}
		}
		if (diskNames.find(nameLower) == diskNames.end()) {
			filteredEmbFiles.push_back(f);
		}
	}
	embFiles = filteredEmbFiles;
	std::sort(diskFiles.begin(), diskFiles.end(),
		[](const RecentFileInfo& a, const RecentFileInfo& b) {
			return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) > 0;
		});
	std::sort(embFiles.begin(), embFiles.begin() + min((size_t)50, embFiles.size()),
		[](const RecentFileInfo& a, const RecentFileInfo& b) {
			return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) > 0;
		});
	std::sort(embFiles.begin() + min((size_t)50, embFiles.size()), embFiles.end(),
		[](const RecentFileInfo& a, const RecentFileInfo& b) {
			return a.filename < b.filename;
		});
	files.clear();
	files.insert(files.end(), diskFiles.begin(), diskFiles.end());
	files.insert(files.end(), embFiles.begin(), embFiles.end());
	HWND hCombo = GetDlgItem(hDlg, comboId);
	if (!hCombo) return;
	SendMessage(hCombo, WM_SETREDRAW, FALSE, 0);
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	size_t count = files.size();
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
		if (file.isEmbedded) {
			LoadEmbeddedConfig(hDlg, file);
		} else {
			if (GetFileAttributes(file.fullpath.c_str()) != INVALID_FILE_ATTRIBUTES) {
				std::basic_string<TCHAR> displayName = file.filename;
				size_t dotPos = displayName.find_last_of(_T('.'));
				if (dotPos != std::basic_string<TCHAR>::npos) {
					displayName = displayName.substr(0, dotPos);
				}
				lastLoadedName = displayName;
				TCHAR exePath[MAX_PATH];
				GetModuleFileName(NULL, exePath, MAX_PATH);
				PathRemoveFileSpec(exePath);
				TCHAR tempFile[MAX_PATH];
				wcscpy_s(tempFile, exePath);
				PathAppend(tempFile, displayName.c_str());
				wcscat_s(tempFile, _T(".mhook"));
				if (CopyFile(file.fullpath.c_str(), tempFile, FALSE)) {
					MHSettings::OpenMHookConfig(hDlg, tempFile);
					SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, 0, (LPARAM)displayName.c_str());
					MHSettings::AfterLoad(hDlg);
					DeleteFile(tempFile);
				} else {
					MHSettings::OpenMHookConfig(hDlg, (TCHAR*)file.fullpath.c_str());
					MHSettings::AfterLoad(hDlg);
				}
			}
		}
	}
}
bool RecentFiles::FindByWindowTitle(HWND hwnd, TCHAR* title) {
	LoadEmbeddedFiles();
	TCHAR titleUpper[256];
	wcscpy_s(titleUpper, title);
	TCHAR titleClean[256];
	int j = 0;
	for (int i = 0; titleUpper[i]; i++) {
		if (titleUpper[i] != _T(' ') && titleUpper[i] != _T('-') && titleUpper[i] != _T('_') &&
			titleUpper[i] != _T('(') && titleUpper[i] != _T(')') && titleUpper[i] != _T('[') && titleUpper[i] != _T(']')) {
			titleClean[j++] = titleUpper[i];
		}
	}
	titleClean[j] = _T('\0');
	CharUpperBuff(titleClean, static_cast<DWORD>(_tcslen(titleClean)));
	int bestScore = 0;
	const RecentFileInfo* bestMatch = nullptr;
	for (const auto& file : embeddedFiles) {
		TCHAR fnameUpper[256];
		wcscpy_s(fnameUpper, file.filename.c_str());
		CharUpperBuff(fnameUpper, static_cast<DWORD>(_tcslen(fnameUpper)));
		int score = 0;
		const TCHAR* p = _tcsstr(fnameUpper, titleClean);
		if (p) {
			score = static_cast<int>(_tcslen(titleClean));
		}
		if (score > bestScore) {
			bestScore = score;
			bestMatch = &file;
		}
	}
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	PathRemoveFileSpec(exePath);
	PathAddBackslash(exePath);
	TCHAR searchPattern[MAX_PATH];
	wcscpy_s(searchPattern, exePath);
	wcscat_s(searchPattern, _T("*.MHOOK"));
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchPattern, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			TCHAR fnameUpper[256];
			wcscpy_s(fnameUpper, fd.cFileName);
			TCHAR fnameClean[256];
			int k = 0;
			for (int i = 0; fnameUpper[i]; i++) {
				if (fnameUpper[i] != _T(' ') && fnameUpper[i] != _T('-') && fnameUpper[i] != _T('_') &&
					fnameUpper[i] != _T('(') && fnameUpper[i] != _T(')') && fnameUpper[i] != _T('[') && fnameUpper[i] != _T(']')) {
					fnameClean[k++] = fnameUpper[i];
				}
			}
			fnameClean[k] = _T('\0');
			CharUpperBuff(fnameClean, static_cast<DWORD>(_tcslen(fnameClean)));
			int score = 0;
			const TCHAR* p = _tcsstr(fnameClean, titleClean);
			if (p) {
				score = static_cast<int>(_tcslen(titleClean));
			}
			if (score > bestScore) {
				bestScore = score;
				bestMatch = nullptr;
				RecentFileInfo info;
				info.filename = fd.cFileName;
				info.fullpath = std::basic_string<TCHAR>(exePath) + fd.cFileName;
				info.lastWriteTime = fd.ftLastWriteTime;
				info.isEmbedded = false;
				files.push_back(info);
				TCHAR displayName[256];
				wcscpy_s(displayName, fd.cFileName);
				TCHAR* dotPos = _tcsrchr(displayName, _T('.'));
				if (dotPos) *dotPos = _T('\0');
				lastLoadedName = displayName;
				OnDialogFileSelected(hwnd, 0, (int)files.size() - 1);
				FindClose(hFind);
				return true;
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	wcscpy_s(searchPattern, exePath);
	wcscat_s(searchPattern, _T("*.mhook"));
	hFind = FindFirstFile(searchPattern, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			TCHAR fnameUpper[256];
			wcscpy_s(fnameUpper, fd.cFileName);
			TCHAR fnameClean[256];
			int k = 0;
			for (int i = 0; fnameUpper[i]; i++) {
				if (fnameUpper[i] != _T(' ') && fnameUpper[i] != _T('-') && fnameUpper[i] != _T('_') &&
					fnameUpper[i] != _T('(') && fnameUpper[i] != _T(')') && fnameUpper[i] != _T('[') && fnameUpper[i] != _T(']')) {
					fnameClean[k++] = fnameUpper[i];
				}
			}
			fnameClean[k] = _T('\0');
			CharUpperBuff(fnameClean, static_cast<DWORD>(_tcslen(fnameClean)));
			int score = 0;
			const TCHAR* p = _tcsstr(fnameClean, titleClean);
			if (p) {
				score = static_cast<int>(_tcslen(titleClean));
			}
			if (score > bestScore) {
				bestScore = score;
				bestMatch = nullptr;
				RecentFileInfo info;
				info.filename = fd.cFileName;
				info.fullpath = std::basic_string<TCHAR>(exePath) + fd.cFileName;
				info.lastWriteTime = fd.ftLastWriteTime;
				info.isEmbedded = false;
				files.push_back(info);
				TCHAR displayName[256];
				wcscpy_s(displayName, fd.cFileName);
				TCHAR* dotPos = _tcsrchr(displayName, _T('.'));
				if (dotPos) *dotPos = _T('\0');
				lastLoadedName = displayName;
				OnDialogFileSelected(hwnd, 0, (int)files.size() - 1);
				FindClose(hFind);
				return true;
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	if (bestMatch) {
		LoadEmbeddedConfig(hwnd, *bestMatch);
		TCHAR displayName[256];
		wcscpy_s(displayName, bestMatch->filename.c_str());
		TCHAR* dotPos = _tcsrchr(displayName, _T('.'));
		if (dotPos) *dotPos = _T('\0');
		lastLoadedName = displayName;
		return true;
	}
	return false;
}
std::basic_string<TCHAR> RecentFiles::GetLastLoadedName() {
	return lastLoadedName;
}
void RecentFiles::SetLastLoadedName(const std::basic_string<TCHAR>& name) {
	lastLoadedName = name;
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