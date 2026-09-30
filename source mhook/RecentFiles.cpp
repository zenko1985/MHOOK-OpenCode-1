#include "RecentFiles.h"
#include "Settings.h"
#include "resource.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <algorithm>
#include <set>
#include <tchar.h>
#include <strsafe.h>
#include <fstream>
#include "miniz.h"
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
	const BYTE* parseData = pData;
	DWORD parseSize = size;
	std::vector<BYTE> decompressed;
	if (size >= 8 && memcmp(pData, ZLIB_MAGIC, 4) == 0) {
		DWORD origSize;
		memcpy(&origSize, pData + 4, sizeof(DWORD));
		decompressed.resize(origSize);
		mz_ulong dstLen = (mz_ulong)origSize;
		int rc = mz_uncompress(decompressed.data(), &dstLen, pData + 8, (mz_ulong)(size - 8));
		if (rc != 0) return;
		parseData = decompressed.data();
		parseSize = (DWORD)dstLen;
	}
	DWORD numFiles;
	memcpy(&numFiles, parseData, sizeof(DWORD));
	const BYTE* ptr = parseData + 4;
	const BYTE* pDataEnd = parseData + parseSize;
	embeddedFiles.clear();
	const DWORD MAX_EMBEDDED_SIZE = 10 * 1024 * 1024; // 10 MB max per entry
	for (DWORD i = 0; i < numFiles; i++) {
		if (ptr + 4 > pDataEnd) break;
		DWORD nameLen;
		memcpy(&nameLen, ptr, sizeof(DWORD));
		ptr += 4;
		if ((size_t)nameLen * 2 > (size_t)(pDataEnd - ptr)) break;
		std::basic_string<TCHAR> name;
		name.resize(nameLen);
		memcpy(&name[0], ptr, nameLen * 2);
		ptr += nameLen * 2;
		if (ptr + 8 > pDataEnd) break;
		ULONGLONG filetime;
		memcpy(&filetime, ptr, sizeof(ULONGLONG));
		ptr += 8;
		if (ptr + 4 > pDataEnd) break;
		DWORD contentLen;
		memcpy(&contentLen, ptr, sizeof(DWORD));
		ptr += 4;
		if (contentLen > MAX_EMBEDDED_SIZE) break;
		if (ptr + contentLen > pDataEnd) break;
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
	if (safeName == _T("..") || safeName.empty()) return false;
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	PathRemoveFileSpec(exePath);
	TCHAR tempFile[MAX_PATH];
	StringCchCopy(tempFile, MAX_PATH, exePath);
	PathAppend(tempFile, safeName.c_str());
	StringCchCat(tempFile, MAX_PATH, _T(".mhook"));
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
				if (displayName == _T("..") || displayName.empty()) return;
				lastLoadedName = displayName;
				TCHAR exePath[MAX_PATH];
				GetModuleFileName(NULL, exePath, MAX_PATH);
				PathRemoveFileSpec(exePath);
				TCHAR tempFile[MAX_PATH];
				StringCchCopy(tempFile, MAX_PATH, exePath);
				PathAppend(tempFile, displayName.c_str());
				StringCchCat(tempFile, MAX_PATH, _T(".mhook"));
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
std::basic_string<TCHAR> RecentFiles::GetFileName(int index) {
	if (index >= 0 && index < (int)files.size()) {
		std::basic_string<TCHAR> name = files[index].filename;
		size_t dotPos = name.find_last_of(_T('.'));
		if (dotPos != std::basic_string<TCHAR>::npos) {
			name = name.substr(0, dotPos);
		}
		return name;
	}
	return _T("");
}
int RecentFiles::FindByPrefix(const TCHAR* prefix) {
	if (!prefix || !prefix[0]) return -1;
	TCHAR prefixUpper[256];
	StringCchCopy(prefixUpper, 256, prefix);
	CharUpperBuff(prefixUpper, static_cast<DWORD>(_tcslen(prefixUpper)));
	for (size_t i = 0; i < files.size(); i++) {
		TCHAR fnameUpper[256];
		StringCchCopy(fnameUpper, 256, files[i].filename.c_str());
		CharUpperBuff(fnameUpper, static_cast<DWORD>(_tcslen(fnameUpper)));
		TCHAR* dotPos = _tcsrchr(fnameUpper, _T('.'));
		if (dotPos) *dotPos = _T('\0');
		if (_tcsncmp(fnameUpper, prefixUpper, _tcslen(prefixUpper)) == 0) {
			return static_cast<int>(i);
		}
	}
	return -1;
}
bool RecentFiles::FindByWindowTitle(HWND hwnd, TCHAR* title) {
	LoadEmbeddedFiles();
	TCHAR titleUpper[256];
	StringCchCopy(titleUpper, 256, title);
	TCHAR titleClean[256];
	int j = 0;
	for (int i = 0; titleUpper[i] && j < 255; i++) {
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
			StringCchCopy(fnameUpper, 256, file.filename.c_str());
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
	StringCchCopy(searchPattern, MAX_PATH, exePath);
	StringCchCat(searchPattern, MAX_PATH, _T("*.MHOOK"));
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchPattern, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			TCHAR fnameUpper[256];
			StringCchCopy(fnameUpper, 256, fd.cFileName);
			TCHAR fnameClean[256];
			int k = 0;
			for (int i = 0; fnameUpper[i] && k < 255; i++) {
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
				StringCchCopy(displayName, 256, fd.cFileName);
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
	StringCchCopy(searchPattern, MAX_PATH, exePath);
	StringCchCat(searchPattern, MAX_PATH, _T("*.mhook"));
	hFind = FindFirstFile(searchPattern, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			TCHAR fnameUpper[256];
			StringCchCopy(fnameUpper, 256, fd.cFileName);
			TCHAR fnameClean[256];
			int k = 0;
			for (int i = 0; fnameUpper[i] && k < 255; i++) {
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
				StringCchCopy(displayName, 256, fd.cFileName);
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
		StringCchCopy(displayName, 256, bestMatch->filename.c_str());
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