

#include "job.h"
#include "building.h"
#include "infrastructure.h"
#include "unittype.h"
#include "sim.h"
#include "../econ/utility.h"
#include "../../game/gui/chattext.h"

bool FindJob(Unit* u)
{
	//if(u->frameslookjobago < LOOKJOB_DELAY_MAX)
	//	return false;

	//bool pathed = false;

	int bestjobtype = UMODE_NONE;
	int besttarget = -1;
	int besttarget2 = -1;
	//float bestDistWage = -1;
	//float distWage;
	int bestutil = -1;
	//bool fullquota;
	int bestctype = CONDUIT_NONE;
	Vec2i bestgoal;

	//Vec3f pos = camera.Position();
	//CResource* res;
	UType* ut = &g_utype[u->type];

	//LastNum("before truck job");

	//Truck jobs
	for(int i=0; i<UNITS; i++)
	{
		Unit* u2 = &g_unit[i];

		if(!u2->on)
			continue;

		if(u2->hp <= 0.0f)
			continue;

		if(u2->type != UNIT_TRUCK)
			continue;

		//Chat("tj0");

		//if(u->mode != AWAITINGDRIVER)
		//	continue;

		if(u2->mode != UMODE_GOSUP &&
		                u2->mode != UMODE_GODEMB &&
		                u2->mode != UMODE_GODEMCD &&
		                u2->mode != UMODE_GOREFUEL)
			continue;

		//Chat("tj1");

		if(u2->driver >= 0 && &g_unit[u2->driver] != u)
			continue;

		//Chat("tj2");

		Player* py = &g_player[u2->owner];


		//Chat("tj3");

		if(py->global[RES_FUNDS] < u2->transpwage)
		{
			Bankrupt(u2->owner, "truck expenses");
			continue;
		}

		//Chat("tj4");

		int cmdist = Magnitude(u->cmpos - u2->cmpos);

		int jobutil = JobUtil(u2->transpwage, cmdist, DRIVE_WORK_DELAY);

		if(jobutil <= bestutil)
			continue;

		bestutil = jobutil;
		besttarget = i;
		bestjobtype = UMODE_GOTRANSP;
		bestgoal = u2->cmpos;
	}

	// Construction jobs
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		if(b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(b->conmat[RES_LABOUR] >= bt->conmat[RES_LABOUR])
			continue;

		Player* py = &g_player[b->owner];

		if(py->global[RES_FUNDS] < b->conwage)
		{
			char reason[32];
			sprintf(reason, "%s construction", bt->name);
			Bankrupt(b->owner, reason);
			continue;
		}

		Vec2i bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		int cmdist = Magnitude(u->cmpos - bcmpos);

		int jobutil = JobUtil(b->conwage, cmdist, WORK_DELAY);

		char msg[128];
		sprintf(msg, "%lld job util %d", g_simframe, jobutil);
		RichText rt(msg);
		AddChat(&rt);

		//if(distWage < bestDistWage)
		if(jobutil <= bestutil)
			continue;

		besttarget = i;
		bestjobtype = UMODE_GOCSTJOB;
		//bestDistWage = distWage;
		bestutil = jobutil;
		bestgoal = bcmpos;
	}

	// Normal/building jobs
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		if(!b->inoperation)
			continue;

		BlType* bt = &g_bltype[b->type];

		//if(b->worker.size() > 0)
		//	continue;

		//if(b->stock[LABOUR] >= t->input[LABOUR])
		//	continue;

		if(b->metout())
			continue;

		if(b->excin(RES_LABOUR))
			continue;

		Player* py = &g_player[b->owner];

		if(py->global[RES_FUNDS] < b->opwage)
		{
			char reason[32];
			sprintf(reason, "%s expenses", bt->name);
			Bankrupt(b->owner, reason);
			continue;
		}

		Vec2i bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		int cmdist = Magnitude(u->cmpos - bcmpos);

		int jobutil = JobUtil(b->conwage, cmdist, WORK_DELAY);

		//if(distWage < bestDistWage)
		if(jobutil <= bestutil)
			continue;

		besttarget = i;
		bestjobtype = UMODE_GOBLJOB;
		//bestDistWage = distWage;
		bestutil = jobutil;
		bestgoal = bcmpos;
	}

	//LastNum("after truck job 2");

	//Infrastructure construction jobs
	for(int ctype=0; ctype<CONDUIT_TYPES; ctype++)
	{
		ConduitType* ct = &g_cotype[ctype];

		for(int x=0; x<g_hmap.m_widthx; x++)
		{
			for(int z=0; z<g_hmap.m_widthz; z++)
			{
				ConduitTile* ctile = GetCo(ctype, x, z, false);

				if(!ctile->on)
					continue;

				if(ctile->finished)
					continue;

				if(ctile->conmat[RES_LABOUR] >= ct->conmat[RES_LABOUR])
					continue;

				Player* py = &g_player[ctile->owner];

				if(py->global[RES_FUNDS] < ctile->conwage)
				{
					Bankrupt(ctile->owner, "infrastructure construction");
					continue;
				}

				Vec2i ccmpos = Vec2i(x,z) * TILE_SIZE + ct->physoff;
				int cmdist = Magnitude(u->cmpos - ccmpos);

				int jobutil = JobUtil(ctile->conwage, cmdist, WORK_DELAY);

				if(jobutil <= bestutil)
					continue;

				besttarget = x;
				besttarget2 = z;
				bestjobtype = UMODE_GOCDJOB;
				bestctype = ctype;
				bestutil = jobutil;
				bestgoal = ccmpos;
			}
		}
	}

	u->frameslookjobago = 0;

	if(bestutil <= 0)
	{
		ResetGoal(u);
		return false;
	}

	ResetMode(u);

	u->mode = bestjobtype;
	u->goal = bestgoal;
	u->target = besttarget;
	u->target2 = besttarget2;
	u->cdtype = bestctype;

	char msg[128];
	sprintf(msg, "new goal d: %d,%d, newmode:%d", u->goal.x - u->cmpos.x, u->goal.y - u->cmpos.y, u->mode);
	RichText rt(msg);
	AddChat(&rt);

#if 0

	//ResetGoal();
	//LastNum("after truck job 5");

	if(bestjobtype == NONE)
	{
		//LastNum("after truck job 5a");
		//char msg[128];
		//sprintf(msg, "none j %d", UnitID(this));
		//Chat(msg);
		if(pathed)
			frameslookjobago = 0;
		ResetMode();
		return false;
	}
	else
	{
		//LastNum("after truck job 5b");
		ResetGoal();
	}
#endif
	return true;
}
