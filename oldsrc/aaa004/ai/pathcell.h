
#ifndef PATHCELL_H
#define PATHCELL_H

#include "../platform.h"
#include "../math/vec2i.h"

class ColliderTile
{
public:
	bool hasroad;
	bool hasland;
	bool haswater;
	bool abrupt;	//abrupt incline?
	list<int> units;
	list<int> foliage;
	int building;

	ColliderTile();
};

extern ColliderTile *g_collidertile;
extern Vec2i g_pathdim;
class Unit;
class Building;

ColliderTile* ColliderTileAt(int cx, int cz);
Vec2i PathNodePos(int cmposx, int cmposz);
unsigned int PathNodeIndex(int cx, int cz);
void FreePathGrid();
void AllocPathGrid(int cmwx, int cmwz);
void FillColliderGrid();
int NodeWalkable(const int cx,
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
int NodeBlockWalkable(const int cx,
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
bool NodeForcedNeighbour(const int cx,
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