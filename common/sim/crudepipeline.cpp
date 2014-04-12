//
//  pipeline.m
//  corpstates
//
//  Created by Denis Ivanov on 2013-05-30.
//  Copyright (c) 2013 DMD 'Ware. All rights reserved.
//

#include "crudepipeline.h"
#include "../render/heightmap.h"
#include "../render/shader.h"
#include "../phys/collision.h"
#include "../render/model.h"
#include "../render/vertexarray.h"
#include "country.h"
#include "building.h"
#include "road.h"
#include "../math/hmapmath.h"
#include "buildingtype.h"
#include "building.h"
#include "../../game/gmain.h"
#include "../../game/gui/ggui.h"
#include "../math/polygon.h"

CrPipeTileType g_crpipeT[CONNECTION_TYPES][2];
CrPipeTile* g_crpipe = NULL;
CrPipeTile* g_crpipeplan = NULL;
int g_crpipecost[RESOURCES];

CrPipeTile::CrPipeTile()
{
	on = false;
	finished = false;
	Zero(conmat);
	netw = -1;
	//drawva.safetyConstructor();
    
	//g_log<<"PowlTile() drawva.numverts = "<<drawva.numverts<<endl;
	//g_log.flush();
}

CrPipeTile::~CrPipeTile()
{
	//drawva.safetyConstructor();
	//g_log<<"powerline calling drawva.free();...";
	//g_log.flush();
	//drawva.free();
	//g_log<<"called."<<endl;
	//g_log.flush();
    
	//g_log<<"PowlTile() delete drawva.numverts = "<<drawva.numverts<<endl;
	//g_log.flush();
}

void CrPipeTileType::draw(int x, int z)
{
	Vec3f pos = CrPipeDrawPos(x, z);
	g_model[model].draw(0, pos, 0);
}

void DefineCrPipe(int type, bool finished, const char* modelfile)
{
	CrPipeTileType* t = &g_crpipeT[type][finished];
	QueueModel(&t->model, modelfile, Vec3f(1,1,1)/16.0f*TILE_SIZE, Vec3f(0,0,TILE_SIZE/8));
}

CrPipeTile* CrPipeAt(int x, int z)
{
	return &g_crpipe[ x + z*g_hmap.m_widthx ];
}

CrPipeTile* CrPipePlanAt(int x, int z)
{
	return &g_crpipeplan[ x + z*g_hmap.m_widthx ];
}

void CrPipeXZ(CrPipeTile* pipe, int& x, int& z)
{
	int x1, z1;
    
	for(x1=0; x1<g_hmap.m_widthx; x1++)
		for(z1=0; z1<g_hmap.m_widthz; z1++)
		{
			if(CrPipeAt(x1, z1) == pipe)
			{
				x = x1;
				z = z1;
				return;
			}
		}
}

int CrPipeTile::netreq(int res)
{
	int netrq = 0;
    
	if(!finished)
	{
		netrq = g_crpipecost[res] - conmat[res];
	}
    
	return netrq;
}

void CrPipeTile::destroy()
{
	Zero(conmat);
	on = false;
	finished = false;
	netw = -1;
}

void CrPipeTile::allocate()
{
#if 0
	CPlayer* p = &g_player[owner];
    
	float alloc;
    
	char transx[32];
	transx[0] = '\0';
    
	for(int i=0; i<RESOURCES; i++)
	{
		if(g_crpipecost[i] <= 0)
			continue;
        
		if(i == LABOUR)
			continue;
        
		alloc = g_crpipecost[i] - conmat[i];
        
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
		CrPipeXZ(this, x, z);
		NewTransx(CrPipeDrawPos(x, z), transx);
	}
    
	checkconstruction();
#endif
}

bool CrPipeTile::checkconstruction()
{
	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < g_crpipecost[i])
			return false;
    
#if 0
	if(owner == g_localP)
	{
		Chat("CrPipe construction complete.");
		ConCom();
	}
    
	finished = true;
	ReCrPipe();
    
	int x, z;
	CrPipeXZ(this, x, z);
	MeshCrPipe(x, z);
    
	if(owner == g_localP)
		OnFinishedB(PIPELINE);
#endif
	return true;
}

bool CrPipeHasRoad(int x, int z)
{
	Vec3i physpos = CrPipePhysPos(x, z);
    
	Vec3i roadtopleft = physpos + Vec3i(-TILE_SIZE/2, 0, -TILE_SIZE/2);
	//Vec3f roadtopright = physpos + Vec3f(TILE_SIZE/2, 0, -TILE_SIZE/2);
	//Vec3f roadbottomleft = physpos + Vec3f(-TILE_SIZE/2, 0, TILE_SIZE/2);
	Vec3i roadbottomright = physpos + Vec3i(TILE_SIZE/2, 0, TILE_SIZE/2);
    
	int tileleft = roadtopleft.x / TILE_SIZE;
	int tiletop = roadtopleft.z / TILE_SIZE;
	int tileright = roadbottomright.x / TILE_SIZE;
	int tilebottom = roadbottomright.z / TILE_SIZE;
    
	if(tileleft >= 0 && tiletop >= 0)
	{
		if(RoadAt(tileleft, tiletop)->on)
			return true;
	}
	if(tileleft >= 0 && tilebottom < g_hmap.m_widthz)
	{
		if(RoadAt(tileleft, tilebottom)->on)
			return true;
	}
	if(tileright < g_hmap.m_widthx && tiletop >= 0)
	{
		if(RoadAt(tileright, tiletop)->on)
			return true;
	}
	if(tileright < g_hmap.m_widthx && tilebottom < g_hmap.m_widthz)
	{
		if(RoadAt(tileright, tilebottom)->on)
			return true;
	}
    
	return false;
}

Vec3i CrPipePhysPos(int x, int z)
{
	//return Vec3f(x*TILE_SIZE, 0, z*TILE_SIZE);
	int cmx = (x+1)*TILE_SIZE;
	int cmz = (z)*TILE_SIZE;
	//float fy = Bilerp(fx, fz);
    
	return Vec3i(cmx, 0, cmz);
}

Vec3f CrPipeDrawPos(int x, int z)
{
	//return Vec3f(x*TILE_SIZE, 0, z*TILE_SIZE);
	float fx = (x+0.5f)*TILE_SIZE;
	float fz = (z+0.5f)*TILE_SIZE;
	//float fy = Bilerp(&g_hmap, fx, fz);
    
	return Vec3f(fx, 0, fz);
}

void DrawCrPipe(int x, int z, bool plan)
{
	CrPipeTile* p;
	if(plan)
		p = CrPipePlanAt(x, z);
	else
		p = CrPipeAt(x, z);
    
	if(!p->on)
	{
		return;
	}

#if 0
	char msg[1204];
	sprintf(msg, "dr %f,%f,%f", p->drawpos.x, p->drawpos.y + Bilerp(&g_hmap, p->drawpos.x, p->drawpos.z), p->drawpos.z);
	MessageBox(g_hWnd, msg, "asd", NULL);
#endif
    
	const float* owncol = g_country[p->stateowner].colorcode;
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);
    
	int model = g_crpipeT[p->type][p->finished].model;
	//int model = g_crpipeT[p->type][1].model;

	Model* m = &g_model[model];
	m->usetex();
    
	DrawVA(&p->drawva, p->drawpos);
}

void DrawCrPipes()
{
	//StartProfile(DRAWPOWERLINES);
    
	//int type;
	//CrPipeTile* p;
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			/*
             p = CrPipeAt(x, z);
             
             if(!p->on)
             continue;
             
             const float* owncol = facowncolor[p->owner];
             glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);
             
             //type = p->type;
             //if(p->finished)
             //	g_crpipeT[type][FINISHED].draw(x, z);
             //else
             //	g_crpipeT[type][CONSTRUCTION].draw(x, z);
             DrawVA(&p->drawva, CrPipeDrawPos(x, z));*/
			DrawCrPipe(x, z, false);
		}
    
#if 1
	if(g_build != BUILDING_CRUDEPIPE)
		return;
    
	//glColor4f(1,1,1,0.5f);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			/*
             p = CrPipePlanAt(x, z);
             if(!p->on)
             continue;
             
             const float* owncol = facowncolor[p->owner];
             glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);
             
             //type = p->type;
             //g_crpipeT[type][FINISHED].draw(x, z);
             DrawVA(&p->drawva, CrPipeDrawPos(x, z));*/
			DrawCrPipe(x, z, true);
		}
    
	//glColor4f(1,1,1,1);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
#endif
	//EndProfile(DRAWPOWERLINES);
}

int GetCrPipeType(int x, int z, bool plan=false)
{
	bool n = false, e = false, s = false, w = false;
    
	if(x+1 < g_hmap.m_widthx && CrPipeAt(x+1, z)->on)
		e = true;
	if(x-1 >= 0 && CrPipeAt(x-1, z)->on)
		w = true;
    
	if(z+1 < g_hmap.m_widthz && CrPipeAt(x, z+1)->on)
		s = true;
	if(z-1 >= 0 && CrPipeAt(x, z-1)->on)
		n = true;
    
	if(plan)
	{
		if(x+1 < g_hmap.m_widthx && CrPipePlanAt(x+1, z)->on)
			e = true;
		if(x-1 >= 0 && CrPipePlanAt(x-1, z)->on)
			w = true;
        
		if(z+1 < g_hmap.m_widthz && CrPipePlanAt(x, z+1)->on)
			s = true;
		if(z-1 >= 0 && CrPipePlanAt(x, z-1)->on)
			n = true;
	}
    
	return GetConnectionType(n, e, s, w);
}

void MeshCrPipe(int x, int z, bool plan)
{
	CrPipeTile* p = CrPipeAt(x, z);
	if(plan)
		p = CrPipePlanAt(x, z);
    
	if(!p->on)
		return;
    
	//CrPipeTileType* t = &g_crpipeT[p->type][(int)p->finished];
	CrPipeTileType* t = &g_crpipeT[p->type][1];
	Model* m = &g_model[t->model];
    
	VertexArray* pva = &p->drawva;
	VertexArray* mva = &m->m_va[0];
    
	pva->free();
    
	//g_log<<"meshpowl allocating "<<mva->numverts<<"...";
	//g_log.flush();
    
	pva->numverts = mva->numverts;
	pva->vertices = new Vec3f[ pva->numverts ];
	pva->texcoords = new Vec2f[ pva->numverts ];
	pva->normals = new Vec3f[ pva->numverts ];
    
	float realx, realz;
	p->drawpos = CrPipeDrawPos(x, z);
    
	for(int i=0; i<pva->numverts; i++)
	{
		pva->vertices[i] = mva->vertices[i];
		pva->texcoords[i] = mva->texcoords[i];
		pva->normals[i] = mva->normals[i];
        
		//realx = ((float)x)*(float)TILE_SIZE + pva->vertices[i].x;
		//realz = ((float)z)*(float)TILE_SIZE + pva->vertices[i].z;
		realx = p->drawpos.x + pva->vertices[i].x;
		realz = p->drawpos.z + pva->vertices[i].z;
        
#if 0
		pva->vertices[i].y += Bilerp(&g_hmap, realx, realz);
#else
		pva->vertices[i].y += g_hmap.accheight2(realx, realz);
#endif
        
		//char msg[128];
		//sprintf(msg, "(%f,%f,%f)", mva->vertices[i].x, mva->vertices[i].y, mva->vertices[i].z);
		//Chat(msg);
	}
    
	//g_log<<"done meshpowl"<<endl;
	//g_log.flush();
}

void TypeCrPipe(int x, int z, bool plan)
{
	CrPipeTile* p = CrPipeAt(x, z);
	if(plan)
		p = CrPipePlanAt(x, z);
    
	if(!p->on)
		return;
    
	p->type = GetCrPipeType(x, z, plan);
	MeshCrPipe(x, z, plan);
}

void TypeCrPipesAround(int x, int z, bool plan)
{
	if(x+1 < g_hmap.m_widthx)
		TypeCrPipe(x+1, z, plan);
	if(x-1 >= 0)
		TypeCrPipe(x-1, z, plan);
    
	if(z+1 < g_hmap.m_widthz)
		TypeCrPipe(x, z+1, plan);
	if(z-1 >= 0)
		TypeCrPipe(x, z-1, plan);
}

bool CrPipePlaceable(int x, int z)
{
	//Vec3f pos = CrPipeDrawPos(x, z);
	Vec3i pp = CrPipePhysPos(x, z);
    
	if(TileUnclimable(pp.x, pp.z))
		return false;
    
	//if(CollidesWithBuildings(pos.x, pos.z, 0, 0))
	if(CollidesWithBuildings(pp.x, pp.z, pp.x, pp.z))
		return false;

	// Directions here do not directly correspond to where they are drawn

	bool north_occupied = false;
	bool south_occupied = false;
	bool west_occupied = false;
	bool east_occupied = false;
	
	Vec3i north_tile_center = pp + Vec3i(-TILE_SIZE/2, 0, -TILE_SIZE/2);
	Vec3i south_tile_center = pp + Vec3i(TILE_SIZE/2, 0, TILE_SIZE/2);
	Vec3i west_tile_center = pp + Vec3i(-TILE_SIZE/2, 0, TILE_SIZE/2);
	Vec3i east_tile_center = pp + Vec3i(TILE_SIZE/2, 0, -TILE_SIZE/2);
    
	// Make sure the powerline isn't surrounded by buildings or map edges

	if(x<=0 || z<=0)
		north_occupied = true;
	else if(CollidesWithBuildings(north_tile_center.x, north_tile_center.z, north_tile_center.x, north_tile_center.z))
		north_occupied = true;

	if(x<=0 || z>=g_hmap.m_widthz-1)
		west_occupied = true;
	else if(CollidesWithBuildings(west_tile_center.x, west_tile_center.z, west_tile_center.x, west_tile_center.z))
		west_occupied = true;

	if(x>=g_hmap.m_widthx-1 || z>=g_hmap.m_widthz-1)
		south_occupied = true;
	else if(CollidesWithBuildings(south_tile_center.x, south_tile_center.z, south_tile_center.x, south_tile_center.z))
		south_occupied = true;

	if(x>=g_hmap.m_widthx-1 || z<=0)
		east_occupied = true;
	else if(CollidesWithBuildings(east_tile_center.x, east_tile_center.z, east_tile_center.x, east_tile_center.z))
		east_occupied = true;

	if( north_occupied && south_occupied && west_occupied && east_occupied )
	{
		//char msg[128];
		//sprintf(msg, "!powp @ %d", (int)GetTickCount());
		//Chat(msg);
        
		return false;
	}
    
	return true;
}

void RepossessCrPipe(int x, int z, int stateowner)
{
	CrPipeTile* pipe = CrPipeAt(x, z);
	pipe->stateowner = stateowner;
	pipe->allocate();
}

void PlaceCrPipe(int x, int z, int owner, bool plan)
{
	if(CrPipeAt(x, z)->on)
	{
		if(!plan)
		{
			RepossessCrPipe(x, z, owner);
            
			return;
		}
	}
    
	if(!CrPipePlaceable(x, z))
		return;
    
	CrPipeTile* p = CrPipeAt(x, z);
	if(plan)
		p = CrPipePlanAt(x, z);
    
#if 0
	if(plan)
		MessageBox(g_hWnd, "asd 1", "asd", NULL);
#endif

	p->on = true;
	p->stateowner = owner;
	p->netw = -1;
    
	if(!plan && g_mode == PLAY)
		p->allocate();
    
	if(g_mode == PLAY)
		p->finished = false;
	//if(plan || g_mode == EDITOR)
    if(plan)
		p->finished = true;

	p->drawpos = CrPipeDrawPos(x, z);
    
	TypeCrPipe(x, z, plan);
	TypeCrPipesAround(x, z, plan);
    
	for(int i=0; i<RESOURCES; i++)
		p->transporter[i] = -1;
}

void ClearCrPipePlans()
{
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			CrPipePlanAt(x, z)->on = false;
		}
}

void PlaceCrPipe()
{
	//MessageBox(g_hWnd, "mesh c", "asd", NULL);

	CrPipeTile* p;
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			p = CrPipePlanAt(x, z);
			if(p->on)
			{
				CrPipeTile* actual = CrPipeAt(x, z);
                
				bool willchange = !actual->on;
                
				PlaceCrPipe(x, z, p->stateowner);
                
#if 0
				if(g_mode == PLAY && willchange)
				{
					NewJob(GOINGTOPIPEJOB, x, z);
				}
#endif
			}
		}
    
	ClearCrPipePlans();
	ReCrPipe();
}

void UpdateCrPipePlans(int stateowner, Vec3f start, Vec3f end)
{
	ClearCrPipePlans();
    
	int x1 = Clipi(start.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z1 = Clipi(start.z/TILE_SIZE, 0, g_hmap.m_widthz-1);
    
	int x2 = Clipi(end.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z2 = Clipi(end.z/TILE_SIZE, 0, g_hmap.m_widthz-1);
    
	/*if(!g_mousekeys[MOUSEKEY_LEFT])
	{
		x1 = x2;
		z1 = z2;
	}*/
    
	PlaceCrPipe(x1, z1, stateowner, true);
	PlaceCrPipe(x2, z2, stateowner, true);
    
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
		PlaceCrPipe(x, z, stateowner, true);
        
		if((int)x != prevx && (int)z != prevz)
			PlaceCrPipe(prevx, z, stateowner, true);
        
		prevx = x;
		prevz = z;
	}
    
	if((int)x2 != prevx && (int)z2 != prevz)
		PlaceCrPipe(prevx, z2, stateowner, true);
    
	for(x1=0; x1<g_hmap.m_widthx; x1++)
		for(z1=0; z1<g_hmap.m_widthz; z1++)
			MeshCrPipe(x1, z1, true);
}

void ClearCrPipes()
{
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
			if(CrPipeAt(x, z)->on && !CrPipePlaceable(x, z))
			{
				CrPipeAt(x, z)->on = false;
				TypeCrPipesAround(x, z);
			}
}

void MergeCrPipe(int A, int B)
{
	int mini = min(A, B);
	int maxi = max(A, B);
    
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];
        
		if(b->crpipenetw == maxi)
			b->crpipenetw = mini;
	}
    
	CrPipeTile* pipe;
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			pipe = CrPipeAt(x, z);
            
			if(!pipe->on)
				continue;
            
			if(!pipe->finished)
				continue;
            
			if(pipe->netw == maxi)
				pipe->netw = mini;
		}
}

bool ReCrPipeB()
{
	bool change = false;
    
	Building* b;
	Building* b2;
    
	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
        
		if(!b->on)
			continue;
        
		if(!ResB(i, RES_CRUDEOIL))
			continue;
        
		for(int j=0; j<BUILDINGS; j++)
		{
			if(j == i)
				continue;
            
			b2 = &g_building[j];
            
			if(!b2->on)
				continue;
            
			if(!ResB(j, RES_CRUDEOIL))
				continue;
            
			if(!BuildingAdjacent(i, j))
				continue;
            
			if(b->crpipenetw < 0 && b2->crpipenetw >= 0)
			{
				b->crpipenetw = b2->crpipenetw;
				change = true;
			}
			else if(b2->crpipenetw < 0 && b->crpipenetw >= 0)
			{
				b2->crpipenetw = b->crpipenetw;
				change = true;
			}
			else if(b->crpipenetw >= 0 && b2->crpipenetw >= 0 && b->crpipenetw != b2->crpipenetw)
			{
				MergeCrPipe(b->crpipenetw, b2->crpipenetw);
				change = true;
			}
		}
	}
    
	return change;
}

bool CompareCrPipe(CrPipeTile* pipe, int x, int z)
{
	CrPipeTile* pipe2 = CrPipeAt(x, z);
    
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
		MergeCrPipe(pipe->netw, pipe2->netw);
		return true;
	}
    
	return false;
}

bool CompareBCrPipe(Building* b, CrPipeTile* pipe)
{
	if(b->crpipenetw < 0 && pipe->netw >= 0)
	{
		b->crpipenetw = pipe->netw;
		return true;
	}
	else if(pipe->netw < 0 && b->crpipenetw >= 0)
	{
		pipe->netw = b->crpipenetw;
		return true;
	}
	else if(pipe->netw >= 0 && b->crpipenetw >= 0 && pipe->netw != b->crpipenetw)
	{
		MergeCrPipe(pipe->netw, b->crpipenetw);
		return true;
	}
    
	return false;
}

bool ReCrPipeCrPipe()
{
	bool change = false;
    
	CrPipeTile* pipe;
	Building* b;
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			pipe = CrPipeAt(x, z);
            
			if(!pipe->on)
				continue;
            
			if(!pipe->finished)
				continue;
            
			//g_log<<"x,z "<<x<<","<<z<<endl;
			//g_log.flush();
            
			if(x > 0 && CompareCrPipe(pipe, x-1, z))
				change = true;
			if(x < g_hmap.m_widthx-1 && CompareCrPipe(pipe, x+1, z))
				change = true;
			if(z > 0 && CompareCrPipe(pipe, x, z-1))
				change = true;
			if(z < g_hmap.m_widthz-1 && CompareCrPipe(pipe, x, z+1))
				change = true;
            
			for(int i=0; i<BUILDINGS; i++)
			{
				b = &g_building[i];
                
				if(!b->on)
					continue;
                
				if(!CrPipeAdjacent(i, x, z))
					continue;
                
				//g_log<<"pipeline adjacent "<<g_buildingT[b->type].name<<endl;
                
				if(!ResB(i, RES_CRUDEOIL))
					continue;
                
				//g_log<<"pipeline adjacent "<<g_buildingT[b->type].name<<endl;
                
				if(CompareBCrPipe(b, pipe))
				{
					//g_log<<"pipe change"<<endl;
					change = true;
				}
			}
		}
    
	return change;
}

void ResetCrPipe()
{
	int lastnetw = 0;
	Building* b;
    
	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
        
		b->crpipenetw = -1;
	}
    
	CrPipeTile* pipe;
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			pipe = CrPipeAt(x, z);
            
			if(!pipe->on)
				continue;
            
			if(!pipe->finished)
				continue;
            
			pipe->netw = -1;
		}
    
	BuildingT* t;
    
	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
        
		if(!b->on)
			continue;
        
		if(!b->finished)
			continue;
        
		t = &g_buildingT[b->type];
        
		if(t->output[RES_CRUDEOIL] <= 0.0f)
			continue;
        
		b->crpipenetw = lastnetw++;
	}
}

//Recalculate power networks
void ReCrPipe()
{
	ResetCrPipe();
    
	bool change;
    
	do
	{
		change = false;
        
		if(ReCrPipeB())
			change = true;
        
		if(ReCrPipeCrPipe())
			change = true;
	}while(change);
}

bool CrPipeIntersect(int x, int z, Vec3f line[])
{
	CrPipeTile* p = CrPipeAt(x, z);
    
	if(!p->on)
		return false;
    
	CrPipeTileType* t = &g_crpipeT[p->type][(int)p->finished];
	Model* m = &g_model[t->model];
    
	VertexArray* va = &m->m_va[0];
	Vec3f poly[3];
    
	Vec3f pos = CrPipeDrawPos(x, z);
    
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
