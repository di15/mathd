


#ifndef POWERLINE_H
#define POWERLINE_H

#pragma warning(disable: 4244)

#include "connection.h"
//#include "map.h"
#include "resource.h"
#include "model.h"

class Shader;

class CPowerlineType
{
public:
	void draw(int x, int z);
	void Define(char* modelfile);

	int model;
};

extern CPowerlineType g_powerlineType[CONNECTION_TYPES][2];

class CPowerline
{
public:
	bool on;
	int type;
	bool finished;
	int owner;
	float conmat[RESOURCES];
	int netw;	//power network
	CVertexArray vertexArray;
	//bool inaccessible;
	int transporter[RESOURCES];

	CPowerline();
	~CPowerline();

	float netreq(int res);
	void destroy();
	void Allocate();
	void Emit(int t);
	bool CheckConstruction();
};

extern CPowerline* g_powerline;
extern CPowerline* g_powerlinePlan;
extern float g_powerlineCost[RESOURCES];

bool PowlHasRoad(int x, int z);
void PlacePowerline(int x, int z, int owner, bool plan=false);
bool PowerlinePlaceable(int x, int z);
Vec3f PowerlinePosition(int x, int z);
CPowerline* PowlAt(int x, int z);
CPowerline* PowlPlanAt(int x, int z);
void MeshPowl(int x, int z, bool plan=false);
void InitPowerlines();
void DrawPowerlines();
void PlacePowerline();
void UpdatePowerlinePlans();
void ClearPowerlines();
void RePow();
bool PowlIntersect(int x, int z, Vec3f line[]);
void TypePowerlinesAround(int x, int z, bool plan=false);
void TypePowerline(int x, int z, bool plan=false);

#endif