//
//  powerline.m
//  corpstates
//
//  Created by Denis Ivanov on 2013-05-30.
//  Copyright (c) 2013 DMD 'Ware. All rights reserved.
//

#include "powerline.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "country.h"
#include "road.h"
#include "../render/shader.h"
#include "../../game/gui/ggui.h"
#include "buildingtype.h"
#include "../phys/collision.h"
#include "../../game/gmain.h"
#include "building.h"
#include "../math/polygon.h"
#include "../sys/workthread.h"

PowlTileType g_powlT[CONNECTION_TYPES][2];
PowlTile* g_powl = NULL;
PowlTile* g_powlplan = NULL;
int g_powlcost[RESOURCES];

PowlTile::PowlTile()
{
	on = false;
	finished = false;
	Zero(conmat);
	netw = -1;
	//drawva.safetyConstructor();
    
	//g_log<<"PowlTile() drawva.numverts = "<<drawva.numverts<<endl;
	//g_log.flush();
}

PowlTile::~PowlTile()
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

void PowlTileType::draw(int x, int z)
{
	Vec3f pos = PowlPosition(x, z);
	g_model[model].draw(0, pos, 0);
}

void DefinePowl(int type, bool finished, const char* modelfile)
{
	PowlTileType* t = &g_powlT[type][(int)finished];
	QueueModel(&t->model, modelfile, Vec3f(1,1,1)/16.0f*TILE_SIZE, Vec3f(0,0,0));
}

PowlTile* PowlAt(int x, int z)
{
	return &g_powl[ x + z*g_hmap.m_widthx ];
}

PowlTile* PowlPlanAt(int x, int z)
{
	return &g_powlplan[ x + z*g_hmap.m_widthx ];
}

void PowlXZ(PowlTile* pow, int& x, int& z)
{
	int x1, z1;
    
	for(x1=0; x1<g_hmap.m_widthx; x1++)
		for(z1=0; z1<g_hmap.m_widthz; z1++)
		{
			if(PowlAt(x1, z1) == pow)
			{
				x = x1;
				z = z1;
				return;
			}
		}
}

int PowlTile::netreq(int res)
{
	float netrq = 0;
    
	if(!finished)
	{
		netrq = g_powlcost[res] - conmat[res];
	}
    
	return netrq;
}

void PowlTile::destroy()
{
	Zero(conmat);
	on = false;
	finished = false;
	netw = -1;
}

void PowlTile::allocate()
{
#if 0
	CPlayer* p = &g_player[owner];
    
	float alloc;
    
	char transx[32];
	transx[0] = '\0';
    
	for(int i=0; i<RESOURCES; i++)
	{
		if(g_powlcost[i] <= 0)
			continue;
        
		if(i == LABOUR)
			continue;
        
		alloc = g_powlcost[i] - conmat[i];
        
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
		PowlXZ(this, x, z);
		NewTransx(PowlPosition(x, z), transx);
	}
    
	checkconstruction();
#endif
}

bool PowlTile::checkconstruction()
{
	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < g_powlcost[i])
			return false;
#if 0
	if(owner == g_localP)
	{
		Chat("Powl construction complete.");
		ConCom();
	}
    
	finished = true;
	RePow();
    
	int x, z;
	PowlXZ(this, x, z);
	MeshPowl(x, z);
    
	if(owner == g_localP)
		OnFinishedB(POWERLINE);
#endif
	return true;
}

bool PowlHasRoad(int x, int z)
{
	Vec3f physpos = PowlPosition(x, z);
    
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

Vec3f PowlPosition(int x, int z)
{
	float fx = x*TILE_SIZE;
	float fz = z*TILE_SIZE;
	//float fy = Bilerp(fx, fz);
    
	return Vec3f(fx, 0, fz);
}

Vec3i PowlPhysPos(int x, int z)
{
	int cmx = x*TILE_SIZE;
	int cmz = z*TILE_SIZE;
	//float fy = Bilerp(fx, fz);
    
	return Vec3i(cmx, 0, cmz);
}

void DrawPowl(int x, int z, bool plan)
{
	PowlTile* p;
	if(plan)
		p = PowlPlanAt(x, z);
	else
		p = PowlAt(x, z);
    
	MutexWait(g_drawmutex);
	if(!p->on)
	{
		MutexRelease(g_drawmutex);
		return;
	}
    
	int model = g_powlT[p->type][(int)p->finished].model;
    
	const float* owncol = g_country[p->stateowner].colorcode;
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);
    
	Model* m = &g_model[model];
	m->usetex();
    
	DrawVA(&p->drawva, p->drawpos);
	MutexRelease(g_drawmutex);
}

void DrawPowls()
{
	//StartProfile(DRAWPOWERLINES);
    
	//int type;
	//PowlTile* p;
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			/*
             p = PowlAt(x, z);
             
             if(!p->on)
             continue;
             
             const float* owncol = facowncolor[p->owner];
             glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);
             
             //type = p->type;
             DrawVA(&p->drawva, PowlPosition(x, z));
             
             //if(p->finished)
             //	g_powlT[type][FINISHED].draw(x, z);
             //else
             //	g_powlT[type][CONSTRUCTION].draw(x, z);*/
			DrawPowl(x, z, false);
		}
#if 1
	if(g_build != BUILDING_POWERL)
		return;
    
	//glColor4f(1,1,1,0.5f);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			/*
             p = PowlPlanAt(x, z);
             if(!p->on)
             continue;
             
             const float* owncol = facowncolor[p->owner];
             glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);
             
             //type = p->type;
             DrawVA(&p->drawva, PowlPosition(x, z));
             //g_powlT[type][FINISHED].draw(x, z);*/
			DrawPowl(x, z, true);
		}
    
	//glColor4f(1,1,1,1);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
#endif
	//EndProfile(DRAWPOWERLINES);
}

int GetPowlType(int x, int z, bool plan=false)
{
	bool n = false, e = false, s = false, w = false;
    
	if(x+1 < g_hmap.m_widthx && PowlAt(x+1, z)->on)
		e = true;
	if(x-1 >= 0 && PowlAt(x-1, z)->on)
		w = true;
    
	if(z+1 < g_hmap.m_widthz && PowlAt(x, z+1)->on)
		s = true;
	if(z-1 >= 0 && PowlAt(x, z-1)->on)
		n = true;
    
	if(plan)
	{
		if(x+1 < g_hmap.m_widthx && PowlPlanAt(x+1, z)->on)
			e = true;
		if(x-1 >= 0 && PowlPlanAt(x-1, z)->on)
			w = true;
        
		if(z+1 < g_hmap.m_widthz && PowlPlanAt(x, z+1)->on)
			s = true;
		if(z-1 >= 0 && PowlPlanAt(x, z-1)->on)
			n = true;
	}
    
	return GetConnectionType(n, e, s, w);
}

void MeshPowl(int x, int z, bool plan)
{
	PowlTile* p = PowlAt(x, z);
	if(plan)
		p = PowlPlanAt(x, z);
    
	if(!p->on)
		return;
    
	int finished = 0;
	if(p->finished)
		finished = 1;
    
	//PowlTileType* t = &g_powlT[p->type][finished];
	PowlTileType* t = &g_powlT[p->type][1];
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
	p->drawpos = PowlPosition(x, z);
    
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

void TypePowl(int x, int z, bool plan)
{
	PowlTile* p = PowlAt(x, z);
	if(plan)
		p = PowlPlanAt(x, z);
    
	if(!p->on)
		return;
    
	p->type = GetPowlType(x, z, plan);
	MeshPowl(x, z, plan);
}

void TypePowlsAround(int x, int z, bool plan)
{
	if(x+1 < g_hmap.m_widthx)
		TypePowl(x+1, z, plan);
	if(x-1 >= 0)
		TypePowl(x-1, z, plan);
    
	if(z+1 < g_hmap.m_widthz)
		TypePowl(x, z+1, plan);
	if(z-1 >= 0)
		TypePowl(x, z-1, plan);
}

bool PowlPlaceable(int x, int z)
{
	Vec3i pp = PowlPhysPos(x, z);
    
	if(TileUnclimable(pp.x, pp.z))
		return false;
    
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
    
	//Make sure the powerline isn't surrounded by buildings or map edges
	
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
		return false;
    
	return true;
}

void RepossessPowl(int x, int z, int stateowner)
{
	PowlTile* powl = PowlAt(x, z);
	powl->stateowner = stateowner;
	powl->allocate();
}

void PlacePowl(int x, int z, int stateowner, bool plan)
{
	//g_log<<"place pow 1"<<endl;
	//g_log.flush();
    
	if(PowlAt(x, z)->on)
	{
		if(!plan)
		{
			RepossessPowl(x, z, stateowner);
            
			return;
		}
	}
    
	//g_log<<"place pow 2"<<endl;
	//g_log.flush();
    
	if(!PowlPlaceable(x, z))
		return;
    
	//g_log<<"place pow 3"<<endl;
	//g_log.flush();
    
	PowlTile* p = PowlAt(x, z);
	if(plan)
		p = PowlPlanAt(x, z);
    
	//g_log<<"place pow 4"<<endl;
	//g_log.flush();
    
	p->on = true;
	p->stateowner = stateowner;
	p->netw = -1;
    
	//g_log<<"place pow 5"<<endl;
	//g_log.flush();
    
	if(!plan && g_mode == PLAY)
		p->allocate();
    
	if(g_mode == PLAY)
		p->finished = false;
	//if(plan || g_mode == EDITOR)
    if(plan)
		p->finished = true;
    
    
	//g_log<<"place pow 6"<<endl;
	//g_log.flush();
    
	TypePowl(x, z, plan);
    
	//g_log<<"place pow 7"<<endl;
	//g_log.flush();
    
	TypePowlsAround(x, z, plan);
    
	//g_log<<"place pow 8"<<endl;
	//g_log.flush();
    
	for(int i=0; i<RESOURCES; i++)
		p->transporter[i] = -1;
}

void ClearPowlPlans()
{
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			PowlPlanAt(x, z)->on = false;
		}
}

void PlacePowl()
{
	PowlTile* p;
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			p = PowlPlanAt(x, z);
			if(p->on)
			{
				PowlTile* actual = PowlAt(x, z);
                
				bool willchange = !actual->on;
                
				PlacePowl(x, z, p->stateowner);
#if 0
				if(g_mode == PLAY && willchange)
				{
					NewJob(GOINGTOPOWLJOB, x, z);
				}
#endif
			}
		}
    
	ClearPowlPlans();
	RePow();
}

void UpdatePowlPlans(int stateowner, Vec3f start, Vec3f end)
{
	ClearPowlPlans();
    
	int x1 = Clipi(start.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z1 = Clipi(start.z/TILE_SIZE, 0, g_hmap.m_widthz-1);
    
	int x2 = Clipi(end.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z2 = Clipi(end.z/TILE_SIZE, 0, g_hmap.m_widthz-1);
    
	/*if(!g_mousekeys[MOUSEKEY_LEFT])
	{
		x1 = x2;
		z1 = z2;
	}*/
    
	PlacePowl(x1, z1, stateowner, true);
	PlacePowl(x2, z2, stateowner, true);
    
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
		PlacePowl(x, z, stateowner, true);
        
		if((int)x != prevx && (int)z != prevz)
			PlacePowl(prevx, z, stateowner, true);
        
		prevx = x;
		prevz = z;
	}
    
	if((int)x2 != prevx && (int)z2 != prevz)
		PlacePowl(prevx, z2, stateowner, true);
    
	for(x1=0; x1<g_hmap.m_widthx; x1++)
		for(z1=0; z1<g_hmap.m_widthz; z1++)
			MeshPowl(x1, z1, true);
}

void ClearPowls()
{
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
			if(PowlAt(x, z)->on && !PowlPlaceable(x, z))
			{
				PowlAt(x, z)->on = false;
				TypePowlsAround(x, z);
			}
}

void MergePow(int A, int B)
{
	int mini = min(A, B);
	int maxi = max(A, B);
    
	Building* b;
    
	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
        
		if(b->pownetw == maxi)
			b->pownetw = mini;
	}
    
	PowlTile* pow;
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			pow = PowlAt(x, z);
            
			if(!pow->on)
				continue;
            
			if(!pow->finished)
				continue;
            
			if(pow->netw == maxi)
				pow->netw = mini;
		}
}

bool RePowB()
{
	bool change = false;
    
	Building* b;
	Building* b2;
    
	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
        
		if(!b->on)
			continue;
        
		for(int j=0; j<BUILDINGS; j++)
		{
			if(j == i)
				continue;
            
			b2 = &g_building[j];
            
			if(!b2->on)
				continue;
            
			if(!BuildingAdjacent(i, j))
				continue;
            
			if(b->pownetw < 0 && b2->pownetw >= 0)
			{
				b->pownetw = b2->pownetw;
				change = true;
			}
			else if(b2->pownetw < 0 && b->pownetw >= 0)
			{
				b2->pownetw = b->pownetw;
				change = true;
			}
			else if(b->pownetw >= 0 && b2->pownetw >= 0 && b->pownetw != b2->pownetw)
			{
				MergePow(b->pownetw, b2->pownetw);
				change = true;
			}
		}
	}
    
	return change;
}

bool ComparePow(PowlTile* pow, int x, int z)
{
	PowlTile* pow2 = PowlAt(x, z);
    
	if(!pow2->on)
		return false;
    
	if(!pow2->finished)
		return false;
    
	if(pow2->netw < 0 && pow->netw >= 0)
	{
		pow2->netw = pow->netw;
		return true;
	}
	else if(pow->netw < 0 && pow2->netw >= 0)
	{
		pow->netw = pow2->netw;
		return true;
	}
	else if(pow->netw >= 0 && pow2->netw >= 0 && pow->netw != pow2->netw)
	{
		MergePow(pow->netw, pow2->netw);
		return true;
	}
    
	return false;
}

bool CompareBPow(Building* b, PowlTile* pow)
{
	if(b->pownetw < 0 && pow->netw >= 0)
	{
		b->pownetw = pow->netw;
		return true;
	}
	else if(pow->netw < 0 && b->pownetw >= 0)
	{
		pow->netw = b->pownetw;
		return true;
	}
	else if(pow->netw >= 0 && b->pownetw >= 0 && pow->netw != b->pownetw)
	{
		MergePow(pow->netw, b->pownetw);
		return true;
	}
    
	return false;
}

bool RePowPow()
{
	bool change = false;
    
	PowlTile* pow;
	Building* b;
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			pow = PowlAt(x, z);
            
			if(!pow->on)
				continue;
            
			if(!pow->finished)
				continue;
            
			//g_log<<"x,z "<<x<<","<<z<<endl;
			//g_log.flush();
            
			if(x > 0 && ComparePow(pow, x-1, z))
				change = true;
			if(x < g_hmap.m_widthx-1 && ComparePow(pow, x+1, z))
				change = true;
			if(z > 0 && ComparePow(pow, x, z-1))
				change = true;
			if(z < g_hmap.m_widthz-1 && ComparePow(pow, x, z+1))
				change = true;
            
			for(int i=0; i<BUILDINGS; i++)
			{
				b = &g_building[i];
                
				if(!b->on)
					continue;
                
				if(!PowlAdjacent(i, x, z))
					continue;
                
				if(CompareBPow(b, pow))
					change = true;
			}
		}
    
	return change;
}

void ResetPow()
{
	int lastnetw = 0;
	Building* b;
    
	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
        
		b->pownetw = -1;
	}
    
	PowlTile* pow;
    
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			pow = PowlAt(x, z);
            
			if(!pow->on)
				continue;
            
			if(!pow->finished)
				continue;
            
			pow->netw = -1;
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
        
		if(t->output[RES_ENERGY] <= 0.0f)
			continue;
        
		b->pownetw = lastnetw++;
	}
}

//Recalculate power networks
void RePow()
{
	ResetPow();
    
	bool change;
    
	do
	{
		change = false;
        
		if(RePowB())
			change = true;
        
		if(RePowPow())
			change = true;
	}while(change);
}

bool PowlIntersect(int x, int z, Vec3f line[])
{
	PowlTile* p = PowlAt(x, z);
    
	if(!p->on)
		return false;
    
	int finished = 0;
	if(p->finished)
		finished = 1;
    
	PowlTileType* t = &g_powlT[p->type][finished];
	Model* m = &g_model[t->model];
    
	VertexArray* va = &m->m_va[0];
	Vec3f poly[3];
    
	Vec3f pos = PowlPosition(x, z);
    
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


