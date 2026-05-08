#ifndef __RECENT_FILES_H
#define __RECENT_FILES_H
#include <Windows.h>
#include <vector>
#include <string>
struct RecentFileInfo {
	std::basic_string<TCHAR> filename;
	std::basic_string<TCHAR> fullpath;
	FILETIME lastWriteTime;
	bool isEmbedded;
};
class RecentFiles {
public:
	static void PopulateDialogList(HWND hDlg, int comboId);
	static void OnDialogFileSelected(HWND hDlg, int listboxId, int index);
	static void Shutdown();
private:
	static HWND hListBox;
	static std::vector<RecentFileInfo> files;
	static std::vector<RecentFileInfo> embeddedFiles;
	static bool initialized;
	static void LoadEmbeddedFiles();
	static std::basic_string<TCHAR> GetExecutableDirectory();
};
#endif