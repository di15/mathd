

#include "unit.h"
#include "job.h"
#include "building.h"
#include "player.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "chat.h"
#include "transaction.h"
#include "main.h"

bool CUnit::FindFood()
{
	CBuilding* b;
	CBuildingType* t;
	CPlayer* p;
	Vec3f pos;
	int best = -1;
	float PD;
	float bestPD = 99999999999.0f;	//best price*distance

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		t = &g_buildingType[b->type];

		if(t->output[CONSUMERGOODS] <= 0.0f)
			continue;

		p = &g_player[b->owner];

		if(b->stock[CONSUMERGOODS] + p->global[CONSUMERGOODS] <= 0.0f)
			continue;

		if(p->price[CONSUMERGOODS] > currency)
			continue;
		
		if(home >= 0)
		{
			CBuilding* hm = &g_building[home];
			CPlayer* hmp = &g_player[hm->owner];
			if(currency <= hmp->price[HOUSING]+EPSILON)
				return false;
		}

		pos = b->pos;
		PD = Magnitude2(pos - camera.Position()) * p->price[CONSUMERGOODS];

		if(PD >= bestPD)
			continue;

		best = i;
		bestPD = PD;
	}

	if(best < 0)
		return false;

	ResetGoal();
	target = best;
	mode = GOINGTOSHOP;
	b = &g_building[target];
	goal = b->pos;

	Pathfind();
	float pathlen = pathlength();
	g_log<<"found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)"<<endl;
	g_log.flush();

	return true;
}

bool CUnit::checkhomefaulty()
{
	int thisID = UnitID(this);

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		CBuildingType* t = &g_buildingType[b->type];

		if(t->output[HOUSING] <= 0.0f)
			continue;

		for(int j=0; j<b->occupier.size(); j++)
		{
			if(b->occupier[j] == thisID)
			{
				home = i;
				return true;
			}
		}
	}

	return false;
}

bool CUnit::FindRest()
{
	CBuilding* b;
	CBuildingType* t;
	CPlayer* p;
	Vec3f pos;
	int best = -1;
	float PD;
	float bestPD = 99999999999.0f;	//best price*distance
	/*
	if(checkhomefaulty())
	{
		GoHome();
		return false;
	}*/

	//Chat("find rest");

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		t = &g_buildingType[b->type];

		if(t->output[HOUSING] <= 0.0f)
			continue;

		p = &g_player[b->owner];

		//if(b->stock[HOUSING] + p->global[HOUSING] <= 0.0f)
		//	continue;

		//Chat("find rest 1");

		if(b->occupier.size() >= t->output[HOUSING])
			continue;

		//Chat("find rest 2");

		//char msg[128];
		//sprintf(msg, "%0.7f > %0.7f ? continue", p->price[HOUSING], currency);
		//Chat(msg);

		if(p->price[HOUSING] > currency && p->price[HOUSING] != currency)
			continue;

		//Chat("find rest 3");

		pos = b->pos;
		PD = Magnitude2(pos - camera.Position()) * p->price[HOUSING];

		if(PD >= bestPD)
			continue;

		best = i;
		bestPD = PD;
	}

	if(best < 0)
		return false;

	ResetGoal();
	target = best;
	mode = GOINGTOREST;
	b = &g_building[target];
	goal = b->pos;
	home = target;
	g_building[target].addoccupier(this);
	
	Pathfind();
	float pathlen = pathlength();
	g_log<<"found rest path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)"<<endl;
	g_log.flush();

	return true;
}

void CUnit::GoHome()
{
	ResetGoal();
	target = home;
	mode = GOINGTOREST;
	CBuilding* b = &g_building[target];
	goal = b->pos;

	Pathfind();
	float pathlen = pathlength();
	g_log<<"go home path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)"<<endl;
	g_log.flush();
}

bool CUnit::CheckConstructionAvailability()
{
	CBuilding* b = &g_building[target];
	CBuildingType* t = &g_buildingType[b->type];
	CPlayer* p = &g_player[b->owner];

	if(labour <= 0)
		return false;

	if(!b->on)
		return false;

	if(b->finished)
		return false;

	if(b->conmat[LABOUR] >= t->cost[LABOUR])
		return false;

	if(p->global[CURRENC] < p->conwage)
	{
		char reason[32];
		sprintf(reason, "%s construction", g_buildingType[b->type].name);
		Bankrupt(b->owner, reason);
		return false;
	}

	return true;
}

bool CUnit::CheckNormalJobAvailability()
{
	CBuilding* b = &g_building[target];
	CBuildingType* t = &g_buildingType[b->type];
	CPlayer* p = &g_player[b->owner];

	if(labour <= 0)
		return false;

	if(!b->on)
		return false;

	if(!b->finished)
		return false;

	LastNum("checknorm1");

	if(b->excessoutput())
		return false;
	
	LastNum("checknorm2");

	if(b->excessinput(LABOUR))
		return false;
		
	LastNum("checknorm3");

	if(b->occupier.size() > 0 && !b->Working(this))
		return false;

	if(p->global[CURRENC] < p->wage[b->type])
	{
		char reason[64];
		sprintf(reason, "%s expenses", g_buildingType[b->type].name);
		Bankrupt(b->owner, reason);
		return false;
	}

	return true;
}

bool CUnit::CheckRoadAvailability()
{
	CRoad* r = RoadAt(target, target2);
	CPlayer* p = &g_player[r->owner];

	if(labour <= 0)
		return false;

	if(r->finished)
		return false;

	if(r->conmat[LABOUR] >= g_roadCost[LABOUR])
		return false;

	if(p->global[CURRENC] < p->conwage)
	{
		Bankrupt(r->owner, "road construction");
		return false;
	}

	return true;
}

bool CUnit::CheckPowerlineAvailability()
{
	CPowerline* pow = PowlAt(target, target2);
	CPlayer* p = &g_player[pow->owner];

	if(labour <= 0)
		return false;

	if(!pow->on)
		return false;

	if(pow->finished)
		return false;

	if(pow->conmat[LABOUR] >= g_powerlineCost[LABOUR])
		return false;

	if(p->global[CURRENC] < p->conwage)
	{
		Bankrupt(pow->owner, "powerline construction");
		return false;
	}

	return true;
}

bool CUnit::CheckPipelineAvailability()
{
	CPipeline* pipe = PipeAt(target, target2);
	CPlayer* p = &g_player[pipe->owner];

	if(labour <= 0)
		return false;

	if(!pipe->on)
		return false;

	if(pipe->finished)
		return false;

	if(pipe->conmat[LABOUR] >= g_pipelineCost[LABOUR])
		return false;

	if(p->global[CURRENC] < p->conwage)
	{
		Bankrupt(pipe->owner, "pipeline construction");
		return false;
	}

	return true;
}

bool CUnit::NeedFood()
{
	if(fuel < LABOURER_FUEL/2.0f)
		return true;

	return false;
}

bool CUnit::NeedRest()
{
	if(labour <= 0)
		return true;

	return false;
}

bool CUnit::CheckShopAvailability()
{
	CBuilding* b = &g_building[target];
	CPlayer* p = &g_player[b->owner];

	if(!b->on)
		return false;

	if(!b->finished)
		return false;

	if(b->stock[CONSUMERGOODS] + p->global[CONSUMERGOODS] <= 0.0f)
		return false;

	if(p->price[CONSUMERGOODS] > currency)
		return false;

	if(fuel >= MULTIPLY_FUEL)
		return false;

	if(home >= 0)
	{
		CBuilding* hm = &g_building[home];
		CPlayer* p = &g_player[hm->owner];
		if(currency <= p->price[HOUSING]+EPSILON)
			return false;
	}

	return true;
}

bool CUnit::CheckApartmentAvailability(bool* eviction)
{
	if(home < 0)
		return false;

	CBuilding* b = &g_building[target];
	CPlayer* p = &g_player[b->owner];
	*eviction = false;

	if(!b->on)
		return false;

	if(!b->finished)
		return false;

	//if(b->stock[HOUSING] + p->global[HOUSING] <= 0.0f)
	//	return false;

	if(p->price[HOUSING] > currency)
	{
				char msg[128];
				sprintf(msg, "eviction %f < %f", currency, p->price[HOUSING]);
				LogTransx(b->owner, 0.0f, msg);
		*eviction = true;
		return false;
	}

	if(labour >= LABOURER_LABOUR)
		return false;

	return true;
}

void CUnit::GoShop()
{
	if(!CheckShopAvailability())
	{
		ResetMode();
		return;
	}
}

void CUnit::checkmultiply()
{
	if(fuel < MULTIPLY_FUEL)
		return;

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

	Chat("Growth!");
}

void CUnit::DoShop()
{
	if(!CheckShopAvailability())
	{
		checkmultiply();
		ResetMode();
		return;
	}

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(framesleft > 0)
	{
		framesleft --;
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

	char msg[128];
	sprintf(msg, "shopping");
	LogTransx(b->owner, p->price[CONSUMERGOODS], msg);

	//b->Emit(SMILEY);
#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
		NewTransx(b->pos, CURRENC, p->price[CONSUMERGOODS], CONSUMERGOODS, -1);
}

void CUnit::evict()
{
	if(home < 0)
		return;

	CBuilding* b = &g_building[home];

	int thisID = UnitID(this);
	for(int i=0; i<b->occupier.size(); i++)
	{
		if(b->occupier[i] == thisID)
		{
			b->occupier.erase( b->occupier.begin() + i );
			break;
		}
	}
	
	home = -1;
}

void CUnit::GoRest()
{
	bool eviction;
	if(!CheckApartmentAvailability(&eviction))
	{
		//Chat("!CheckApartmentAvailability()");
		if(eviction)
		{
			Chat("Eviction");
			evict();
		}
		ResetMode();
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

void CUnit::DoRest()
{
	bool eviction;
	if(!CheckApartmentAvailability(&eviction))
	{
		//Chat("!CheckApartmentAvailability()");
		if(eviction)
		{
			Chat("Eviction");
			evict();
		}
		ResetMode();
		return;
	}

	if(framesleft > 0)
	{
		framesleft --;
		return;
	}

	framesleft = WORK_DELAY/1000.0f * FRAME_RATE;
	labour += 1.0f;
}

bool CUnit::CheckTruckAvailability()
{
	CUnit* u = &g_unit[target];
	CPlayer* p = &g_player[u->owner];

	if(!u->on)
		return false;

	if(u->hp <= 0.0f)
		return false;

	if(u->driver >= 0 && &g_unit[u->driver] != this)
		return false;

	//if(u->mode != GOINGTOSUPPLIER && u->mode != GOINGTODEMANDERB && u->mode != GOINGTODEMROAD && u->mode != GOINGTODEMPOWL && u->mode != GOINGTODEMPIPE && u->mode != GOINGTOREFUEL)
	//	return false;

	if(labour <= 0.0f)
		return false;

	if(p->global[CURRENC] < p->truckwage)
	{
		Bankrupt(u->owner, "truck expenses");
		return false;
	}

	return true;
}

void CUnit::ArrivedAtTruck()
{
	CUnit* u = &g_unit[target];

	mode = DRIVING;

	u->driver = UnitID(this);
	/*
	if(u->fuelStation >= 0)
		u->mode = GOINGTOREFUEL;
	else if(u->target >= 0 && u->target2 >= 0)
		u->mode = GOINGTOSUPPLIER;
	else if(u->target >= 0)
		u->mode = GOINGTODEMANDERB;*/
}

void CUnit::DoDrive()
{/*
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

	if(!CheckTruckAvailability())
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
		g_unit[target].driver = -1;
		Disembark();
		ResetMode();
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

	if(framesleft > 0)
	{
		framesleft --;
		return;
	}

	//last = GetTickCount();
	framesleft = DRIVE_WORK_DELAY/1000.0f * FRAME_RATE;

	CUnit* truck = &g_unit[target];
	CPlayer* p = &g_player[truck->owner];

	labour -= 1.0f;

	p->global[CURRENC] -= p->truckwage;
	currency += p->truckwage;

	LogTransx(truck->owner, -p->truckwage, "driver wage");

#ifdef LOCAL_TRANSX
	if(truck->owner == g_localP)
#endif
		NewTransx(truck->camera.Position(), CURRENC, -p->truckwage);
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

void CUnit::Disembark()
{
	if(mode == GOINGTOTRUCK)
	{
		ResetMode();
		return;
	}
	
	if(mode != DRIVING)
		return;

	CUnit* u = &g_unit[target];
	//camera.MoveTo( u->camera.Position() );
	Vec3f truckpos = u->camera.Position();
	CUnitType* t = &g_unitType[u->type];
	PlaceUAround(this, truckpos.x-t->radius, truckpos.z-t->radius, truckpos.x+t->radius, truckpos.z+t->radius, false);
	ResetMode();

	u->driver = -1;
	/*u->driver = -1;

	if(u->mode != NONE)
		u->mode = AWAITINGDRIVER;*/
}

void CUnit::UpdateLabourer()
{
	fuel -= LABOURER_CONSUMPTIONRATE;
	frameslookjobago ++;

	if(fuel <= 0)
	{
		Chat("Starvation!");
		destroy();
		return;
	}

	/*
	
					if(UnitID(this) == 0)
					{
						g_log<<"0 mode="<<mode<<" home="<<home<<" pathsize="<<path.size()<<endl;
						g_log<<"pos="<<camera.Position().x<<","<<camera.Position().y<<","<<camera.Position().z<<endl;
						g_log<<"goal="<<goal.x<<","<<goal.y<<","<<goal.z<<endl;
						g_log.flush();
					}*/

	//int uID = UnitID(this);

	switch(mode)
	{
	case NONE:
		{
			if(NeedFood() && rand()%5==1 && FindFood())
			{/*
				if(uID == 2)
				{
					g_log<<"u[2]findfood"<<endl;
					g_log.flush();
				}*/
			}
			else if(NeedRest())
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
				if(home >= 0)
				{/*
					if(UnitID(this) == 0)
					{
						g_log<<"home >= 0"<<endl;
						g_log.flush();
					}
					*/
					//Chat("go home");
					GoHome();
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
					FindRest();
				}
			}
			else if(labour > 0)
			{/*
				if(uID == 2)
				{
					g_log<<"u[2]findjob"<<endl;
					g_log.flush();
				}*/
				if(!FindJob())
				{
					if(rand()%(FRAME_RATE*2) == 1)
					{
						goal = camera.Position() + Vec3f(rand()%TILE_SIZE - TILE_SIZE/2, 0, rand()%TILE_SIZE - TILE_SIZE/2);
					}
				}
			}
		}break;
	case GOINGTOCONJOB:		GoToConJob();		break;
	case CONJOB:			DoConJob();			break;
	case GOINGTONORMJOB:	GoToNormJob();		break;
	case NORMJOB:			DoNormJob();		break;
	case GOINGTOROADJOB:	GoToRoadJob();		break;
	case ROADJOB:			DoRoadJob();		break;
	case GOINGTOPOWLJOB:	GoToPowerlineJob();	break;
	case POWLJOB:			DoPowerlineJob();	break;
	case GOINGTOPIPEJOB:	GoToPipelineJob();	break;
	case PIPEJOB:			DoPipelineJob();	break;
	case GOINGTOSHOP:		GoShop();			break;
	case GOINGTOREST:		GoRest();			break;
	case SHOPPING:			DoShop();			break;
	case RESTING:			DoRest();			break;
	case GOINGTOTRUCK:		GoToTruck();		break;
	case DRIVING:			DoDrive();			break;
	default: break;
	}
}

void CUnit::relocate()
{
	CUnitType* t = &g_unitType[type];
	float r = t->radius;
	Vec3f p = camera.Position();
	float left = p.x-r;
	float top = p.z-r;
	float right = p.x+r;
	float bottom = p.z+r;
	PlaceUAround(this, left, top, right, bottom, false);
}