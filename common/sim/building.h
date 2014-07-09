
#ifndef BUILDING_H
#define BUILDING_H

#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "../platform.h"
#include "resources.h"
#include "buildingtype.h"
#include "../render/vertexarray.h"

class VertexArray;

class Building
{
public:
	bool on;
	int type;
	int owner;

	Vec2i tilepos;	//position in tiles
	Vec3f drawpos;	//drawing position in centimeters

	bool finished;

	int pownetw;
	int crpipenetw;
	std::list<int> roadnetw;

	int stocked[RESOURCES];
	int inuse[RESOURCES];

	EmitterCounter emitterco[MAX_B_EMITTERS];

	VertexArray drawva;

	int conmat[RESOURCES];
	int maxcost[RESOURCES];
	bool inoperation;

	void destroy();
	void fillcollider();
	void freecollider();
	void allocres();
	void remesh();
	bool checkconstruction();
	Building();
	~Building();
};

#define BUILDINGS	256

extern Building g_building[BUILDINGS];

int NewBuilding();
void FreeBuildings();
void DrawBl();
void UpdateBuildings();
void StageCopyVA(VertexArray* to, VertexArray* from, float completion);
void HeightCopyVA(VertexArray* to, VertexArray* from, float completion);
void HugTerrain(VertexArray* va, Vec3f pos);
void Explode(Building* b);
float CompletPct(int* cost, int* current);

#endif
