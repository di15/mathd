

#include "3dmath.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4244)

class SceneryType
{
public:
	char name[32];
	int model;
	float hsize;
	float vsize;
};

enum SCENERYTYPE{TREE1, SCENERY_TYPES};

extern SceneryType g_sceneryType[SCENERY_TYPES];

class Scenery
{
public:
	bool on;
	int type;
	Vec3f pos;
	float yaw;

	Scenery();
};

#define SCENERY	128

extern Scenery g_scenery[SCENERY];

extern int g_scT;

void DrawScenery();
void ClearScenery(float x, float z, float hwx, float hwz);
void ClearScenery();
void InitScenery();
void PlaceScenery();