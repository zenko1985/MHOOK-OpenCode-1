// Обработчик хука, виртуальный класс
#ifndef __MH_HOOKHANDLER
#define __MH_HOOKHANDLER
#include "MHKeypad.h"
class MHookHandler
{
public:
	MHookHandler():rbutton_pressed(false),initialized(false),dx(0),dy(0),last_x(0),last_y(0),position_mem(-1),last_button5_time(0),mouse_path_squared(0){};
	virtual int OnMouseMove(LONG _x, LONG _y)=0;
	virtual void OnMouseScroll(LONG _x, LONG _y);
	virtual bool OnRDown()=0;
	virtual bool OnRUp()=0;
	virtual void OnLDown();
	virtual void OnLUp();
	virtual __forceinline int GetPosition() { return MHKeypad::GetPosition(); }
	virtual void OnTimer(){};
	virtual void OnDraw(HDC hdc, LONG window_size){};
	virtual void Halt(){};
	__forceinline void HaltGeneral(){	rbutton_pressed=false; initialized=false; position_mem=-1; mouse_path_squared=0; }
	__forceinline void Deinitialize(){initialized=false;}
	void TopLeftCornerTimer();
	void OnFastMove(LONG _dx, LONG _dy);
protected:
	__forceinline void ClampToScreen(LONG &lx, LONG &ly, LONG sx, LONG sy) {
		if(lx < 0) lx = 0;
		if(ly < 0) ly = 0;
		if(lx >= sx) lx = sx - 1;
		if(ly >= sy) ly = sy - 1;
	}
	bool rbutton_pressed, initialized;
	LONG dx,dy,last_x,last_y;
	int position_mem;
	DWORD last_button5_time;
	LONG mouse_path_squared;
};
#endif