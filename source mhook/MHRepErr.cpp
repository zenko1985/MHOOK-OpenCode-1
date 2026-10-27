/*
*	Сообщает об ошибках времени исполнения
*/
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include "MHRepErr.h"
#include "Localization.h"
// Сообщает о СИСТЕМНЫХ ошибках времени исполнения
void MHReportError(TCHAR *SourceFile, TCHAR *FuncName, int LineNumber, HWND hwnd)
{
	DWORD res;				// Результат функции FormatMessage
	const TCHAR *BKBStringError;	// Указатель на строку для получения системной ошибки
	TCHAR BKBMessage[1024];	// Это строка, в которой формируется сообщение об ошибке
	// ВНИМАНИЕ: память для BKBStringError выделяет только FormatMessage с флагом
	// FORMAT_MESSAGE_ALLOCATE_BUFFER. Во всех остальных случаях здесь стоит
	// указатель на строковый литерал из секции .rdata, и LocalFree по нему -
	// это вызов HeapFree по чужому адресу, то есть порча кучи
	bool needLocalFree = false;
	DWORD BKBLastError=GetLastError(); // Получили код системной ошибки
	if (BKBLastError!=(DWORD)0) // Получить строку, если код не равен нулю
	{
		// LANG_NEUTRAL с FORMAT_MESSAGE_FROM_SYSTEM не подходит - нужно брать
		// язык пользователя по умолчанию, иначе текст ошибки не находится
		res=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL, BKBLastError,
						MAKELANGID((WORD)(GetUserDefaultLangID()>>10), SUBLANG_DEFAULT),
						(LPTSTR) &BKBStringError, 0, NULL );
		if(res==(DWORD)0) BKBStringError=L(LOC_ERR_MSG_NOT_FOUND);
		else needLocalFree = true;
	}
	else
	{
		BKBStringError=L(LOC_ERR_NO_SYS_ERROR);
	}
	// Сформировать строку с полным описанием ошибки
	swprintf_s(BKBMessage, _countof(BKBMessage),
			L"Module: %s\r\nFunction: %s\r\nLine number: %d\r\nSysErr: %d (%s)",
			SourceFile, FuncName, LineNumber,
			BKBLastError, BKBStringError);
	//Освобождаем память, которую выделила функция FormatMessage
	if (needLocalFree)
	{
		LocalFree( (LPVOID)BKBStringError );
	}
	//Печатаем сообщение об ошибке (если возможно, на экран)
	MessageBox(hwnd,BKBMessage,L(LOC_ERR_CAPTION),MB_OK|MB_ICONINFORMATION );
}
// Для НЕсистемных ошибок (перегружена)
// Реально не видел, где бы она использовалась
void MHReportError(TCHAR *Error, HWND hwnd)
{
	//Печатаем сообщение об ошибке (если возможно, на экран)
	MessageBox(hwnd,Error,L(LOC_ERR_DISORDER),MB_OK|MB_ICONINFORMATION );
}