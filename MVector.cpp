#include <Windows.h>
#include <math.h>
#include "MVector.h"
#include "Settings.h"
extern LONG quad_x, quad_y;
#define MH_WINDOW_SIZE 200
static double PI = 3.14159265359;
int MHVector::x=0, MHVector::y=0, MHVector::vector_position=-1;
int MHVector::NewValues(LONG dx, LONG dy)
{
	int angle, new_position;
	x+=dx; y+=dy;
	LONGLONG sq = (LONGLONG)x * x + (LONGLONG)y * y;
	double dist = sqrt((double)sq);
	if (dist < (double)MHSettings::GetMouseSensitivity()) {
		return -2;
	}
	double invDist = 1.0 / dist;
	quad_x=(LONG)(MH_WINDOW_SIZE/2*x*invDist);
	quad_y=(LONG)(MH_WINDOW_SIZE/2*y*invDist);
	angle=(int)(atan2((double)y,(double)dx)*180.0/PI);
	if(4==MHSettings::GetNumPositions())
	{
		angle=angle+135;
		if(angle<0) angle+=360;
		new_position=angle/90;
		if(new_position>3) new_position=3;
	}
	else
	{
		angle=angle+112;
		if(angle<0) angle+=360;
		new_position=angle/45;
		if(new_position>7) new_position=7;
	}
	x=0;y=0;
	if (vector_position == new_position) {
		return -1;
	} else {
		vector_position=new_position;
		return vector_position;
	}
}