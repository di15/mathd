//
//  road.m
//  corpstates
//
//  Created by Denis Ivanov on 2013-05-30.
//  Copyright (c) 2013 DMD 'Ware. All rights reserved.
//

#include "road.h"
#include "../render/heightmap.h"
#include "player.h"
#include "../render/shader.h"
#include "../math/hmapmath.h"
#include "../render/foliage.h"
#include "../phys/collision.h"
#include "../math/3dmath.h"
#include "building.h"
#include "buildingtype.h"
#include "../../game/gmain.h"
#include "../render/water.h"
#include "../math/polygon.h"
#include "../../game/gui/ggui.h"
#include "../utils.h"
#include "../ai/pathnode.h"
#include "../ai/collidertile.h"
#include "../sim/selection.h"
#include "../gui/gui.h"
#include "../gui/widgets/spez/constructionview.h"
#include "../../game/gui/gviewport.h"

RoadTileType g_roadT[CONNECTION_TYPES][2];
RoadTile* g_road = NULL;
RoadTile* g_roadplan = NULL;
int g_roadcost[RESOURCES];

RoadTile::RoadTile()
{
	on = false;
	finished = false;
	Zero(conmat);
	netw = -1;
}

RoadTile::~RoadTile()
{
	//drawva.safetyConstructor();
	//int x, z;
	//RoadXZ(this, x, z);
	//g_log<<"road "<<x<<","<<z<<" calling drawva.free();...";
	//g_log.flush();
	//drawva.free();
	//g_log<<"called."<<endl;
	//g_log.flush();

	//g_log<<"RoadTile() "<<g_debug1<<" delete drawva.numverts = "<<drawva.numverts<<endl;
	//g_log.flush();
}

void RoadTileType::draw(int x, int z)
{
	Vec3f pos = RoadPosition(x, z);
	g_model[model].draw(0, pos, 0);
}

void DefineRoad(int type, bool finished, const char* modelfile)
{
	RoadTileType* t = &g_roadT[type][finished];
	QueueModel(&t->model, modelfile, Vec3f(1,1,1)/16.0f*TILE_SIZE, Vec3f(0,0,0));
}

RoadTile* RoadAt(int x, int z)
{
	return &(g_road[ x + z*g_hmap.m_widthx ]);
}

Vec2i RoadPos(RoadTile* r)
{
	int i = r - g_road;
	Vec2i t;
	t.x = i % g_hmap.m_widthx;
	t.y = i / g_hmap.m_widthx;
	return t;
}

Vec2i RoadPlanPos(RoadTile* r)
{
	int i = r - g_roadplan;
	Vec2i t;
	t.x = i % g_hmap.m_widthx;
	t.y = i / g_hmap.m_widthx;
	return t;
}

RoadTile* RoadPlanAt(int x, int z)
{
	return &(g_roadplan[ x + z*g_hmap.m_widthx ]);
}

void RoadXZ(RoadTile* r, int& x, int& z)
{
	int x1, z1;

	for(x1=0; x1<g_hmap.m_widthx; x1++)
		for(z1=0; z1<g_hmap.m_widthz; z1++)
		{
			if(RoadAt(x1, z1) == r)
			{
				x = x1;
				z = z1;
				return;
			}
		}
}

int RoadTile::netreq(int res)
{
	int netrq = 0;

	if(!finished)
	{
		netrq = g_roadcost[res] - conmat[res];
	}

	return netrq;
}

void RoadTile::destroy()
{
	Zero(conmat);
	on = false;
	finished = false;
	netw = -1;
	freecollider();
}

void RoadTile::freecollider()
{
	Vec2i t = RoadPos(this);

	int cmminx = t.x * TILE_SIZE;
	int cmminz = t.y * TILE_SIZE;
	int cmmaxx = (t.x+1) * TILE_SIZE - 1;
	int cmmaxz = (t.y+1) * TILE_SIZE - 1;

	int nminx = cmminx / PATHNODE_SIZE;
	int nminz = cmminz / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxz = cmmaxz / PATHNODE_SIZE;

	for(int x=nminx; x<=nmaxx; x++)
		for(int z=nminz; z<=nmaxz; z++)
		{
			ColliderTile* c = ColliderTileAt(x, z);
			//c->hasroad = false;
			c->flags &= ~FLAG_HASROAD;
		}

#if 0
		int roadx, roadz;

		RoadXZ(this, roadx, roadz);
		Vec3f pos = RoadPosition(roadx, roadz);

		//BuildingT* bt = &g_buildingT[type];
		float hwx = TILE_SIZE/2.0f;
		float hwz = TILE_SIZE/2.0f;

		float fstartx = pos.x-hwx;
		float fstartz = pos.z-hwz;
		float fendx = pos.x+hwx;
		float fendz = pos.z+hwz;

		//int extentx = ceil(r/cellwx);
		//int extentz = ceil(r/cellwz);
		const float cellwx = MIN_RADIUS*2.0f;
		const float cellwz = MIN_RADIUS*2.0f;

		int startx = fstartx/cellwx;
		int startz = fstartz/cellwz;
		int endx = ceil(fendx/cellwx);
		int endz = ceil(fendz/cellwz);
		//int endx = startx+extentx;
		//int endz = startz+extentz;

		for(int x=startx; x<endx; x++)
			for(int z=startz; z<endz; z++)
			{
				bool found = false;
				ColliderTile* cell = ColliderTileAt(x, z);
				for(int i=0; i<cell->colliders.size(); i++)
				{
					Collider* c = &cell->colliders[i];
					if(c->type == COLLIDER_NOROAD)
					{
						found = true;
						break;
					}
				}

				if(!found)
					cell->colliders.push_back(Collider(COLLIDER_NOROAD, -1));
			}
#endif
}

void RoadTile::fillcollider()
{
	Vec2i t = RoadPos(this);

	int cmminx = t.x * TILE_SIZE;
	int cmminz = t.y * TILE_SIZE;
	int cmmaxx = (t.x+1) * TILE_SIZE - 1;
	int cmmaxz = (t.y+1) * TILE_SIZE - 1;

	int nminx = cmminx / PATHNODE_SIZE;
	int nminz = cmminz / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxz = cmmaxz / PATHNODE_SIZE;

	for(int x=nminx; x<=nmaxx; x++)
		for(int z=nminz; z<=nmaxz; z++)
		{
			ColliderTile* c = ColliderTileAt(x, z);
			//c->hasroad = true;
			c->flags ^= FLAG_HASROAD;
		}

#if 0
		int roadx, roadz;

		RoadXZ(this, roadx, roadz);
		Vec3f pos = RoadPosition(roadx, roadz);

		//BuildingT* bt = &g_buildingT[type];
		float hwx = TILE_SIZE/2.0f;
		float hwz = TILE_SIZE/2.0f;

		float fstartx = pos.x-hwx;
		float fstartz = pos.z-hwz;
		float fendx = pos.x+hwx;
		float fendz = pos.z+hwz;

		//int extentx = ceil(r/cellwx);
		//int extentz = ceil(r/cellwz);
		const float cellwx = MIN_RADIUS*2.0f;
		const float cellwz = MIN_RADIUS*2.0f;

		int startx = fstartx/cellwx;
		int startz = fstartz/cellwz;
		int endx = ceil(fendx/cellwx);
		int endz = ceil(fendz/cellwz);
		//int endx = startx+extentx;
		//int endz = startz+extentz;

		for(int x=startx; x<endx; x++)
			for(int z=startz; z<endz; z++)
			{
				ColliderTile* cell = ColliderTileAt(x, z);
				//cell->colliders.push_back(Collider(COLLIDER_UNIT, uID));
				for(int i=0; i<cell->colliders.size(); i++)
				{
					Collider* c = &cell->colliders[i];
					if(c->type == COLLIDER_NOROAD)
					{
						cell->colliders.erase( cell->colliders.begin() + i );
						//break;
						i--;
					}
				}
			}
#endif
}

void RoadTile::allocate()
{
#if 0
	CPlayer* p = &g_player[owner];

	float alloc;

	char transx[32];
	transx[0] = '\0';

	for(int i=0; i<RESOURCES; i++)
	{
		if(g_roadcost[i] <= 0)
			continue;

		if(i == LABOUR)
			continue;

		alloc = g_roadcost[i] - conmat[i];

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
		RoadXZ(this, x, z);
		NewTransx(RoadPosition(x, z), transx);
	}

	checkconstruction();
#endif
}

bool RoadTile::checkconstruction()
{
	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < g_roadcost[i])
			return false;

#if 0
	if(owner == g_localP)
	{
		Chat("Road construction complete.");
		ConCom();
	}

	finished = true;
	fillcollider();

	int x, z;
	RoadXZ(this, x, z);
	MeshRoad(x, z);
	ReRoadNetw();

	if(owner == g_localP)
		OnFinishedB(ROAD);
#endif

	fillcollider();

	return true;
}

Vec3f RoadPosition(int x, int z)
{
	//return Vec3f(x*TILE_SIZE, 0, z*TILE_SIZE);
	float fx = (x+0.5f)*TILE_SIZE;
	float fz = (z+0.5f)*TILE_SIZE;
	//float fy = Bilerp(fx, fz);

	return Vec3f(fx, 0, fz);

	//return Vec3f((x+0.5f)*TILE_SIZE, 0, (z+0.5f)*TILE_SIZE);
}

Vec3i RoadPhysPos(int x, int z)
{
	int cmx = x*TILE_SIZE + TILE_SIZE/2;
	int cmz = z*TILE_SIZE + TILE_SIZE/2;
	//float fy = Bilerp(fx, fz);

	return Vec3i(cmx, 0, cmz);
}

void DrawRoad(int x, int z)
{
	RoadTile* r = RoadAt(x, z);

	if(!r->on)
	{
		return;
	}

	//int model = g_roadT[r->type][(int)r->finished].model;
	int model = g_roadT[r->type][1].model;

	const float* owncol = g_player[r->stateowner].colorcode;
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

	Model* m = &g_model[model];
	m->usetex();

#if 0
	DrawVA(&r->drawva, Vec3f(0,0,0));
#endif

#if 0
	g_log<<"drew road xz"<<x<<","<<z<<" ty"<<r->type<<" fini"<<(int)r->finished<<" owncol"<<owncol[0]<<","<<owncol[1]<<","<<owncol[2]<<","<<owncol[3]<<" mod"<<model<<" v"<<r->drawva.numverts<<" "<<endl;
	g_log.flush();
#endif

#if 1
	DrawVA(&r->drawva, r->drawpos);
#endif
}

void DrawRoadPlan(int x, int z)
{
	RoadTile* r = RoadPlanAt(x, z);

	if(!r->on)
	{
		return;
	}

	int model = g_roadT[r->type][(int)r->finished].model;

	const float* owncol = g_player[r->stateowner].colorcode;
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

	Model* m = &g_model[model];
	m->usetex();

	DrawVA(&r->drawva, r->drawpos);
}

void DrawRoads()
{
	//StartTimer(DRAWROADS);

	//int type;
	//RoadTile* r;

	Player* py = &g_player[g_currP];

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			/*
			r = RoadAt(x, z);

			if(!r->on)
			continue;

			type = r->type;
			if(r->finished)
			g_roadT[type][FINISHED].Draw(x, z, s);
			else
			g_roadT[type][CONSTRUCTION].Draw(x, z, s);*/
			DrawRoad(x, z);
		}

		if(py->build != BUILDING_ROAD)
			return;

		//glColor4f(1,1,1,0.5f);
		glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 0.5f);

		for(int x=0; x<g_hmap.m_widthx; x++)
			for(int z=0; z<g_hmap.m_widthz; z++)
			{
				/*
				r = RoadPlanAt(x, z);
				if(r->on)
				{
				type = r->type;
				g_roadT[type][FINISHED].Draw(x, z, s);
				}*/
				DrawRoadPlan(x, z);
			}

			//glColor4f(1,1,1,1);
			glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

			//StopTimer(DRAWROADS);
}

int GetRoadType(int x, int z, bool plan=false)
{
	bool n = false, e = false, s = false, w = false;

	if(x+1 < g_hmap.m_widthx && RoadAt(x+1, z)->on)
		e = true;
	if(x-1 >= 0 && RoadAt(x-1, z)->on)
		w = true;

	if(z+1 < g_hmap.m_widthz && RoadAt(x, z+1)->on)
		s = true;
	if(z-1 >= 0 && RoadAt(x, z-1)->on)
		n = true;

	if(plan)
	{
		if(x+1 < g_hmap.m_widthx && RoadPlanAt(x+1, z)->on)
			e = true;
		if(x-1 >= 0 && RoadPlanAt(x-1, z)->on)
			w = true;

		if(z+1 < g_hmap.m_widthz && RoadPlanAt(x, z+1)->on)
			s = true;
		if(z-1 >= 0 && RoadPlanAt(x, z-1)->on)
			n = true;
	}

	return GetConnectionType(n, e, s, w);
}

void MeshRoad(int x, int z, bool plan)
{
	RoadTile* r = RoadAt(x, z);
	if(plan)
		r = RoadPlanAt(x, z);

	if(!r->on)
		return;
#if 0
	g_log<<"mesh on "<<x<<","<<z<<endl;
	g_log.flush();
#endif

	//RoadTileType* t = &g_roadT[r->type][(int)r->finished];
	RoadTileType* t = &g_roadT[r->type][1];
	Model* m = &g_model[t->model];

	VertexArray* rva = &r->drawva;
	VertexArray* mva = &m->m_va[0];

	rva->free();

	//g_log<<"meshroad allocating "<<mva->numverts<<"...";
	//g_log.flush();

	rva->numverts = mva->numverts;
	rva->vertices = new Vec3f[ rva->numverts ];
	rva->texcoords = new Vec2f[ rva->numverts ];
	rva->normals = new Vec3f[ rva->numverts ];

	if(!rva->vertices)
		OutOfMem(__FILE__, __LINE__);

	if(!rva->texcoords)
		OutOfMem(__FILE__, __LINE__);

	if(!rva->normals)
		OutOfMem(__FILE__, __LINE__);

	float realx, realz;
	r->drawpos = RoadPosition(x, z);

	for(int i=0; i<rva->numverts; i++)
	{
		rva->vertices[i] = mva->vertices[i];
		rva->texcoords[i] = mva->texcoords[i];
		rva->normals[i] = mva->normals[i];

		//realx = ((float)x+0.5f)*(float)TILE_SIZE + rva->vertices[i].x;
		//realz = ((float)z+0.5f)*(float)TILE_SIZE + rva->vertices[i].z;
		realx = r->drawpos.x + rva->vertices[i].x;
		realz = r->drawpos.z + rva->vertices[i].z;

#if 0
		rva->vertices[i].y += Bilerp(&g_hmap, realx, realz);
#else
		rva->vertices[i].y += g_hmap.accheight2(realx, realz);
#endif

		//char msg[128];
		//sprintf(msg, "(%f,%f,%f)", mva->vertices[i].x, mva->vertices[i].y, mva->vertices[i].z);
		//Chat(msg);
	}

	//g_log<<"done meshroad"<<endl;
	//g_log.flush();
}

void TypeRoad(int x, int z, bool plan)
{
	RoadTile* r = RoadAt(x, z);
	if(plan)
		r = RoadPlanAt(x, z);

	if(!r->on)
		return;

	r->type = GetRoadType(x, z, plan);
	MeshRoad(x, z, plan);
}

void TypeRoadsAround(int x, int z, bool plan)
{
	if(x+1 < g_hmap.m_widthx)
		TypeRoad(x+1, z, plan);
	if(x-1 >= 0)
		TypeRoad(x-1, z, plan);

	if(z+1 < g_hmap.m_widthz)
		TypeRoad(x, z+1, plan);
	if(z-1 >= 0)
		TypeRoad(x, z-1, plan);
}

bool RoadPlaceable(int x, int z)
{
	Vec3i pos = RoadPhysPos(x, z);

	if(TileUnclimable(pos.x, pos.z))
	{
		g_log<<"tile unclim"<<endl;

		return false;
	}

	if(CollidesWithBuildings(pos.x, pos.z, pos.x, pos.z))
	{
		g_log<<"buil coll"<<endl;

		return false;
	}

	return true;
}

void ResetRoadNetw()
{
	int lastnetw = 0;
	Building* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		b->roadnetw.clear();
	}

	RoadTile* r;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			r = RoadAt(x, z);

			if(!r->on)
				continue;

			if(!r->finished)
				continue;

			r->netw = lastnetw++;
		}
}

bool ReRoadB()
{
	bool change = false;
	/*
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
	}*/

	return change;
}

void MergeRoad(int A, int B)
{
	int mini = min(A, B);
	int maxi = max(A, B);

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		bool found = false;
		auto netwiter = b->roadnetw.begin();

		while(netwiter != b->roadnetw.end())
		{
			if(*netwiter == maxi)
			{
				if(!found)
				{
					*netwiter = mini;
					found = true;
				}
				else
				{
					netwiter = b->roadnetw.erase( netwiter );
					continue;
				}
			}

			netwiter++;
		}
	}

	RoadTile* r;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			r = RoadAt(x, z);

			if(!r->on)
				continue;

			if(!r->finished)
				continue;

			if(r->netw == maxi)
				r->netw = mini;
		}
}

bool CompareRoad(RoadTile* r, int x, int z)
{
	RoadTile* r2 = RoadAt(x, z);

	if(!r2->on)
		return false;

	if(!r2->finished)
		return false;

	if(r2->netw < 0 && r->netw >= 0)
	{
		r2->netw = r->netw;
		return true;
	}
	else if(r->netw < 0 && r2->netw >= 0)
	{
		r->netw = r2->netw;
		return true;
	}
	else if(r->netw >= 0 && r2->netw >= 0 && r->netw != r2->netw)
	{
		MergeRoad(r->netw, r2->netw);
		return true;
	}

	return false;
}

bool RoadAdjacent(int i, int x, int z)
{
	Building* b = &g_building[i];
	BuildingT* t = &g_buildingT[b->type];

	Vec2i tp = b->tilepos;

	int minx = (tp.x - t->widthx/2)*TILE_SIZE;
	int minz = (tp.y - t->widthz/2)*TILE_SIZE;
	int maxx = minx + t->widthx*TILE_SIZE;
	int maxz = minz + t->widthz*TILE_SIZE;

	Vec3i p2 = RoadPhysPos(x, z);

	int hwx2 = TILE_SIZE/2;
	int hwz2 = TILE_SIZE/2;
	int minx2 = p2.x - hwx2;
	int minz2 = p2.z - hwz2;
	int maxx2 = p2.x + hwx2;
	int maxz2 = p2.z + hwz2;

	if(maxx >= minx2 && maxz >= minz2 && minx <= maxx2 && minz <= maxz2)
		return true;

	return false;
}

bool CompareBRoad(Building* b, RoadTile* r)
{
	if(b->roadnetw.size() <= 0 && r->netw >= 0)
	{
		b->roadnetw.push_back(r->netw);
		return true;
	}/*
	 else if(r->netw < 0 && b->roadnetw >= 0)
	 {
	 pow->netw = b->pownetw;
	 return true;
	 }
	 else if(pow->netw >= 0 && b->pownetw >= 0 && pow->netw != b->pownetw)
	 {
	 MergePow(pow->netw, b->pownetw);
	 return true;
	 }*/
	else if(b->roadnetw.size() > 0 && r->netw >= 0)
	{
		bool found = false;
		for(auto netwiter = b->roadnetw.begin(); netwiter != b->roadnetw.end(); netwiter++)
		{
			if(*netwiter == r->netw)
			{
				found = true;
				break;
			}
		}
		if(!found)
			b->roadnetw.push_back(r->netw);
	}

	return false;
}

bool ReRoadRoad()
{
	bool change = false;

	RoadTile* r;
	Building* b;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			r = RoadAt(x, z);

			if(!r->on)
				continue;

			if(!r->finished)
				continue;

			if(x > 0 && CompareRoad(r, x-1, z))
				change = true;
			if(x < g_hmap.m_widthx-1 && CompareRoad(r, x+1, z))
				change = true;
			if(z > 0 && CompareRoad(r, x, z-1))
				change = true;
			if(z < g_hmap.m_widthz-1 && CompareRoad(r, x, z+1))
				change = true;

			for(int i=0; i<BUILDINGS; i++)
			{
				b = &g_building[i];

				if(!b->on)
					continue;

				if(!RoadAdjacent(i, x, z))
					continue;

				if(CompareBRoad(b, r))
					change = true;
			}
		}

		return change;
}

void CheckRoadAccess()
{
	list<int> sharednetw;

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		for(auto netwiter = b->roadnetw.begin(); netwiter != b->roadnetw.end(); netwiter++)
		{
			bool found = false;
			for(auto sharednetwiter = sharednetw.begin(); sharednetwiter != sharednetw.end(); sharednetwiter++)
			{
				if(*sharednetwiter == *netwiter)
				{
					found = true;
					break;
				}
			}

			if(!found)
				sharednetw.push_back(*netwiter);
		}
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		auto sharednetwiter = sharednetw.begin();
		while( sharednetwiter != sharednetw.end() )
		{
			bool found = false;

			for(auto netwiter = b->roadnetw.begin(); netwiter != b->roadnetw.end(); netwiter++)
			{
				if(*sharednetwiter == *netwiter)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				sharednetwiter = sharednetw.erase( sharednetwiter );
				continue;
			}

			sharednetwiter++;
		}

#if 0
		if(sharednetw.size() <= 0)
			OnRoadInacc();
#endif
	}

#if 0
	if(sharednetw.size() > 0)
		OnAllRoadAc();
#endif
}

//Recalculate road networks
void ReRoadNetw()
{
	ResetRoadNetw();

	bool change;

	do
	{
		change = false;

		if(ReRoadB())
			change = true;

		if(ReRoadRoad())
			change = true;
	}while(change);

#if 0
	CheckRoadAccess();
#endif
}

void RepossessRoad(int x, int z, int stateowner)
{
	RoadTile* r = RoadAt(x, z);
	r->stateowner = stateowner;
	r->allocate();
}

void PlaceRoad(int x, int z, int stateowner, bool plan)
{
#if 0
	g_log<<"place road ...2"<<endl;
	g_log.flush();
#endif

	if(RoadAt(x, z)->on)
	{
#if 0
		g_log<<"place road ...2 repo"<<endl;
		g_log.flush();
#endif

		if(!plan)
		{
			RepossessRoad(x, z, stateowner);

			return;
		}
	}

	if(!RoadPlaceable(x, z))
	{
#if 0
		g_log<<"place road ...2 not placeable "<<x<<","<<z<<endl;
		g_log.flush();
#endif

		return;
	}

#if 0
	g_log<<"place road ...3"<<endl;
	g_log.flush();
#endif

	RoadTile* r = RoadAt(x, z);
	if(plan)
		r = RoadPlanAt(x, z);

	r->on = true;
	r->stateowner = stateowner;
	Zero(r->maxcost);

	//if(!plan && g_mode == APPMODE_PLAY)
	//	r->allocate();

	r->drawpos = RoadPosition(x, z);

	if(g_mode == APPMODE_PLAY)
		r->finished = false;
	//if(plan || g_mode == APPMODE_EDITOR)
	if(plan)
		r->finished = true;

	TypeRoad(x, z, plan);
	TypeRoadsAround(x, z, plan);

	for(int i=0; i<RESOURCES; i++)
		r->transporter[i] = -1;

	//if(!plan)
	//	ReRoadNetw();
#if 0
	g_log<<"place road ...4"<<endl;
	g_log.flush();
#endif

	if(!plan)
	{
		Vec3f pos = RoadPosition(x, z);
		ClearFoliage(pos.x - TILE_SIZE/2, pos.z - TILE_SIZE/2, pos.x + TILE_SIZE/2, pos.z + TILE_SIZE/2);

		g_hmap.hidetile(pos.x/TILE_SIZE, pos.z/TILE_SIZE);
	}
}

void ClearRoadPlans()
{
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
			RoadPlanAt(x, z)->on = false;
}

void PlaceRoad()
{
#if 0
	g_log<<"place road 1"<<endl;
	g_log.flush();
#endif
	Player* py = &g_player[g_currP];

	if(g_mode == APPMODE_PLAY)
	{
		ClearSel(&py->sel);
	}

	for(int x=0; x<g_hmap.m_widthx; x++)
	{
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
#if 0
			g_log<<"place road 1.1 "<<x<<","<<z<<endl;
			g_log.flush();
#endif

			RoadTile* plan = RoadPlanAt(x, z);
			if(plan->on)
			{
#if 0
				g_log<<"place road 1.2 "<<x<<","<<z<<endl;
				g_log.flush();
#endif

				RoadTile* actual = RoadAt(x, z);

				bool willchange = !actual->on;

				PlaceRoad(x, z, plan->stateowner);

				if(g_mode == APPMODE_PLAY && !actual->finished)
					py->sel.roads.push_back(Vec2i(x,z));

				if(g_mode == APPMODE_PLAY && willchange)
				{
#if 0
					NewJob(GOINGTOROADJOB, x, z);
#endif
				}
			}
		}
	}

	ClearRoadPlans();
	ReRoadNetw();

	if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_currP];
		GUI* gui = &py->gui;

		if(py->sel.roads.size() > 0)
		{
			ConstructionView* cv = (ConstructionView*)gui->get("construction view")->get("construction view");
			cv->regen(&py->sel);
			gui->open("construction view");
		}
	}
}

bool RoadLevel(float iterx, float iterz, float testx, float testz, float dx, float dz, int i, float d, RoadTile* (*planfunc)(int x, int z))
{
	//return true;

	bool n = false, e = false, s = false, w = false;
	int ix, iz;

	if(i > 0)
	{
		float x = iterx - dx;
		float z = iterz - dz;

		ix = x;
		iz = z;

		if(ix == testx)
		{
			if(iz == testz+1)	n = true;
			else if(iz == testz-1)	s = true;
		}
		else if(iz == testz)
		{
			if(ix == testx+1)	w = true;
			else if(ix == testx-1)	e = true;
		}

		float prevx = x - dx;
		float prevz = z - dz;

		if((int)x != prevx && (int)z != prevz)
		{
			ix = prevx;

			if(ix == testx)
			{
				if(iz == testz+1)	n = true;
				else if(iz == testz-1)	s = true;
			}
			else if(iz == testz)
			{
				if(ix == testx+1)	w = true;
				else if(ix == testx-1)	e = true;
			}
		}
	}

	if(i < d)
	{
		float x = iterx + dx;
		float z = iterz + dz;

		ix = x;
		iz = z;

		if(ix == testx)
		{
			if(iz == testz+1)	n = true;
			else if(iz == testz-1)	s = true;
		}
		else if(iz == testz)
		{
			if(ix == testx+1)	w = true;
			else if(ix == testx-1)	e = true;
		}

		if(i > 0)
		{
			float prevx = x - dx;
			float prevz = z - dz;

			if((int)x != prevx && (int)z != prevz)
			{
				ix = prevx;

				if(ix == testx)
				{
					if(iz == testz+1)	n = true;
					else if(iz == testz-1)	s = true;
				}
				else if(iz == testz)
				{
					if(ix == testx+1)	w = true;
					else if(ix == testx-1)	e = true;
				}
			}
		}
	}

	ix = testx;
	iz = testz;

	if(g_hmap.getheight(ix, iz) <= WATER_LEVEL)		return false;
	if(g_hmap.getheight(ix+1, iz) <= WATER_LEVEL)	return false;
	if(g_hmap.getheight(ix, iz+1) <= WATER_LEVEL)	return false;
	if(g_hmap.getheight(ix+1, iz+1) <= WATER_LEVEL)	return false;

	if(ix > 0)
	{
		if(RoadAt(ix-1, iz)->on)	w = true;
		//if(RoadPlanAt(ix-1, iz)->on)	w = true;
		if(planfunc != NULL)
			if(planfunc(ix-1, iz)->on) w = true;
	}

	if(ix < g_hmap.m_widthx-1)
	{
		if(RoadAt(ix+1, iz)->on)	e = true;
		//if(RoadPlanAt(ix+1, iz)->on)	e = true;
		if(planfunc != NULL)
			if(planfunc(ix+1, iz)->on) w = true;
	}

	if(iz > 0)
	{
		if(RoadAt(ix, iz-1)->on)	s = true;
		//if(RoadPlanAt(ix, iz-1)->on)	s = true;
		if(planfunc != NULL)
			if(planfunc(ix, iz-1)->on) w = true;
	}

	if(iz < g_hmap.m_widthz-1)
	{
		if(RoadAt(ix, iz+1)->on)	n = true;
		//if(RoadPlanAt(ix, iz+1)->on)	n = true;
		if(planfunc != NULL)
			if(planfunc(ix, iz+1)->on) w = true;
	}
#if 0
	g_log<<"level? ix"<<ix<<","<<iz<<endl;
	g_log.flush();
#endif
	if((n && e && s && w) || (n && e && s && !w) || (n && e && !s && w) || (n && e && !s && !w) || (n && !e && s && w)
		|| (n && !e && !s && w) || (!n && e && s && !w) || (!n && !e && s && w) || (!n && !e && !s && !w) || (!n && e && s && w))
	{
		float compare = g_hmap.getheight(ix, iz);
		if(fabs(g_hmap.getheight(ix+1, iz) - compare) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - compare) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz+1) - compare) > ROAD_MAX_SIDEW_INCLINE)	return false;
	}
	else if((n && !e && s && !w) || (n && !e && !s && !w) || (!n && !e && s && !w))
	{
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix+1, iz)) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - g_hmap.getheight(ix+1, iz+1)) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix, iz+1)) > ROAD_MAX_FOREW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz) - g_hmap.getheight(ix+1, iz+1)) > ROAD_MAX_FOREW_INCLINE)	return false;
	}
	else if((!n && e && !s && w) || (!n && e && !s && !w) || (!n && !e && !s && w))
	{
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix+1, iz)) > ROAD_MAX_FOREW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - g_hmap.getheight(ix+1, iz+1)) > ROAD_MAX_FOREW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix, iz+1)) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz) - g_hmap.getheight(ix+1, iz+1)) > ROAD_MAX_SIDEW_INCLINE)	return false;
	}

#if 0
	g_log<<"level yes! ix"<<ix<<","<<iz<<endl;
	g_log.flush();
#endif

	return true;
}

void UpdateRoadPlans(int stateowner, Vec3f start, Vec3f end)
{
	ClearRoadPlans();

	int x1 = Clipi(start.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z1 = Clipi(start.z/TILE_SIZE, 0, g_hmap.m_widthz-1);

	int x2 = Clipi(end.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z2 = Clipi(end.z/TILE_SIZE, 0, g_hmap.m_widthz-1);

#if 0
	g_log<<"road plan "<<x1<<","<<z1<<"->"<<x2<<","<<z2<<endl;
	g_log.flush();
#endif
	/*if(!py->mousekeys[MOUSE_LEFT])
	{
	x1 = x2;
	z1 = z2;
	}*/

	float dx = x2-x1;
	float dz = z2-z1;

	float d = sqrtf(dx*dx + dz*dz);

	dx /= d;
	dz /= d;

	int prevx = x1;
	int prevz = z1;

	int i = 0;

	//PlaceRoad(x1, z1, g_localPlayer, true);
	//if(RoadLevel(x2, z2, dx, dz, i, d))
	//	PlaceRoad(x2, z2, g_localPlayer, true);

	for(float x=x1, z=z1; i<=d; x+=dx, z+=dz, i++)
	{
		if(RoadLevel(x, z, x, z, dx, dz, i, d))
		{
#if 0
			g_log<<"place road urp "<<x<<","<<z<<endl;
			g_log.flush();
#endif
			PlaceRoad(x, z, stateowner, true);
		}

		if((int)x != prevx && (int)z != prevz)
		{
			if(RoadLevel(x, z, prevx, z, dx, dz, i, d))
				PlaceRoad(prevx, z, stateowner, true);
		}

		prevx = x;
		prevz = z;

#if 0
		g_log<<"place road "<<x<<","<<z<<endl;
		g_log.flush();
#endif
	}

	if((int)x2 != prevx && (int)z2 != prevz)
	{
		if(RoadLevel(x2, z1, prevx, z2, dx, dz, i, d))
			PlaceRoad(prevx, z2, stateowner, true);
	}

	for(x1=0; x1<g_hmap.m_widthx; x1++)
		for(z1=0; z1<g_hmap.m_widthz; z1++)
			MeshRoad(x1, z1, true);
}

bool RoadIntersect(int x, int z, Vec3f line[])
{
	RoadTile* r = RoadAt(x, z);

	if(!r->on)
		return false;

	RoadTileType* t = &g_roadT[r->type][r->finished];
	Model* m = &g_model[t->model];

	VertexArray* va = &m->m_va[0];
	Vec3f poly[3];

	Vec3f pos = RoadPosition(x, z);
	pos = Vec3f(0,0,0);

	for(int v=0; v<va->numverts; v+=3)
	{
		poly[0] = va->vertices[v+0] + pos;
		poly[1] = va->vertices[v+1] + pos;
		poly[2] = va->vertices[v+2] + pos;

		if(InterPoly(poly, line, 3))
			return true;
	}

	return false;
}
