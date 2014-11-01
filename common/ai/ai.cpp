#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"
#include "../sim/building.h"
#include "../sim/buildingtype.h"
#include "../sim/build.h"
#include "../sim/sim.h"
#include "../econ/utility.h"
#include "../sim/unit.h"

void UpdAI()
{
	//return;	//do nothing for now

	for(int i=0; i<PLAYERS; i++)
	{
		if(i == g_localP)
			continue;
        
		Player* p = &g_player[i];
        
		//if(p->activity == ACTIVITY_NONE)
		//	continue;

		if(!p->ai)
			continue;
        
		UpdAI(p);
		//BuildPipe(i);
		//BuildPow(i);
		//AdjustPQuotas(i);	//remove this
	}
}

void BuildStuff(int player)
{
	Player* p = &g_player[player];
    
	if(p->bbframesago < BUILDSTUFF_DELAY)
		return;
    
	BuiltStuff(player);
    
	BuyBs(player);
    
	if(!AllConstF())
		return;
    
	//g_log<<"build stuff "<<player<<endl;
	//g_log.flush();
    
	int havenum[BUILDING_TYPES];
	int anyphavenum[BUILDING_TYPES];
    
	for(int i=0; i<BUILDING_TYPES; i++)
	{
		havenum[i] = 0;
		anyphavenum[i] = 0;
	}
    
	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];
        
		if(!b->on)
			continue;
        
		anyphavenum[b->type]++;
        
		//CPlayer* p2 = &g_player[b->owner];
        
		if(!g_diplomacy[player][b->owner])
			continue;
        
		havenum[b->type]++;
	}
    
	int leasthave = 999999;
	int leasthaveB = -1;
	vector<int> leasthaves;
    
	bool noapt = false;
	bool noshopcplx = false;
	bool nopow = false;
	bool noderk = false;
	bool norefy = false;
	bool nofac = false;
	bool nomine = false;
	bool nosmelt = false;
	bool noquarry = false;
	bool nocemplant = false;
	bool nofarm = false;
	bool nochem = false;
    
	for(int i=0; i<BUILDING_TYPES; i++)
	{
		if(leasthaveB >= 0 && havenum[i] > leasthave)
			continue;
        
		if(i == RNDFACILITY)
			continue;
        
		if(havenum[i] < leasthave)
		{
			leasthaves.clear();
			noapt = false;
			noshopcplx = false;
			nopow = false;
			noderk = false;
			norefy = false;
			nofac = false;
			nomine = false;
			nosmelt = false;
			noquarry = false;
			nocemplant = false;
			nofarm = false;
			nochem = false;
		}
        
		if(i == APARTMENT)	noapt = true;
		if(i == SHOPCPLX)	noshopcplx = true;
		if(i == REACTOR)	nopow = true;
		if(i == REFINERY)	norefy = true;
		if(i == DERRICK)	noderk = true;
		if(i == FACTORY)	nofac = true;
		if(i == MINE)		nomine = true;
		if(i == SMELTER)	nosmelt = true;
		if(i == QUARRY)		noquarry = true;
		if(i == CEMPLANT)	nocemplant = true;
		if(i == FARM)		nofarm = true;
		if(i == CHEMPLANT)	nochem = true;
        
		leasthave = havenum[i];
		leasthaveB = i;
		leasthaves.push_back(i);
	}
    
	int numlab = 0;
    
	for(int i=0; i<UNITS; i++)
	{
		CUnit* u = &g_unit[i];
        
		if(!u->on)
			continue;
        
		if(u->type != LABOURER)
			continue;
        
		numlab++;
	}
    
	CBuildingType* aptt = &g_buildingType[APARTMENT];
    
	if(aptt->output[HOUSING]*anyphavenum[APARTMENT] < numlab)
		noapt = true;
    
	//g_log<<"build stuff1.5 "<<player<<endl;
	//g_log.flush();
    
	//if(leasthave <= 0)
	if(leasthave < numlab/(1.5f*g_buildingType[APARTMENT].output[HOUSING]) || leasthave == 0)
	{
		//else
		if(noapt)
			AIBuild(player, APARTMENT);
		else if(noshopcplx)
			AIBuild(player, SHOPCPLX);
		else if(noquarry)
			AIBuild(player, QUARRY);
		else if(nocemplant)
			AIBuild(player, CEMPLANT);
		//else if(nomine)
		//	AIBuild(player, MINE);
		//else if(nosmelt)
		//	AIBuild(player, SMELTER);
		else if(nofac)
			AIBuild(player, FACTORY);
		else if(nopow)
			AIBuild(player, REACTOR);
		else if(noderk)
			AIBuild(player, DERRICK);
		else if(norefy)
			AIBuild(player, REFINERY);
		else if(nofarm)
			AIBuild(player, FARM);
		else if(nochem)
			AIBuild(player, CHEMPLANT);
		else
			AIBuild(player, leasthaves[ rand()%leasthaves.size() ]);
        
		//if(rand()%2 == 0)
		//	AIBuild(player, DERRICK);
		//else
		//	AIBuild(player, REFINERY);
	}
    
    
	//g_log<<"build stuff2 "<<player<<endl;
	//g_log.flush();
    
    
	//g_log<<"build stuff3 "<<player<<endl;
	//g_log.flush();
	BuildPipe(player);
	BuildPow(player);
}

void UpdAI(Player* p)
{
	//if(p != g_player)
	//	return;

	p->bbframesago++;
	p->buframesago++;
	p->priceadjframes++;
    
	//if(p->activity != ACTIVITY_NOBUILDBU)
	{
		//if(p->activity != ACTIVITY_NOBUILDB)
		{
            //LastNum("build s");
			BuildStuff(player);
		}
        
		//if(p->activity != ACTIVITY_NOBUILDU)
		{
            //LastNum("manuf s");
			ManufStuff(player);
		}
	}
    
	if(p->priceadjframes >= CYCLE_FRAMES)
	{
        //LastNum("adj pr");
		AdjustPrices(player);
		p->priceadjframes = 0;
	}
}
