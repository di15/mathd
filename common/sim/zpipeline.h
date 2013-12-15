

#ifndef PIPELINE_H
#define PIPELINE_H

#pragma warning(disable: 4244)

#include "connection.h"
//#include "map.h"
#include "resource.h"
#include "model.h"

class Shader;

class CPipelineType
{
public:
	void draw(int x, int z);
	void Define(char* modelfile);

	int model;
};

extern CPipelineType g_pipelineType[CONNECTION_TYPES][2];

class CPipeline
{
public:
	bool on;
	int type;
	bool finished;
	int owner;
	float conmat[RESOURCES];
	int netw;	//pipe network
	CVertexArray vertexArray;
	//bool inaccessible;
	int transporter[RESOURCES];

	CPipeline();
	~CPipeline();

	float netreq(int res);
	void destroy();
	void Allocate();
	void Emit(int t);
	bool CheckConstruction();
};

extern CPipeline* g_pipeline;
extern CPipeline* g_pipelinePlan;
extern float g_pipelineCost[RESOURCES];

bool PipeHasRoad(int x, int z);
void PlacePipeline(int x, int z, int owner, bool plan=false);
bool PipelinePlaceable(int x, int z);
CPipeline* PipeAt(int x, int z);
CPipeline* PipePlanAt(int x, int z);
void MeshPipe(int x, int z, bool plan=false);
Vec3f PipelinePhysPos(int x, int z);
Vec3f PipelinePosition(int x, int z);
void InitPipelines();
void DrawPipelines();
void PlacePipeline();
void UpdatePipelinePlans();
void ClearPipelines();
void RePipe();
bool PipeIntersect(int x, int z, Vec3f line[]);
void TypePipelinesAround(int x, int z, bool plan=false);
void TypePipeline(int x, int z, bool plan=false);

#endif