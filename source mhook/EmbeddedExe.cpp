#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#include "resource.h"
#include "EmbeddedExe.h"
static TCHAR g_autoclickPath[MAX_PATH] = {0};
static TCHAR g_wheelPath[MAX_PATH] = {0};
// Слоты под запущенные нами AHK-скрипты
#define MH_NUM_SCRIPT_SLOTS 2
// Максимум потомков, которых мы готовы погасить у одного скрипта
#define MH_MAX_SCRIPT_CHILDREN 64
// Сколько ждём завершения корневого процесса. Меньше LowLevelHooksTimeout (1000 мс),
// чтобы оконная процедура не задерживала хук слишком долго
#define MH_AHK_STOP_WAIT_MS 300
struct MHScriptSlot {
	int resId;
	DWORD rootPid;	// Процесс, вернувшийся из ShellExecuteEx (лаунчер или сам интерпретатор)
	FILETIME rootCreated;	// Время создания процесса - защита от переиспользования PID
	HANDLE hProc;	// Хендл на rootPid с правами на завершение
};
static MHScriptSlot g_scripts[MH_NUM_SCRIPT_SLOTS] = {
	{ IDR_AHK_AUTOCLICK, 0, {0,0}, NULL },
	{ IDR_AHK_WHEEL, 0, {0,0}, NULL }
};
static MHScriptSlot* FindSlot(int resourceId)
{
	for(int i=0;i<MH_NUM_SCRIPT_SLOTS;i++)
		if(g_scripts[i].resId==resourceId) return &g_scripts[i];
	return NULL;
}
// Тот ли это ещё процесс? Windows переиспользует PID после завершения,
// и под старым номером может оказаться совершенно чужой процесс.
static bool IsSameProcess(HANDLE hProc, const FILETIME *created)
{
	FILETIME c, e, k, u;
	if(!GetProcessTimes(hProc, &c, &e, &k, &u)) return false;
	return c.dwHighDateTime==created->dwHighDateTime && c.dwLowDateTime==created->dwLowDateTime;
}
static void ReleaseSlot(MHScriptSlot *slot)
{
	if(slot->hProc) { CloseHandle(slot->hProc); slot->hProc=NULL; }
	slot->rootPid=0;
	slot->rootCreated.dwLowDateTime=0;
	slot->rootCreated.dwHighDateTime=0;
}
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
//====================================================================================
// Запуск и остановка скриптов
//====================================================================================
// Собрать всех потомков процесса rootPid (включая внуков)
// Возвращает количество найденных PID, они записываются в out в порядке
// обхода в ширину: сначала ближние потомки, потом более дальние
static int CollectDescendants(DWORD rootPid, DWORD *out, int maxCount)
{
	if(!rootPid || maxCount<=0) return 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnap == INVALID_HANDLE_VALUE) return 0;
	// Таблица "процесс -> родитель" одним проходом
	struct MHParentEntry { DWORD pid, parent; };
	static MHParentEntry table[4096];
	int count = 0;
	PROCESSENTRY32 pe;
	ZeroMemory(&pe, sizeof(pe));
	pe.dwSize = sizeof(pe);
	if(Process32First(hSnap, &pe)) {
		do {
			if(count < 4096) {
				table[count].pid    = pe.th32ProcessID;
				table[count].parent = pe.th32ParentProcessID;
				count++;
			}
		} while(Process32Next(hSnap, &pe));
	}
	CloseHandle(hSnap);
	// Раскрываем дерево в ширину. Порядок записей в снапшоте произвольный,
	// поэтому обход делаем по волнам, пока новые потомки ещё появляются.
	static DWORD known[MH_MAX_SCRIPT_CHILDREN+1];
	int knownCount = 0, found = 0;
	known[knownCount++] = rootPid;
	bool grew = true;
	while(grew && found < maxCount) {
		grew = false;
		for(int i=0; i<count && found<maxCount; i++) {
			if(!table[i].pid) continue;
			bool already = false;
			for(int k=0; k<knownCount; k++)
				if(known[k]==table[i].pid) { already=true; break; }
			if(already) continue;
			bool parentKnown = false;
			for(int k=0; k<knownCount; k++)
				if(known[k]==table[i].parent) { parentKnown=true; break; }
			if(!parentKnown) continue;
			known[knownCount++] = table[i].pid;
			out[found++] = table[i].pid;
			grew = true;
		}
	}
	return found;
}
// Завершить процесс, не дожидаясь его выхода.
// Дожидаться нельзя: остановка зовётся из оконной процедуры, а на установленном
// WH_MOUSE_LL хуке Windows молча снимает хук, если callback задержался дольше
// LowLevelHooksTimeout (по умолчанию 1000 мс).
static void TerminateNoWait(DWORD pid)
{
	HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if(!h) return;
	TerminateProcess(h, 0);
	CloseHandle(h);
}
bool EmbeddedExe_LaunchScript(int resourceId, TCHAR *scriptPath, TCHAR *exeDir)
{
	MHScriptSlot *slot = FindSlot(resourceId);
	if(!slot || !scriptPath) return false;
	// Уже работает - второй раз не запускаем, иначе копии накапливаются
	if(slot->hProc && WaitForSingleObject(slot->hProc, 0)==WAIT_TIMEOUT
		&& IsSameProcess(slot->hProc, &slot->rootCreated))
		return true;
	ReleaseSlot(slot);
	SHELLEXECUTEINFO sei;
	ZeroMemory(&sei, sizeof(sei));
	sei.cbSize = sizeof(sei);
	// SEE_MASK_NOCLOSEPROCESS даёт нам хендл процесса, который запустился
	sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
	sei.lpVerb = _T("open");
	sei.lpFile = scriptPath;
	sei.lpDirectory = exeDir;
	sei.nShow = SW_SHOW;
	if(!ShellExecuteEx(&sei)) return false;
	if(!sei.hProcess) return false;
	DWORD launchedPid = GetProcessId(sei.hProcess);
	CloseHandle(sei.hProcess);	// Свой хендл нам не нужен, открываем свой с нужными правами
	if(!launchedPid) return false;
	// Запоминаем именно корневой процесс. Через ассоциацию .ahk запускается
	// лаунчер AutoHotkeyUX.exe, а настоящий интерпретатор стартует отдельным
	// процессом заметно позже (проверено: появляется через ~300 мс), поэтому
	// в момент запуска его PID ещё не существует. Дерево потомков разбираем
	// при остановке, когда весь скрипт уже на месте.
	slot->rootPid = launchedPid;
	slot->hProc = OpenProcess(PROCESS_TERMINATE|SYNCHRONIZE|PROCESS_QUERY_LIMITED_INFORMATION, FALSE, launchedPid);
	if(slot->hProc) {
		// ВНИМАНИЕ: GetProcessTimes пишет во все четыре структуры и не проверяет
		// указатели на NULL - передача NULL приводит к падению по ACCESS_VIOLATION
		FILETIME create, exitTime, kernel, user;
		if(GetProcessTimes(slot->hProc, &create, &exitTime, &kernel, &user))
			slot->rootCreated = create;
	}
	return true;
}
void EmbeddedExe_StopScript(int resourceId)
{
	MHScriptSlot *slot = FindSlot(resourceId);
	if(!slot) return;
	if(!slot->hProc) { ReleaseSlot(slot); return; }
	// Процесс уже завершился сам (пользователь закрыл скрипт через трей) -
	// обходить потомков нечего
	if(WaitForSingleObject(slot->hProc, 0) != WAIT_TIMEOUT) { ReleaseSlot(slot); return; }
	// PID мог быть переиспользован - тогда трогать нечего
	if(!IsSameProcess(slot->hProc, &slot->rootCreated)) { ReleaseSlot(slot); return; }
	// Сначала гасим потомков - это интерпретатор AutoHotkey, который реально
	// выполняет скрипт. Обход в обратном порядке: сначала самые глубокие.
	// Ждать их завершения нельзя: это оконная процедура с установленным
	// WH_MOUSE_LL хуком, Windows снимает его если callback задержится.
	DWORD kids[MH_MAX_SCRIPT_CHILDREN];
	int n = CollectDescendants(slot->rootPid, kids, MH_MAX_SCRIPT_CHILDREN);
	for(int i=n-1; i>=0; i--) TerminateNoWait(kids[i]);
	// Теперь сам лаунчер
	TerminateProcess(slot->hProc, 0);
	// Короткое ожидание: скрипт не должен кликать поверх открывающегося диалога,
	// но и блокировать оконную процедуру дольше таймаута хука нельзя
	WaitForSingleObject(slot->hProc, MH_AHK_STOP_WAIT_MS);
	ReleaseSlot(slot);
}
void EmbeddedExe_StopAllScripts()
{
	for(int i=0;i<MH_NUM_SCRIPT_SLOTS;i++) EmbeddedExe_StopScript(g_scripts[i].resId);
}
bool EmbeddedExe_IsScriptRunning(int resourceId)
{
	MHScriptSlot *slot = FindSlot(resourceId);
	return slot && slot->hProc && WaitForSingleObject(slot->hProc, 0)==WAIT_TIMEOUT;
}