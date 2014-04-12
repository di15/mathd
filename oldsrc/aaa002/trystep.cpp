

#include "trystep.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"

TryStep::TryStep(int startx, int startz, int endx, int endz, int cx, int cz, TryStep* prev, int totalD, int stepD)
{
	this->cx = cx;
	this->cz = cz;
	int G = totalD + stepD;
	runningD = G;
	//float G = Magnitude2(Vec3f(startx-x,0,startz-z));
	//int H = Magnitude(Vec2i(endx-cx,endz-cz));
	int H = Manhattan(Vec2i(endx-cx,endz-cz));
	//int G = abs(startx-x) + abs(startz-z);
	//int H = abs(endx-x) + abs(endz-z);
	F = G + H;
	//F = H;
	previous = prev;
	//tried = false;
}

TryStep::TryStep(int startx, int startz, int endx, int endz, int cx, int cz, TryStep* prev, int totalD, int stepD, unsigned char expan)
{
	this->cx = cx;
	this->cz = cz;
	int G = totalD + stepD;
	runningD = G;
	//float G = Magnitude2(Vec3f(startx-x,0,startz-z));
	//float H = Magnitude(Vec2i(endx-cx,endz-cz));
	int H = Manhattan(Vec2i(endx-cx,endz-cz));
	//int G = abs(startx-x) + abs(startz-z);
	//int H = abs(endx-x) + abs(endz-z);
	F = G + H;
	//F = H;
	previous = prev;
	//tried = false;
	expansion = expan;
}