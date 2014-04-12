

#ifndef COLLISION_H
#define COLLISION_H

#include "../platform.h"

#define MAX_CLIMB_INCLINE		(TILE_SIZE)

#define COLLIDER_NONE		-1
#define COLLIDER_UNIT		0
#define COLLIDER_BUILDING	1
#define COLLIDER_TERRAIN	2
#define COLLIDER_NOROAD		3
#define COLLIDER_OTHER		4
extern int g_lastcollider;
extern int g_collidertype;
extern bool g_ignored;

class Unit;


class Unit;

bool BuildingAdjacent(int i, int j);

bool PowlAdjacent(int i, int x, int z);
bool CrPipeAdjacent(int i, int x, int z);
#if 0
bool AnyWateri(int tx, int tz);
bool AnyWater(float px, float pz);
bool AtWater(float px, float pz);
bool TileUnclimablei(int tx, int tz);
bool TileUnclimable(float px, float pz);

#endif

bool CollidesWithBuildings(int minx, int minz, int maxx, int maxz, int ignore=-1);
bool CollidesWithUnits(int minx, int minz, int maxx, int maxz, bool isunit=false, Unit* thisu=NULL, Unit* ignore=NULL);

#if 0

bool BuildingCollides(int type, Vec3f pos);
bool BuildingLevel(int type, Vec3f pos);

#endif

#endif