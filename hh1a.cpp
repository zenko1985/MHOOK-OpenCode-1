#include <Windows.h>
#include "hh1a.h"
#include "MVector.h"
#include "MHKeypad.h"
#include "Settings.h"
#include "WM_USER_messages.h"
extern LONG screen_x, screen_y;
int MHookHandler1a::OnMouseMove(LONG _x, LONG _y)
{
	int position;
	DWORD time_now;
	// При нажатой правой кнопке мыши не передаём её движения в MHVector,
	// НО! продолжаем отслеживать last_x и last_y, не сбрасывая initialized!
	if((initialized)&&(!rbutton_pressed)) // известно последнее положение мыши, правая кнопка не нажата
	{
		dx=(_x-last_x);
		dy=(_y-last_y);
		position=MHVector::NewValues(dx,dy);
		// !!! Здесь мы учитываем и повторное движение в том же направлении
		if(0<=position) position_mem=position;
		// Поэтому не 0<=..., а -1<=...
		if(-1<=position) // -2=мышь ваще не двигалась, -1= направление не изменилось
		{
			time_now=timeGetTime();
			// если направление изменилось - время с последнего нажатия не имеет значения
			// если не изменилось -  важно, чтобы с момента последнего нажатия прошло достаточно времени
			if((0<=position)||(time_now-last_time>MHSettings::time_between_pushes))
			{
				MHKeypad::Tap(position_mem);
				last_time=time_now;
				// Без этого окно джойстика не перерисовывается: клавиши жмутся, а картинка стоит
				ThrottledInvalidate();
			}
		}
	}
	if(!initialized) initialized=true;
	last_x=_x;
	last_y=_y;
	ClampToScreen(last_x, last_y, screen_x, screen_y);
	return 0; // Новая директива ВЦСПС
}
bool MHookHandler1a::OnRDown()
{
	rbutton_pressed=true;
	MHKeypad::Reset(); // Отпускаем нажатые кнопки
	return true; // подавляйте правый клик
}
bool MHookHandler1a::OnRUp()
{
	rbutton_pressed=false;
	// Начинаем новый отсчет движений
	MHVector::Reset();
	return true; // подавляйте правый клик
}