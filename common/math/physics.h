
#ifndef PHYSICS_H
#define PHYSICS_H

#include "3dmath.h"
#include "building.h"

#define EPSILON			0.03125f //0.1f	//1.2f
#define FRICTION	1.5f

#define COLLIDER_NONE		-1
#define COLLIDER_UNIT		0
#define COLLIDER_BUILDING	1
#define COLLIDER_TERRAIN	2
#define COLLIDER_NOROAD		3
extern int g_lastCollider;
extern int g_colliderType;
extern bool g_ignored;

class CUnit;

bool BuildingAdjacent(int i, int j);
bool PowerlineAdjacent(int i, int x, int z);
bool PipelineAdjacent(int i, int x, int z);
bool AnyWateri(int tx, int tz);
bool AnyWater(float px, float pz);
bool AtWater(float px, float pz);
bool TileUnclimablei(int tx, int tz);
bool TileUnclimable(float px, float pz);
bool CollidesWithBuildings(float x, float z, float wx, float wz, int ignore=-1, float eps=EPSILON);
bool CollidesWithUnits(float x, float z, float hwx, float hwz, bool isUnit=false, CUnit* thisU=NULL, CUnit* ignore=NULL, bool checkPass=true, float eps=EPSILON);
bool BuildingCollides(int type, Vec3f pos);
bool BuildingLevel(int type, Vec3f pos);

#endif