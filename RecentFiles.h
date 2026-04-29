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
	std::vector<char> content;
};
class RecentFiles {
public:
	static void PopulateDialogList(HWND hDlg, int comboId);
	static void OnDialogFileSelected(HWND hDlg, int listboxId, int index);
	static void Shutdown();
	static bool FindByWindowTitle(HWND hwnd, TCHAR* title);
	static std::basic_string<TCHAR> GetLastLoadedName();
	static void SetLastLoadedName(const std::basic_string<TCHAR>& name);
private:
	static HWND hListBox;
	static std::vector<RecentFileInfo> files;
	static std::vector<RecentFileInfo> embeddedFiles;
	static bool initialized;
	static std::basic_string<TCHAR> lastLoadedName;
	static void LoadEmbeddedFiles();
	static bool LoadEmbeddedConfig(HWND hwnd, const RecentFileInfo& file);
	static std::basic_string<TCHAR> GetExecutableDirectory();
};
#endif