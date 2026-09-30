#include <Windows.h>
#include <vector>
#include <strsafe.h>
#include "resource.h"
#include "Bitmap.h"
#include "Settings.h"
#include "MHRepErr.h"
#include "miniz.h"
extern HINSTANCE	MHInst;
HDC MHBitmap::hdc4=0, MHBitmap::hdc8=0;
HDC MHBitmap::hdc4red=0, MHBitmap::hdc8red=0;
HBITMAP MHBitmap::hbm4=0, MHBitmap::hbm8=0;
HBITMAP MHBitmap::hbm4red=0, MHBitmap::hbm8red=0;
extern HBRUSH red_brush;
extern bool right_button_down;
extern bool left_button_down;
// Количество картинок в контейнере EmbeddedBitmaps.bin.
// Порядок должен совпадать со списком BITMAPS в generate_embedded_bitmaps.py
#define MH_NUM_EMBEDDED_BITMAPS 4
// Заголовок распакованного контейнера: DWORD count + по 4 DWORD на картинку
#define MH_BITMAP_ENTRY_SIZE 16
// Распакованный контейнер занимает около 1,9 МБ, запас берём с большим избытком
#define MH_BITMAP_MAX_SIZE (16*1024*1024)
// Показывает ошибку с текстом. Строковые литералы передавать нельзя - функция
// принимает TCHAR*, поэтому используем буфер, как в Settings.cpp
static void ReportTextError(const TCHAR *text)
{
	TCHAR buffer[256];
	StringCchCopy(buffer,_countof(buffer),text);
	MHReportError(buffer);
}
// Распаковывает встроенный в exe контейнер с картинками джойстика.
// Раньше картинки хранились как BITMAP-ресурсы и занимали 1,92 МБ, теперь они
// лежат в одном сжатом файле EmbeddedBitmaps.bin (53 КБ), который распаковывается
// через miniz - тот же механизм, что и для EmbeddedSettings.bin
static bool LoadEmbeddedBitmaps(std::vector<BYTE> &data)
{
	data.clear();
	HRSRC hRes=FindResource(NULL,MAKEINTRESOURCE(IDR_EMBEDDEDBITMAPS),RT_RCDATA);
	if(!hRes)
	{
		MHReportError(__WIDEFILE__,L"FindResource (IDR_EMBEDDEDBITMAPS)",__LINE__);
		return false;
	}
	HGLOBAL hData=LoadResource(NULL,hRes);
	if(!hData)
	{
		MHReportError(__WIDEFILE__,L"LoadResource",__LINE__);
		return false;
	}
	DWORD size=SizeofResource(NULL,hRes);
	const BYTE* pData=(const BYTE*)LockResource(hData);
	static const BYTE zlib_magic[4]={'Z','L','I','B'};
	if(!pData||size<8||memcmp(pData,zlib_magic,4)!=0)
	{
		ReportTextError(L"Повреждён встроенный ресурс с картинками");
		return false;
	}
	DWORD origSize=0;
	memcpy(&origSize,pData+4,sizeof(DWORD));
	if(origSize==0||origSize>MH_BITMAP_MAX_SIZE)
	{
		ReportTextError(L"Некорректный размер встроенных картинок");
		return false;
	}
	data.resize(origSize);
	mz_ulong dstLen=(mz_ulong)origSize;
	if(mz_uncompress(data.data(),&dstLen,pData+8,(mz_ulong)(size-8))!=0)
	{
		ReportTextError(L"Не удалось распаковать встроенные картинки");
		data.clear();
		return false;
	}
	DWORD count=0;
	memcpy(&count,data.data(),sizeof(DWORD));
	if(dstLen<4+MH_BITMAP_ENTRY_SIZE*MH_NUM_EMBEDDED_BITMAPS||count!=MH_NUM_EMBEDDED_BITMAPS)
	{
		ReportTextError(L"Неверное количество встроенных картинок");
		data.clear();
		return false;
	}
	return true;
}
// Создаёт HBITMAP по распакованным данным одной картинки.
// Пиксели хранятся 24 битами (BGR), строки снизу вверх - как в .bmp,
// поэтому biHeight положительный, данные копируются без переворота
static HBITMAP CreateBitmapFromData(HDC hdc,int index,const std::vector<BYTE> &data)
{
	const BYTE* entry=data.data()+4+MH_BITMAP_ENTRY_SIZE*index;
	LONG width=0,height=0;
	DWORD offset=0,pixels_size=0;
	memcpy(&width,entry,sizeof(LONG));
	memcpy(&height,entry+sizeof(LONG),sizeof(LONG));
	memcpy(&offset,entry+8,sizeof(DWORD));
	memcpy(&pixels_size,entry+12,sizeof(DWORD));
	if(width<=0||height<=0||width>4096||height>4096) return NULL;
	DWORD stride=(((DWORD)width*3+3)/4)*4;
	if(offset>(DWORD)data.size()||pixels_size!=stride*(DWORD)height) return NULL;
	HBITMAP hbm=CreateCompatibleBitmap(hdc,width,height);
	if(!hbm)
	{
		MHReportError(__WIDEFILE__,L"CreateCompatibleBitmap",__LINE__);
		return NULL;
	}
	BITMAPINFO bi;
	ZeroMemory(&bi,sizeof(bi));
	bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth=width;
	bi.bmiHeader.biHeight=height;
	bi.bmiHeader.biPlanes=1;
	bi.bmiHeader.biBitCount=24;
	bi.bmiHeader.biCompression=BI_RGB;
	if(SetDIBits(hdc,hbm,0,height,data.data()+offset,&bi,DIB_RGB_COLORS)==0)
	{
		MHReportError(__WIDEFILE__,L"SetDIBits",__LINE__);
		DeleteObject(hbm);
		return NULL;
	}
	return hbm;
}
void MHBitmap::Init(HWND hwnd)
{
	HDC hdc=GetDC(hwnd);
	hdc4=CreateCompatibleDC(hdc);
	hdc8=CreateCompatibleDC(hdc);
	hdc4red=CreateCompatibleDC(hdc);
	hdc8red=CreateCompatibleDC(hdc);
	std::vector<BYTE> data;
	if(LoadEmbeddedBitmaps(data))
	{
		// Порядок картинок в контейнере: bm4w, bm4wred, bm8w, bm8wred
		hbm4=CreateBitmapFromData(hdc,0,data);
		hbm4red=CreateBitmapFromData(hdc,1,data);
		hbm8=CreateBitmapFromData(hdc,2,data);
		hbm8red=CreateBitmapFromData(hdc,3,data);
	}
	// Данные больше не нужны, освобождаем их сразу - 1,92 МБ
	data.clear();
	data.shrink_to_fit();
	if(hbm4) SelectObject(hdc4,hbm4);
	if(hbm8) SelectObject(hdc8,hbm8);
	if(hbm4red) SelectObject(hdc4red,hbm4red);
	if(hbm8red) SelectObject(hdc8red,hbm8red);
	ReleaseDC(hwnd,hdc);
}
void MHBitmap::Halt()
{
	DeleteDC(hdc8);
	DeleteDC(hdc4);
	DeleteDC(hdc8red);
	DeleteDC(hdc4red);
	if(hbm4) DeleteObject(hbm4);
	if(hbm8) DeleteObject(hbm8);
	if(hbm4red) DeleteObject(hbm4red);
	if(hbm8red) DeleteObject(hbm8red);
}
void MHBitmap::OnDraw(HDC hdc,int position)
{
	switch(MHSettings::GetNumPositions())
	{
	case 4:
		// Сначала рисует незакрашенный кружочек
		BitBlt(hdc,0,0,200,200,hdc4,0,0,SRCCOPY);
		// Красные нажатия мыши
		if(left_button_down) BitBlt(hdc,0,0,100,100,hdc4red,0,0,SRCCOPY);
		if(right_button_down) BitBlt(hdc,100,0,100,100,hdc4red,100,0,SRCCOPY);
		switch(position)
		{
		case 0: // стрелка вверх
			BitBlt(hdc,0,0,200,100,hdc4,200,0,SRCCOPY);
			if(left_button_down) BitBlt(hdc,0,0,100,100,hdc4red,200,0,SRCCOPY);
			if(right_button_down) BitBlt(hdc,100,0,100,100,hdc4red,300,0,SRCCOPY);
			break;
		case 1: // стрелка вправо
			if(right_button_down) BitBlt(hdc,100,0,100,200,hdc4red,100,199,SRCCOPY);
			else BitBlt(hdc,100,0,100,200,hdc4,100,199,SRCCOPY);
			break;
		case 2: // стрелка вниз
			BitBlt(hdc,0,100,200,100,hdc4,200,100,SRCCOPY);
			break;
		case 3: // стрелка влево
			if(left_button_down) BitBlt(hdc,0,0,100,200,hdc4red,0,199,SRCCOPY);
			else BitBlt(hdc,0,0,100,200,hdc4,0,199,SRCCOPY);
			break;
		}
		break;
	case 8:
		// Сначала рисует незакрашенный кружочек
		BitBlt(hdc,0,0,200,200,hdc8,0,0,SRCCOPY);
		// Красные нажатия мыши
		if(left_button_down) BitBlt(hdc,0,0,100,100,hdc8red,0,0,SRCCOPY);
		if(right_button_down) BitBlt(hdc,100,0,100,100,hdc8red,100,0,SRCCOPY);
		switch(position)
		{
		case 0: // стрелка вверх
			BitBlt(hdc,0,0,200,100,hdc8,200,0,SRCCOPY);
			if(left_button_down) BitBlt(hdc,0,0,100,100,hdc8red,200,0,SRCCOPY);
			if(right_button_down) BitBlt(hdc,100,0,100,100,hdc8red,300,0,SRCCOPY);
			break;
		case 2: // стрелка вправо
			if(right_button_down) BitBlt(hdc,100,0,100,200,hdc8red,100,200,SRCCOPY);
			else BitBlt(hdc,100,0,100,200,hdc8,100,200,SRCCOPY);
			break;
		case 4: // стрелка вниз
			BitBlt(hdc,0,100,200,100,hdc8,200,100,SRCCOPY);
			break;
		case 6: // стрелка влево
			if(left_button_down) BitBlt(hdc,0,0,100,200,hdc8red,0,200,SRCCOPY);
			else BitBlt(hdc,0,0,100,200,hdc8,0,200,SRCCOPY);
			break;
		case 1: // стрелка вверх-вправо
			if(right_button_down) BitBlt(hdc,100,0,100,100,hdc8red,300,201,SRCCOPY);
			else BitBlt(hdc,100,0,100,100,hdc8,300,201,SRCCOPY);
			break;
		case 3: // стрелка вниз-вправо
			BitBlt(hdc,100,100,100,100,hdc8,300,301,SRCCOPY);
			break;
		case 5: // стрелка вниз-влево
			BitBlt(hdc,0,100,100,100,hdc8,200,301,SRCCOPY);
			break;
		case 7: // стрелка вверх-влево
			if(left_button_down) BitBlt(hdc,0,0,100,100,hdc8red,200,201,SRCCOPY);
			else BitBlt(hdc,0,0,100,100,hdc8,200,201,SRCCOPY);
			break;
		}
		break;
	}
}