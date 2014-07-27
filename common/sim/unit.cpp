#include "unit.h"
#include "../render/shader.h"
#include "unittype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../render/model.h"
#include "../math/hmapmath.h"
#include "unitmove.h"
#include "sim.h"
#include "labourer.h"
#include "../debug.h"
#include "../math/frustum.h"

Unit g_unit[UNITS];

Unit::Unit()
{
	on = false;
	threadwait = false;
}

Unit::~Unit()
{
	destroy();
}

void Unit::destroy()
{
	on = false;
	threadwait = false;
}

void Unit::resetpath()
{
	path.clear();
	subgoal = cmpos;
	goal = cmpos;
	pathblocked = false;
}

void DrawUnits()
{
	for(int i=0; i<UNITS; i++)
	{
		StartTimer(TIMER_DRAWUMAT);

		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UnitT* t = &g_utype[u->type];

		Vec3f vmin(u->drawpos.x - t->size.x/2, u->drawpos.y, u->drawpos.z - t->size.x/2);
		Vec3f vmax(u->drawpos.x + t->size.x/2, u->drawpos.y + t->size.y, u->drawpos.z + t->size.x/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		Model* m = &g_model[t->model];

		StopTimer(TIMER_DRAWUMAT);

		m->draw(u->frame[BODY_LOWER], u->drawpos, u->rotation.y);
	}
}

int NewUnit()
{
	for(int i=0; i<UNITS; i++)
		if(!g_unit[i].on)
			return i;

	return -1;
}

void StartingBelongings(Unit* u)
{
	Zero(u->belongings);
	u->car = -1;
	u->home = -1;

	if(u->type == UNIT_LABOURER)
	{
		if(u->owner >= 0)
		{
			u->belongings[ RES_FUNDS ] = 100;
		}

		u->belongings[ RES_RETFOOD ] = STARTING_RETFOOD;
		u->belongings[ RES_LABOUR ] = STARTING_LABOUR;
	}
}

bool PlaceUnit(int type, Vec3i cmpos, int owner)
{
	int i = NewUnit();

	if(i < 0)
		return false;

#if 0
	bool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;

	/*
	The draw (floating-point) position vectory is used for drawing.
	*/
	Vec3f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec3i cmpos;
	Vec3f facing;
	Vec2f rotation;

	deque<Vec2i> path;
	Vec2i goal;

	int step;
	int target;
	int target2;
	bool targetu;
	bool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	bool passive;
	Vec2i prevpos;
	int taskframe;
	bool pathblocked;
	int frameslookjobago;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//std::vector<TransportJob> bids;

	float frame[2];
#endif

	Unit* u = &g_unit[i];
	UnitT* t = &g_utype[type];

	u->on = true;
	u->type = type;
	u->cmpos = Vec2i(cmpos.x, cmpos.z);
	u->drawpos = Vec3f(cmpos.x, cmpos.y, cmpos.z);
	u->owner = owner;
	u->path.clear();
	u->goal = Vec2i(cmpos.x, cmpos.z);
	u->target = -1;
	u->target2 = -1;
	u->targetu = false;
	u->underorder = false;
	u->fuelstation = -1;
	StartingBelongings(u);
	u->hp = t->starthp;
	u->passive = false;
	u->prevpos = u->cmpos;
	u->taskframe = 0;
	u->pathblocked = false;
	u->frameslookjobago = 0;
	u->supplier = -1;
	u->reqamt = 0;
	u->targtype = -1;
	u->frame[BODY_LOWER] = 0;
	u->frame[BODY_UPPER] = 0;
	u->subgoal = u->goal;

	u->mode = -1;
	u->pathdelay = 0;
	u->lastpath = g_simframe;

	u->fillcollider();

	return true;
}

void FreeUnits()
{
	for(int i=0; i<UNITS; i++)
	{
		g_unit[i].destroy();
		g_unit[i].on = false;
	}
}

bool Unit::hidden() const
{
	return false;
}

void AnimUnit(Unit* u)
{
	UnitT* t = &g_utype[u->type];

	if(u->type == UNIT_ROBOSOLDIER || u->type == UNIT_LABOURER)
	{
		if(u->prevpos == u->cmpos)
		{
			u->frame[BODY_LOWER] = 0;
			return;
		}

		PlayAnimation(u->frame[BODY_LOWER], 0, 29, true, 1.0f);
	}
}

void UpdateAI(Unit* u)
{
	if(u->type == UNIT_LABOURER)
		UpdLab(u);
}

void UpdUnits()
{
	for(int i = 0; i < UNITS; i++)
	{
		StartTimer(TIMER_UPDUONCHECK);

		Unit* u = &g_unit[i];

		if(!u->on)
		{
			StopTimer(TIMER_UPDUONCHECK);
			continue;
		}

		StopTimer(TIMER_UPDUONCHECK);

		StartTimer(TIMER_UPDUNITAI);
		UpdateAI(u);
		StopTimer(TIMER_UPDUNITAI);
		StartTimer(TIMER_MOVEUNIT);
		MoveUnit(u);
		StopTimer(TIMER_MOVEUNIT);
		StartTimer(TIMER_ANIMUNIT);
		AnimUnit(u);
		StopTimer(TIMER_ANIMUNIT);
	}
}
