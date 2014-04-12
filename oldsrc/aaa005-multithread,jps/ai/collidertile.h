
#ifndef PATHCELL_H
#define PATHCELL_H

#include "../platform.h"
#include "../math/vec2i.h"

// byte-align structures
#pragma pack(push, 1)

class ColliderTile
{
public:
	bool hasroad;
	bool hasland;
	bool haswater;
	bool abrupt;	//abrupt incline?
	int building;
	short units[4];
	list<int> foliage;

	ColliderTile();
};

#pragma pack(pop)

extern ColliderTile *g_collidertile;

class Unit;
class Building;
class PathJob;

ColliderTile* ColliderTileAt(int nx, int nz);
Vec2i PathNodePos(int cmposx, int cmposz);
void FreePathGrid();
void AllocPathGrid(int cmwx, int cmwz);
void FillColliderGrid();
bool Walkable(const PathJob* pj, const int nx, const int nz);
bool Walkable2(PathJob* pj, int cmposx, int cmposz);

#endif