
#ifndef PATHFINDING_H
#define PATHFINDING_H

#pragma warning(disable: 4244)

#include <vector>
#include "unit.h"
#include "map.h"
#include "3dmath.h"
#include "physics.h"

using namespace std;

class CTryStep
{
public:
	float F;
	//int F;
	int x, z;
	int previous;
	float runningD;
	//bool tried;
	CTryStep() 
	{ 
		//tried = false; 
	};
	CTryStep(int startx, int startz, int endx, int endz, int X, int Z, int prev, float totalD, float stepD);
};

extern vector<CTryStep> g_tryStep;
extern vector<CTryStep> g_triedStep;

class Collider
{
public:
	int type;
	int ID;

	Collider(int typ, int id)
	{
		type = typ;
		ID = id;
	}
};

class ColliderCell
{
public:
	vector<Collider> colliders;
};

extern ColliderCell *g_collidercells;

//#define MIN_RADIUS	4.0f
#define MIN_RADIUS		2.0f
//#define PATHFIND_SIZE		(int)(MAP_SIZE*TILE_SIZE/(MIN_RADIUS*2.0f))

#define PATH_DELAY		100

//#define GOAL_PROXIMITY		0.5f

//#define NO_DIAGONAL			//disallow diagonal movement (simplifies pathfinding)

extern int g_pathfindszx;
extern int g_pathfindszz;
//extern bool g_open[PATHFIND_SIZE][PATHFIND_SIZE];
extern unsigned int* g_open;
extern unsigned int g_curpath;

//#define MAX_SEARCH_DEPTH	(g_pathfindszx*g_pathfindszz*10)
#define MAX_SEARCH_DEPTH	(g_pathfindszx*g_pathfindszz/10)

#define MAX_CLIMB_INCLINE		(TILE_SIZE/2)

unsigned int& OpenAt(int x, int z);
void DrawGrid();
void DrawUnitSquares();
void DrawPaths();
void DrawVelocities();
void FreeColliderCells();
void FillColliderCells();
ColliderCell* ColliderCellAt(int x, int z);

#endif