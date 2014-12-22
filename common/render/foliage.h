#ifndef FOLIAGE_H
#define FOLIAGE_H

#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../math/vec3i.h"
#include "../math/matrix.h"
#include "vertexarray.h"

class FlType
{
public:
	char name[64];
	Vec3i size;
	unsigned int texindex;
	int model;
};


#define FL_TREE1		0
#define FL_TREE2		1
#define FL_TREE3		2
#if 0
#define FL_BUSH1		3
#define FL_TYPES		4
#else
#define FL_TYPES		3
#endif

extern FlType g_fltype[FL_TYPES];

// byte-align structures
#pragma pack(push, 1)
class Foliage
{
public:
	bool on;
	unsigned char type;
	Vec2i cmpos;
	Vec3f drawpos;
	float yaw;
	unsigned char lastdraw;	//used for preventing repeats

	Foliage();
	void reinstance();
	void fillcollider();
	void freecollider();
};
#pragma pack(pop)

//#define FOLIAGES	128
//#define FOLIAGES	2048
//#define FOLIAGES	6000
//#define FOLIAGES	30000
#define FOLIAGES	60000
//#define FOLIAGES	240000

extern Foliage g_foliage[FOLIAGES];

void DefF(int type, const char* modelrelative, Vec3f scale, Vec3f translate, Vec3i size);
bool PlaceFol(int type, Vec3i ipos);
void DrawFol(Vec3f zoompos, Vec3f vertical, Vec3f horizontal);
void ClearFol(int cmminx, int cmminy, int cmmaxx, int cmmaxy);
void FreeFol();
void FillForest();
#endif
