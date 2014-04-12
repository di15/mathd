
#ifndef TRYSTEP_H
#define TRYSTEP_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"

#define PATHNODE_SIZE	(TILE_SIZE/8)
#define PATHNODE_DIAG	(sqrt(PATHNODE_SIZE*PATHNODE_SIZE*2))

// JPS expansion directions
#define EXP_N			0
#define EXP_E			1
#define EXP_S			2
#define EXP_W			3
#define EXP_NE			4
#define EXP_NE_N		5
#define EXP_NE_E		6
#define EXP_SE			7
#define EXP_SE_S		8
#define EXP_SE_E		9
#define EXP_SW			10
#define EXP_SW_S		11
#define EXP_SW_W		12
#define EXP_NW			13
#define EXP_NW_N		14
#define EXP_NW_W		15

// byte-align structures
#pragma pack(push, 1)

class PathNode
{
public:
	int F;
	short nx;
	short nz;
	int totalD;
	unsigned char expansion;
	PathNode* previous;
	//bool tried;
	bool opened;
	bool closed;
	PathNode()
	{
		//tried = false;
		previous = NULL;
		opened = false;
		closed = false;
	};
	PathNode(int startx, int startz, int endx, int endz, int nx, int nz, PathNode* prev, int totalD, int stepD);
	PathNode(int startx, int startz, int endx, int endz, int nx, int nz, PathNode* prev, int totalD, int stepD, unsigned char expan);
};

#pragma pack(pop)

extern Vec2i g_pathdim;

class PathJob;
class WorkThread;

Vec2i PathNodePos(WorkThread* wt, PathNode* node);
PathNode* PathNodeAt(WorkThread* wt, int nx, int nz);
int PathNodeIndex(int nx, int nz);
bool AtGoal(PathJob* pj, PathNode* node);
void SnapToNode(PathJob* pj);

#endif