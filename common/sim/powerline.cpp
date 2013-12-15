


#include "powerline.h"
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
#include "debug.h"
#include "road.h"

CPowerlineType g_powerlineType[CONNECTION_TYPES][2];
CPowerline* g_powerline = NULL;
CPowerline* g_powerlinePlan = NULL;
float g_powerlineCost[RESOURCES];

CPowerline::CPowerline() 
{ 
	on = false; 
	finished = false;
	Zero(conmat);
	netw = -1;
	//vertexArray.safetyConstructor();

	//g_log<<"CPowerline() vertexArray.numverts = "<<vertexArray.numverts<<endl;
	//g_log.flush();
}

CPowerline::~CPowerline()
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

void CPowerlineType::draw(int x, int z) 
{
	Vec3f pos = PowerlinePosition(x, z);
	g_model[model].draw(0, pos, 0);
}

void CPowerlineType::Define(char* modelfile)
{
	QueueModel(&model, modelfile, Vec3f(1,1,1), Vec3f(0,0,0));
}

CPowerline* PowlAt(int x, int z)
{
	return &g_powerline[ x + z*g_hmap.m_widthX ];
}

CPowerline* PowlPlanAt(int x, int z)
{
	return &g_powerlinePlan[ x + z*g_hmap.m_widthX ];
}

void PowlXZ(CPowerline* pow, int& x, int& z)
{
	int x1, z1;

	for(x1=0; x1<g_hmap.m_widthX; x1++)
		for(z1=0; z1<g_hmap.m_widthZ; z1++)
		{
			if(PowlAt(x1, z1) == pow)
			{
				x = x1;
				z = z1;
				return;
			}
		}
}

float CPowerline::netreq(int res)
{
	float netrq = 0;

	if(!finished)
	{
		netrq = g_powerlineCost[res] - conmat[res];
	}

	return netrq;
}

void CPowerline::destroy()
{
	Zero(conmat);
	on = false;
	finished = false;
	netw = -1;
}

void CPowerline::Allocate()
{
	CPlayer* p = &g_player[owner];

	float alloc;

	char transx[32];
	transx[0] = '\0';

	for(int i=0; i<RESOURCES; i++)
	{
		if(g_powerlineCost[i] <= 0)
			continue;

		if(i == LABOUR)
			continue;

		alloc = g_powerlineCost[i] - conmat[i];

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
		NewTransx(PowerlinePosition(x, z), transx);
	}

	CheckConstruction();
}

bool CPowerline::CheckConstruction()
{
	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < g_powerlineCost[i])
			return false;

	if(owner == g_localP)
	{
		Chat("Powerline construction complete.");
		ConCom();
	}

	finished = true;
	RePow();

	int x, z;
	PowlXZ(this, x, z);
	MeshPowl(x, z);
	
	if(owner == g_localP)
		OnFinishedB(POWERLINE);

	return true;
}

void CPowerline::Emit(int t)
{
	Vec3f pos;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			if(PowlAt(x, z) != this)
				continue;
			
			pos = PowerlinePosition(x, z);
			EmitParticle(t, pos + Vec3f(0, 10, 0));
			return;
		}
}

bool PowlHasRoad(int x, int z)
{
	Vec3f physpos = PowerlinePosition(x, z);

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

Vec3f PowerlinePosition(int x, int z)
{
	float fx = x*TILE_SIZE;
	float fz = z*TILE_SIZE;
	//float fy = Bilerp(fx, fz);

	return Vec3f(fx, 0, fz);
}

void InitPowerlines()
{
	Zero(g_powerlineCost);
	g_powerlineCost[LABOUR] = 1;
	g_powerlineCost[METAL] = 1;

	g_powerlineType[NOCONNECTION][CONSTRUCTION].Define("models\\powerline\\1_c.ms3d");
	g_powerlineType[NORTH][CONSTRUCTION].Define("models\\powerline\\n_c.ms3d");
	g_powerlineType[EAST][CONSTRUCTION].Define("models\\powerline\\e_c.ms3d");
	g_powerlineType[SOUTH][CONSTRUCTION].Define("models\\powerline\\s_c.ms3d");
	g_powerlineType[WEST][CONSTRUCTION].Define("models\\powerline\\w_c.ms3d");
	g_powerlineType[NORTHEAST][CONSTRUCTION].Define("models\\powerline\\ne_c.ms3d");
	g_powerlineType[NORTHSOUTH][CONSTRUCTION].Define("models\\powerline\\ns_c.ms3d");
	g_powerlineType[EASTSOUTH][CONSTRUCTION].Define("models\\powerline\\es_c.ms3d");
	g_powerlineType[NORTHWEST][CONSTRUCTION].Define("models\\powerline\\nw_c.ms3d");
	g_powerlineType[EASTWEST][CONSTRUCTION].Define("models\\powerline\\ew_c.ms3d");
	g_powerlineType[SOUTHWEST][CONSTRUCTION].Define("models\\powerline\\sw_c.ms3d");
	g_powerlineType[EASTSOUTHWEST][CONSTRUCTION].Define("models\\powerline\\esw_c.ms3d");
	g_powerlineType[NORTHSOUTHWEST][CONSTRUCTION].Define("models\\powerline\\nsw_c.ms3d");
	g_powerlineType[NORTHEASTWEST][CONSTRUCTION].Define("models\\powerline\\new_c.ms3d");
	g_powerlineType[NORTHEASTSOUTH][CONSTRUCTION].Define("models\\powerline\\nes_c.ms3d");
	g_powerlineType[NORTHEASTSOUTHWEST][CONSTRUCTION].Define("models\\powerline\\nesw_c.ms3d");
	g_powerlineType[NOCONNECTION][FINISHED].Define("models\\powerline\\1.ms3d");
	g_powerlineType[NORTH][FINISHED].Define("models\\powerline\\n.ms3d");
	g_powerlineType[EAST][FINISHED].Define("models\\powerline\\e.ms3d");
	g_powerlineType[SOUTH][FINISHED].Define("models\\powerline\\s.ms3d");
	g_powerlineType[WEST][FINISHED].Define("models\\powerline\\w.ms3d");
	g_powerlineType[NORTHEAST][FINISHED].Define("models\\powerline\\ne.ms3d");
	g_powerlineType[NORTHSOUTH][FINISHED].Define("models\\powerline\\ns.ms3d");
	g_powerlineType[EASTSOUTH][FINISHED].Define("models\\powerline\\es.ms3d");
	g_powerlineType[NORTHWEST][FINISHED].Define("models\\powerline\\nw.ms3d");
	g_powerlineType[EASTWEST][FINISHED].Define("models\\powerline\\ew.ms3d");
	g_powerlineType[SOUTHWEST][FINISHED].Define("models\\powerline\\sw.ms3d");
	g_powerlineType[EASTSOUTHWEST][FINISHED].Define("models\\powerline\\esw.ms3d");
	g_powerlineType[NORTHSOUTHWEST][FINISHED].Define("models\\powerline\\nsw.ms3d");
	g_powerlineType[NORTHEASTWEST][FINISHED].Define("models\\powerline\\new.ms3d");
	g_powerlineType[NORTHEASTSOUTH][FINISHED].Define("models\\powerline\\nes.ms3d");
	g_powerlineType[NORTHEASTSOUTHWEST][FINISHED].Define("models\\powerline\\nesw.ms3d");
}

void DrawPowerline(int x, int z, bool plan)
{
	CPowerline* p;
	if(plan)
		p = PowlPlanAt(x, z);
	else
		p = PowlAt(x, z);
	
	if(!p->on)
		return;

	Vec3f pos = PowerlinePosition(x, z);
	
	CModel* m;
	int model;

	if(p->finished)
		model = g_powerlineType[p->type][FINISHED].model;
	else
		model = g_powerlineType[p->type][CONSTRUCTION].model;

	const float* owncol = facowncolor[p->owner];
	glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

	m = &g_model[model];
	m->UseTex();

	DrawVA(&p->vertexArray, pos);
}

void DrawPowerlines()
{
	StartProfile(DRAWPOWERLINES);

	//int type;
	//CPowerline* p;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			/*
			p = PowlAt(x, z);

			if(!p->on)
				continue;

			const float* owncol = facowncolor[p->owner];
			glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

			//type = p->type;
			DrawVA(&p->vertexArray, PowerlinePosition(x, z));
			
			//if(p->finished)
			//	g_powerlineType[type][FINISHED].draw(x, z);
			//else
			//	g_powerlineType[type][CONSTRUCTION].draw(x, z);*/
			DrawPowerline(x, z, false);
		}

	if(g_build != POWERLINE)
		return;

	//glColor4f(1,1,1,0.5f);
	glUniform4f(g_shader[g_curS].m_slot[SLOT::COLOR], 1, 1, 1, 0.5f);
		
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			/*
			p = PowlPlanAt(x, z);
			if(!p->on)
				continue;

			const float* owncol = facowncolor[p->owner];
			glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

			//type = p->type;
			DrawVA(&p->vertexArray, PowerlinePosition(x, z));
			//g_powerlineType[type][FINISHED].draw(x, z);*/
			DrawPowerline(x, z, true);
		}

	//glColor4f(1,1,1,1);
	glUniform4f(g_shader[g_curS].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	EndProfile(DRAWPOWERLINES);
}

int GetPowerlineType(int x, int z, bool plan=false)
{
	bool n = false, e = false, s = false, w = false;

	if(x+1 < g_hmap.m_widthX && PowlAt(x+1, z)->on)
		e = true;
	if(x-1 >= 0 && PowlAt(x-1, z)->on)
		w = true;
	
	if(z+1 < g_hmap.m_widthZ && PowlAt(x, z+1)->on)
		s = true;
	if(z-1 >= 0 && PowlAt(x, z-1)->on)
		n = true;

	if(plan)
	{
		if(x+1 < g_hmap.m_widthX && PowlPlanAt(x+1, z)->on)
			e = true;
		if(x-1 >= 0 && PowlPlanAt(x-1, z)->on)
			w = true;
	
		if(z+1 < g_hmap.m_widthZ && PowlPlanAt(x, z+1)->on)
			s = true;
		if(z-1 >= 0 && PowlPlanAt(x, z-1)->on)
			n = true;
	}

	return GetConnectionType(n, e, s, w);
}

void MeshPowl(int x, int z, bool plan)
{
	CPowerline* p = PowlAt(x, z);
	if(plan)
		p = PowlPlanAt(x, z);

	if(!p->on)
		return;

	int finished = 0;
	if(p->finished)
		finished = 1;

	CPowerlineType* t = &g_powerlineType[p->type][finished];
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
	Vec3f pos = PowerlinePosition(x, z);
	
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

void TypePowerline(int x, int z, bool plan)
{
	CPowerline* p = PowlAt(x, z);
	if(plan)
		p = PowlPlanAt(x, z);

	if(!p->on)
		return;

	p->type = GetPowerlineType(x, z, plan);
	MeshPowl(x, z, plan);
}

void TypePowerlinesAround(int x, int z, bool plan)
{
	if(x+1 < g_hmap.m_widthX)
		TypePowerline(x+1, z, plan);
	if(x-1 >= 0)
		TypePowerline(x-1, z, plan);
	
	if(z+1 < g_hmap.m_widthZ)
		TypePowerline(x, z+1, plan);
	if(z-1 >= 0)
		TypePowerline(x, z-1, plan);
}

bool PowerlinePlaceable(int x, int z)
{
	Vec3f pos = PowerlinePosition(x, z);

	if(TileUnclimable(pos.x, pos.z))
		return false;

	if(CollidesWithBuildings(pos.x, pos.z, 0, 0))
		return false;

	//Make sure the powerline isn't surrounded by buildings or map edges
	if( ((x<=0 || z<=0) || CollidesWithBuildings((x-0.5f)*TILE_SIZE, (z-0.5f)*TILE_SIZE, 0, 0)) &&
		((x<=0 || z>=g_hmap.m_widthZ) || CollidesWithBuildings((x-0.5f)*TILE_SIZE, (z+0.5f)*TILE_SIZE, 0, 0)) &&
		((x>=g_hmap.m_widthX || z>=g_hmap.m_widthZ) || CollidesWithBuildings((x+0.5f)*TILE_SIZE, (z+0.5f)*TILE_SIZE, 0, 0)) &&
		((x>=g_hmap.m_widthX || z<=0) || CollidesWithBuildings((x+0.5f)*TILE_SIZE, (z-0.5f)*TILE_SIZE, 0, 0)) )
		return false;

	return true;
}

void RepossessPowl(int x, int z, int owner)
{
	CPowerline* powl = PowlAt(x, z);
	powl->owner = owner;
	powl->Allocate();
}

void PlacePowerline(int x, int z, int owner, bool plan)
{
	//g_log<<"place pow 1"<<endl;
	//g_log.flush();

	if(PowlAt(x, z)->on)
	{
		if(!plan)
		{
			RepossessPowl(x, z, owner);

			return;
		}
	}
	
	//g_log<<"place pow 2"<<endl;
	//g_log.flush();

	if(!PowerlinePlaceable(x, z))
		return;
	
	//g_log<<"place pow 3"<<endl;
	//g_log.flush();

	CPowerline* p = PowlAt(x, z);
	if(plan)
		p = PowlPlanAt(x, z);
	
	//g_log<<"place pow 4"<<endl;
	//g_log.flush();

	p->on = true;
	p->owner = owner;
	p->netw = -1;
	
	//g_log<<"place pow 5"<<endl;
	//g_log.flush();

	if(!plan && g_mode == PLAY)
		p->Allocate();
	
	if(g_mode == PLAY)
		p->finished = false;
	if(plan || g_mode == EDITOR)
		p->finished = true;

	
	//g_log<<"place pow 6"<<endl;
	//g_log.flush();

	TypePowerline(x, z, plan);
	
	//g_log<<"place pow 7"<<endl;
	//g_log.flush();

	TypePowerlinesAround(x, z, plan);
	
	//g_log<<"place pow 8"<<endl;
	//g_log.flush();

	for(int i=0; i<RESOURCES; i++)
		p->transporter[i] = -1;
}

void ClearPowerlinePlans()
{
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			PowlPlanAt(x, z)->on = false;
		}
}

void PlacePowerline()
{
	CPowerline* p;
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			p = PowlPlanAt(x, z);
			if(p->on)
			{
				CPowerline* actual = PowlAt(x, z);

				bool willchange = !actual->on;

				PlacePowerline(x, z, p->owner);
				
				if(g_mode == PLAY && willchange)
				{
					NewJob(GOINGTOPOWLJOB, x, z);
				}
			}
		}

	ClearPowerlinePlans();
	RePow();
}

void UpdatePowerlinePlans()
{
	ClearPowerlinePlans();
	
	int x1 = Clip(g_vStart.x/TILE_SIZE, 0, g_hmap.m_widthX-1);
	int z1 = Clip(g_vStart.z/TILE_SIZE, 0, g_hmap.m_widthZ-1);

	int x2 = Clip(g_vTile.x/TILE_SIZE, 0, g_hmap.m_widthX-1);
	int z2 = Clip(g_vTile.z/TILE_SIZE, 0, g_hmap.m_widthZ-1);

	if(!g_mousekeys[0])
	{
		x1 = x2;
		z1 = z2;
	}

	PlacePowerline(x1, z1, g_selP, true);
	PlacePowerline(x2, z2, g_selP, true);

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
		PlacePowerline(x, z, g_selP, true);

		if((int)x != prevx && (int)z != prevz)
			PlacePowerline(prevx, z, g_selP, true);

		prevx = x;
		prevz = z;
	}
	
	if((int)x2 != prevx && (int)z2 != prevz)
		PlacePowerline(prevx, z2, g_selP, true);
	
	for(x1=0; x1<g_hmap.m_widthX; x1++)
		for(z1=0; z1<g_hmap.m_widthZ; z1++)
			MeshPowl(x1, z1, true);
}

void ClearPowerlines()
{
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
			if(PowlAt(x, z)->on && !PowerlinePlaceable(x, z))
			{
				PowlAt(x, z)->on = false;
				TypePowerlinesAround(x, z);
			}
}


void MergePow(int A, int B)
{
	int mini = min(A, B);
	int maxi = max(A, B);

	CBuilding* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(b->pownetw == maxi)
			b->pownetw = mini;
	}

	CPowerline* pow;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
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
	}

	return change;
}

bool ComparePow(CPowerline* pow, int x, int z)
{
	CPowerline* pow2 = PowlAt(x, z);

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

bool CompareBPow(CBuilding* b, CPowerline* pow)
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

	CPowerline* pow;
	CBuilding* b;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
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
			if(x < g_hmap.m_widthX-1 && ComparePow(pow, x+1, z))
				change = true;
			if(z > 0 && ComparePow(pow, x, z-1))
				change = true;
			if(z < g_hmap.m_widthZ-1 && ComparePow(pow, x, z+1))
				change = true;

			for(int i=0; i<BUILDINGS; i++)
			{
				b = &g_building[i];

				if(!b->on)
					continue;

				if(!PowerlineAdjacent(i, x, z))
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
	CBuilding* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		b->pownetw = -1;
	}

	CPowerline* pow;

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			pow = PowlAt(x, z);

			if(!pow->on)
				continue;

			if(!pow->finished)
				continue;

			pow->netw = -1;
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

		if(t->output[ELECTRICITY] <= 0.0f)
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
	CPowerline* p = PowlAt(x, z);

	if(!p->on)
		return false;

	int finished = 0;
	if(p->finished)
		finished = 1;

	CPowerlineType* t = &g_powerlineType[p->type][finished];
	CModel* m = &g_model[t->model];

	CVertexArray* va = &m->vertexArrays[0];
	Vec3f poly[3];

	Vec3f pos = PowerlinePosition(x, z);

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