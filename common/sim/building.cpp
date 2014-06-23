

#include "building.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "buildingtype.h"
#include "../render/model.h"
#include "../render/foliage.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../render/shader.h"
#include "../math/barycentric.h"
#include "../math/physics.h"
#include "player.h"
#include "../render/transaction.h"
#include "selection.h"
#include "../gui/gui.h"
#include "../gui/widget.h"
#include "../gui/widgets/spez/constructionview.h"
#include "powl.h"
#include "road.h"
#include "crpipe.h"
#include "../../game/gmain.h"
#include "../render/shadow.h"

Building g_building[BUILDINGS];

void Building::destroy()
{
	on = false;
}

Building::Building()
{
	on = false;
}

Building::~Building()
{
	destroy();
}

void FreeBuildings()
{
	for(int i=0; i<BUILDINGS; i++)
	{
		g_building[i].destroy();
		g_building[i].on = false;
	}
}

int NewBuilding()
{
	for(int i=0; i<BUILDINGS; i++)
		if(!g_building[i].on)
			return i;

	return -1;
}

float CompletPct(int* cost, int* current)
{
	int totalreq = 0;

	for(int i=0; i<RESOURCES; i++)
	{
		totalreq += cost[i];
	}

	int totalhave = 0;

	for(int i=0; i<RESOURCES; i++)
	{
		totalhave += std::min(cost[i], current[i]);
	}

	return (float)totalhave/(float)totalreq;
}

bool PlaceBuilding(int type, Vec2i pos, bool finished, int owner)
{
	int i = NewBuilding();

	if(i < 0)
		return false;

	Building* b = &g_building[i];
	b->on = true;
	b->type = type;
	b->tilepos = pos;

	BuildingT* t = &g_buildingT[type];

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = pos.x - t->widthx/2;
	tmin.y = pos.y - t->widthz/2;
	tmax.x = tmin.x + t->widthx;
	tmax.y = tmin.y + t->widthz;

	b->drawpos = Vec3f(pos.x*TILE_SIZE, Lowest(tmin.x, tmin.y, tmax.x, tmax.y), pos.y*TILE_SIZE);

	if(t->foundation == FOUNDATION_SEA)
		b->drawpos.y = WATER_LEVEL;

#if 1
	if(t->widthx % 2 == 1)
		b->drawpos.x += TILE_SIZE/2;

	if(t->widthz % 2 == 1)
		b->drawpos.z += TILE_SIZE/2;

	b->owner = owner;

	b->finished = finished;

	Zero(b->conmat);
	Zero(b->stocked);
	Zero(b->maxcost);

	int cmminx = tmin.x*TILE_SIZE;
	int cmminz = tmin.y*TILE_SIZE;
	int cmmaxx = cmminx + t->widthx*TILE_SIZE;
	int cmmaxz = cmminz + t->widthz*TILE_SIZE;

	ClearFoliage(cmminx, cmminz, cmmaxx, cmmaxz);
#if 0
	ClearPowerlines(cmminx, cmminz, cmmaxx, cmmaxz);
	ClearPipelines(cmminx, cmminz, cmmaxx, cmmaxz);
	RePow();
	RePipe();
	ReRoadNetw();
#endif

#endif

	b->remesh();

	if(g_mode == APPMODE_PLAY)
	{
		//b->allocres();
		b->inoperation = false;

		Player* py = &g_player[g_currP];

		ClearSel(&py->sel);
		py->sel.buildings.push_back(i);

		if(!b->finished)
		{
			Player* py = &g_player[g_currP];
			GUI* gui = &py->gui;
			ConstructionView* cv = (ConstructionView*)gui->get("construction view")->get("construction view");
			cv->regen(&py->sel);
			gui->open("construction view");
		}
	}
	else
	{
		b->inoperation = true;
	}

	b->fillcollider();

	return true;
}

void Building::allocres()
{
	BuildingT* t = &g_buildingT[type];
	Player* py = &g_player[owner];

	int alloc;

	RichText transx;

	for(int i=0; i<RESOURCES; i++)
	{
		if(t->conmat[i] <= 0)
			continue;

		if(i == RES_LABOUR)
			continue;

		alloc = t->conmat[i] - conmat[i];

		if(py->global[i] < alloc)
			alloc = py->global[i];

		conmat[i] += alloc;
		py->global[i] -= alloc;

		if(alloc > 0)
		{
			char numpart[128];
			sprintf(numpart, "%+d", -alloc);
			transx.m_part.push_back( RichTextP( numpart ) );
			transx.m_part.push_back( RichTextP( RICHTEXT_ICON, g_resource[i].icon ) );
			transx.m_part.push_back( RichTextP( g_resource[i].name.c_str() ) );
			transx.m_part.push_back( RichTextP( "\n" ) );
		}
	}

	if(transx.m_part.size() > 0
#ifdef LOCAL_TRANSX
       && owner == g_localP
#endif
       )
		NewTransx(drawpos, &transx);

	checkconstruction();
}

void Building::remesh()
{
	BuildingT* t = &g_buildingT[type];

	if(finished)
		CopyVA(&drawva, &g_model[t->model].m_va[0]);
	else
	{
		float pct = 0.2f + 0.8f * CompletPct(t->conmat, conmat);
		StageCopyVA(&drawva, &g_model[t->cmodel].m_va[0], pct);
	}

	if(t->hugterr)
		HugTerrain(&drawva, drawpos);
}

bool Building::checkconstruction()
{
	BuildingT* t = &g_buildingT[type];

	bool haveall = true;

	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < t->conmat[i])
		{
			haveall = false;
			break;
		}

#if 0
	if(owner == g_localP)
	{
		char msg[128];
		sprintf(msg, "%s construction complete.", t->name);
		Chat(msg);
		ConCom();
	}
#endif

	if(haveall && !finished)
	{
		RePow();
		ReCrPipe();
		ReRoadNetw();
	}

	//if(owner == g_localP)
	//	OnFinishedB(type);

	finished = haveall;

	remesh();

	return finished;
}

void DrawBl()
{
	Shader* s = &g_shader[g_curS];

	//return;
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
		{
			continue;
		}

		const BuildingT* t = &g_buildingT[b->type];
		//const BuildingT* t = &g_buildingT[BUILDING_APARTMENT];
		Model* m = &g_model[ t->model ];

		if(!b->finished)
			m = &g_model[ t->cmodel ];

		/*
		m->draw(0, b->drawpos, 0);
		*/

		float pitch = 0;
		float yaw = 0;
		Matrix modelmat;
		float radians[] = {static_cast<float>(DEGTORAD(pitch)), static_cast<float>(DEGTORAD(yaw)), 0};
		modelmat.translation((const float*)&b->drawpos);
		Matrix rotation;
		rotation.rotrad(radians);
		modelmat.postmult(rotation);
		glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

		Matrix modelview;
		modelview.set(modelmat.m_matrix);
#ifdef SPECBUMPSHADOW
		modelview.postmult(g_camview);
#endif
		//modelview.set(g_camview.m_matrix);
		//modelview.postmult(modelmat);
		Matrix modelviewinv;
		Transpose(modelview, modelview);
		Inverse2(modelview, modelviewinv);
		//Transpose(modelviewinv, modelviewinv);
		glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

		VertexArray* va = &b->drawva;

		m->usetex();

		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);

		if(s->m_slot[SSLOT_NORMAL] != -1)
			glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, va->numverts);
	}
}

void UpdateBuildings()
{
#if 0
	static float completion = 2.0f;

	if(completion+EPSILON >= 1 && completion-EPSILON <= 1)
		Explode(&g_building[1]);

#if 1
	//StageCopyVA(&g_building[0].drawva, &g_model[g_buildingT[g_building[0].type].model].m_va[0], completion);
	HeightCopyVA(&g_building[1].drawva, &g_model[g_buildingT[g_building[1].type].model].m_va[0], Clipf(completion, 0, 1));

	completion -= 0.01f;

	if(completion < -1.0f)
		completion = 2.0f;
#elif 1
	HeightCopyVA(&g_building[0].drawva, &g_model[g_buildingT[g_building[0].type].model].m_va[0], completion);

	completion *= 0.95f;

	if(completion < 0.001f)
		completion = 1.0f;
#endif
#elif 0
	static float completion = 2.0f;

	StageCopyVA(&g_building[0].drawva, &g_model[g_buildingT[g_building[0].type].cmodel].m_va[0], completion);

	completion += 0.005f;

	if(completion < 2.0f && completion >= 1.0f)
	{
		g_building[0].finished = true;
		CopyVA(&g_building[0].drawva, &g_model[g_buildingT[g_building[0].type].model].m_va[0]);
	}
	if(completion >= 2.0f)
	{
		completion = 0.1f;
		g_building[0].finished = false;
	}
#endif

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		BuildingT* t = &g_buildingT[b->type];
		EmitterPlace* ep;
		ParticleT* pt;

		if(!b->finished)
			continue;

		for(int j=0; j<MAX_B_EMITTERS; j++)
		{
			//first = true;

			//if(completion < 1)
			//	continue;

			ep = &t->emitterpl[j];

			if(!ep->on)
				continue;

			pt = &g_particleT[ep->type];

			if(b->emitterco[j].EmitNext(pt->delay))
				EmitParticle(ep->type, b->drawpos + ep->offset);
		}
	}
}

void StageCopyVA(VertexArray* to, VertexArray* from, float completion)
{
	CopyVA(to, from);

	float maxy = 0;
	for(int i=0; i<to->numverts; i++)
		if(to->vertices[i].y > maxy)
			maxy = to->vertices[i].y;

	float limity = maxy*completion;

	for(int tri=0; tri<to->numverts/3; tri++)
	{
		Vec3f* belowv = NULL;
		Vec3f* belowv2 = NULL;

		for(int v=tri*3; v<tri*3+3; v++)
		{
			if(to->vertices[v].y <= limity)
			{
				if(!belowv)
					belowv = &to->vertices[v];
				else if(!belowv2)
					belowv2 = &to->vertices[v];

				//break;
			}
		}

		Vec3f prevt[3];
		prevt[0] = to->vertices[tri*3+0];
		prevt[1] = to->vertices[tri*3+1];
		prevt[2] = to->vertices[tri*3+2];

		Vec2f prevc[3];
		prevc[0] = to->texcoords[tri*3+0];
		prevc[1] = to->texcoords[tri*3+1];
		prevc[2] = to->texcoords[tri*3+2];

		for(int v=tri*3; v<tri*3+3; v++)
		{
			if(to->vertices[v].y > limity)
			{
				float prevy = to->vertices[v].y;
				to->vertices[v].y = limity;
#if 0
#if 0
				void barycent(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2,
                         double vx, double vy, double vz,
                         double *u, double *v, double *w)
#endif

				double ratio0 = 0;
				double ratio1 = 0;
				double ratio2 = 0;

				barycent(prevt[0].x, prevt[0].y, prevt[0].z,
					prevt[1].x, prevt[1].y, prevt[1].z,
					prevt[2].x, prevt[2].y, prevt[2].z,
					to->vertices[v].x, to->vertices[v].y, to->vertices[v].z,
					&ratio0, &ratio1, &ratio2);

				to->texcoords[v].x = ratio0 * prevc[0].x + ratio1 * prevc[1].x + ratio2 * prevc[2].x;
				to->texcoords[v].y = ratio0 * prevc[0].y + ratio1 * prevc[1].y + ratio2 * prevc[2].y;
#elif 0
				Vec3f* closebelowv = NULL;

				if(belowv)
					closebelowv = belowv;

				if(belowv2 && (!closebelowv || Magnitude2((*closebelowv) - to->vertices[v]) > Magnitude2((*belowv2) - to->vertices[v])))
					closebelowv = belowv2;

				float yratio = (closebelowv->y - prevy);
#endif
			}
		}
	}
}


void HeightCopyVA(VertexArray* to, VertexArray* from, float completion)
{
	CopyVA(to, from);

	float maxy = 0;
	for(int i=0; i<to->numverts; i++)
		if(to->vertices[i].y > maxy)
			maxy = to->vertices[i].y;

	float dy = maxy*(1.0f-completion);

	for(int tri=0; tri<to->numverts/3; tri++)
	{
		Vec3f* belowv = NULL;
		Vec3f* belowv2 = NULL;

		for(int v=tri*3; v<tri*3+3; v++)
		{
			to->vertices[v].y -= dy;
		}
	}
}

void HugTerrain(VertexArray* va, Vec3f pos)
{
	for(int i=0; i<va->numverts; i++)
	{
		va->vertices[i].y += Bilerp(&g_hmap, pos.x + va->vertices[i].x, pos.z + va->vertices[i].z) - pos.y;
	}
}

void Explode(Building* b)
{
	BuildingT* t = &g_buildingT[b->type];
	float hwx = t->widthx*TILE_SIZE/2.0f;
	float hwz = t->widthz*TILE_SIZE/2.0f;
	Vec3f p;

	for(int i=0; i<5; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(PARTICLE_FIREBALL, p + b->drawpos);
	}

	for(int i=0; i<10; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(PARTICLE_FIREBALL2, p + b->drawpos);
	}
	/*
     for(int i=0; i<5; i++)
     {
     p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
     p.y = 8;
     p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
     EmitParticle(SMOKE, p + pos);
     }

     for(int i=0; i<5; i++)
     {
     p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
     p.y = 8;
     p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
     EmitParticle(SMOKE2, p + pos);
     }
     */
	for(int i=0; i<20; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(PARTICLE_DEBRIS, p + b->drawpos);
	}
}
