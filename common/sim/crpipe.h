//
//  pipeline.h
//  corpstates
//
//  Created by polyf on 2013-05-30.
//  Copyright (c) 2013 DMD 'Ware. All rights reserved.
//


#ifndef PIPELINE_H
#define PIPELINE_H

#include "connectable.h"
#include "resources.h"
#include "../render/model.h"
#include "../render/vertexarray.h"
#include "../math/vec3i.h"
#include "infrastructure.h"

class CrPipeTile : ConduitTile
{
public:
	bool on;
	int type;
	bool finished;
	int stateowner;
	int conmat[RESOURCES];
	int netw;	//pipe network
	VertexArray drawva;
	//bool inaccessible;
	int transporter[RESOURCES];
	Vec3f drawpos;
	int maxcost[RESOURCES];

	CrPipeTile();
	~CrPipeTile();

	char condtype();
	int netreq(int res);
	void destroy();
	void allocate();
	bool checkconstruction();
};

extern CrPipeTile* g_crpipe;
extern CrPipeTile* g_crpipeplan;
extern int g_crpipecost[RESOURCES];

void DefCrPipe(int type, bool finished, const char* modelfile);
bool CrPipeHasRoad(int x, int z);
void PlaceCrPipe(int x, int z, int stateowner, bool plan=false);
bool CrPipePlaceable(int x, int z);
CrPipeTile* CrPipeAt(int x, int z);
CrPipeTile* CrPipePlanAt(int x, int z);
void MeshCrPipe(int x, int z, bool plan=false);
Vec3i CrPipePhysPos(int x, int z);
Vec3f CrPipeDrawPos(int x, int z);
void DrawCrPipes();
void PlaceCrPipe();
void UpdateCrPipePlans(int stateowner, Vec3f start, Vec3f end);
void ClearCrPipes();
void ReCrPipe();
bool CrPipeIntersect(int x, int z, Vec3f line[]);
void TypeCrPipesAround(int x, int z, bool plan=false);
void TypeCrPipe(int x, int z, bool plan=false);
void ClearCrPipePlans();

#endif

