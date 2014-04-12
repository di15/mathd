
#ifndef PATHCELL_H
#define PATHCELL_H

#include "../platform.h"
#include "../math/vec2i.h"

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
class Unit;
class Building;

ColliderCell* ColliderCellAt(int cx, int cz);
Vec2i PathCellPos(int cmposx, int cmposz);
unsigned int PathCellIndex(int cx, int cz);
void FreePathCells();
void AllocPathCells(int cmwx, int cmwz);
void FillPathCells();
int CellOpen(const int cx,
					const int cz,
					const int cminx,
					const int cminz,
					const int cmaxx,
					const int cmaxz,
					const int cmminx,
					const int cmminz,
					const int cmmaxx,
					const int cmmaxz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb);
int CellBlockOpen(const int cx,
					const int cz,
					const int cminx,
					const int cminz,
					const int cmaxx,
					const int cmaxz,
					const int cmminx,
					const int cmminz,
					const int cmmaxx,
					const int cmmaxz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb);
bool CellForcedNeighbour(const int cx,
					const int cz,
					const int cminx,
					const int cminz,
					const int cmaxx,
					const int cmaxz,
					const int cmminx,
					const int cmminz,
					const int cmmaxx,
					const int cmmaxz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb);

#endif