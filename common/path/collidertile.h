#ifndef PATHCELL_H
#define PATHCELL_H

#include "../platform.h"
#include "../math/vec2i.h"


//#define FLAG_HASROAD	4
#define FLAG_HASLAND	1
#define FLAG_ABRUPT		2

#define LARGEST_UNIT_NODES		4
#define MAX_COLLIDER_UNITS		4

// byte-align structures
#pragma pack(push, 1)
class ColliderTile
{
public:
	//bool hasroad;
	//bool hasland;
	//bool haswater;
	//bool abrupt;	//abrupt incline?
	unsigned char flags;
	short building;
	short units[MAX_COLLIDER_UNITS];
	unsigned short foliage;

	ColliderTile();
};
#pragma pack(pop)

extern ColliderTile *g_collidertile;

class Unit;
class Building;
class PathJob;

ColliderTile* ColliderAt(int nx, int nz);
Vec2i PathNodePos(int cmposx, int cmposz);
void FreePathGrid();
void AllocPathGrid(int cmwx, int cmwz);
void FillColliderGrid();
bool Standable(const PathJob* pj, const int nx, const int nz);
bool Standable2(const PathJob* pj, int cmposx, int cmposz);
bool TileStandable(const PathJob* pj, const int nx, const int nz);

#endif
