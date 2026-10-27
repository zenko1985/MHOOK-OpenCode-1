// Выдвет сообщение о (системных и несистемных) ошибках
#ifndef __MH_REPERR
#define __MH_REPERR
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WIDEFILE__ WIDEN(__FILE__)
void MHReportError(TCHAR *SourceFile, TCHAR *FuncName, int LineNumber, HWND hwnd=NULL);
void MHReportError(TCHAR *Error, HWND hwnd=NULL); // Для НЕсистемных ошибок (перегружена)
#endif