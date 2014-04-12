
#ifndef TRYSTEP_H
#define TRYSTEP_H

#include "../platform.h"

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
	short cx;
	short cz;
	int totalD;
	unsigned char expansion;
	PathNode* previous;
	char comment[128];
	//bool tried;
	PathNode()
	{
		//tried = false;
		previous = NULL;
		comment[0] = '\0';
	};
	PathNode(int startx, int startz, int endx, int endz, int cx, int cz, PathNode* prev, int totalD, int stepD);
	PathNode(int startx, int startz, int endx, int endz, int cx, int cz, PathNode* prev, int totalD, int stepD, unsigned char expan);
	PathNode(int startx, int startz, int endx, int endz, int cx, int cz, PathNode* prev, int totalD, int stepD, unsigned char expan, const char* comment);
};

#pragma pack(pop)

extern PathNode *g_pathnode;

#endif