
#ifndef PATHNODE_H
#define PATHNODE_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"

#define PATHNODE_SIZE	(TILE_SIZE/8)
#define PATHNODE_DIAG	(sqrt(PATHNODE_SIZE*PATHNODE_SIZE*2))

extern Vec2i straightoffsets[4];
extern Vec2i diagonaloffsets[4];

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

class Heap;
class PathJob;

extern Vec2i g_pathdim;
extern PathNode* g_pathnode;
extern Heap g_openlist;

Vec2i PathNodePos(PathNode* node);
PathNode* PathNodeAt(int nx, int nz);
int PathNodeIndex(int nx, int nz);
bool AtGoal(PathJob* pj, PathNode* node);
void SnapToNode(PathJob* pj);
void ResetPathNodes();

#endif