




#include "pipeline.h"
#include "map.h"
#include "main.h"
#include "physics.h"
#include "job.h"
#include "model.h"
#include "particle.h"
#include "resource.h"
#include "player.h"
#include "chat.h"
#include "shader.h"
#include "transaction.h"
#include "sound.h"
#include "script.h"
#include "road.h"

CPipelineType g_pipelineType[CONNECTION_TYPES][2];
CPipeline* g_pipeline = NULL;
CPipeline* g_pipelinePlan = NULL;
float g_pipelineCost[RESOURCES];

CPipeline::CPipeline() 
{ 
	on = false; 
	finished = false;
	Zero(conmat);
	netw = -1;
	//vertexArray.safetyConstructor();

	//g_log<<"CPowerline() vertexArray.numverts = "<<vertexArray.numverts<<endl;
	//g_log.flush();
}

CPipeline::~CPipeline()
{
	//vertexArray.safetyConstructor();
	//g_log<<"powerline calling vertexArray.free();...";
	//g_log.flush();
	//vertexArray.free();
	//g_log<<"called."<<endl;
	//g_log.flush();
	
	//g_log<<"CPowerline() delete vertexArray.numverts = "<<vertexArray.numverts<<endl;
	//g_log.flush();
}

void CPipelineType::draw(int x, int z) 
{
	Vec3f pos = PipelinePosition(x, z);
	g_model[model].draw(0, pos, 0);
}

void CPipelineType::Define(char* modelfile)
{
	QueueModel(&model, modelfile, Vec3f(1,1,1), Vec3f(0,0,0));
}

CPipeline* PipeAt(int x, int z)
{
	return &g_pipeline[ x + z*g_hmap.m_widthX ];
}

CPipeline* PipePlanAt(int x, int z)
{
	return &g_pipelinePlan[ x + z*g_hmap.m_widthX ];
}

void PipeXZ(CPipeline* pipe, int& x, int& z)
{
	int x1, z1;

	for(x1=0; x1<g_hmap.m_widthX; x1++)
		for(z1=0; z1<g_hmap.m_widthZ; z1++)
		{
			if(PipeAt(x1, z1) == pipe)
			{
				x = x1;
				z = z1;
				return;
			}
		}
}

float CPipeline::netreq(int res)
{
	float netrq = 0;

	if(!finished)
	{
		netrq = g_pipelineCost[res] - conmat[res];
	}

	return netrq;
}

void CPipeline::destroy()
{
	Zero(conmat);
	on = false;
	finished = false;
	netw = -1;
}

void CPipeline::Allocate()
{
	CPlayer* p = &g_player[owner];

	float alloc;

	char transx[32];
	transx[0] = '\0';

	for(int i=0; i<RESOURCES; i++)
	{
		if(g_pipelineCost[i] <= 0)
			continue;

		if(i == LABOUR)
			continue;

		alloc = g_pipelineCost[i] - conmat[i];

		if(p->global[i] < alloc)
			alloc = p->global[i];

		conmat[i] += alloc;
		p->global[i] -= alloc;

		if(alloc > 0.0f)
			TransxAppend(transx, i, -alloc);
	}

	if(transx[0] != '\0'
#ifdef LOCAL_TRANSX
		&& owner == g_localP
#endif
		)
	{
		int x, z;
		PipeXZ(this, x, z);
		NewTransx(PipelinePosition(x, z), transx);
	}

	CheckConstruction();
}

bool CPipeline::CheckConstruction()
{
	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < g_pipelineCost[i])
			return false;

	if(owner == g_localP)
	{
		Chat("Pipe construction complete.");
		ConCom();
	}

	finished = true;
	RePipe();

	int x, z;
	PipeXZ(this, x, z);
	MeshPipe(x, z);
	
	if(owner == g_localP)
		OnFinishedB(PIPELINE);

	return true;
}

void CPipeline::Emit(int t)
{
	Vec3f pos;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			if(PipeAt(x, z) != this)
				continue;
			
			pos = PipelinePosition(x, z);
			EmitParticle(t, pos + Vec3f(0, 10, 0));
			return;
		}
}

bool PipeHasRoad(int x, int z)
{
	Vec3f physpos = PipelinePhysPos(x, z);

	Vec3f roadtopleft = physpos + Vec3f(-TILE_SIZE/2, 0, -TILE_SIZE/2);
	//Vec3f roadtopright = physpos + Vec3f(TILE_SIZE/2, 0, -TILE_SIZE/2);
	//Vec3f roadbottomleft = physpos + Vec3f(-TILE_SIZE/2, 0, TILE_SIZE/2);
	Vec3f roadbottomright = physpos + Vec3f(TILE_SIZE/2, 0, TILE_SIZE/2);

	int tileleft = roadtopleft.x / TILE_SIZE;
	int tiletop = roadtopleft.z / TILE_SIZE;
	int tileright = roadbottomright.x / TILE_SIZE;
	int tilebottom = roadbottomright.z / TILE_SIZE;

	if(tileleft >= 0 && tiletop >= 0)
	{
		if(RoadAt(tileleft, tiletop)->on)
			return true;
	}
	if(tileleft >= 0 && tilebottom < g_hmap.m_widthZ)
	{
		if(RoadAt(tileleft, tilebottom)->on)
			return true;
	}
	if(tileright < g_hmap.m_widthX && tiletop >= 0)
	{
		if(RoadAt(tileright, tiletop)->on)
			return true;
	}
	if(tileright < g_hmap.m_widthX && tilebottom < g_hmap.m_widthZ)
	{
		if(RoadAt(tileright, tilebottom)->on)
			return true;
	}

	return false;
}

Vec3f PipelinePhysPos(int x, int z)
{
	//return Vec3f(x*TILE_SIZE, 0, z*TILE_SIZE);
	float fx = (x+1)*TILE_SIZE;
	float fz = (z)*TILE_SIZE;
	//float fy = Bilerp(fx, fz);

	return Vec3f(fx, 0, fz);
}

Vec3f PipelinePosition(int x, int z)
{
	//return Vec3f(x*TILE_SIZE, 0, z*TILE_SIZE);
	float fx = (x+0.5f)*TILE_SIZE;
	float fz = (z+0.5f)*TILE_SIZE;
	//float fy = Bilerp(fx, fz);

	return Vec3f(fx, 0, fz);
}

void InitPipelines()
{
	Zero(g_pipelineCost);
	g_pipelineCost[LABOUR] = 1;
	g_pipelineCost[METAL] = 1;

	g_pipelineType[NOCONNECTION][CONSTRUCTION].Define("models\\pipeline\\1_c.ms3d");
	g_pipelineType[NORTH][CONSTRUCTION].Define("models\\pipeline\\n_c.ms3d");
	g_pipelineType[EAST][CONSTRUCTION].Define("models\\pipeline\\e_c.ms3d");
	g_pipelineType[SOUTH][CONSTRUCTION].Define("models\\pipeline\\s_c.ms3d");
	g_pipelineType[WEST][CONSTRUCTION].Define("models\\pipeline\\w_c.ms3d");
	g_pipelineType[NORTHEAST][CONSTRUCTION].Define("models\\pipeline\\ne_c.ms3d");
	g_pipelineType[NORTHSOUTH][CONSTRUCTION].Define("models\\pipeline\\ns_c.ms3d");
	g_pipelineType[EASTSOUTH][CONSTRUCTION].Define("models\\pipeline\\es_c.ms3d");
	g_pipelineType[NORTHWEST][CONSTRUCTION].Define("models\\pipeline\\nw_c.ms3d");
	g_pipelineType[EASTWEST][CONSTRUCTION].Define("models\\pipeline\\ew_c.ms3d");
	g_pipelineType[SOUTHWEST][CONSTRUCTION].Define("models\\pipeline\\sw_c.ms3d");
	g_pipelineType[EASTSOUTHWEST][CONSTRUCTION].Define("models\\pipeline\\esw_c.ms3d");
	g_pipelineType[NORTHSOUTHWEST][CONSTRUCTION].Define("models\\pipeline\\nsw_c.ms3d");
	g_pipelineType[NORTHEASTWEST][CONSTRUCTION].Define("models\\pipeline\\new_c.ms3d");
	g_pipelineType[NORTHEASTSOUTH][CONSTRUCTION].Define("models\\pipeline\\nes_c.ms3d");
	g_pipelineType[NORTHEASTSOUTHWEST][CONSTRUCTION].Define("models\\pipeline\\nesw_c.ms3d");
	g_pipelineType[NOCONNECTION][FINISHED].Define("models\\pipeline\\1.ms3d");
	g_pipelineType[NORTH][FINISHED].Define("models\\pipeline\\n.ms3d");
	g_pipelineType[EAST][FINISHED].Define("models\\pipeline\\e.ms3d");
	g_pipelineType[SOUTH][FINISHED].Define("models\\pipeline\\s.ms3d");
	g_pipelineType[WEST][FINISHED].Define("models\\pipeline\\w.ms3d");
	g_pipelineType[NORTHEAST][FINISHED].Define("models\\pipeline\\ne.ms3d");
	g_pipelineType[NORTHSOUTH][FINISHED].Define("models\\pipeline\\ns.ms3d");
	g_pipelineType[EASTSOUTH][FINISHED].Define("models\\pipeline\\es.ms3d");
	g_pipelineType[NORTHWEST][FINISHED].Define("models\\pipeline\\nw.ms3d");
	g_pipelineType[EASTWEST][FINISHED].Define("models\\pipeline\\ew.ms3d");
	g_pipelineType[SOUTHWEST][FINISHED].Define("models\\pipeline\\sw.ms3d");
	g_pipelineType[EASTSOUTHWEST][FINISHED].Define("models\\pipeline\\esw.ms3d");
	g_pipelineType[NORTHSOUTHWEST][FINISHED].Define("models\\pipeline\\nsw.ms3d");
	g_pipelineType[NORTHEASTWEST][FINISHED].Define("models\\pipeline\\new.ms3d");
	g_pipelineType[NORTHEASTSOUTH][FINISHED].Define("models\\pipeline\\nes.ms3d");
	g_pipelineType[NORTHEASTSOUTHWEST][FINISHED].Define("models\\pipeline\\nesw.ms3d");
}

void DrawPipeline(int x, int z, bool plan)
{
	CPipeline* p;
	if(plan)
		p = PipePlanAt(x, z);
	else
		p = PipeAt(x, z);
	
	if(!p->on)
		return;

	Vec3f pos = PipelinePosition(x, z);
	
	CModel* m;
	int model;

	if(p->finished)
		model = g_pipelineType[p->type][FINISHED].model;
	else
		model = g_pipelineType[p->type][CONSTRUCTION].model;

	const float* owncol = facowncolor[p->owner];
	glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

	m = &g_model[model];
	m->UseTex();

	DrawVA(&p->vertexArray, pos);
}

void DrawPipelines()
{
	//StartProfile(DRAWPOWERLINES);

	//int type;
	//CPipeline* p;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			/*
			p = PipeAt(x, z);

			if(!p->on)
				continue;
			
			const float* owncol = facowncolor[p->owner];
			glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

			//type = p->type;
			//if(p->finished)
			//	g_pipelineType[type][FINISHED].draw(x, z);
			//else
			//	g_pipelineType[type][CONSTRUCTION].draw(x, z);
			DrawVA(&p->vertexArray, PipelinePosition(x, z));*/
			DrawPipeline(x, z, false);
		}

	if(g_build != PIPELINE)
		return;

	//glColor4f(1,1,1,0.5f);
	glUniform4f(g_shader[g_curS].m_slot[SLOT::COLOR], 1, 1, 1, 0.5f);
		
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			/*
			p = PipePlanAt(x, z);
			if(!p->on)
				continue;

			const float* owncol = facowncolor[p->owner];
			glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

			//type = p->type;
			//g_pipelineType[type][FINISHED].draw(x, z);
			DrawVA(&p->vertexArray, PipelinePosition(x, z));*/
			DrawPipeline(x, z, true);
		}

	//glColor4f(1,1,1,1);
	glUniform4f(g_shader[g_curS].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	//EndProfile(DRAWPOWERLINES);
}

int GetPipelineType(int x, int z, bool plan=false)
{
	bool n = false, e = false, s = false, w = false;

	if(x+1 < g_hmap.m_widthX && PipeAt(x+1, z)->on)
		e = true;
	if(x-1 >= 0 && PipeAt(x-1, z)->on)
		w = true;
	
	if(z+1 < g_hmap.m_widthZ && PipeAt(x, z+1)->on)
		s = true;
	if(z-1 >= 0 && PipeAt(x, z-1)->on)
		n = true;

	if(plan)
	{
		if(x+1 < g_hmap.m_widthX && PipePlanAt(x+1, z)->on)
			e = true;
		if(x-1 >= 0 && PipePlanAt(x-1, z)->on)
			w = true;
	
		if(z+1 < g_hmap.m_widthZ && PipePlanAt(x, z+1)->on)
			s = true;
		if(z-1 >= 0 && PipePlanAt(x, z-1)->on)
			n = true;
	}

	return GetConnectionType(n, e, s, w);
}

void MeshPipe(int x, int z, bool plan)
{
	CPipeline* p = PipeAt(x, z);
	if(plan)
		p = PipePlanAt(x, z);

	if(!p->on)
		return;

	int finished = 0;
	if(p->finished)
		finished = 1;

	CPipelineType* t = &g_pipelineType[p->type][finished];
	CModel* m = &g_model[t->model];

	CVertexArray* pva = &p->vertexArray;
	CVertexArray* mva = &m->vertexArrays[0];

	pva->free();

	//g_log<<"meshpowl allocating "<<mva->numverts<<"...";
	//g_log.flush();

	pva->numverts = mva->numverts;
	pva->vertices = new Vec3f[ pva->numverts ];
	pva->texcoords = new Vec2f[ pva->numverts ];
	pva->normals = new Vec3f[ pva->numverts ];

	float realx, realz;
	Vec3f pos = PipelinePosition(x, z);
	
	for(int i=0; i<pva->numverts; i++)
	{
		pva->vertices[i] = mva->vertices[i];
		pva->texcoords[i] = mva->texcoords[i];
		pva->normals[i] = mva->normals[i];
		
		//realx = ((float)x)*(float)TILE_SIZE + pva->vertices[i].x;
		//realz = ((float)z)*(float)TILE_SIZE + pva->vertices[i].z;
		realx = pos.x + pva->vertices[i].x;
		realz = pos.z + pva->vertices[i].z;

		pva->vertices[i].y += Bilerp(realx, realz);

		//char msg[128];
		//sprintf(msg, "(%f,%f,%f)", mva->vertices[i].x, mva->vertices[i].y, mva->vertices[i].z);
		//Chat(msg);
	}
	
	//g_log<<"done meshpowl"<<endl;
	//g_log.flush();
}

void TypePipeline(int x, int z, bool plan)
{
	CPipeline* p = PipeAt(x, z);
	if(plan)
		p = PipePlanAt(x, z);

	if(!p->on)
		return;

	p->type = GetPipelineType(x, z, plan);
	MeshPipe(x, z, plan);
}

void TypePipelinesAround(int x, int z, bool plan)
{
	if(x+1 < g_hmap.m_widthX)
		TypePipeline(x+1, z, plan);
	if(x-1 >= 0)
		TypePipeline(x-1, z, plan);
	
	if(z+1 < g_hmap.m_widthZ)
		TypePipeline(x, z+1, plan);
	if(z-1 >= 0)
		TypePipeline(x, z-1, plan);
}

bool PipelinePlaceable(int x, int z)
{
	//Vec3f pos = PipelinePosition(x, z);
	Vec3f pp = PipelinePhysPos(x, z);

	if(TileUnclimable(pp.x, pp.z))
		return false;

	//if(CollidesWithBuildings(pos.x, pos.z, 0, 0))
	if(CollidesWithBuildings(pp.x, pp.z, 0, 0))
		return false;

	//Make sure the powerline isn't surrounded by buildings or map edges
	if( ((x<=0 || z<=0) || CollidesWithBuildings(pp.x+(-0.5f)*(float)TILE_SIZE, pp.z+(-0.5f)*(float)TILE_SIZE, 0, 0)) &&
		((x<=0 || z>=g_hmap.m_widthZ-1) || CollidesWithBuildings(pp.x+(-0.5f)*(float)TILE_SIZE, pp.z+(0.5f)*(float)TILE_SIZE, 0, 0)) &&
		((x>=g_hmap.m_widthX-1 || z>=g_hmap.m_widthZ-1) || CollidesWithBuildings(pp.x+(0.5f)*TILE_SIZE, pp.z+(0.5f)*(float)TILE_SIZE, 0, 0)) &&
		((x>=g_hmap.m_widthX-1 || z<=0) || CollidesWithBuildings(pp.x+(0.5f)*(float)TILE_SIZE, pp.z+(-0.5f)*(float)TILE_SIZE, 0, 0)) )
	{
		//char msg[128];
		//sprintf(msg, "!powp @ %d", (int)GetTickCount());
		//Chat(msg);

		return false;
	}

	return true;
}

void RepossessPipe(int x, int z, int owner)
{
	CPipeline* pipe = PipeAt(x, z);
	pipe->owner = owner;
	pipe->Allocate();
}

void PlacePipeline(int x, int z, int owner, bool plan)
{
	if(PipeAt(x, z)->on)
	{
		if(!plan)
		{
			RepossessPipe(x, z, owner);

			return;
		}
	}

	if(!PipelinePlaceable(x, z))
		return;

	CPipeline* p = PipeAt(x, z);
	if(plan)
		p = PipePlanAt(x, z);

	p->on = true;
	p->owner = owner;
	p->netw = -1;

	if(!plan && g_mode == PLAY)
		p->Allocate();
	
	if(g_mode == PLAY)
		p->finished = false;
	if(plan || g_mode == EDITOR)
		p->finished = true;

	TypePipeline(x, z, plan);
	TypePipelinesAround(x, z, plan);

	for(int i=0; i<RESOURCES; i++)
		p->transporter[i] = -1;
}

void ClearPipelinePlans()
{
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			PipePlanAt(x, z)->on = false;
		}
}

void PlacePipeline()
{
	CPipeline* p;
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			p = PipePlanAt(x, z);
			if(p->on)
			{
				CPipeline* actual = PipeAt(x, z);

				bool willchange = !actual->on;

				PlacePipeline(x, z, p->owner);
				
				if(g_mode == PLAY && willchange)
				{
					NewJob(GOINGTOPIPEJOB, x, z);
				}
			}
		}

	ClearPipelinePlans();
	RePipe();
}

void UpdatePipelinePlans()
{
	ClearPipelinePlans();
	
	int x1 = Clip(g_vStart.x/TILE_SIZE, 0, g_hmap.m_widthX-1);
	int z1 = Clip(g_vStart.z/TILE_SIZE, 0, g_hmap.m_widthZ-1);

	int x2 = Clip(g_vTile.x/TILE_SIZE, 0, g_hmap.m_widthX-1);
	int z2 = Clip(g_vTile.z/TILE_SIZE, 0, g_hmap.m_widthZ-1);

	if(!g_mousekeys[0])
	{
		x1 = x2;
		z1 = z2;
	}

	PlacePipeline(x1, z1, g_selP, true);
	PlacePipeline(x2, z2, g_selP, true);

	float dx = x2-x1;
	float dz = z2-z1;

	float d = sqrtf(dx*dx + dz*dz);

	dx /= d;
	dz /= d;

	int prevx = x1;
	int prevz = z1;

	int i = 0;

	for(float x=x1, z=z1; i<=d; x+=dx, z+=dz, i++)
	{
		PlacePipeline(x, z, g_selP, true);

		if((int)x != prevx && (int)z != prevz)
			PlacePipeline(prevx, z, g_selP, true);

		prevx = x;
		prevz = z;
	}
	
	if((int)x2 != prevx && (int)z2 != prevz)
		PlacePipeline(prevx, z2, g_selP, true);
	
	for(x1=0; x1<g_hmap.m_widthX; x1++)
		for(z1=0; z1<g_hmap.m_widthZ; z1++)
			MeshPipe(x1, z1, true);
}

void ClearPipelines()
{
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
			if(PipeAt(x, z)->on && !PipelinePlaceable(x, z))
			{
				PipeAt(x, z)->on = false;
				TypePipelinesAround(x, z);
			}
}


void MergePipe(int A, int B)
{
	int mini = min(A, B);
	int maxi = max(A, B);

	CBuilding* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(b->pipenetw == maxi)
			b->pipenetw = mini;
	}

	CPipeline* pipe;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			pipe = PipeAt(x, z);

			if(!pipe->on)
				continue;

			if(!pipe->finished)
				continue;

			if(pipe->netw == maxi)
				pipe->netw = mini;
		}
}

bool RePipeB()
{
	bool change = false;

	CBuilding* b;
	CBuilding* b2;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;
		
		if(!ResB(i, CRUDE))
			continue;

		for(int j=0; j<BUILDINGS; j++)
		{
			if(j == i)
				continue;

			b2 = &g_building[j];

			if(!b2->on)
				continue;

			if(!ResB(j, CRUDE))
				continue;

			if(!BuildingAdjacent(i, j))
				continue;

			if(b->pipenetw < 0 && b2->pipenetw >= 0)
			{
				b->pipenetw = b2->pipenetw;
				change = true;
			}
			else if(b2->pipenetw < 0 && b->pipenetw >= 0)
			{
				b2->pipenetw = b->pipenetw;
				change = true;
			}
			else if(b->pipenetw >= 0 && b2->pipenetw >= 0 && b->pipenetw != b2->pipenetw)
			{
				MergePipe(b->pipenetw, b2->pipenetw);
				change = true;
			}
		}
	}

	return change;
}

bool ComparePipe(CPipeline* pipe, int x, int z)
{
	CPipeline* pipe2 = PipeAt(x, z);

	if(!pipe2->on)
		return false;

	if(!pipe2->finished)
		return false;

	if(pipe2->netw < 0 && pipe->netw >= 0)
	{
		pipe2->netw = pipe->netw;
		return true;
	}
	else if(pipe->netw < 0 && pipe2->netw >= 0)
	{
		pipe->netw = pipe2->netw;
		return true;
	}
	else if(pipe->netw >= 0 && pipe2->netw >= 0 && pipe->netw != pipe2->netw)
	{
		MergePipe(pipe->netw, pipe2->netw);
		return true;
	}

	return false;
}

bool CompareBPipe(CBuilding* b, CPipeline* pipe)
{
	if(b->pipenetw < 0 && pipe->netw >= 0)
	{
		b->pipenetw = pipe->netw;
		return true;
	}
	else if(pipe->netw < 0 && b->pipenetw >= 0)
	{
		pipe->netw = b->pipenetw;
		return true;
	}
	else if(pipe->netw >= 0 && b->pipenetw >= 0 && pipe->netw != b->pipenetw)
	{
		MergePipe(pipe->netw, b->pipenetw);
		return true;
	}

	return false;
}

bool RePipePipe()
{
	bool change = false;

	CPipeline* pipe;
	CBuilding* b;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			pipe = PipeAt(x, z);

			if(!pipe->on)
				continue;

			if(!pipe->finished)
				continue;

			//g_log<<"x,z "<<x<<","<<z<<endl;
			//g_log.flush();

			if(x > 0 && ComparePipe(pipe, x-1, z))
				change = true;
			if(x < g_hmap.m_widthX-1 && ComparePipe(pipe, x+1, z))
				change = true;
			if(z > 0 && ComparePipe(pipe, x, z-1))
				change = true;
			if(z < g_hmap.m_widthZ-1 && ComparePipe(pipe, x, z+1))
				change = true;

			for(int i=0; i<BUILDINGS; i++)
			{
				b = &g_building[i];

				if(!b->on)
					continue;

				if(!PipelineAdjacent(i, x, z))
					continue;

				//g_log<<"pipeline adjacent "<<g_buildingType[b->type].name<<endl;

				if(!ResB(i, CRUDE))
					continue;

				//g_log<<"pipeline adjacent "<<g_buildingType[b->type].name<<endl;

				if(CompareBPipe(b, pipe))
				{
					//g_log<<"pipe change"<<endl;
					change = true;
				}
			}
		}

	return change;
}

void ResetPipe()
{
	int lastnetw = 0;
	CBuilding* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		b->pipenetw = -1;
	}

	CPipeline* pipe;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			pipe = PipeAt(x, z);

			if(!pipe->on)
				continue;

			if(!pipe->finished)
				continue;

			pipe->netw = -1;
		}
	
	CBuildingType* t;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		t = &g_buildingType[b->type];

		if(t->output[CRUDE] <= 0.0f)
			continue;

		b->pipenetw = lastnetw++;
	}
}

//Recalculate power networks
void RePipe()
{
	ResetPipe();

	bool change;

	do
	{
		change = false;

		if(RePipeB())
			change = true;

		if(RePipePipe())
			change = true;
	}while(change);
}

bool PipeIntersect(int x, int z, Vec3f line[])
{
	CPipeline* p = PipeAt(x, z);

	if(!p->on)
		return false;

	int finished = 0;
	if(p->finished)
		finished = 1;

	CPipelineType* t = &g_pipelineType[p->type][finished];
	CModel* m = &g_model[t->model];

	CVertexArray* va = &m->vertexArrays[0];
	Vec3f poly[3];

	Vec3f pos = PipelinePosition(x, z);

	for(int v=0; v<va->numverts; v+=3)
	{
		poly[0] = va->vertices[v+0] + pos;
		poly[1] = va->vertices[v+1] + pos;
		poly[2] = va->vertices[v+2] + pos;

		if(IntersectedPolygon(poly, line, 3))
			return true;
	}

	return false;
}