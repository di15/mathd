#include "labourer.h"
#include "../platform.h"
#include "../econ/demand.h"
#include "unittype.h"
#include "player.h"
#include "building.h"
#include "../econ/utility.h"
#include "../script/console.h"

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

		BuildingT* bt = &g_bltype[b->type];

		if(bt->output[RES_RETFOOD] <= 0)
			continue;

		Player* py = &g_player[b->owner];

		if(b->stocked[RES_RETFOOD] + py->global[RES_RETFOOD] <= 0)
			continue;

		if(b->prodprice[RES_RETFOOD] > u->belongings[RES_FUNDS])
			continue;

		if(u->home >= 0)
		{
			Building* hm = &g_building[u->home];
			if(u->belongings[RES_FUNDS] <= hm->prodprice[RES_HOUSING])
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
	u->mode = UMODE_GOINGTOSHOP;
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
	u->mode = UMODE_GOINGTOREST;
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

		BuildingT* bt = &g_bltype[b->type];

		if(bt->output[RES_HOUSING] <= 0)
			continue;

		//if(b->stocked[RES_HOUSING] <= 0)
		//	continue;

		if(bt->output[RES_HOUSING] - b->inuse[RES_HOUSING] <= 0)
			continue;

		if(b->prodprice[RES_HOUSING] > u->belongings[RES_FUNDS])
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
	u->mode = UMODE_GOINGTOREST;
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

bool FindJob(Unit* u)
{
	return false;
}

// go to construction job
void GoCstJob(Unit* u)
{
}

// do construction job
void DoCstJob(Unit* u)
{
}

// go to building job
void GoBlJob(Unit* u)
{
}

// do building job
void DoBlJob(Unit* u)
{
}

// go to conduit (construction) job
void GoCdJob(Unit* u)
{
}

// do conduit (construction) job
void DoCdJob(Unit* u)
{
}

// go shop
void GoShop(Unit* u)
{
}

// go rest
void GoRest(Unit* u)
{
}

// check shop availability
bool CheckShopAvail(Unit* u)
{
	Building* b = &g_building[u->target];
	Player* p = &g_player[b->owner];
    
	if(!b->on)
		return false;
    
	if(!b->finished)
		return false;
    
	if(b->stocked[RES_RETFOOD] + p->global[RES_RETFOOD] <= 0)
		return false;
    
	if(b->prodprice[RES_RETFOOD] > u->belongings[RES_FUNDS])
		return false;
    
	if(u->belongings[RES_RETFOOD] >= STARTING_RETFOOD*2)
		return false;
    
	if(u->home >= 0)
	{
		Building* hm = &g_building[u->home];
		if(u->belongings[RES_FUNDS] <= hm->prodprice[RES_HOUSING])
			return false;
	}
    
	return true;
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

	if(!PlaceUAbout(UNIT_LABOURER, u->cmpos, &cmpos))
		return;

	int ui = -1;

	if(!PlaceUnit(UNIT_LABOURER, cmpos, -1, &ui))
		return;

	Unit* u2 = &g_unit[ui];
	StartBel(u2);

	u->belongings[RES_RETFOOD] -= STARTING_RETFOOD;

	RichText gr(UString("Growth!"));
	SubmitConsole(&gr);
}

// do shop
void DoShop(Unit* u)
{
	if(!CheckShopAvail(u))
	{
		CheckMul(u);
		ResetMode(u);
		return;
	}

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(u->framesleft > 0)
	{
		u->framesleft --;
		return;
	}

	//last = GetTickCount();
	framesleft = WORK_DELAY/1000.0f * FRAME_RATE;

	CBuilding* b = &g_building[target];
	CPlayer* p = &g_player[b->owner];

	if(p->global[CONSUMERGOODS] > 0.0f)
		p->global[CONSUMERGOODS] -= 1.0f;
	else
	{
		b->stock[CONSUMERGOODS] -= 1.0f;
		p->local[CONSUMERGOODS] -= 1.0f;
	}

	fuel += 1.0f;
	p->global[CURRENC] += p->price[CONSUMERGOODS];
	currency -= p->price[CONSUMERGOODS];
	b->recenth.consumed[CONSUMERGOODS] += 1.0f;

	//char msg[128];
	//sprintf(msg, "shopping");
	//LogTransx(b->owner, p->price[CONSUMERGOODS], msg);

	//b->Emit(SMILEY);
#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
		NewTransx(b->pos, CURRENC, p->price[CONSUMERGOODS], CONSUMERGOODS, -1);
}

// do rest
void DoRest(Unit* u)
{
}

// go to transport for drive job
void GoToTra(Unit* u)
{
}

// do transport drive job
void DoDrTra(Unit* u)
{
}


void UpdLab(Unit* u)
{
	u->belongings[RES_RETFOOD] -= LABOURER_FOODCONSUM;
	u->frameslookjobago ++;
    
	if(u->belongings[RES_RETFOOD] <= 0)
	{
		//Chat("Starvation!");
		u->destroy();
		return;
	}

	switch(u->mode)
	{
	case UMODE_NONE:
		{
			if(NeedFood(u) /* && rand()%5==1 */ && FindFood(u))
			{/*
			 if(uID == 2)
			 {
			 g_log<<"u[2]findfood"<<endl;
			 g_log.flush();
			 }*/
			}
			else if(NeedRest(u))
			{/*
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
				{/*
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
			{/*
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
		}break;

#if 0
#define UMODE_NONE					0
#define UMODE_GOINGTOBLJOB			1
#define UMODE_BLJOB					2
#define UMODE_GOINGTOCSTJOB			3	//going to construction job
#define UMODE_CSTJOB				4
#define UMODE_GOINGTOCDJOB			5	//going to conduit job
#define UMODE_CDJOB					6
#define UMODE_GOINGTOSHOP			7
#define UMODE_SHOPPING				8
#define UMODE_GOINGTOREST			9
#define UMODE_RESTING				10
#define	UMODE_GOINGTOTRANSP			11	//going to transport job
#define UMODE_TRANSPDRV				12	//driving transport
#define UMODE_GOINGTOSUPPLIER		13	//transporter going to supplier
#define UMODE_GOINGTODEMANDERB		14	//transporter going to demander bl
#define UMODE_GOINGTOREFUEL			15
#define UMODE_REFUELING				16
#define UMODE_ATDEMANDERB			17
#define UMODE_ATSUPPLIER			18
#define UMODE_GOINGTODEMCD			19	//going to demander conduit
#define UMODE_ATDEMCD				20	//at demander conduit
#endif

	case UMODE_GOINGTOCSTJOB:		GoCstJob(u);		break;
	case UMODE_CSTJOB:				DoCstJob(u);		break;
	case UMODE_GOINGTOBLJOB:		GoBlJob(u);		break;
	case UMODE_BLJOB:				DoBlJob(u);			break;
	case UMODE_GOINGTOCDJOB:		GoCdJob(u);		break;
	case UMODE_CDJOB:				DoCdJob(u);			break;
	case UMODE_GOINGTOSHOP:			GoShop(u);			break;
	case UMODE_GOINGTOREST:			GoRest(u);			break;
	case UMODE_SHOPPING:			DoShop(u);			break;
	case UMODE_RESTING:				DoRest(u);			break;
	case UMODE_GOINGTOTRANSP:		GoToTra(u);			break;
	case UMODE_TRANSPDRV:			DoDrTra(u);			break;
	default: break;
	}
}
