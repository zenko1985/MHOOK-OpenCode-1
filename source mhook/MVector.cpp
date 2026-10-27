#include <Windows.h>
#include <stdlib.h>
#include "MVector.h"
#include "Settings.h"
extern LONG quad_x, quad_y;
#define MH_WINDOW_SIZE 200
int MHVector::x=0, MHVector::y=0, MHVector::vector_position=-1;
// Целочисленный квадратный корень через двоичный поиск.
// Раньше здесь было (int)sq, то есть усечение LONGLONG до int, и бит
// поиска начинался с 1<<15 - при больших значениях вектор считался неверно.
// Все вычисления ведём в 64 битах, результат ограничиваем INT_MAX
__forceinline int isqrt(LONGLONG sq) {
	if(sq <= 0) return 0;
	ULONGLONG left = 0, right = 0xFFFFFFFFULL, root = 0;
	while(left <= right) {
		ULONGLONG mid = left + ((right - left) >> 1);
		if(mid * mid <= (ULONGLONG)sq) {
			root = mid;
			if(mid > 0xFFFFFFFFULL - mid) break;	// не переполняем right
			left = mid + 1;
		} else {
			if(mid == 0) break;
			right = mid - 1;
		}
	}
	return (root > (ULONGLONG)INT_MAX) ? INT_MAX : (int)root;
}
int MHVector::NewValues(LONG dx, LONG dy)
{
	x+=dx; y+=dy;
	// Накопление ниже порога чувствительности не сбрасывается - это даёт
	// "память" о последней мелкой активности мыши. Но без ограничения x и y
	// растут бесконечно и переполняют int. Держим их в разумных границах
	const int MH_VECTOR_CLAMP = 0x10000;	// 65536, заведомо больше любой области
	if(x >  MH_VECTOR_CLAMP) x =  MH_VECTOR_CLAMP;
	if(x < -MH_VECTOR_CLAMP) x = -MH_VECTOR_CLAMP;
	if(y >  MH_VECTOR_CLAMP) y =  MH_VECTOR_CLAMP;
	if(y < -MH_VECTOR_CLAMP) y = -MH_VECTOR_CLAMP;
	LONGLONG sq = (LONGLONG)x * x + (LONGLONG)y * y;
	int sens = MHSettings::GetMouseSensitivity();
	if (sq < (LONGLONG)sens * sens) {
		return -2;
	}
	int dist = isqrt(sq);
	if(dist > 0) {
		int half = MH_WINDOW_SIZE / 2;
		quad_x = (LONG)(half * x / dist);
		quad_y = (LONG)(half * y / dist);
	}
	int new_position;
	int ax = abs(x), ay = abs(y);
	if(4 == MHSettings::GetNumPositions())
	{
		if(ax > ay) new_position = (x > 0) ? 1 : 3;
		else        new_position = (y > 0) ? 2 : 0;
	}
	else
	{
		if(ax > ay * 2)          new_position = (x > 0) ? 2 : 6;
		else if(ay > ax * 2)    new_position = (y > 0) ? 4 : 0;
		else if(x > 0 && y > 0) new_position = 3;
		else if(x > 0 && y < 0) new_position = 1;
		else if(x < 0 && y > 0) new_position = 5;
		else                     new_position = 7;
	}
	x=0; y=0;
	if (vector_position == new_position) {
		return -1;
	} else {
		vector_position=new_position;
		return vector_position;
	}
}