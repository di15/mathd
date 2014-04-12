


#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../sys/workthread.h"
#include "../platform.h"

#define PATHCELL_SIZE	(TILE_SIZE/8)
#define PATHCELL_DIAG	(sqrt(PATHCELL_SIZE*PATHCELL_SIZE+PATHCELL_SIZE*PATHCELL_SIZE))

class ColliderCell
{
public:
	bool hasroad;
	bool hasland;
	bool haswater;
	bool abrupt;	//abrupt incline?
	list<int> units;
	list<int> foliage;
	int building;

	ColliderCell();
};

extern ColliderCell *g_collidercell;
extern Vec2i g_pathdim;

ColliderCell* ColliderCellAt(int cx, int cz);
Vec2i PathCellPos(int cmposx, int cmposz);
unsigned int PathCellIndex(int cx, int cz);
void FreePathCells();
void AllocPathCells(int cmwx, int cmwz);
void FillPathCells();

class TryStep
{
public:
	int F;
	int cx;
	int cz;
	TryStep* previous;
	int runningD;
	//bool tried;
	TryStep()
	{
		//tried = false;
	};
	TryStep(int startx, int startz, int endx, int endz, int cx, int cz, TryStep* prev, int totalD, int stepD);
};

class Unit;
class Building;

extern Unit* g_pathunit;

bool FindPath(int wthread, int utype, int umode, int cmstartx, int cmstartz, int target, int targetx, int targetz, 
			  list<Vec2i> &path, Vec2i &subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb, 
			  int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminz, int cmgoalmaxx, int cmgoalmaxz);

void DrawGrid();
void DrawUnitSquares();
void DrawPaths();
void DrawVelocities();
void LogPathDebug();

#endif