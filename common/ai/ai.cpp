#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"
#include "../sim/building.h"
#include "../sim/buildingtype.h"
#include "../sim/build.h"
#include "../sim/sim.h"

void UpdateAI()
{
	static long long lastthink = -CYCLE_FRAMES;

	if(g_simframe - lastthink < CYCLE_FRAMES/30)
		return;

	//CalcDem1();

	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		if(!p->on)
			continue;

		if(!p->ai)
			continue;

		UpdateAI(p);
	}

	if(g_simframe - lastthink >= CYCLE_FRAMES/30)
		lastthink = g_simframe;
}

void Build(Player* p)
{
	//DemTree* dm = &g_demtree;
	int pi = p - g_player;
	DemTree* dm = &g_demtree2[pi];

	for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter ++)
	{
		DemsAtB* demb = (DemsAtB*)*biter;

		if(demb->bi >= 0)
			continue;

		int btype = demb->btype;

		Vec2i tpos;

		if(!PlaceBAb(btype, Vec2i(g_hmap.m_widthx/2, g_hmap.m_widthz/2), &tpos))
			continue;

		PlaceBl(btype, tpos, false, pi, &demb->bi);

		return;
	}
}

void Manuf(Player* p)
{
}

void AdjustPrices(Player* p)
{
}

void UpdateAI(Player* p)
{
	//if(p != g_player)
	//	return;

	static bool once = false;

	if(once)
		return;

	once = true;

	CalcDem2(p);
	Build(p);
	//Manuf(p);
	//AdjustPrices(p);
}
