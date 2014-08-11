#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"
#include "../sim/building.h"
#include "../sim/buildingtype.h"
#include "../sim/build.h"
#include "../sim/sim.h"
#include "../econ/utility.h"

void UpdateAI()
{
#if 0
	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		if(!p->on)
			continue;

		if(!p->ai)
			continue;

		AdjPr(p);
	}
#endif

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

//build buildings
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
		
#if 1
		{
			char msg[256];
			sprintf(msg, "placebl p%d margpr%d profit%d minutil%d", pi, demb->bid.marginpr, demb->bid.maxbid, demb->bid.minutil);
			InfoMessage("r", msg);
		}
#endif

		PlaceBl(btype, tpos, false, pi, &demb->bi);

		return;
	}
}

//manufacture units
void Manuf(Player* p)
{
}

//adjust prices at building
void AdjPr(Building* b)
{
	int pi = b->owner;
	Player* p = &g_player[pi];
	DemTree* dm = &g_demtree2[pi];
	BlType* bt = &g_bltype[b->type];
	Vec2i supcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	//for each output resource adjust price
	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;

		Resource* r = &g_resource[ri];
		std::list<DemNode*> rdems;

		int maxramt = 0;

		for(auto diter=dm->rdemcopy.begin(); diter!=dm->rdemcopy.end(); diter++)
		{
			RDemNode* rdem = (RDemNode*)*diter;

			if(rdem->rtype != ri)
				continue;

			rdems.push_back(rdem);
			maxramt += rdem->ramt;

			int cmdist = -1;
			Vec2i demcmpos;
			//Vec2i supcmpos;

			bool havedem = false;
			//bool havesup = false;

#if 0
			//does this rdem have a supplier bl?
			if(rdem->bi >= 0)
			{
				Building* b2 = &g_building[rdem->bi];
				supcmpos = b2->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
				havesup = true;
			}
			else
			{

			}
#endif

			DemNode* pardem = rdem->parent;
			havedem = DemCmPos(pardem, &demcmpos);

			if(havedem /* && havesup */)
				cmdist = Magnitude(demcmpos - supcmpos);
			else
				cmdist = MAX_UTIL;	//willingness to go anywhere

			//we need price in this case
			//we have a given building that we're optimizing, so distance is given

			int margpr = r->physical ? InvPhUtilP(rdem->bid.minutil, cmdist) : InvGlUtilP(rdem->bid.minutil);

			//if there's no utility limit and only a limit on price
			if(rdem->bid.minutil < 0)
				margpr = rdem->bid.maxbid;

#if 1
			//if(ri == RES_HOUSING)
			//if(ri == RES_RETFOOD)
			{
				char msg[256];
				sprintf(msg, "p%d %s b%d margpr%d minutil%d cmdist%d", pi, g_resource[ri].name.c_str(), b-g_building, margpr, rdem->bid.minutil, cmdist);
				InfoMessage("r", msg);
			}
#endif

			rdem->bid.marginpr = margpr;
		}

		Bid bid;

		CombCo(b->type, &bid, ri, maxramt);

		int prevprc = -1;
		bool dupdm = false;
		int bestprofit = -1;
		int bestmaxr = -1;
		int bestprc = -1;
		int blmaxr = bt->output[ri];
		int maxrev = 0;

		//evalute max projected revenue at tile and bltype
		//try all the price levels from smallest to greatest
		while(true)
		{
			int leastnext = prevprc;
			maxrev = 0;

			//while there's another possible price, see if it will generate more total profit

			for(auto diter=rdems.begin(); diter!=rdems.end(); diter++)
				if(leastnext < 0 || ((*diter)->bid.marginpr < leastnext && (*diter)->bid.marginpr > prevprc))
					leastnext = (*diter)->bid.marginpr;

			if(leastnext == prevprc)
				break;

			prevprc = leastnext;

			//see how much profit this price level will generate

			int demramt = 0;	//how much will be demanded at this price level

			for(auto diter=rdems.begin(); diter!=rdems.end(); diter++)
				if((*diter)->bid.marginpr >= leastnext)
				{
					RDemNode* rdem = (RDemNode*)*diter;
					//curprofit += diter->ramt * leastnext;
					demramt += rdem->ramt;
					maxrev += rdem->bid.maxbid;
				}

			//if demanded exceeds bl's max out
			if(demramt > blmaxr)
				demramt = blmaxr;

			int proramt = 0;	//how much is most profitable to produce in this case

			//find max profit based on cost composition and price
			int curprofit = MaxPro(bid.costcompo, leastnext, demramt, &proramt);

			//int ofmax = Ceili(proramt * RATIO_DENOM, bestmaxr);	//how much of max demanded is
			//curprofit += ofmax * bestrecur / RATIO_DENOM;	//bl recurring costs, scaled to demanded qty

			if(curprofit <= bestprofit)
				continue;

			if(curprofit > maxrev)
				curprofit = maxrev;

			bestprofit = curprofit;
			bestmaxr = blmaxr;
			bestprc = leastnext;
		}

		if(bestprofit <= 0)
		{
			b->prodprice[ri] = 1;
			continue;
		}

		b->prodprice[ri] = bestprc;

#if 1
		//if(ri == RES_HOUSING)
		//if(ri == RES_RETFOOD)
		{
			char msg[128];
			int bi = b - g_building;

			sprintf(msg, "adjpr %s b%d to$%d", g_resource[ri].name.c_str(), bi, bestprc);
			InfoMessage("info", msg);
		}
#endif
	}
}

//adjust prices
void AdjPr(Player* p)
{
	int pi = p - g_player;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(b->owner != pi)
			continue;

		AdjPr(b);
	}
}

void UpdateAI(Player* p)
{
	//if(p != g_player)
	//	return;

#if 0
	static bool once = false;

	if(once)
		return;

	once = true;
#endif

	CalcDem2(p);
	Build(p);
	Manuf(p);
	AdjPr(p);
}
