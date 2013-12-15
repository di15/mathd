
#include "road.h"
#include "map.h"
#include "main.h"
#include "physics.h"
#include "job.h"
#include "resource.h"
#include "player.h"
#include "chat.h"
#include "model.h"
#include "shader.h"
#include "transaction.h"
#include "sound.h"
#include "water.h"
#include "script.h"
#include "debug.h"
#include "scenery.h"

CRoadType g_roadType[CONNECTION_TYPES][2];
CRoad* g_road = NULL;
CRoad* g_roadPlan = NULL;
float g_roadCost[RESOURCES];

CRoad::CRoad() 
{ 
	on = false; 
	finished = false; 
	Zero(conmat);
	netw = -1;
}

CRoad::~CRoad()
{
	//vertexArray.safetyConstructor();
	//int x, z;
	//RoadXZ(this, x, z);
	//g_log<<"road "<<x<<","<<z<<" calling vertexArray.free();...";
	//g_log.flush();
	//vertexArray.free();
	//g_log<<"called."<<endl;
	//g_log.flush();

	//g_log<<"CRoad() "<<g_debug1<<" delete vertexArray.numverts = "<<vertexArray.numverts<<endl;
	//g_log.flush();
}

void CRoadType::draw(int x, int z) 
{
	Vec3f pos = RoadPosition(x, z);
	g_model[model].draw(0, pos, 0);
}

void CRoadType::Define(char* modelfile)
{
	QueueModel(&model, modelfile, Vec3f(1,1,1), Vec3f(0,0,0));
}

CRoad* RoadAt(int x, int z)
{
	return &(g_road[ x + z*g_hmap.m_widthX ]);
}

CRoad* RoadPlanAt(int x, int z)
{
	return &(g_roadPlan[ x + z*g_hmap.m_widthX ]);
}

void RoadXZ(CRoad* r, int& x, int& z)
{
	int x1, z1;

	for(x1=0; x1<g_hmap.m_widthX; x1++)
		for(z1=0; z1<g_hmap.m_widthZ; z1++)
		{
			if(RoadAt(x1, z1) == r)
			{
				x = x1;
				z = z1;
				return;
			}
		}
}

float CRoad::netreq(int res)
{
	float netrq = 0;

	if(!finished)
	{
		netrq = g_roadCost[res] - conmat[res];
	}

	return netrq;
}

void CRoad::destroy()
{
	Zero(conmat);
	on = false;
	finished = false;
	netw = -1;
	freecollidercells();
}

void CRoad::freecollidercells()
{
	int roadx, roadz;

	RoadXZ(this, roadx, roadz);
	Vec3f pos = RoadPosition(roadx, roadz);

	//CBuildingType* bt = &g_buildingType[type];
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
			ColliderCell* cell = ColliderCellAt(x, z);
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
}

void CRoad::fillcollidercells()
{
	int roadx, roadz;

	RoadXZ(this, roadx, roadz);
	Vec3f pos = RoadPosition(roadx, roadz);

	//CBuildingType* bt = &g_buildingType[type];
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
			ColliderCell* cell = ColliderCellAt(x, z);
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

}

void CRoad::Allocate()
{
	CPlayer* p = &g_player[owner];

	float alloc;

	char transx[32];
	transx[0] = '\0';

	for(int i=0; i<RESOURCES; i++)
	{
		if(g_roadCost[i] <= 0)
			continue;

		if(i == LABOUR)
			continue;

		alloc = g_roadCost[i] - conmat[i];

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

	CheckConstruction();
}

bool CRoad::CheckConstruction()
{
	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < g_roadCost[i])
			return false;

	if(owner == g_localP)
	{
		Chat("Road construction complete.");
		ConCom();
	}
	
	finished = true;
	fillcollidercells();

	int x, z;
	RoadXZ(this, x, z);
	MeshRoad(x, z);
	ReRoadNetw();
	
	if(owner == g_localP)
		OnFinishedB(ROAD);

	return true;
}

void CRoad::Emit(int t)
{
	Vec3f pos;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			if(RoadAt(x, z) != this)
				continue;
			
			pos = RoadPosition(x, z);
			EmitParticle(t, pos + Vec3f(0, 10, 0));
			return;
		}
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

void InitRoads()
{
	Zero(g_roadCost);
	g_roadCost[LABOUR] = 1;
	g_roadCost[CEMENT] = 1;

	g_roadType[NOCONNECTION][CONSTRUCTION].Define("models\\road\\1_c.ms3d");
	g_roadType[NORTH][CONSTRUCTION].Define("models\\road\\n_c.ms3d");
	g_roadType[EAST][CONSTRUCTION].Define("models\\road\\e_c.ms3d");
	g_roadType[SOUTH][CONSTRUCTION].Define("models\\road\\s_c.ms3d");
	g_roadType[WEST][CONSTRUCTION].Define("models\\road\\w_c.ms3d");
	g_roadType[NORTHEAST][CONSTRUCTION].Define("models\\road\\ne_c.ms3d");
	g_roadType[NORTHSOUTH][CONSTRUCTION].Define("models\\road\\ns_c.ms3d");
	g_roadType[EASTSOUTH][CONSTRUCTION].Define("models\\road\\es_c.ms3d");
	g_roadType[NORTHWEST][CONSTRUCTION].Define("models\\road\\nw_c.ms3d");
	g_roadType[EASTWEST][CONSTRUCTION].Define("models\\road\\ew_c.ms3d");
	g_roadType[SOUTHWEST][CONSTRUCTION].Define("models\\road\\sw_c.ms3d");
	g_roadType[EASTSOUTHWEST][CONSTRUCTION].Define("models\\road\\esw_c.ms3d");
	g_roadType[NORTHSOUTHWEST][CONSTRUCTION].Define("models\\road\\nsw_c.ms3d");
	g_roadType[NORTHEASTWEST][CONSTRUCTION].Define("models\\road\\new_c.ms3d");
	g_roadType[NORTHEASTSOUTH][CONSTRUCTION].Define("models\\road\\nes_c.ms3d");
	g_roadType[NORTHEASTSOUTHWEST][CONSTRUCTION].Define("models\\road\\nesw_c.ms3d");
	g_roadType[NOCONNECTION][FINISHED].Define("models\\road\\1.ms3d");
	//g_roadType[NOCONNECTION][FINISHED].Define("models\\road\\flat.ms3d");
	g_roadType[NORTH][FINISHED].Define("models\\road\\n.ms3d");
	g_roadType[EAST][FINISHED].Define("models\\road\\e.ms3d");
	g_roadType[SOUTH][FINISHED].Define("models\\road\\s.ms3d");
	g_roadType[WEST][FINISHED].Define("models\\road\\w.ms3d");
	g_roadType[NORTHEAST][FINISHED].Define("models\\road\\ne.ms3d");
	g_roadType[NORTHSOUTH][FINISHED].Define("models\\road\\ns.ms3d");
	g_roadType[EASTSOUTH][FINISHED].Define("models\\road\\es.ms3d");
	g_roadType[NORTHWEST][FINISHED].Define("models\\road\\nw.ms3d");
	g_roadType[EASTWEST][FINISHED].Define("models\\road\\ew.ms3d");
	g_roadType[SOUTHWEST][FINISHED].Define("models\\road\\sw.ms3d");
	g_roadType[EASTSOUTHWEST][FINISHED].Define("models\\road\\esw.ms3d");
	g_roadType[NORTHSOUTHWEST][FINISHED].Define("models\\road\\nsw.ms3d");
	g_roadType[NORTHEASTWEST][FINISHED].Define("models\\road\\new.ms3d");
	g_roadType[NORTHEASTSOUTH][FINISHED].Define("models\\road\\nes.ms3d");
	g_roadType[NORTHEASTSOUTHWEST][FINISHED].Define("models\\road\\nesw.ms3d");
}

void DrawRoad(int x, int z)
{
	CRoad* r;
	r = RoadAt(x, z);
	
	if(!r->on)
		return;

	Vec3f pos = RoadPosition(x, z);
	
	CModel* m;
	int model;

	if(r->finished)
		model = g_roadType[r->type][FINISHED].model;
	else
		model = g_roadType[r->type][CONSTRUCTION].model;

	const float* owncol = facowncolor[r->owner];
	glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

	m = &g_model[model];
	m->UseTex();

	DrawVA(&r->vertexArray, pos);
}

void DrawRoadPlan(int x, int z)
{
	CRoad* r;
	r = RoadPlanAt(x, z);
	
	if(!r->on)
		return;
	
	Vec3f pos = RoadPosition(x, z);
	
	CModel* m;
	int model;

	if(r->finished)
		model = g_roadType[r->type][FINISHED].model;
	else
		model = g_roadType[r->type][CONSTRUCTION].model;

	const float* owncol = facowncolor[r->owner];
	glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

	m = &g_model[model];
	m->UseTex();

	DrawVA(&r->vertexArray, pos);
}

void DrawRoads()
{
	StartProfile(DRAWROADS);

	//int type;
	//CRoad* r;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			/*
			r = RoadAt(x, z);

			if(!r->on)
				continue;

			type = r->type;
			if(r->finished)
				g_roadType[type][FINISHED].Draw(x, z, s);
			else
				g_roadType[type][CONSTRUCTION].Draw(x, z, s);*/
			DrawRoad(x, z);
		}

	if(g_build != ROAD)
		return;

	//glColor4f(1,1,1,0.5f);
	glUniform4f(g_shader[g_curS].m_slot[SLOT::COLOR], 1, 1, 1, 0.5f);
		
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			/*
			r = RoadPlanAt(x, z);
			if(r->on)
			{
				type = r->type;
				g_roadType[type][FINISHED].Draw(x, z, s);
			}*/
			DrawRoadPlan(x, z);
		}

	//glColor4f(1,1,1,1);
	glUniform4f(g_shader[g_curS].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	EndProfile(DRAWROADS);
}

int GetRoadType(int x, int z, bool plan=false)
{
	bool n = false, e = false, s = false, w = false;

	if(x+1 < g_hmap.m_widthX && RoadAt(x+1, z)->on)
		e = true;
	if(x-1 >= 0 && RoadAt(x-1, z)->on)
		w = true;
	
	if(z+1 < g_hmap.m_widthZ && RoadAt(x, z+1)->on)
		s = true;
	if(z-1 >= 0 && RoadAt(x, z-1)->on)
		n = true;

	if(plan)
	{
		if(x+1 < g_hmap.m_widthX && RoadPlanAt(x+1, z)->on)
			e = true;
		if(x-1 >= 0 && RoadPlanAt(x-1, z)->on)
			w = true;
	
		if(z+1 < g_hmap.m_widthZ && RoadPlanAt(x, z+1)->on)
			s = true;
		if(z-1 >= 0 && RoadPlanAt(x, z-1)->on)
			n = true;
	}

	return GetConnectionType(n, e, s, w);
}

void MeshRoad(int x, int z, bool plan)
{
	CRoad* r = RoadAt(x, z);
	if(plan)
		r = RoadPlanAt(x, z);

	if(!r->on)
		return;

	int finished = 0;
	if(r->finished)
		finished = 1;

	CRoadType* t = &g_roadType[r->type][finished];
	CModel* m = &g_model[t->model];

	CVertexArray* rva = &r->vertexArray;
	CVertexArray* mva = &m->vertexArrays[0];

	rva->free();

	//g_log<<"meshroad allocating "<<mva->numverts<<"...";
	//g_log.flush();

	rva->numverts = mva->numverts;
	rva->vertices = new Vec3f[ rva->numverts ];
	rva->texcoords = new Vec2f[ rva->numverts ];
	rva->normals = new Vec3f[ rva->numverts ];

	float realx, realz;
	Vec3f pos = RoadPosition(x, z);
	
	for(int i=0; i<rva->numverts; i++)
	{
		rva->vertices[i] = mva->vertices[i];
		rva->texcoords[i] = mva->texcoords[i];
		rva->normals[i] = mva->normals[i];
		
		//realx = ((float)x+0.5f)*(float)TILE_SIZE + rva->vertices[i].x;
		//realz = ((float)z+0.5f)*(float)TILE_SIZE + rva->vertices[i].z;
		realx = pos.x + rva->vertices[i].x;
		realz = pos.z + rva->vertices[i].z;

		rva->vertices[i].y += Bilerp(realx, realz);

		//char msg[128];
		//sprintf(msg, "(%f,%f,%f)", mva->vertices[i].x, mva->vertices[i].y, mva->vertices[i].z);
		//Chat(msg);
	}
	
	//g_log<<"done meshroad"<<endl;
	//g_log.flush();
}

void TypeRoad(int x, int z, bool plan)
{
	CRoad* r = RoadAt(x, z);
	if(plan)
		r = RoadPlanAt(x, z);

	if(!r->on)
		return;

	r->type = GetRoadType(x, z, plan);
	MeshRoad(x, z, plan);
}

void TypeRoadsAround(int x, int z, bool plan)
{
	if(x+1 < g_hmap.m_widthX)
		TypeRoad(x+1, z, plan);
	if(x-1 >= 0)
		TypeRoad(x-1, z, plan);
	
	if(z+1 < g_hmap.m_widthZ)
		TypeRoad(x, z+1, plan);
	if(z-1 >= 0)
		TypeRoad(x, z-1, plan);
}

bool RoadPlaceable(int x, int z)
{
	Vec3f pos = RoadPosition(x, z);

	if(TileUnclimable(pos.x, pos.z))
		return false;

	if(CollidesWithBuildings(pos.x, pos.z, TILE_SIZE/2, TILE_SIZE/2))
		return false;

	return true;
}

void ResetRoadNetw()
{
	int lastnetw = 0;
	CBuilding* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		b->roadnetw.clear();
	}

	CRoad* r;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
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
	CBuilding* b;
	CBuilding* b2;

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

	CBuilding* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		bool found = false;
		for(int j=0; j<b->roadnetw.size(); j++)
		{
			if(b->roadnetw[j] == maxi)
			{
				if(!found)
				{
					b->roadnetw[j] = mini;
					found = true;
				}
				else
				{
					b->roadnetw.erase( b->roadnetw.begin() + j );
					j--;
				}
			}
		}
	}

	CRoad* r;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
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

bool CompareRoad(CRoad* r, int x, int z)
{
	CRoad* r2 = RoadAt(x, z);

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
	CBuilding* b = &g_building[i];
	CBuildingType* t = &g_buildingType[b->type];
	
	Vec3f p = b->pos;

	float hwx = t->widthX*TILE_SIZE/2.0f;
	float hwz = t->widthZ*TILE_SIZE/2.0f;

	Vec3f p2 = RoadPosition(x, z);

	float hwx2 = TILE_SIZE/2.0f;
	float hwz2 = TILE_SIZE/2.0f;

	if(fabs(p.x-p2.x) <= hwx+hwx2 && fabs(p.z-p2.z) <= hwz+hwz2)
		return true;

	return false;
}

bool CompareBRoad(CBuilding* b, CRoad* r)
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
		for(int i=0; i<b->roadnetw.size(); i++)
		{
			if(b->roadnetw[i] == r->netw)
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

	CRoad* r;
	CBuilding* b;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			r = RoadAt(x, z);

			if(!r->on)
				continue;

			if(!r->finished)
				continue;

			if(x > 0 && CompareRoad(r, x-1, z))
				change = true;
			if(x < g_hmap.m_widthX-1 && CompareRoad(r, x+1, z))
				change = true;
			if(z > 0 && CompareRoad(r, x, z-1))
				change = true;
			if(z < g_hmap.m_widthZ-1 && CompareRoad(r, x, z+1))
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
	vector<int> sharednetw;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		for(int j=0; j<b->roadnetw.size(); j++)
		{
			bool found = false;
			for(int k=0; k<sharednetw.size(); k++)
			{
				if(sharednetw[k] == b->roadnetw[j])
				{
					found = true;
					break;
				}
			}

			if(!found)
				sharednetw.push_back(b->roadnetw[j]);
		}
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		for(int j=0; j<sharednetw.size(); j++)
		{
			bool found = false;

			for(int k=0; k<b->roadnetw.size(); k++)
			{
				if(b->roadnetw[k] == sharednetw[j])
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				sharednetw.erase( sharednetw.begin() + j );
				j--;
			}
		}

		if(sharednetw.size() <= 0)
			OnRoadInacc();
	}

	if(sharednetw.size() > 0)
		OnAllRoadAc();
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

	CheckRoadAccess();
}

void RepossessRoad(int x, int z, int owner)
{
	CRoad* r = RoadAt(x, z);
	r->owner = owner;
	r->Allocate();
}

void PlaceRoad(int x, int z, int owner, bool plan)
{
	if(RoadAt(x, z)->on)
	{
		if(!plan)
		{
			RepossessRoad(x, z, owner);

			return;
		}
	}

	if(!RoadPlaceable(x, z))
		return;

	CRoad* r = RoadAt(x, z);
	if(plan)
		r = RoadPlanAt(x, z);

	r->on = true;
	r->owner = owner;

	if(!plan && g_mode == PLAY)
		r->Allocate();
	
	if(g_mode == PLAY)
		r->finished = false;
	if(plan || g_mode == EDITOR)
		r->finished = true;

	TypeRoad(x, z, plan);
	TypeRoadsAround(x, z, plan);

	for(int i=0; i<RESOURCES; i++)
		r->transporter[i] = -1;

	//if(!plan)
	//	ReRoadNetw();

	if(!plan)
	{
		Vec3f pos = RoadPosition(x, z);
		ClearScenery(pos.x, pos.z, TILE_SIZE/2.0f, TILE_SIZE/2.0f);
	}
}

void ClearRoadPlans()
{
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
			RoadPlanAt(x, z)->on = false;
}

void PlaceRoad()
{

	for(int x=0; x<g_hmap.m_widthX; x++)
	{
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CRoad* plan = RoadPlanAt(x, z);
			if(plan->on)
			{
				CRoad* actual = RoadAt(x, z);

				bool willchange = !actual->on;
				
				PlaceRoad(x, z, plan->owner);
				
				if(g_mode == PLAY && willchange)
				{
					NewJob(GOINGTOROADJOB, x, z);
				}
			}
		}
	}

	ClearRoadPlans();
	ReRoadNetw();
}

bool RoadLevel(float iterx, float iterz, float testx, float testz, float dx, float dz, int i, float d, CRoad* (*planfunc)(int x, int z))
{
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
	
	if(g_hmap.getheight(ix, iz) <= WATER_HEIGHT)		return false;
	if(g_hmap.getheight(ix+1, iz) <= WATER_HEIGHT)	return false;
	if(g_hmap.getheight(ix, iz+1) <= WATER_HEIGHT)	return false;
	if(g_hmap.getheight(ix+1, iz+1) <= WATER_HEIGHT)	return false;

	if(ix > 0)
	{
		if(RoadAt(ix-1, iz)->on)	w = true;
		//if(RoadPlanAt(ix-1, iz)->on)	w = true;
		if(planfunc != NULL)
			if(planfunc(ix-1, iz)->on) w = true;
	}

	if(ix < g_hmap.m_widthX-1)
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

	if(iz < g_hmap.m_widthZ-1)
	{
		if(RoadAt(ix, iz+1)->on)	n = true;
		//if(RoadPlanAt(ix, iz+1)->on)	n = true;
		if(planfunc != NULL)
			if(planfunc(ix, iz+1)->on) w = true;
	}

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

	return true;
}

void UpdateRoadPlans()
{
	ClearRoadPlans();

	int x1 = Clip(g_vStart.x/TILE_SIZE, 0, g_hmap.m_widthX-1);
	int z1 = Clip(g_vStart.z/TILE_SIZE, 0, g_hmap.m_widthZ-1);

	int x2 = Clip(g_vTile.x/TILE_SIZE, 0, g_hmap.m_widthX-1);
	int z2 = Clip(g_vTile.z/TILE_SIZE, 0, g_hmap.m_widthZ-1);

	if(!g_mousekeys[0])
	{
		x1 = x2;
		z1 = z2;
	}

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
			PlaceRoad(x, z, g_selP, true);

		if((int)x != prevx && (int)z != prevz)
		{
			if(RoadLevel(x, z, prevx, z, dx, dz, i, d))
				PlaceRoad(prevx, z, g_selP, true);
		}

		prevx = x;
		prevz = z;
	}
	
	if((int)x2 != prevx && (int)z2 != prevz)
	{
		if(RoadLevel(x2, z1, prevx, z2, dx, dz, i, d))
			PlaceRoad(prevx, z2, g_selP, true);
	}

	for(x1=0; x1<g_hmap.m_widthX; x1++)
		for(z1=0; z1<g_hmap.m_widthZ; z1++)
			MeshRoad(x1, z1, true);
}

bool RoadIntersect(int x, int z, Vec3f line[])
{
	CRoad* r = RoadAt(x, z);

	if(!r->on)
		return false;

	int finished = 0;
	if(r->finished)
		finished = 1;

	CRoadType* t = &g_roadType[r->type][finished];
	CModel* m = &g_model[t->model];

	CVertexArray* va = &m->vertexArrays[0];
	Vec3f poly[3];

	Vec3f pos = RoadPosition(x, z);

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