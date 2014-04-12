

#include "pathnode.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"

PathNode *g_pathnode = NULL;

PathNode::PathNode(int startx, int startz, int endx, int endz, int cx, int cz, PathNode* prev, int totalD, int stepD)
{
	this->cx = cx;
	this->cz = cz;
	int G = totalD + stepD;
	this->totalD = G;
	//float G = Magnitude2(Vec3f(startx-x,0,startz-z));
	//int H = Magnitude(Vec2i(endx-cx,endz-cz));
	int H = Manhattan(Vec2i(endx-cx,endz-cz));
	//int G = abs(startx-x) + abs(startz-z);
	//int H = abs(endx-x) + abs(endz-z);
	F = G + H;
	//F = H;
	previous = prev;
	//tried = false;
	sprintf(this->comment, ".");
}

PathNode::PathNode(int startx, int startz, int endx, int endz, int cx, int cz, PathNode* prev, int totalD, int stepD, unsigned char expan)
{
	this->cx = cx;
	this->cz = cz;
	int G = totalD + stepD;
	this->totalD = G;
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
	sprintf(this->comment, ".");
}

PathNode::PathNode(int startx, int startz, int endx, int endz, int cx, int cz, PathNode* prev, int totalD, int stepD, unsigned char expan, const char* comment)
{
	this->cx = cx;
	this->cz = cz;
	int G = totalD + stepD;
	this->totalD = G;
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
	sprintf(this->comment, "%s", comment);
}
