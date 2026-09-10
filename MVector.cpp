#include <Windows.h>
#include <stdlib.h>
#include "MVector.h"
#include "Settings.h"
extern LONG quad_x, quad_y;
#define MH_WINDOW_SIZE 200
int MHVector::x=0, MHVector::y=0, MHVector::vector_position=-1;
__forceinline int isqrt(LONGLONG sq) {
	int result = (int)sq;
	int bit = 1 << 15;
	while(bit) {
		if(result >= bit + (result >> 1)) result = (result >> 1) + bit;
		else result = result >> 1;
		bit >>= 1;
	}
	return result;
}
int MHVector::NewValues(LONG dx, LONG dy)
{
	x+=dx; y+=dy;
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