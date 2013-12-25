
#include "unit.h"
#include "building.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "player.h"
#include "particle.h"
#include "chat.h"
#include "selection.h"
#include "transaction.h"
#include "main.h"
#include "transportation.h"
#include "job.h"

void CUnit::GoToDemander()
{
	if(mode == GOINGTODEMANDERB)
	{
		CBuilding* b = &g_building[target];

		if(!g_diplomacy[b->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMROAD)
	{
		CRoad* road = RoadAt(target, target2);
		
		if(!g_diplomacy[road->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMPOWL)
	{
		CPowerline* powl = PowlAt(target, target2);
		
		if(!g_diplomacy[powl->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMPIPE)
	{
		CPipeline* pipe = PipeAt(target, target2);
		
		if(!g_diplomacy[pipe->owner][owner])
			ResetMode();
	}

	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
	if(CheckIfArrived())
		OnArrived();
}

void CUnit::GoToSupplier()
{
	CBuilding* b = &g_building[supplier];

	if(!g_diplomacy[b->owner][owner])
		ResetMode();

	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
}

void CUnit::GoToRefuel()
{
	CBuilding* b = &g_building[fuelStation];

	if(!g_diplomacy[b->owner][owner])
		ResetMode();

	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
}

void CUnit::AtDemanderB()
{
	//LastNum("at demander b");
	
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

	if(b->finished)
		//b->stock[transportRes] += 1.0f;
		b->stock[transportRes] += transportAmt;
	else
	{
		//b->conmat[transportRes] += 1.0f;
		b->conmat[transportRes] += transportAmt;
		b->CheckConstruction();
	}

#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
		//NewTransx(b->pos, transportRes, 1.0f);
		NewTransx(b->pos, transportRes, transportAmt);
	
	//transportAmt -= 1.0f;
	transportAmt = 0;

	//char msg[128];
	//sprintf(msg, "unloaded %0.2f/%0.2f %s", 1.0f, transportAmt, g_resource[transportRes].name);
	//Chat(msg);

	if(transportAmt <= 0.0f)
	{
		b->transporter[transportRes] = -1;
		ResetMode();
	}
}

void CUnit::AtDemRoad()
{
	//LastNum("at demander b");
	
	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(framesleft > 0)
	{
		framesleft --;
		return;
	}

	//last = GetTickCount();
	framesleft = WORK_DELAY/1000.0f * FRAME_RATE;

	CRoad* r = RoadAt(target, target2);

	r->conmat[transportRes] += 1.0f;
	transportAmt -= 1.0f;
	
#ifdef LOCAL_TRANSX
	if(r->owner == g_localP)
#endif
		NewTransx(RoadPosition(target, target2), transportRes, 1.0f);

	if(transportAmt <= 0.0f)
	{
		r->transporter[transportRes] = -1;
		ResetMode();
	}
	
	r->CheckConstruction();
}

void CUnit::AtDemPowl()
{
	//LastNum("at demander b");
	
	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(framesleft > 0)
	{
		framesleft --;
		return;
	}

	//last = GetTickCount();
	framesleft = WORK_DELAY/1000.0f * FRAME_RATE;

	CPowerline* p = PowlAt(target, target2);

	p->conmat[transportRes] += 1.0f;
	transportAmt -= 1.0f;
	
#ifdef LOCAL_TRANSX
	if(p->owner == g_localP)
#endif
		NewTransx(PowerlinePosition(target, target2), transportRes, 1.0f);

	if(transportAmt <= 0.0f)
	{
		p->transporter[transportRes] = -1;
		ResetMode();
	}
	
	p->CheckConstruction();
}

void CUnit::AtDemPipe()
{
	//LastNum("at demander b");
	
	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(framesleft > 0)
	{
		framesleft --;
		return;
	}

	//last = GetTickCount();
	framesleft = WORK_DELAY/1000.0f * FRAME_RATE;

	CPipeline* p = PipeAt(target, target2);

	p->conmat[transportRes] += 1.0f;
	transportAmt -= 1.0f;

#ifdef LOCAL_TRANSX
	if(p->owner == g_localP)
#endif
		NewTransx(PipelinePhysPos(target, target2), transportRes, 1.0f);

	if(transportAmt <= 0.0f)
	{
		p->transporter[transportRes] = -1;
		ResetMode();
	}
	
	p->CheckConstruction();
}

void CUnit::DoneAtSupplier()
{
	//g_log<<"after bid done at sup";
	//g_log.flush();

	LastNum("dat sup 1");

	if(transportRes <= 0.0f)
	{
		ResetMode();
		return;
	}
	LastNum("dat sup 2");
	
	g_building[supplier].transporter[transportRes] = -1;
	mode = targtype;
	
	LastNum("dat sup 3");
	if(targtype == GOINGTODEMANDERB)
	{
		goal = g_building[target].pos;
	}
	else if(targtype == GOINGTODEMROAD)
	{
		goal = RoadPosition(target, target2);
	}
	else if(targtype == GOINGTODEMPOWL)
	{
		goal = PowerlinePosition(target, target2);
	}
	else if(targtype == GOINGTODEMPIPE)
	{
		goal = PipelinePhysPos(target, target2);
	}
	LastNum("dat sup 4");

	NewJob(GOINGTOTRUCK, UnitID(this), -1);
	LastNum("dat sup 5");
}

void CUnit::AtSupplier()
{
	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	LastNum("at sup 1");

	if(framesleft > 0)
	{
		framesleft --;
		return;
	}

	//last = GetTickCount();
	framesleft = WORK_DELAY/1000.0f * FRAME_RATE;
	
	CBuilding* supb = &g_building[supplier];
	CBuilding* demb;
	CPlayer* demp;
	CPlayer* supp = &g_player[supb->owner];
	CBuildingType* demt;
	CRoad* demroad;
	CPowerline* dempowl;
	CPipeline* dempipe;
	LastNum("at sup 2");

	//char msg[128];
	//sprintf(msg, "at sup targtype=%d @%d,%d", targtype, target, target2);
	//LastNum(msg);

	int demplayer;

	if(targtype == GOINGTODEMANDERB)
	{
		demb = &g_building[target];
		demplayer = demb->owner;
		demp = &g_player[demb->owner];
		demt = &g_buildingType[demb->type];
	}
	else if(targtype == GOINGTODEMROAD)
	{
		demroad = RoadAt(target, target2);
		demplayer = demroad->owner;
		demp = &g_player[demroad->owner];
	}
	else if(targtype == GOINGTODEMPOWL)
	{
		dempowl = PowlAt(target, target2);
		demplayer = dempowl->owner;
		demp = &g_player[dempowl->owner];
	}
	else if(targtype == GOINGTODEMPIPE)
	{
		dempipe = PipeAt(target, target2);
		demplayer = dempipe->owner;
		demp = &g_player[dempipe->owner];
	}
	
	LastNum("at sup 3");
	//if(supb->stock[transportRes] + supp->global[transportRes] <= 0.0f)
	//	DoneAtSupplier();

	//LastNum("at sup 1");

	//if(demb->stock[transportRes] + demp->global[transportRes] + transportAmt >= demt->input[transportRes])
	if(transportAmt >= reqAmt)
		DoneAtSupplier();

	//LastNum("at sup 2");
	LastNum("at sup 4");

	//float required = demt->input[transportRes] - transportAmt - demb->stock[transportRes] - demp->global[transportRes];
	float required = reqAmt - transportAmt;
	float got = required;

	if(supb->stock[transportRes] + supp->global[transportRes] < required)
		got = supb->stock[transportRes] + supp->global[transportRes];

	float paid;
	
	LastNum("at sup 5");
	//LastNum("at sup 3");
	//if(targtype == GOINGTODEMANDERB)
	{
		//if(demb->owner != supb->owner)
		if(demp != supp)
		{
			if(demp->global[CURRENC] < supp->price[transportRes])
			{
				char reason[64];
				sprintf(reason, "buying %s", g_resource[transportRes].name);
				Bankrupt(demplayer, reason);
				DoneAtSupplier();
				return;
			}

			float canafford = got;
			
	LastNum("at sup 5.2");
			if(supp->price[transportRes] > 0.0f)
				canafford = demp->global[CURRENC] / supp->price[transportRes];

			if(canafford < got)
				got = canafford;
	//LastNum("at sup 4");

			paid = got * supp->price[transportRes];
			
	LastNum("at sup 5.3");
			demp->global[CURRENC] -= paid;
			supp->global[CURRENC] += paid;

			supb->recenth.produced[CURRENC] += paid;
			if(targtype == GOINGTODEMANDERB)
				demb->recenth.consumed[CURRENC] += paid;
			else
				demp->recentth.consumed[CURRENC] += paid;
			supp->recentph[transportRes].earnings += paid;
			
	LastNum("at sup 5.4");
			char msg[128];
			sprintf(msg, "purchase %s", g_resource[transportRes].name);
			LogTransx(supb->owner, paid, msg);
			LogTransx(demplayer, -paid, msg);
		}
	}
	LastNum("at sup 6");
	
	//LastNum("at sup 5");
	if(supp->global[transportRes] >= got)
		supp->global[transportRes] -= got;
	else
	{
	//LastNum("at sup 6");
		float subgot = got;
		subgot -= supp->global[transportRes];
		supp->global[transportRes] = 0;

		supb->stock[transportRes] -= subgot;
		supp->local[transportRes] -= subgot;
	}
	
	LastNum("at sup 7");
	//LastNum("at sup 7");
	transportAmt += got;
	supb->recenth.consumed[transportRes] += got;

	Vec3f transxpos = supb->pos;
	
#ifdef LOCAL_TRANSX
	if(supb->owner == g_localP && demb->owner != supb->owner)
		NewTransx(transxpos, CURRENC, paid, transportRes, -got);
	if(demb->owner == g_localP && demb->owner != supb->owner)
		NewTransx(transxpos, transportRes, got, CURRENC, -paid);
#else
		NewTransx(transxpos, transportRes, -got);
#endif
		
	LastNum("at sup 8");
	DoneAtSupplier();
	LastNum("at sup 9");
}

void CUnit::DoRefuel()
{
	if(fuel >= TRUCK_FUEL)
		DoneRefueling();

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(framesleft > 0)
	{
		framesleft --;
		return;
	}

	//last = GetTickCount();
	framesleft = WORK_DELAY/1000.0f * FRAME_RATE;

	CBuilding* b = &g_building[fuelStation];
	CPlayer* thisp = &g_player[owner];
	CPlayer* bp = &g_player[b->owner];

	if(b->stock[ZETROL] + bp->global[ZETROL] <= 0.0f)
		DoneRefueling();

	if(fuel >= TRUCK_FUEL)
		DoneRefueling();

	float required = TRUCK_FUEL - fuel;
	float got = required;

	if(b->stock[ZETROL] + bp->global[ZETROL] < required)
		got = b->stock[ZETROL] + bp->global[ZETROL];

	float paid;

	if(owner != b->owner)
	{
		LogTransx(owner, -bp->price[ZETROL], "buy fuel");
		LogTransx(b->owner, bp->price[ZETROL], "buy fuel");

		if(thisp->global[CURRENC] < bp->price[ZETROL])
		{
			Bankrupt(owner, "fuel costs");
			DoneRefueling();
			return;
		}

		float canafford = got;

		if(bp->price[transportRes] > 0.0f)
			canafford = thisp->global[CURRENC] / bp->price[transportRes];

		if(canafford < got)
			got = canafford;

		paid = got * bp->price[transportRes];

		thisp->global[CURRENC] -= paid;
		bp->global[CURRENC] += paid;

		thisp->recentth.consumed[CURRENC] += paid;
		bp->recentph[ZETROL].earnings += paid;

		if(thisp->global[CURRENC] <= 0.0f)
			Bankrupt(owner, "fuel costs");
	}

	if(bp->global[ZETROL] >= got)
		bp->global[ZETROL] -= got;
	else
	{
		float subgot = got;
		subgot -= bp->global[ZETROL];
		bp->global[ZETROL] = 0;

		b->stock[ZETROL] -= got;
		bp->local[ZETROL] -= got;
	}

	b->recenth.consumed[ZETROL] += got;
	fuel += got;

	//b->Emit(BARREL);
	if(
#ifdef LOCAL_TRANSX
		b->owner == g_localP && 
#endif
		b->owner != owner)
		NewTransx(b->pos, CURRENC, paid, ZETROL, -got);
	if(
#ifdef LOCAL_TRANSX
		owner == g_localP && 
#endif
		b->owner != owner)
		NewTransx(b->pos, ZETROL, got, CURRENC, -paid);

	DoneRefueling();
}

void CUnit::DoneRefueling()
{
	//g_log<<"after bid done refueling";
	//g_log.flush();

	fuelStation = -1;
	/*
	if(target2 >= 0)
	{
		mode = GOINGTOSUPPLIER;
		goal = g_building[target].pos;
	}
	else if(target >= 0)
	{
		mode = GOINGTODEMANDERB;
		goal = g_building[target].pos;
	}*//*
	if(transportAmt > 0)
	{
		if(targtype == GOINGTODEMANDERB)
		{
			mode = GOINGTODEMANDERB;
			goal = g_building[target].pos;
		}
		else if(targtype == GOINGTODEMROAD)
		{
			mode = GOINGTODEMROAD;
			goal = RoadPosition(target, target2);
		}
		else if(targtype == GOINGTODEMPIPE)
		{
			mode = GOINGTODEMPIPE;
			goal = PipelinePhysPos(target, target2);
		}
		else if(targtype == GOINGTODEMPOWL)
		{
			mode = GOINGTODEMPOWL;
			goal = PowerlinePosition(target, target2);
		}
		else
		{
			mode = targtype;
			ResetGoal();
			Pathfind();
		}
	}
	else if(targtype != NONE)
	{
		mode = GOINGTOSUPPLIER;
		goal = g_building[supplier].pos;
	}
	else*/
	{
		ResetMode();
	}
	//else
	//	ResetMode();
}

void CUnit::UpdateTruck()
{
	if(fuel <= 0)
	{
		ResetMode();
		if(driver >= 0)
			g_unit[driver].Disembark();
		driver = -1;
		destroy();
		return;
	}

	//g_log<<"Truck "<<UnitID(this)<<" mode = "<<mode<<endl;
	//g_log.flush();

	//char msg[128];
	//sprintf(msg, "upd truck mode=%d", mode);
	//LastNum(msg);

	switch(mode)
	{
	case NONE:
		{
			if(NeedFood() && FindFuel())
			{
				//g_log<<"find fuel"<<endl;
				//g_log.flush();
			}
			else if(mode == NONE && target < 0)
			{
			//	FindDemander();
				if(frameslookjobago >= TBID_DELAY)
				{
					g_freetrucks.push_back(UnitID(this));
					//g_log<<"free truck"<<endl;
					//g_log.flush();
				}
				frameslookjobago ++;
			}
		}break;
	case GOINGTODEMROAD:
	case GOINGTODEMPOWL:
	case GOINGTODEMPIPE:
	case GOINGTODEMANDERB:		
		GoToDemander();		
		break;
	case GOINGTOSUPPLIER:		GoToSupplier();		break;
	case GOINGTOREFUEL:			GoToRefuel();		break;
	case REFUELING:				DoRefuel();			break;
	case ATDEMANDERB:			AtDemanderB();		break;
	case ATDEMROAD:				AtDemRoad();		break;
	case ATDEMPOWL:				AtDemPowl();		break;
	case ATDEMPIPE:				AtDemPipe();		break;
	case ATSUPPLIER:			AtSupplier();		break;
	default: break;
	}
}

bool CUnit::FindFuel()
{	
	float bestPD = 99999999999.0f;	// best price*distance
	int price;
	int best = -1;
	CBuilding* b;
	CBuildingType* t;
	CPlayer* p;
	float PD;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		t = &g_buildingType[b->type];

		if(t->output[ZETROL] <= 0.0f)
			continue;

		p = &g_player[b->owner];

		//Chat("f f 1");

		if(b->stock[ZETROL] + p->global[ZETROL] <= 0.0f)
			continue;
		
		//Chat("f f 2");

		if(b->owner != owner && p->price[ZETROL] > g_player[owner].global[CURRENC])
			continue;
		
		//Chat("f f 3");

		price = p->price[ZETROL];

		if(b->owner == owner)
			price = 0.001f;

		PD = Magnitude2(b->pos - camera.Position()) * price;

		if(PD > bestPD)
			continue;

		bestPD = PD;
		best = i;
	}

	if(best < 0)
		return false;
	
	ResetGoal();
	fuelStation = best;
	//if(driver >= 0)
	mode = GOINGTOREFUEL;
	goal = g_building[fuelStation].pos;

	return true;
}

int CUnit::FindSupplier(int r)
{
	float bestPD = 99999999999.0f;	// best price*distance
	int price;
	int best = -1;
	CBuilding* b;
	CBuildingType* t;
	CPlayer* p;
	float PD;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		t = &g_buildingType[b->type];

		if(t->output[r] <= 0.0f)
			continue;

		p = &g_player[b->owner];

		if(b->stock[r] + p->global[r] <= 0.0f)
			continue;

		if(b->owner != owner && p->price[r] > g_player[owner].global[CURRENC])
			continue;

		price = p->price[r];

		if(b->owner == owner)
			price = 0.001f;

		PD = Magnitude2(b->pos - camera.Position()) * price;

		if(PD > bestPD)
			continue;

		bestPD = PD;
		best = i;
	}

	return best;
}

bool CUnit::Supply(int type)
{/*
	CBuilding* b;
	CBuildingType* t = &g_buildingType[type];

	vector<int> nearest;
	vector<float> distance;
	float tempD;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		if(b->type != type)
			continue;
		
		tempD = Magnitude2(b->pos - camera.Position());

		if(nearest.size() <= 0)
		{
			nearest.push_back(i);
			distance.push_back(tempD);
			continue;
		}

		for(int j=0; j<nearest.size(); j++)
		{
			if(tempD > distance[j])
				continue;

			nearest.insert( nearest.begin() + j, i );
			distance.insert( distance.begin() + j, tempD );

			break;
		}
	}

	if(nearest.size() <= 0)
		return false;

	CResource* res;
	float input;
	float have;
	float need;
	int supplier;
	CPlayer* p;

	for(int i=0; i<nearest.size(); i++)
	{
		int id = nearest[i];
		b = &g_building[id];
		p = &g_player[b->owner];

		for(int r=0; r<RESOURCES; r++)
		{
			res = &g_resource[r];

			if(!res->physical)
				continue;

			input = t->input[r];
			have = b->stock[r] + p->global[r];
			need = input - have;

			if(need <= 0.0f)
				continue;

			supplier = FindSupplier(r);

			if(supplier < 0)
				continue;

			target2 = id;
			target = supplier;
			transportRes = r;
			goal = g_building[supplier].pos;
			ResetPath();

			//if(driver < 0)
			//	mode = AWAITINGDRIVER;
			//else
				mode = GOINGTOSUPPLIER;

			return true;
		}
	}
	*/
	return false;
}

bool CUnit::CheckNeedCG()
{
	float needCG = 0;
	CUnit* u;

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != LABOURER)
			continue;

		needCG = min(LABOURER_FUEL, LABOURER_FUEL - u->fuel);
	}

	float availCG = 0;
	CBuilding* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		availCG += b->stock[CONSUMERGOODS];
	}

	if(availCG < needCG)
	{
		if(Supply(SHOPCPLX))
			return true;
	}

	return false;
}


bool CUnit::CheckNeedE()
{
	float needE = 0;
	float availE = 0;
	CBuildingType* t;
	CBuilding* b;
	
	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		if(b->excessoutput())
			continue;

		t = &g_buildingType[b->type];

		if(t->output[ELECTRICITY] > 0.0f)
			continue;

		needE += t->input[ELECTRICITY];
		availE += b->stock[ELECTRICITY];
	}

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

		availE += b->stock[ELECTRICITY];
	}

	if(availE < needE)
	{
		if(Supply(REACTOR))
			return true;

		if(Supply(COMBUSTOR))
			return true;
	}

	return false;
}

bool CUnit::FindDemander()
{
	if(CheckNeedCG())
		return true;

	if(CheckNeedE())
		return true;

	if(Supply(FACTORY))
		return true;

	if(Supply(SMELTER))
		return true;

	//if(Supply(CONCPLANT))
	//	return true;

	return true;
}

void CUnit::Accum(float d)
{
	accum += d;
	
	//if(accum >= TILE_SIZE*TILE_SIZE)
	if(accum >= FUEL_DISTANCE)
	{
		fuel -= TRUCK_CONSUMPTIONRATE;
		accum -= TILE_SIZE*TILE_SIZE;

		CPlayer* p = &g_player[owner];
		CUnit* u = &g_unit[driver];

		//u->currency += p->truckwage;
		//p->global[CURRENC] -= p->truckwage;
		//u->labour -= 1.0f;

		//if(UnitSelected(this))
		//	RedoLeftPanel();
	}
}