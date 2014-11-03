#include "labourer.h"
#include "../platform.h"
#include "../econ/demand.h"
#include "unittype.h"
#include "player.h"
#include "building.h"
#include "../econ/utility.h"
#include "../script/console.h"
#include "simdef.h"
#include "job.h"
#include "unitmove.h"
#include "../../game/gui/chattext.h"
#include "../render/transaction.h"

bool NeedFood(Unit* u)
{
	if(u->belongings[RES_RETFOOD] < STARTING_RETFOOD/2)
		return true;

	return false;
}

bool FindFood(Unit* u)
{
	Vec2i bcmpos;
	int bestbi = -1;
	int bestutil = -1;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_RETFOOD] <= 0)
			continue;

		Player* py = &g_player[b->owner];

		if(b->stocked[RES_RETFOOD] + py->global[RES_RETFOOD] <= 0)
			continue;

		if(b->prodprice[RES_RETFOOD] > u->belongings[RES_DOLLARS])
			continue;

		if(u->home >= 0)
		{
			Building* hm = &g_building[u->home];
			if(u->belongings[RES_DOLLARS] <= hm->prodprice[RES_HOUSING])
				return false;
		}

		bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = Magnitude(pos - camera.Position()) * p->price[CONSUMERGOODS];
		int cmdist = Magnitude(bcmpos - u->cmpos);
		int thisutil = PhUtil(b->prodprice[RES_RETFOOD], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return false;

	ResetGoal(u);
	u->target = bestbi;
	u->mode = UMODE_GOSHOP;
	Building* b = &g_building[u->target];
	u->goal = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	//Pathfind();
	//float pathlen = pathlength();
	//g_log<<"found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)"<<endl;
	//g_log.flush();

	return true;
}

bool NeedRest(Unit* u)
{
	if(u->belongings[RES_LABOUR] <= 0)
		return true;

	return false;
}

void GoHome(Unit* u)
{
	ResetGoal(u);
	u->target = u->home;
	u->mode = UMODE_GOREST;
	Building* b = &g_building[u->target];
	u->goal = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	//Pathfind();
	//float pathlen = pathlength();
	//g_log<<"go home path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)"<<endl;
	//g_log.flush();
}

bool FindRest(Unit* u)
{
	Vec2i bcmpos;
	int bestbi = -1;
	int bestutil = -1;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_HOUSING] <= 0)
			continue;

		//if(b->stocked[RES_HOUSING] <= 0)
		//	continue;

		if(bt->output[RES_HOUSING] - b->inuse[RES_HOUSING] <= 0)
			continue;

		if(b->prodprice[RES_HOUSING] > u->belongings[RES_DOLLARS])
			continue;

		bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = Magnitude(pos - camera.Position()) * p->price[CONSUMERGOODS];
		int cmdist = Magnitude(bcmpos - u->cmpos);
		int thisutil = PhUtil(b->prodprice[RES_HOUSING], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return false;

	ResetGoal(u);
	u->target = bestbi;
	u->mode = UMODE_GOREST;
	Building* b = &g_building[u->target];
	u->goal = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	u->home = u->target;
	//g_building[target].addoccupier(this);

	//Pathfind();
	//float pathlen = pathlength();
	//g_log<<"found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)"<<endl;
	//g_log.flush();

	return true;
}

// check building construction availability
bool CanCstBl(Unit* u)
{
	if(u->belongings[RES_LABOUR] <= 0)
		return false;

	Building* b = &g_building[u->target];
	BlType* bt = &g_bltype[b->type];
	Player* py = &g_player[b->owner];

	if(!b->on)
		return false;

	if(b->finished)
		return false;

	if(b->conmat[RES_LABOUR] >= bt->conmat[RES_LABOUR])
		return false;

	if(py->global[RES_DOLLARS] < b->conwage)
	{
		char reason[32];
		sprintf(reason, "%s construction", bt->name);
		Bankrupt(b->owner, reason);
		return false;
	}

	return true;
}

// go to construction job
void GoCstJob(Unit* u)
{
	if(!CanCstBl(u))
	{
		ResetMode(u);
		return;
	}
}

// do construction job
void DoCstJob(Unit* u)
{
	if(!CanCstBl(u))
	{
		ResetMode(u);
		return;
	}

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;
	
	//if(u->framesleft > 0)
	if(u->cyframes > 0)
	{
		//u->framesleft --;
		return;
	}

	//last = GetTickCount();
	//u->framesleft = WORK_DELAY;

	Building* b = &g_building[u->target];
	Player* py = &g_player[b->owner];

	b->conmat[RES_LABOUR] += 1;
	u->belongings[RES_LABOUR] -= 1;

	py->global[RES_DOLLARS] -= b->conwage;
	u->belongings[RES_DOLLARS] += b->conwage;

#if 0
	//b->Emit(HAMMER);
#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
		NewTransx(b->pos, LABOUR, 1, CURRENC, -p->conwage);
#endif

	b->checkconstruction();

	//char msg[128];
	//sprintf(msg, "construction %s", g_buildingType[b->type].name);
	//LogTransx(b->owner, -p->conwage, msg);
}

// check target building's job availability
bool CanBlJob(Unit* u)
{
	//CBuildingType* t = &g_buildingType[b->type];

	if(u->belongings[RES_LABOUR] <= 0)
		return false;

	Building* b = &g_building[u->target];

	if(!b->on)
		return false;

	if(!b->finished)
		return false;

	//LastNum("checknorm1");

	//if(b->worker.size() > 0)
	//	continue;

	//if(b->stock[LABOUR] >= t->input[LABOUR])
	//	continue;

	if(b->metout())
		return false;

	if(b->excin(RES_LABOUR))
		return false;

	//LastNum("checknorm3");

	//if(b->occupier.size() > 0 && !b->hasworker(u - g_unit))
	//	return false;

	Player* py = &g_player[b->owner];

	if(py->global[RES_DOLLARS] < b->opwage)
	{
		char reason[64];
		sprintf(reason, "%s expenses", g_bltype[b->type].name);
		Bankrupt(b->owner, reason);
		return false;
	}

	return true;
}

// go to building job
void GoBlJob(Unit* u)
{
	//LastNum("gotonormjob1");
	if(!CanBlJob(u))
	{
		//LastNum("gotonormjob2!");
		ResetMode(u);
		//LastNum("gotonormjob3!");
		return;
	}
}

// do building job
void DoBlJob(Unit* u)
{
	//LastNum("gotonormjob1");
	if(!CanBlJob(u))
	{
		//LastNum("gotonormjob2!");
		ResetMode(u);
		//LastNum("gotonormjob3!");
		return;
	}

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(u->cyframes > 0)
		return;

	//last = GetTickCount();
	Building* b = &g_building[u->target];
	Player* py = &g_player[b->owner];

	b->stocked[RES_LABOUR] += 1;
	u->belongings[RES_LABOUR] -= 1;

	py->global[RES_DOLLARS] -= b->opwage;
	u->belongings[RES_DOLLARS] += b->opwage;

	if(!b->tryprod())
	{
#if 0
		//b->Emit(HAMMER);
#ifdef LOCAL_TRANSX
		if(b->owner == g_localP)
#endif
			NewTransx(b->pos, LABOUR, 1, CURRENC, -p->wage[b->type]);
#endif
	}

	//char msg[128];
	//sprintf(msg, "job %s", g_buildingType[b->type].name);
	//LogTransx(b->owner, -p->wage[b->type], msg);
}

// check conduit construction job availability
bool CanCstCd(Unit* u)
{
	if(u->belongings[RES_LABOUR] <= 0)
		return false;

	ConduitTile* ctile = GetCo(u->cdtype, u->target, u->target2, false);
	Player* py = &g_player[ctile->owner];

	if(ctile->finished)
		return false;

	ConduitType* ct = &g_cotype[u->cdtype];

	if(ctile->conmat[RES_LABOUR] >= ct->conmat[RES_LABOUR])
		return false;

	if(py->global[RES_DOLLARS] < ctile->conwage)
	{
		Bankrupt(ctile->owner, "conduit construction");
		return false;
	}

	return true;
}

// go to conduit (construction) job
void GoCdJob(Unit* u)
{
	if(!CanCstCd(u))
	{
		ResetMode(u);
		return;
	}

	if(CheckIfArrived(u))
		OnArrived(u);
}

// do conduit (construction) job
void DoCdJob(Unit* u)
{
	if(!CanCstCd(u))
	{
		ResetMode(u);
		return;
	}

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(u->cyframes > 0)
		return;

	//last = GetTickCount();
	ConduitTile* ctile = GetCo(u->cdtype, u->target, u->target2, false);
	Player* py = &g_player[ctile->owner];

	ctile->conmat[RES_LABOUR] += 1;
	u->belongings[RES_LABOUR] -= 1;

	py->global[RES_DOLLARS] -= ctile->conwage;
	u->belongings[RES_DOLLARS] += ctile->conwage;

#if 0
	//r->Emit(HAMMER);
#ifdef LOCAL_TRANSX
	if(r->owner == g_localP)
#endif
		NewTransx(RoadPosition(target, target2), CURRENC, -p->conwage);
	//NewTransx(RoadPosition(target, target2), LABOUR, 1, CURRENC, -p->conwage);
#endif

	ctile->checkconstruction();

	//LogTransx(r->owner, -p->conwage, "road job");
}

// check shop availability
bool CanShop(Unit* u)
{
	Building* b = &g_building[u->target];
	Player* p = &g_player[b->owner];

	if(!b->on)
		return false;

	if(!b->finished)
		return false;

	if(b->stocked[RES_RETFOOD] + p->global[RES_RETFOOD] <= 0)
		return false;

	if(b->prodprice[RES_RETFOOD] > u->belongings[RES_DOLLARS])
		return false;

	if(u->belongings[RES_RETFOOD] >= STARTING_RETFOOD*2)
		return false;

	if(u->home >= 0)
	{
		Building* hm = &g_building[u->home];
		if(u->belongings[RES_DOLLARS] <= hm->prodprice[RES_HOUSING])
			return false;
	}

	return true;
}

// go shop
void GoShop(Unit* u)
{
	if(!CanShop(u))
	{
		ResetMode(u);
		return;
	}
}

// check apartment availabillity
bool CanRest(Unit* u, bool* eviction)
{
	if(u->home < 0)
		return false;

	Building* b = &g_building[u->target];
	Player* p = &g_player[b->owner];
	*eviction = false;

	if(!b->on)
		return false;

	if(!b->finished)
		return false;

	//if(b->stock[HOUSING] + p->global[HOUSING] <= 0.0f)
	//	return false;

	if(b->prodprice[RES_HOUSING] > u->belongings[RES_DOLLARS])
	{
		//char msg[128];
		//sprintf(msg, "eviction %f < %f", currency, p->price[HOUSING]);
		//LogTransx(b->owner, 0.0f, msg);
		*eviction = true;
		return false;
	}

	if(u->belongings[RES_LABOUR] >= STARTING_LABOUR)
		return false;

	return true;
}

void Evict(Unit* u)
{
	if(u->home < 0)
		return;

	Building* b = &g_building[u->home];

	int ui = u - g_unit;

	for(auto uiter = b->occupier.begin(); uiter != b->occupier.end(); uiter++)
	{
		if(*uiter == ui)
		{
			b->occupier.erase( uiter );
			break;
		}
	}

	u->home = -1;
}

// go rest
void GoRest(Unit* u)
{
	bool eviction;
	if(!CanRest(u, &eviction))
	{
		//Chat("!CanRest()");
		if(eviction)
		{
			RichText em(UString("Eviction"));
			//SubmitConsole(&em);
			AddChat(&em);
			Evict(u);
		}
		ResetMode(u);
		return;
	}
	/*
	if(goal - camera.Position() == Vec3f(0,0,0))
	{
	char msg[128];
	sprintf(msg, "faulty %d", UnitID(this));
	Chat(msg);
	ResetMode();
	}*/
}

// check if labourer has enough food to multiply
void CheckMul(Unit* u)
{
	if(u->belongings[RES_RETFOOD] < STARTING_RETFOOD*2)
		return;

#if 0
	int i = NewUnit();
	if(i < 0)
		return;

	CUnit* u = &g_unit[i];
	u->on = true;
	u->type = LABOURER;
	u->home = -1;

	CUnitType* t = &g_unitType[LABOURER];

	PlaceUAround(u, camera.Position().x-t->radius, camera.Position().z-t->radius, camera.Position().x+t->radius, camera.Position().z+t->radius, false);

	u->ResetLabourer();
	u->fuel = MULTIPLY_FUEL/2.0f;
	fuel -= MULTIPLY_FUEL/2.0f;
#endif

	Vec2i cmpos;

	if(!PlaceUAb(UNIT_LABOURER, u->cmpos, &cmpos))
		return;

	int ui = -1;

	if(!PlaceUnit(UNIT_LABOURER, cmpos, -1, &ui))
		return;

	Unit* u2 = &g_unit[ui];
	StartBel(u2);

	u->belongings[RES_RETFOOD] -= STARTING_RETFOOD;

	RichText gr(UString("Growth!"));
	//SubmitConsole(&gr);
	AddChat(&gr);
}

// do shop
void DoShop(Unit* u)
{
	if(!CanShop(u))
	{
		CheckMul(u);
		ResetMode(u);
		return;
	}

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;
	
	// TO DO: make consume 50-15 per sec from py->global and then b->stocked

	if(u->cyframes > 0)
	//if(u->cyframes % 2 == 0)
		return;

	//last = GetTickCount();

	Building* b = &g_building[u->target];
	Player* p = &g_player[b->owner];

#if 0
	if(p->global[RES_RETFOOD] > 0)
		p->global[RES_RETFOOD] -= 1;
	else
	{
		b->stocked[RES_RETFOOD] -= 1;
		p->local[RES_RETFOOD] -= 1;
	}

	u->belongings[RES_RETFOOD] += 1;
	p->global[RES_DOLLARS] += b->prodprice[RES_RETFOOD];
	u->belongings[RES_DOLLARS] -= b->prodprice[RES_RETFOOD];
	//b->recenth.consumed[CONSUMERGOODS] += 1.0f;
#endif

	for(int trysub=SHOP_RATE; trysub>0; trysub--)
	{
		if(b->prodprice[RES_RETFOOD] * trysub > u->belongings[RES_DOLLARS])
			continue;

		int sub[RESOURCES];
		Zero(sub);

		if(!TrySub(sub, p->global, b->stocked, p->local, NULL, NULL))
			continue;

		u->belongings[RES_RETFOOD] += trysub;
		p->global[RES_DOLLARS] += b->prodprice[RES_RETFOOD] * trysub;
		u->belongings[RES_DOLLARS] -= b->prodprice[RES_RETFOOD] * trysub;

#if 1
	//b->Emit(SMILEY);
#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
	{
		RichText tt;	//transaction text

		tt.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_DOLLARS));

		char mt[32];	//money text
		sprintf(mt, "%+d ", b->prodprice[RES_RETFOOD]);
		tt.m_part.push_back(RichPart(UString(mt)));

		tt.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_RETFOOD));

		char ft[32];	//food text
		sprintf(ft, "%+d ", -trysub);
		tt.m_part.push_back(RichPart(UString(ft)));

		NewTransx(u->drawpos, &tt);
	}
#endif

		break;
	}
}

// do rest
void DoRest(Unit* u)
{
	bool eviction;
	if(!CanRest(u, &eviction))
	{
		//Chat("!CheckApartmentAvailability()");
		if(eviction)
		{
			RichText em(UString("Eviction"));
			//SubmitConsole(&em);
			AddChat(&em);
			Evict(u);
		}
		ResetMode(u);
		return;
	}

	if(u->cyframes > 0)
		return;

	u->belongings[RES_LABOUR] += 1;
}

// check transport vehicle availability
bool CanDrTra(Unit* op)
{
	Unit* tr = &g_unit[op->target];

	if(!tr->on)
		return false;

	if(tr->hp <= 0)
		return false;

	if(tr->driver >= 0 && &g_unit[tr->driver] != op)
		return false;

	//if(u->mode != GOINGTOSUPPLIER && u->mode != GOINGTODEMANDERB && u->mode != GOINGTODEMROAD && u->mode != GOINGTODEMPOWL && u->mode != GOINGTODEMPIPE && u->mode != GOINGTOREFUEL)
	//	return false;

	if(op->belongings[RES_LABOUR] <= 0)
		return false;

	Player* p = &g_player[tr->owner];

	if(p->global[RES_DOLLARS] < tr->transpwage)
	{
		Bankrupt(tr->owner, "truck expenses");
		return false;
	}

	return true;
}

// go to transport for drive job
void GoToTra(Unit* u)
{
	if(!CanDrTra(u))
	{
		ResetMode(u);
		return;
	}

	if(CheckIfArrived(u))
		OnArrived(u);
}

//driver labourer to disembark driven transport vehicle
void Disembark(Unit* op)
{
	if(op->mode == UMODE_GOTRANSP)
	{
		ResetMode(op);
		return;
	}

	if(op->mode != UMODE_DRTRANSP)
		return;

	Unit* tr = &g_unit[op->target];
	//camera.MoveTo( u->camera.Position() );
	Vec2i trpos = tr->cmpos;
	UType* t = &g_utype[tr->type];
	Vec2i oppos;
	PlaceUAb(op->type, trpos, &oppos);
	op->cmpos = oppos;
	ResetMode(op);
	op->drawpos = Vec3f(oppos.x, g_hmap.accheight(oppos.x, oppos.y), oppos.y);

	tr->driver = -1;
	/*u->driver = -1;

	if(u->mode != NONE)
	u->mode = AWAITINGDRIVER;*/
}

// do transport drive job
void DoDrTra(Unit* op)
{
	/*
	int uID = UnitID(this);

	if(uID == 2)
	{
	g_log<<"u[2]dodrive"<<endl;
	g_log.flush();


	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
	Vec3f vel = g_unit[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velbeforedodrive=%f,%f,%f", vel.x, vel.y, vel.z);
	int unID = 5;

	g_log<<"u["<<unID<<"]: "<<msg<<endl;
	g_log.flush();

	Chat(msg);
	}
	}*/

	if(!CanDrTra(op))
	{
		/*
		if(uID == 2)
		{
		g_log<<"u[2]DISEMBARK"<<endl;
		g_log.flush();
		}
		*/
		//g_log<<"disembark";
		//g_log.flush();
		g_unit[op->target].driver = -1;
		Disembark(op);
		ResetMode(op);
		return;
	}
	/*
	if(uID == 2)
	{
	g_log<<"u[2]dodrive"<<endl;
	g_log.flush();


	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
	Vec3f vel = g_unit[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velaftercheckava=%f,%f,%f", vel.x, vel.y, vel.z);
	int unID = 5;

	g_log<<"u["<<unID<<"]: "<<msg<<endl;
	g_log.flush();

	Chat(msg);
	}
	}*/

	if(op->cyframes > 0)
		return;

	//last = GetTickCount();
	//op->framesleft = DRIVE_WORK_DELAY;

	Unit* tr = &g_unit[op->target];
	Player* py = &g_player[tr->owner];

	op->belongings[RES_LABOUR] --;

	py->global[RES_DOLLARS] -= tr->transpwage;
	op->belongings[RES_DOLLARS] += tr->transpwage;

	//LogTransx(truck->owner, -p->truckwage, "driver wage");

#if 0
#ifdef LOCAL_TRANSX
	if(truck->owner == g_localP)
#endif
		NewTransx(truck->camera.Position(), CURRENC, -p->truckwage);
#endif

	/*
	if(uID == 2)
	{
	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
	Vec3f vel = g_unit[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velafterdodrive=%f,%f,%f", vel.x, vel.y, vel.z);
	int unID = 5;

	g_log<<"u["<<unID<<"]: "<<msg<<endl;
	g_log.flush();

	Chat(msg);
	}
	}*/
}


void UpdLab(Unit* u)
{
	//return;	//do nothing for now

	u->frameslookjobago ++;
	u->cyframes--;

	if(u->cyframes < 0)
		u->cyframes = WORK_DELAY-1;
	
	if(u->cyframes == 0)
		u->belongings[RES_RETFOOD] -= LABOURER_FOODCONSUM;

	if(u->belongings[RES_RETFOOD] <= 0)
	{
		RichText sr(UString("Starvation!"));
		//SubmitConsole(&sr);
		AddChat(&sr);
		u->destroy();
		return;
	}

	switch(u->mode)
	{
	case UMODE_NONE:
	{
		if(NeedFood(u) /* && rand()%5==1 */ && FindFood(u))
		{	/*
			if(uID == 2)
			{
			g_log<<"u[2]findfood"<<endl;
			g_log.flush();
			}*/
		}
		else if(NeedRest(u))
		{	/*
			if(uID == 2)
			{
			g_log<<"u[2]needrest"<<endl;
			g_log.flush();
			}*/
			/*
			if(UnitID(this) == 0)
			{
			g_log<<"0 needrest"<<endl;
			g_log.flush();
			}
			*/
			if(u->home >= 0)
			{	/*
				if(UnitID(this) == 0)
				{
				g_log<<"home >= 0"<<endl;
				g_log.flush();
				}
				*/
				//Chat("go home");
				GoHome(u);
			}
			else
			{
				/*
				if(UnitID(this) == 0)
				{
				g_log<<"findrest"<<endl;
				g_log.flush();
				}*/

				//Chat("find rest");
				FindRest(u);
			}
		}
		else if(u->belongings[RES_LABOUR] > 0)
		{	/*
			if(uID == 2)
			{
			g_log<<"u[2]findjob"<<endl;
			g_log.flush();
			}*/
			if(!FindJob(u))
			{
				//if(rand()%(FRAME_RATE*2) == 1)
				{
					//move randomly?
					//goal = camera.Position() + Vec3f(rand()%TILE_SIZE - TILE_SIZE/2, 0, rand()%TILE_SIZE - TILE_SIZE/2);
				}
			}
		}
	} break;

	case UMODE_GOCSTJOB:
		GoCstJob(u);
		break;
	case UMODE_CSTJOB:
		DoCstJob(u);
		break;
	case UMODE_GOBLJOB:
		GoBlJob(u);
		break;
	case UMODE_BLJOB:
		DoBlJob(u);
		break;
	case UMODE_GOCDJOB:
		GoCdJob(u);
		break;
	case UMODE_CDJOB:
		DoCdJob(u);
		break;
	case UMODE_GOSHOP:
		GoShop(u);
		break;
	case UMODE_GOREST:
		GoRest(u);
		break;
	case UMODE_SHOPPING:
		DoShop(u);
		break;
	case UMODE_RESTING:
		DoRest(u);
		break;
	case UMODE_GOTRANSP:
		GoToTra(u);
		break;
	case UMODE_DRTRANSP:
		DoDrTra(u);
		break;
	default:
		break;
	}
}
