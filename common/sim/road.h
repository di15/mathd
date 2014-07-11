#ifndef ROAD_H
#define ROAD_H

#include "connectable.h"
#include "resources.h"
#include "../render/vertexarray.h"
#include "../render/model.h"
#include "../math/vec3i.h"
#include "infrastructure.h"


class RoadTile : ConduitTile
{
public:
	bool on;
	int type;
	bool finished;
	int stateowner;
	int conmat[RESOURCES];
	int netw;
	VertexArray drawva;
	int transporter[RESOURCES];
	Vec3f drawpos;
	int maxcost[RESOURCES];

	RoadTile();
	~RoadTile();

	char condtype();
	int netreq(int res);
	void destroy();
	void allocate();
	bool checkconstruction();
	void fillcollider();
	void freecollider();
};

#define ROAD_MAX_FOREW_INCLINE		(TILE_SIZE)
#define ROAD_MAX_SIDEW_INCLINE		(TILE_SIZE/2)

extern RoadTile* g_road;
extern RoadTile* g_roadplan;
extern int g_roadcost[RESOURCES];

void DefRoad(int type, bool finished, const char* modelfile);
void PlaceRoad(int x, int z, int stateowner, bool plan=false);
bool RoadPlaceable(int x, int z);
void ReRoadNetw();
RoadTile* RoadPlanAt(int x, int z);
bool RoadLevel(float iterx, float iterz, float testx, float testz, float dx=0, float dz=0, int i=0, float d=0, RoadTile* (*planfunc)(int x, int z)=RoadPlanAt);
void RoadXZ(RoadTile* r, int& x, int& z);
RoadTile* RoadAt(int x, int z);
void MeshRoad(int x, int z, bool plan=false);
Vec3f RoadPosition(int x, int z);
Vec3i RoadPhysPos(int x, int z);
void InitRoads();
void DrawRoads();
void PlaceRoad();
void UpdateRoadPlans(int stateowner, Vec3f start, Vec3f end);
bool RoadIntersect(int x, int z, Vec3f line[]);
void TypeRoadsAround(int x, int z, bool plan=false);
void TypeRoad(int x, int z, bool plan=false);
Vec2i RoadPos(RoadTile* r);
Vec2i RoadPlanPos(RoadTile* r);
void ClearRoadPlans();

#endif

