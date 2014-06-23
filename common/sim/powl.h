//
//  powl.h
//  corpstates
//
//  Created by polyf Ivanov on 2013-05-30.
//  Copyright (c) 2013 DMD 'Ware. All rights reserved.
//


#ifndef POWERLINE_H
#define POWERLINE_H

#include "connectable.h"
#include "resources.h"
#include "../render/model.h"
#include "../render/vertexarray.h"
#include "../math/vec3i.h"

class Shader;

class PowlTileType
{
public:
	void draw(int x, int z);
	void Define(const char* modelfile);

	int model;
};

extern PowlTileType g_powlT[CONNECTION_TYPES][2];

class PowlTile
{
public:
	bool on;
	int type;
	bool finished;
	int stateowner;
	int conmat[RESOURCES];
	int netw;	//power network
	VertexArray drawva;
	//bool inaccessible;
	int transporter[RESOURCES];
	Vec3f drawpos;
	int maxcost[RESOURCES];

	PowlTile();
	~PowlTile();

	int netreq(int res);
	void destroy();
	void allocate();
	bool checkconstruction();
};

extern PowlTile* g_powl;
extern PowlTile* g_powlplan;
extern int g_powlcost[RESOURCES];

void DefinePowl(int type, bool finished, const char* modelfile);
bool PowlHasRoad(int x, int z);
void PlacePowl(int x, int z, int stateowner, bool plan=false);
bool PowlPlaceable(int x, int z);
Vec3f PowlPosition(int x, int z);
Vec3i PowlPhysPos(int x, int z);
PowlTile* PowlAt(int x, int z);
PowlTile* PowlPlanAt(int x, int z);
void MeshPowl(int x, int z, bool plan=false);
void DrawPowls();
void PlacePowl();
void UpdatePowlPlans(int stateowner, Vec3f start, Vec3f end);
void ClearPowls();
void RePow();
bool PowlIntersect(int x, int z, Vec3f line[]);
void TypePowlsAround(int x, int z, bool plan=false);
void TypePowl(int x, int z, bool plan=false);
void ClearPowlPlans();

#endif

