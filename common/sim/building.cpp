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
#include "../math/frustum.h"
#include "unit.h"
#include "labourer.h"
#include "../econ/demand.h"
#include "../math/fixmath.h"

Building g_building[BUILDINGS];

void Building::destroy()
{
	while(occupier.size() > 0)
		Evict(&g_unit[*occupier.begin()]);

	while(worker.size() > 0)
		ResetMode(&g_unit[*worker.begin()]);

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

int Building::stillreq(int rtype)
{
	int prodleft = prodlevel - cymet;

	if(prodleft <= 0)
		return 0;
	
	BuildingT* bt = &g_bltype[type];

#if 0
	int lowr = -1;
	int lowamt = -1;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		if(lowr >= 0 && lowamt <= bt->input[ri])
			continue;

		lowr = ri;
		lowamt = bt->input[ri];
	}

	if(lowr < 0)
		return 0;
#endif

	int netreq = Ceili(bt->input[rtype] * prodleft, RATIO_DENOM);
	netreq -= stocked[rtype];

	Resource* r = &g_resource[rtype];
	Player* py = &g_player[owner];

	if(r->physical)
		netreq -= py->global[rtype];

	return netreq;
}

bool Building::excin(int rtype)
{
	//CBuildingType* bt = &g_buildingType[type];
	Player* py = &g_player[owner];

	int got = stocked[rtype];

	if(rtype != RES_LABOUR && g_resource[rtype].physical)
		got += py->global[rtype];

	//if(got >= prodquota * bt->input[res])
	if(got >= stillreq(rtype))
		return true;

	return false;
}

bool Building::metout()
{
	if(cymet >= prodlevel)
		return true;

	return false;
}

bool Building::hasworker(int ui)
{
	for(auto witer=worker.begin(); witer!=worker.end(); witer++)
		if(*witer == ui)
			return true;

	return false;
}

//try to produce if a minimum bundle of resources is had
bool Building::tryprod()
{
	//TO DO: produce, update cymet

	int minr = -1;
	int minamt = -1;

	BuildingT* bt = &g_bltype[type];

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		if(minr >= 0 && bt->input[ri] >= minamt)
			continue;

		minr = ri;
		minamt = bt->input[ri];
	}

	if(minr < 0)
		return true;

	int total[RESOURCES];
	Zero(total);
	Player* py = &g_player[owner];

	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		total[ri] += stocked[ri];

		if(r->physical)
			total[ri] += py->global[ri];
	}

	for(auto csiter=capsup.begin(); csiter!=capsup.end(); csiter++)
	{
		CapSup* cs = &*csiter;
		total[cs->rtype] += cs->amt;
	}

	int minbund[RESOURCES];
	Zero(minbund);
	
	int minlevel = total[minr] * RATIO_DENOM / bt->input[minr];

	for(int ri=0; ri<RESOURCES; ri++)
	{
		minbund[ri] = bt->input[ri] * minlevel / RATIO_DENOM;

		if(minbund[ri] < total[ri])
			return false;
	}

	//enough to produce, go ahead

	cymet += minlevel;

	//create output resources
	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(r->capacity)
		{
			//TO DO: check
			continue;
		}

		stocked[ri] += bt->output[ri] * minlevel / RATIO_DENOM;

		//TO DO: capacity, what if not enough supplied
	}

	//subtract used raw inputs
	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		int take = minbund[ri];

		if(r->physical)
		{
			int takeg = imin(take, py->global[ri]);
			py->global[ri] -= takeg;
			take -= takeg;
		}

		int takel = imin(take, stocked[ri]);
		py->local[ri] -= takel;
		stocked[ri] -= takel;
		take -= takel;
	}

	//TO DO: capacity resources like electricity have to be handled completely differently
	//amount input depends on required output, depends on duration of cycle, must be upward limited by max gen capacity of that res for that bl type
	//so lower bound of cycle at 60 sec, and upper bound? for those bl's that output capacity? or just disallow change cycle delay for those types of bl's?

	return true;
}

void RemWorker(Unit* w)
{
	Building* b = &g_building[w->target];

	int ui = w - g_unit;

	for(auto witer=b->worker.begin(); witer!=b->worker.end(); witer++)
		if(*witer == ui)
		{
			b->worker.erase(witer);
			return;
		}
}

void FreeBls()
{
	for(int i=0; i<BUILDINGS; i++)
	{
		g_building[i].destroy();
		g_building[i].on = false;
	}
}

int NewBl()
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
		totalhave += imin(cost[i], current[i]);
	}

	return (float)totalhave/(float)totalreq;
}

void Building::allocres()
{
	BuildingT* t = &g_bltype[type];
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
	BuildingT* t = &g_bltype[type];

	if(finished)
		CopyVA(&drawva, &g_model[t->model].m_va[0]);
	else
	{
		float pct = 0.2f + 0.8f * CompletPct(t->conmat, conmat);
		StageCopyVA(&drawva, &g_model[t->cmodel].m_va[0], pct);
	}

	if(t->hugterr)
		HugTerrain(&drawva, drawpos);

	drawva.genvbo();
}

bool Building::checkconstruction()
{
	BuildingT* t = &g_bltype[type];

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
			for(char ctype=0; ctype<CONDUIT_TYPES; ctype++)
				ReNetw(ctype);

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
			continue;

		const BuildingT* t = &g_bltype[b->type];
		//const BuildingT* t = &g_bltype[BUILDING_APARTMENT];
		Model* m = &g_model[ t->model ];

		Vec3f vmin(b->drawpos.x - t->widthx*TILE_SIZE/2, b->drawpos.y, b->drawpos.z - t->widthz*TILE_SIZE/2);
		Vec3f vmax(b->drawpos.x + t->widthx*TILE_SIZE/2, b->drawpos.y + (t->widthx+t->widthz)*TILE_SIZE/2, b->drawpos.z + t->widthz*TILE_SIZE/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		if(!b->finished)
			m = &g_model[ t->cmodel ];

		/*
		m->draw(0, b->drawpos, 0);
		*/

		float pitch = 0;
		float yaw = 0;
		Matrix modelmat;
		float radians[] = {(float)DEGTORAD(pitch), (float)DEGTORAD(yaw), 0};
		modelmat.translation((const float*)&b->drawpos);
		Matrix rotation;
		rotation.rotrad(radians);
		modelmat.postmult(rotation);
		glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

		Matrix modelview;
#ifdef SPECBUMPSHADOW
		modelview.set(g_camview.m_matrix);
#endif
		modelview.postmult(modelmat);
		glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

		Matrix mvp;
#if 0
		mvp.set(modelview.m_matrix);
		mvp.postmult(g_camproj);
#elif 0
		mvp.set(g_camproj.m_matrix);
		mvp.postmult(modelview);
#else
		mvp.set(g_camproj.m_matrix);
		mvp.postmult(g_camview);
		mvp.postmult(modelmat);
#endif
		glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

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

void UpdBls()
{
#if 0
	static float completion = 2.0f;

	if(completion+EPSILON >= 1 && completion-EPSILON <= 1)
		Explode(&g_building[1]);

#if 1
	//StageCopyVA(&g_building[0].drawva, &g_model[g_bltype[g_building[0].type].model].m_va[0], completion);
	HeightCopyVA(&g_building[1].drawva, &g_model[g_bltype[g_building[1].type].model].m_va[0], Clipf(completion, 0, 1));

	completion -= 0.01f;

	if(completion < -1.0f)
		completion = 2.0f;
#elif 1
	HeightCopyVA(&g_building[0].drawva, &g_model[g_bltype[g_building[0].type].model].m_va[0], completion);

	completion *= 0.95f;

	if(completion < 0.001f)
		completion = 1.0f;
#endif
#elif 0
	static float completion = 2.0f;

	StageCopyVA(&g_building[0].drawva, &g_model[g_bltype[g_building[0].type].cmodel].m_va[0], completion);

	completion += 0.005f;

	if(completion < 2.0f && completion >= 1.0f)
	{
		g_building[0].finished = true;
		CopyVA(&g_building[0].drawva, &g_model[g_bltype[g_building[0].type].model].m_va[0]);
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

		BuildingT* t = &g_bltype[b->type];
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
	BuildingT* t = &g_bltype[b->type];
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
