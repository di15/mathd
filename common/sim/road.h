
#ifndef _ROAD_H
#define _ROAD_H

#pragma warning(disable: 4244)

#include "connection.h"
//#include "map.h"
#include "resource.h"
#include "model.h"

class Shader;

class CRoadType
{
public:
	void draw(int x, int z);
	void Define(char* modelfile);

	int model;
};

extern CRoadType g_roadType[CONNECTION_TYPES][2];

class CRoad
{
public:
	bool on;
	int type;
	bool finished;
	int owner;
	float conmat[RESOURCES];
	int netw;
	CVertexArray vertexArray;
	int transporter[RESOURCES];

	CRoad();
	~CRoad();

	float netreq(int res);
	void destroy();
	void Allocate();
	bool CheckConstruction();
	void Emit(int t);
	void fillcollidercells();
	void freecollidercells();
};

#define ROAD_MAX_FOREW_INCLINE		(TILE_SIZE/2)
#define ROAD_MAX_SIDEW_INCLINE		(TILE_SIZE/16)

extern CRoad* g_road;
extern CRoad* g_roadPlan;
extern float g_roadCost[RESOURCES];

void PlaceRoad(int x, int z, int owner, bool plan=false);
bool RoadPlaceable(int x, int z);
void ReRoadNetw();
CRoad* RoadPlanAt(int x, int z);
bool RoadLevel(float iterx, float iterz, float testx, float testz, float dx=0, float dz=0, int i=0, float d=0, CRoad* (*planfunc)(int x, int z)=RoadPlanAt);
void RoadXZ(CRoad* r, int& x, int& z);
CRoad* RoadAt(int x, int z);
void MeshRoad(int x, int z, bool plan=false);
Vec3f RoadPosition(int x, int z);
void InitRoads();
void DrawRoads();
void PlaceRoad();
void UpdateRoadPlans();
bool RoadIntersect(int x, int z, Vec3f line[]);
void TypeRoadsAround(int x, int z, bool plan=false);
void TypeRoad(int x, int z, bool plan=false);

#endif