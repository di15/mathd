#include "demand.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/sim.h"
#include "../sim/labourer.h"
#include "../utils.h"
#include "utility.h"

DemTree g_demtree;
DemTree g_demtree2[PLAYERS];

DemNode::DemNode()
{
	demtype = DEM_NODE;
	parent = NULL;
	bid.maxbid = 0;
	profit = 0;
}

int CountU(int utype)
{
	int cnt = 0;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type == utype)
			cnt++;
	}

	return cnt;
}

int CountB(int btype)
{
	int cnt = 0;

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		if(b->type == btype)
			cnt++;
	}

	return cnt;
}

#define MAX_REQ		1000

/*
Add road, powerline, and/or crude oil pipeline infrastructure between two buildings as required,
according to the trade between the two buildings and presence or absense of a body of a water in between.
Connect the buildings to the power and road grid, and crude oil pipeline grid if necessary.
*/
void AddInf(DemTree* dm, std::list<DemNode*>* nodes, DemNode* parent, DemNode* parent2, int rtype, int ramt, int depth)
{
	// TO DO: roads and infrastructure to suppliers

	Resource* r = &g_resource[rtype];

	if(r->conduit == CONDUIT_NONE)
		return;

	//If no supplier specified, find one or make one, and then call this function again with that supplier specified.
	if(!parent2)
	{
		// TO DO: don't worry about this for now.
		return;
	}

	char ctype = r->conduit;
	ConduitType* ct = &g_cotype[ctype];


}

void AddReq(DemTree* dm, std::list<DemNode*>* nodes, DemNode* parent, int rtype, int ramt, int depth)
{
#ifdef DEBUG
	if(ramt <= 0)
	{
		g_log<<"0 req: "<<g_resource[rtype].name<<" "<<ramt<<std::endl;
		g_log.flush();

		return;
	}

	g_log<<"demand "<<rtype<<" ramt "<<ramt<<" depth "<<depth<<std::endl;
	g_log.flush();
#endif

	if(depth > MAX_REQ)
		return;

	int rremain = ramt;
	//int bidremain = bid;
	
#ifdef DEBUG
	g_log<<"bls"<<std::endl;
	
	int en = 0;
	int uran = 0;

	for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter++)
	{
		uran += (*biter)->supplying[RES_URANIUM];
		en += (*biter)->supplying[RES_ENERGY];
		g_log<<"\t\tbuilding "<<g_bltype[(*biter)->btype].name<<" supplying ur"<<(*biter)->supplying[RES_URANIUM]<<" en"<<(*biter)->supplying[RES_ENERGY]<<std::endl;
		g_log.flush();
	}
	
	g_log<<"/bls uran = "<<uran<<std::endl;
	g_log<<"/bls en = "<<en<<std::endl;
	g_log.flush();
#endif

	for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter++)
	{
		DemsAtB* demb = *biter;
		BuildingT* bt = &g_bltype[demb->btype];

		if(bt->output[rtype] <= 0)
			continue;

		int capleft = bt->output[rtype];
		capleft -= demb->supplying[rtype];
		
#ifdef DEBUG
		g_log<<"\tcapleft "<<capleft<<std::endl;
		g_log.flush();
#endif

		if(capleft <= 0)
			continue;

		int suphere = imin(capleft, rremain);
		rremain -= suphere;
		
#ifdef DEBUG
		g_log<<"\tsuphere "<<suphere<<" remain "<<remain<<std::endl;
		g_log.flush();
#endif

		RDemNode* rdem = new RDemNode;
		rdem->bi = (*biter)->bi;
		rdem->btype = (*biter)->btype;
		rdem->supbp = *biter;
		rdem->parent = parent;
		rdem->rtype = rtype;
		rdem->ramt = suphere;
		rdem->ui = -1;
		rdem->utype = -1;
		// TO DO: unit transport
		nodes->push_back(rdem);

		//int producing = bt->output[rtype] * demb->prodratio / RATIO_DENOM;
		//int overprod = producing - demb->supplying[rtype] - suphere;
		
		int newprodlevel = (demb->supplying[rtype] + suphere) * RATIO_DENOM / bt->output[rtype];
		newprodlevel = imax(1, newprodlevel);
		demb->supplying[rtype] += suphere;
		
#ifdef DEBUG
		g_log<<"suphere"<<suphere<<" of total"<<demb->supplying[rtype]<<" of remain"<<remain<<" of res "<<g_resource[rtype].name<<" newprodlevel "<<demb->prodratio<<" -> "<<newprodlevel<<std::endl;
		g_log.flush();
#endif

		if(newprodlevel > demb->prodratio)
		{
			int extraprodlev = newprodlevel - demb->prodratio;
			int oldprodratio = demb->prodratio;
			demb->prodratio = newprodlevel;

			for(int ri=0; ri<RESOURCES; ri++)
			{
				if(bt->input[ri] <= 0)
					continue;

				//int rreq = extraprodlev * bt->input[ri] / RATIO_DENOM;
				int oldreq = oldprodratio * bt->input[ri] / RATIO_DENOM;
				int newreq = newprodlevel * bt->input[ri] / RATIO_DENOM;
				int rreq = newreq - oldreq;

				if(rreq <= 0)
				{
#ifdef DEBUG
					g_log<<"rreq 0 at "<<__LINE__<<" = "<<rreq<<" of "<<g_resource[ri].name<<std::endl;
					g_log.flush();
#endif
					continue;
				}

				AddReq(dm, &demb->proddems, demb, ri, rreq, depth+1);
			}
		}

		AddInf(dm, nodes, parent, *biter, rtype, ramt, depth);

		if(rremain <= 0)
			return;
	}

	if(rremain <= 0)
		return;

	struct Producer
	{
		int btype;
		int cap;
		int placed;
	};

	std::list<Producer> producers;
	int leastpl = -1;
	int leastplb = -1;

	for(int i=0; i<BUILDING_TYPES; i++)
	{
		BuildingT* t = &g_bltype[i];

		if(t->output[rtype] <= 0)
			continue;

		Producer pro;
		pro.btype = i;
		pro.cap = t->output[rtype];
		pro.placed = CountB(i);
		producers.push_back(pro);

		if(leastpl < 0 || pro.placed < leastpl)
		{
			leastpl = pro.placed;
			leastplb = i;
		}
	}

	if(leastplb < 0)
		return;

	BuildingT* t = &g_bltype[leastplb];
	//int reqnb = imax(1, Ceili(ramt, t->output[rtype]));

	do
	{
		DemsAtB* demb = new DemsAtB();

		demb->parent = NULL;
		demb->prodratio = 0;
		Zero(demb->supplying);
		Zero(demb->condem);
		demb->bi = -1;
		demb->btype = leastplb;

		dm->supbpcopy.push_back(demb);

		// need to set this before req of conmat because of possible recursive loop

		BuildingT* bt = &g_bltype[demb->btype];

		// requisites for production, calc prodratio
		
		demb->prodratio = rremain * RATIO_DENOM / bt->output[rtype];
		demb->prodratio = imin(RATIO_DENOM, demb->prodratio);
		demb->prodratio = imax(1, demb->prodratio);
		// prodratio could be modified next, we need the original value
		int oldprodratio = demb->prodratio;
		
#ifdef DEBUG
		g_log<<"\toldprodr "<<oldprodratio<<std::endl;
		g_log.flush();
#endif
		
		int prodamt = bt->output[rtype] * demb->prodratio / RATIO_DENOM;
		prodamt = imax(1, prodamt);
		demb->supplying[rtype] += prodamt;
		
#ifdef DEBUG
		g_log<<"\tprodamt "<<prodamt<<std::endl;
		g_log.flush();
		
		g_log<<"\t1. prodamt"<<prodamt<<" of total"<<demb->supplying[rtype]<<" of remain"<<remain<<" of res "<<g_resource[rtype].name<<" newprodlevel "<<oldprodratio<<" -> "<<demb->prodratio<<std::endl;
		g_log.flush();
#endif
		
		RDemNode* rdem = new RDemNode;
		rdem->bi = -1;
		rdem->supbp = demb;
		rdem->btype = demb->btype;
		rdem->parent = parent;
		rdem->rtype = rtype;
		rdem->ramt = prodamt;
		rdem->ui = -1;
		rdem->utype = -1;
		// TO DO: unit transport
		nodes->push_back(rdem);

		// TO DO: roads and infrastructure to suppliers

		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->conmat[ri] <= 0)
				continue;

			AddReq(dm, &demb->condems, demb, ri, bt->conmat[ri], depth+1);
			demb->condem[ri] += bt->conmat[ri];
		}

#ifdef DEBUG
		g_log<<"\t2. prodamt"<<prodamt<<" of total"<<demb->supplying[rtype]<<" of remain"<<remain<<" of res "<<g_resource[rtype].name<<" newprodlevel "<<oldprodratio<<" -> "<<demb->prodratio<<std::endl;
		g_log.flush();
#endif

		// we add prod reqs second because construction is first priority

		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;
			
			int rreq = Ceili(bt->input[ri] * oldprodratio, RATIO_DENOM);
			//rreq = imax(1, rreq);

			if(rreq <= 0)
			{
#ifdef DEBUG
				g_log<<"rreq 0 at "<<__LINE__<<" = "<<rreq<<" of "<<g_resource[ri].name<<std::endl;
				g_log.flush();
#endif
				continue;
			}

			AddReq(dm, &demb->proddems, demb, ri, rreq, depth+1);
		}
		
#ifdef DEBUG
		g_log<<"\t3. prodamt"<<prodamt<<" of total"<<demb->supplying[rtype]<<" of remain"<<remain<<" of res "<<g_resource[rtype].name<<" newprodlevel "<<oldprodratio<<" -> "<<demb->prodratio<<std::endl;
		g_log.flush();
#endif
		
		AddInf(dm, nodes, parent, demb, rtype, ramt, depth);

		rremain -= prodamt;
	}while(rremain > 0);

	// TO DO: outlets for global player cache/reserves
}

void AddBl(DemTree* dm)
{
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		DemsAtB* demb = new DemsAtB();

		demb->parent = NULL;
		demb->prodratio = 0;
		Zero(demb->supplying);
		Zero(demb->condem);
		demb->bi = i;
		demb->btype = b->type;

		dm->supbpcopy.push_back(demb);
	}
}

void BlConReq(DemTree* dm)
{
	for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter++)
	{
		const int bi = (*biter)->bi;

		BuildingT* bt = NULL;
		int conmat[RESOURCES];
		bool finished = false;

		if(bi >= 0)
		{
			Building* b = &g_building[bi];
			bt = &g_bltype[b->type];
			memcpy(conmat, b->conmat, sizeof(int)*RESOURCES);
			finished = b->finished;
		}
		else
		{
			bt = &g_bltype[(*biter)->btype];
			memcpy(conmat, (*biter)->condem, sizeof(int)*RESOURCES);
			finished = false;
		}

		if(!finished)
		{
			for(int i=0; i<RESOURCES; i++)
			{
				const int req = bt->conmat[i] - conmat[i];
				if(req > 0)
					AddReq(dm, &(*biter)->condems, *biter, i, req, 0);
			}
		}
		else
		{
		}
	}
}

// Calculate demands where there is insufficient supply,
// not where a cheaper supplier might create a profit.
// This is player-non-specific, shared by all players.
// No positional information is considered, no branching,
// no bidding price information included.
// Roads and infrastructure considered?
void CalcDem1()
{
	g_demtree.free();
	AddBl(&g_demtree);
	BlConReq(&g_demtree);

	int nlab = CountU(UNIT_LABOURER);

	int labfunds = 0;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != UNIT_LABOURER)
			continue;

		labfunds += u->belongings[RES_FUNDS];
	}
	
	AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_HOUSING, nlab, 0);
	AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_RETFOOD, LABOURER_FOODCONSUM * CYCLE_FRAMES, 0);
	AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_ENERGY, nlab * LABOURER_ENERGYCONSUM, 0);
}

// Housing demand
void LabDemH(DemTree* dm, Unit* u, int* fundsleft)
{
	// Housing
	RDemNode* homedem = new RDemNode;
	if(u->home >= 0)
	{
		// If there's already a home,
		// there's only an opportunity
		// for certain lower-cost apartments 
		// within distance.
		Building* homeb = &g_building[u->home];
		int homepr = homeb->prodprice[RES_HOUSING];
		int homedist = Magnitude(homeb->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2,TILE_SIZE/2) - u->cmpos);

		// As long as it's at least -1 cheaper and/or -1 units closer,
		// it's a more preferable alternative. How to express
		// alternatives that aren't closer but much cheaper?
		// Figure that out as needed using utility function?

		Bid *altbid = &homedem->bid;
		altbid->maxbid = homepr;
		altbid->maxdist = homedist;
		altbid->cmpos = u->cmpos;
		altbid->tpos = u->cmpos/TILE_SIZE;
		altbid->minutil = -1;	//any util

		*fundsleft -= homepr;
	}
	else
	{
		// If there are alternatives/competitors
		DemsAtB* bestdemb = NULL;
		RDemNode best;
		best.bi = -1;
		int bestutil = -1;

		//for(int bi=0; bi<BUILDINGS; bi++)
		for(auto biter=dm->supbpcopy.begin(); biter!=dm->supbpcopy.end(); biter++)
		{
			int bi = (*biter)->bi;
			Building* b = &g_building[bi];

			if(!b->on)
				continue;
			
			if(!b->finished)
				continue;

			BuildingT* bt = &g_bltype[b->type];

			if(bt->output[RES_HOUSING] <= 0)
				continue;
			
			int stockqty = b->stocked[RES_HOUSING] - (*biter)->supplying[RES_HOUSING];
			
			if(stockqty <= 0)
				continue;

			int marginpr = b->prodprice[RES_HOUSING];

			if(marginpr > *fundsleft)
				continue;

			int cmdist = Magnitude(b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2) - u->cmpos);
			int thisutil = PhUtil(marginpr, cmdist);

			if(thisutil <= bestutil && bestutil >= 0)
				continue;

			bestutil = thisutil;

			best.bi = bi;
			best.btype = b->type;
			best.rtype = RES_HOUSING;
			best.ramt = 1;
			best.bid.minbid = 1 * marginpr;
			best.bid.maxbid = 1 * marginpr;
			best.bid.tpos = b->tilepos;
			best.bid.cmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
			best.bid.maxdist = cmdist;
			best.bid.minutil = thisutil;

			bestdemb = *biter;
		}

		if(best.bi < 0)
		{
			// How to express distance-dependent
			// cash opportunity?
		
			Bid *altbid = &homedem->bid;
			altbid->maxbid = *fundsleft;	//willingness to spend all funds
			altbid->maxdist = -1;	//negative distance to indicate willingness to travel any distance
			altbid->cmpos = u->cmpos;
			altbid->tpos = u->cmpos/TILE_SIZE;
			altbid->minutil = -1;	//any util
		}
		else
		{
			// mark building consumption, so that other lab's consumption goes elsewhere
			bestdemb->supplying[RES_HOUSING] += best.ramt;
		
			//Just need to be as good or better than the last competitor.
			//Thought: but then that might not get all of the potential market.
			//So what to do?
			//Answer for now: it's probably not likely that more than one
			//shop will be required, so just get the last one.
			//Better answer: actually, the different "layers" can be
			//segmented, to create a demand for each with the associated
			//market/bid/cash.

			*homedem = best;

			//Don't subtract anything from fundsleft yet, purely speculative
		}
	}
	homedem->bi = -1;
	homedem->supbp = NULL;
	homedem->btype = -1;
	homedem->parent = NULL;
	homedem->rtype = RES_HOUSING;
	homedem->ramt = 1;
	homedem->ui = -1;
	homedem->utype = -1;
	homedem->demui = u - g_unit;
	dm->nodes.push_back(homedem);
}

// Food demand, bare necessity
void LabDemF(DemTree* dm, Unit* u, int* fundsleft)
{
	// Food
	// Which shop will the labourer shop at?
	// There might be more than one, if
	// supplies are limited or if money is still
	// left over.
	std::list<RDemNode> alts;	//alternatives
	//int fundsleft = u->belongings[RES_FUNDS];
	int fundsleft2 = *fundsleft;	//fundsleft2 includes subtractions from speculative, non-existent food suppliers, which isn't supposed to be subtracted from real funds
	int reqfood = CYCLE_FRAMES * LABOURER_FOODCONSUM;

	bool changed = false;
	
	// If there are alternatives/competitors
	do
	{
		DemsAtB* bestdemb = NULL;
		RDemNode best;
		best.bi = -1;
		changed = false;
		int bestutil = -1;

		//for(int bi=0; bi<BUILDINGS; bi++)
		for(auto biter=dm->supbpcopy.begin(); biter!=dm->supbpcopy.end(); biter++)
		{
			int bi = (*biter)->bi;
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			if(!b->finished)
				continue;

			BuildingT* bt = &g_bltype[b->type];

			if(bt->output[RES_RETFOOD] <= 0)
				continue;

			bool found = false;

			for(auto altiter=alts.begin(); altiter!=alts.end(); altiter++)
			{
				if(altiter->bi == bi)
				{
					found = true;
					break;
				}
			}

			if(found)
				continue;

			int marginpr = b->prodprice[RES_RETFOOD];
			int stockqty = b->stocked[RES_RETFOOD] - (*biter)->supplying[RES_RETFOOD];
			
			if(stockqty <= 0)
				continue;

			int cmdist = Magnitude(b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2) - u->cmpos);
			int thisutil = PhUtil(marginpr, cmdist);

			if(thisutil <= bestutil && bestutil >= 0)
				continue;

			bestutil = thisutil;

			int reqqty = imin(stockqty, reqfood);
			int affordqty = imin(reqqty, fundsleft2 / marginpr);
			stockqty -= affordqty;
			reqfood -= affordqty;

			if(affordqty <= 0)
				continue;

			changed = true;

			best.bi = bi;
			best.btype = b->type;
			best.rtype = RES_RETFOOD;
			best.ramt = affordqty;
			best.bid.minbid = affordqty * marginpr;
			best.bid.maxbid = affordqty * marginpr;
			best.bid.tpos = b->tilepos;
			best.bid.cmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
			best.bid.maxdist = cmdist;
			best.bid.minutil = thisutil;

			bestdemb = *biter;
		}

		if(best.bi < 0)
			break;
		
		alts.push_back(best);
		*fundsleft -= best.bid.maxbid;
		fundsleft2 -= best.bid.maxbid;
		
		// mark building consumption, so that other lab's consumption goes elsewhere
		bestdemb->supplying[RES_RETFOOD] += best.ramt;
		
		//Just need to be as good or better than the last competitor.
		//Thought: but then that might not get all of the potential market.
		//So what to do?
		//Answer for now: it's probably not likely that more than one
		//shop will be required, so just get the last one.
		//Better answer: actually, the different "layers" can be
		//segmented, to create a demand for each with the associated
		//market/bid/cash.

		RDemNode* rdem = new RDemNode;
		*rdem = best;
		dm->nodes.push_back(rdem);

	}while(changed && fundsleft2 > 0);

	//Note: if there is no more money, yet still a requirement
	//for more food, then food is too expensive to be afforded
	//and something is wrong. 
	//Create demand for cheap food?
	if(fundsleft2 <= 0)
		return;

	// If reqfood > 0 still
	if(reqfood > 0)
	{
		RDemNode* demremain = new RDemNode;
		demremain->bi = -1;
		demremain->btype = -1;
		demremain->rtype = RES_RETFOOD;
		demremain->ramt = reqfood;
		demremain->bid.minbid = fundsleft2;
		demremain->bid.maxbid = fundsleft2;
		demremain->bid.tpos = u->cmpos / TILE_SIZE;
		demremain->bid.cmpos = u->cmpos;
		demremain->bid.maxdist = -1;	//any distance
		demremain->bid.minutil = -1;	//any util
		dm->nodes.push_back(demremain);
	}

	// If there is any money not spent on available food
	// Possible demand for more food (luxury) in LabDemF2();
}

// Food demand, luxury
void LabDemF2(DemTree* dm, Unit* u, int* fundsleft)
{
	// Food
	// Which shop will the labourer shop at?
	// There might be more than one, if
	// supplies are limited or if money is still
	// left over.
	std::list<RDemNode> alts;	//alternatives
	//int fundsleft = u->belongings[RES_FUNDS];
	int fundsleft2 = *fundsleft;	//fundsleft2 includes subtractions from speculative, non-existent food suppliers, which isn't supposed to be subtracted from real funds

	bool changed = false;
	
	// If there are alternatives/competitors
	do
	{
		DemsAtB* bestdemb = NULL;
		RDemNode best;
		best.bi = -1;
		changed = false;
		int bestutil = -1;

		//for(int bi=0; bi<BUILDINGS; bi++)
		for(auto biter=dm->supbpcopy.begin(); biter!=dm->supbpcopy.end(); biter++)
		{
			int bi = (*biter)->bi;
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			if(!b->finished)
				continue;

			BuildingT* bt = &g_bltype[b->type];

			if(bt->output[RES_RETFOOD] <= 0)
				continue;

			bool found = false;

			for(auto altiter=alts.begin(); altiter!=alts.end(); altiter++)
			{
				if(altiter->bi == bi)
				{
					found = true;
					break;
				}
			}

			if(found)
				continue;

			int marginpr = b->prodprice[RES_RETFOOD];
			int stockqty = b->stocked[RES_RETFOOD] - (*biter)->supplying[RES_RETFOOD];

			if(stockqty <= 0)
				continue;

			int cmdist = Magnitude(b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2) - u->cmpos);
			int thisutil = PhUtil(marginpr, cmdist);

			if(thisutil <= bestutil && bestutil >= 0)
				continue;

			bestutil = thisutil;

			int luxuryqty = imin(stockqty, fundsleft2 / marginpr);
			stockqty -= luxuryqty;

			if(luxuryqty <= 0)
				continue;

			changed = true;

			best.bi = bi;
			best.btype = b->type;
			best.rtype = RES_RETFOOD;
			best.ramt = luxuryqty;
			best.bid.minbid = 0;
			best.bid.maxbid = luxuryqty * marginpr;
			best.bid.tpos = b->tilepos;
			best.bid.cmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
			best.bid.maxdist = cmdist;
			best.bid.minutil = thisutil;

			bestdemb = *biter;
		}

		if(best.bi < 0)
			break;
		
		alts.push_back(best);
		*fundsleft -= best.bid.maxbid;
		fundsleft2 -= best.bid.maxbid;
		
		// mark building consumption, so that other lab's consumption goes elsewhere
		bestdemb->supplying[RES_RETFOOD] += best.ramt;
		
		//Just need to be as good or better than the last competitor.
		//Thought: but then that might not get all of the potential market.
		//So what to do?
		//Answer for now: it's probably not likely that more than one
		//shop will be required, so just get the last one.
		//Better answer: actually, the different "layers" can be
		//segmented, to create a demand for each with the associated
		//market/bid/cash.

		RDemNode* rdem = new RDemNode;
		*rdem = best;
		dm->nodes.push_back(rdem);

	}while(changed && fundsleft2 > 0);

	if(fundsleft2 <= 0)
		return;

	// If there is any money not spent on available food
	// Possible demand for more food (luxury) in LabDemF2();

	RDemNode* demremain = new RDemNode;
	demremain->bi = -1;
	demremain->btype = -1;
	demremain->rtype = RES_RETFOOD;
	demremain->ramt = 1;	//should really be unspecified amount, but it might as well be 1 because labourer is willing to spend all his remaining luxury money on 1
	demremain->bid.minbid = 0;
	demremain->bid.maxbid = fundsleft2;
	demremain->bid.tpos = u->cmpos / TILE_SIZE;
	demremain->bid.cmpos = u->cmpos;
	demremain->bid.maxdist = -1;	//any distance
	demremain->bid.minutil = -1;	//any util
	dm->nodes.push_back(demremain);
}

// Electricity demand, bare necessity
void LabDemE(DemTree* dm, Unit* u, int* fundsleft)
{
	// Electricity
	// Which provider will the labourer get energy from?
	// There might be more than one, if
	// supplies are limited or if money is still
	// left over.
	std::list<RDemNode> alts;	//alternatives
	//int fundsleft = u->belongings[RES_FUNDS];
	int fundsleft2 = *fundsleft;	//fundsleft2 includes subtractions from speculative, non-existent food suppliers, which isn't supposed to be subtracted from real funds
	int reqelec = LABOURER_ENERGYCONSUM;

	bool changed = false;
	
	// If there are alternatives/competitors
	do
	{
		DemsAtB* bestdemb = NULL;
		RDemNode best;
		best.bi = -1;
		changed = false;
		int bestutil = -1;

		//for(int bi=0; bi<BUILDINGS; bi++)
		for(auto biter=dm->supbpcopy.begin(); biter!=dm->supbpcopy.end(); biter++)
		{
			int bi = (*biter)->bi;
			Building* b = &g_building[bi];

			if(!b->on)
				continue;
			
			if(!b->finished)
				continue;

			BuildingT* bt = &g_bltype[b->type];

			if(bt->output[RES_ENERGY] <= 0)
				continue;

			bool found = false;

			for(auto altiter=alts.begin(); altiter!=alts.end(); altiter++)
			{
				if(altiter->bi == bi)
				{
					found = true;
					break;
				}
			}

			if(found)
				continue;

			int marginpr = b->prodprice[RES_ENERGY];
			int stockqty = b->stocked[RES_ENERGY] - (*biter)->supplying[RES_ENERGY];
			
			//int cmdist = Magnitude(b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2) - u->cmpos);
			int thisutil = GlUtil(marginpr);

			if(thisutil <= bestutil && bestutil >= 0)
				continue;

			bestutil = thisutil;

			int reqqty = imin(stockqty, reqelec);
			int affordqty = imin(reqqty, fundsleft2 / marginpr);
			stockqty -= affordqty;
			reqelec -= affordqty;

			if(affordqty <= 0)
				continue;

			changed = true;

			best.bi = bi;
			best.btype = b->type;
			best.rtype = RES_ENERGY;
			best.ramt = affordqty;
			best.bid.minbid = affordqty * marginpr;
			best.bid.maxbid = affordqty * marginpr;
			best.bid.tpos = b->tilepos;
			best.bid.cmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
			best.bid.maxdist = -1;	//any distance
			best.bid.minutil = thisutil;

			bestdemb = *biter;
		}

		if(best.bi < 0)
			break;
		
		alts.push_back(best);
		*fundsleft -= best.bid.maxbid;
		fundsleft2 -= best.bid.maxbid;
		
		// mark building consumption, so that other lab's consumption goes elsewhere
		bestdemb->supplying[RES_RETFOOD] += best.ramt;
		
		//Just need to be as good or better than the last competitor.
		//Thought: but then that might not get all of the potential market.
		//So what to do?
		//Answer for now: it's probably not likely that more than one
		//shop will be required, so just get the last one.
		//Better answer: actually, the different "layers" can be
		//segmented, to create a demand for each with the associated
		//market/bid/cash.

		RDemNode* rdem = new RDemNode;
		*rdem = best;
		dm->nodes.push_back(rdem);

	}while(changed && fundsleft2 > 0);

	//Note: if there is no more money, yet still a requirement
	//for more electricity, then electricity or other necessities are
	//too expensive to be afforded and something is wrong. 
	//Create demand for cheap food/electricity/housing?
	if(fundsleft2 <= 0)
		return;

	// If reqelec > 0 still
	if(reqelec > 0)
	{
		RDemNode* demremain = new RDemNode;
		demremain->bi = -1;
		demremain->btype = -1;
		demremain->rtype = RES_ENERGY;
		demremain->ramt = reqelec;
		demremain->bid.minbid = fundsleft2;
		demremain->bid.maxbid = fundsleft2;
		demremain->bid.tpos = u->cmpos / TILE_SIZE;
		demremain->bid.cmpos = u->cmpos;
		demremain->bid.maxdist = -1;	//any distance
		demremain->bid.minutil = -1;	//any util
		dm->nodes.push_back(demremain);
	}

	// If there is any money not spent on available electricity
	// Possible demand for more electricity (luxury) in LabDemE2();
}

//See how much construction of bl, roads+infrastructure will cost, if location is suitable
//If it is, all the proposed construction will be added to the demtree
bool TryBl(DemTree* dm, Player* p, int* concost)
{

	return false;
}

//Duplicate demb's of demtree, without dem lists yet
void DupDemB(DemTree* orig, DemTree* copy)
{
#if 0
	int bi;
	int btype;
	std::list<DemNode*> condems;	//construction material
	std::list<DemNode*> proddems;	//production input raw materials
	std::list<DemNode*> manufdems;	//manufacturing input raw materials
	std::list<CrPipeDem*> crpipedems;
	std::list<PowlDem*> powldems;
	std::list<RoadDem*> roaddems;
	int prodratio;
	int condem[RESOURCES];
	int supplying[RESOURCES];
#endif

	for(auto diter=orig->supbpcopy.begin(); diter!=orig->supbpcopy.end(); diter++)
	{
		DemsAtB* newdem = new DemsAtB;

		//TODO

		orig->supbpcopy.push_back(newdem);
	}
}

//Duplicate dem nodes, but don't link yet
void DupRDem(DemTree* orig, DemTree* copy)
{

}

//Link dem nodes
void LinkDem(DemTree* orig, DemTree* copy)
{
}

//Duplicate demtree
void DupDT(DemTree* orig, DemTree* copy)
{
#if 0
	std::list<DemNode*> nodes;
	std::list<DemsAtB*> supbpcopy;	//master copy, this one will be freed
	std::list<DemsAtU*> supupcopy;	//master copy, this one will be freed
	std::list<DemNode*> codems[CONDUIT_TYPES];	//conduit placements
#endif

	DupDemB(orig, copy);
	// TODO:
	//DupDemU(orig, copy);
	//DupDemCo(orig, copy);
	DupRDem(orig, copy);
	LinkDem(orig, copy);
}

/*
Determine cost composition for proposed building production, so don't create r dems
Almost estimate transportation
Roads and infrastructure might not be present yet, so just ignore that, as long as they're on the same island
*/
void DetCompo(Player* p, int btype, int rtype, int ramt, Vec2i tpos, Bid* bid)
{

}


/*
Based on the dems in the tree, find the best price for this tile and res, if a profit can be generated
*/
void TileDem(DemTree* dm, Player* p, int ri, RDemNode* pt, int x, int z)
{
	std::list<RDemNode> rdems;

	for(auto diter=dm->nodes.begin(); diter!=dm->nodes.end(); diter++)
	{
		DemNode* dn = *diter;

		if(dn->demtype != DEM_RNODE)
			continue;

		RDemNode* rdn = (RDemNode*)dn;

		if(rdn->rtype != ri)
			continue;

		int requtil = rdn->bid.minutil+1;
		int cmdist = Magnitude(Vec2i(x,z)*TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2 - rdn->bid.cmpos);
		int maxpr = InvPhUtilP(requtil, cmdist);

		while(PhUtil(maxpr, cmdist) < requtil)
			maxpr--;

		if(maxpr <= 0)
			continue;

		int maxrev = maxpr * rdn->ramt;

		if(rdn->bid.minutil < 0)
			maxrev = imin(maxrev, rdn->bid.maxbid);	//unecessary? might be necessary if minutil is -1 (any util) and there's only a budget constraint

		RDemNode proj;	//projected revenue
		proj.ramt = rdn->ramt;
		proj.bid.marginpr = maxpr;
		proj.bid.maxbid = maxrev;
		proj.bid.minbid = maxrev;

		int bestprofit = -1;
		int bestbtype = -1;

		//TODO: variable cost of resource production and raw input transport
		//Try for all supporting bltypes
		for(int btype=0; btype<BUILDING_TYPES; btype++)
		{
			BuildingT* bt = &g_bltype[btype];

			if(bt->output[ri] <= 0)
				continue;

			RDemNode testproj = proj;
			DetCompo(p, btype, ri, proj.ramt, Vec2i(x,z), &testproj.bid);

			int maxrev = 0;

			Bid* bid = &testproj.bid;
			auto& costco = bid->costcompo;
			int bprofit = 0;

			for(auto citer=costco.begin(); citer!=costco.end(); citer++)
			{
				int subprofit = maxpr * citer->ramt - (citer->fixcost + citer->margcost * citer->ramt);

				if(subprofit > 0)
					bprofit += subprofit;
			}

			if(bprofit > bestprofit || bestprofit < 0)
			{
				bestprofit = bprofit;
				bestbtype = btype;
			}
		}

		if(bestbtype >= 0)
		{
			proj.btype = bestbtype;
			rdems.push_back(proj);
		}
	}

	if(rdems.size() <= 0)
		return;

	int bestprc = -1;
	int bestprofit = -1;
	int prevprc = -1;

	//evalute max projected revenue at tile
	//try all the price levels from smallest to greatest
	while(true)
	{
		int leastnext = prevprc;

		//while there's another possible price, see if it will generate more total revenue
		for(auto diter=rdems.begin(); diter!=rdems.end(); diter++)
			if((diter->bid.marginpr < leastnext && diter->bid.marginpr > prevprc) || leastnext < 0)
				leastnext = diter->bid.marginpr;

		if(leastnext == prevprc)
			break;

		prevprc = leastnext;

		//see how much profit this price level will generate
		int curprofit = 0;

		for(auto diter=rdems.begin(); diter!=rdems.end(); diter++)
			if(diter->bid.marginpr >= leastnext)
			{
				//find max profit based on cost composition and price

				//curprofit += diter->ramt * leastnext;

				Bid* bid = &diter->bid;
				auto& costco = bid->costcompo;

				for(auto citer=costco.begin(); citer!=costco.end(); citer++)
				{
					int subprofit = leastnext * citer->ramt - (citer->fixcost + citer->margcost * citer->ramt);

					if(subprofit > 0)
						curprofit += subprofit;
				}
			}

			if(curprofit > bestprofit)
			{
				bestprofit = curprofit;
				bestprc = leastnext;
			}
	}

	//if no profit can be generated
	if(bestprofit <= 0)
		return;

	pt->bid.maxbid = bestprofit;
	pt->bid.marginpr = bestprc;
}

/*
Check which spots will generate profit for this res and given tile prices and info
*/
void BlSpots(DemTree* dm, Player* p, int ri, RDemNode* tiles)
{
	//go through all locations, evalute road+infrastructure costs, and check best location, if it will generate profit
	int bestprofit = -1;
	Vec2i besttile;
	DemTree bestdm;

	for(int z=0; z<g_hmap.m_widthz; z++)
		for(int x=0; x<g_hmap.m_widthx; x++)
		{
			RDemNode* pt = &tiles[ z*g_hmap.m_widthx + x ];
			int profit = pt->bid.maxbid;

			// TODO: plot roads and infrastructure costs
			//...

			DemTree prop;	//proposed
			DupDT(dm, &prop);
			int concost = 0;

			if(!TryBl(&prop, p, &concost))
				continue;

			profit -= concost;

			Bid* bid = &pt->bid;
			auto& costco = bid->costcompo;

			for(auto citer=costco.begin(); citer!=costco.end(); citer++)
			{
				int subprofit = bid->marginpr * citer->ramt - (citer->fixcost + citer->margcost * citer->ramt);

				if(subprofit > 0)
					profit += subprofit;
			}

			if(profit > bestprofit || bestprofit < 0)
			{
				bestprofit = profit;
				besttile = Vec2i(x,z);
				bestdm.free();
				DupDT(&prop, &bestdm);
			}
		}

	if(bestprofit > 0)
	{
		//check if profit is enough to justify construction

		//if true
		dm->free();
		DupDT(&bestdm, dm);
	}
}

/*
For each resource demand,
for each building that supplies that resource,
plot the maximum revenue obtainable at each
tile for that building, consider the cost
of connecting roads and infrastructure,
and choose which is most profitable.
*/
void TryBl(DemTree* dm, Player* p)
{
	//For resources that must be transported physically
	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(!r->physical)
			continue;

		RDemNode* tiles = new RDemNode[ g_hmap.m_widthx * g_hmap.m_widthz ];

		for(int z=0; z<g_hmap.m_widthz; z++)
			for(int x=0; x<g_hmap.m_widthx; x++)
			{
				RDemNode* pt = &tiles[ z*g_hmap.m_widthx + x ];
				TileDem(dm, p, ri, pt, x, z);
			}

		BlSpots(dm, p, ri, tiles);

		// TODO ...

		delete [] tiles;
	}
	

	//For non-physical res suppliers, distance
	//to clients doesn't matter, and only distance of
	//its suppliers and road+infrastructure costs
	//matter.

	//TODO: ...
}

// 1. Opportunities where something is overpriced
// and building a second supplier would be profitable.
// 2. Profitability of building for primary demands (from consumers)
// including positional information. Funnel individual demands into
// position candidates? Also, must be within consideration of existing
// suppliers. 
// 3. Profitability of existing secondary etc. demands (inter-industry).
// 4. Trucks, infrastructure.
void CalcDem2(Player* p)
{
	int pi = p - g_player;
	DemTree* dm = &g_demtree2[pi];

	AddBl(dm);

	// Point #2 - building for primary demands

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != UNIT_LABOURER)
			continue;

		int fundsleft = u->belongings[RES_FUNDS];
		
		LabDemF(dm, u, &fundsleft);
		LabDemH(dm, u, &fundsleft);
		LabDemE(dm, u, &fundsleft);
		LabDemF2(dm, u, &fundsleft);
	}

	// To do: inter-industry demand
	// TODO :...

	// A thought about funneling point demands
	// to choose an optimal building location.
	// Each point demand presents a radius in which
	// a supplier would be effective. The intersection
	// of these circles brings the most profit.
	// It is necessary to figure out the minimum 
	// earning which would be necessary to be worthy
	// of constructing the building. 
	// This gives the minimum earning combination
	// of intersections necessary. 
	// The best opportunity though is the one with
	// the highest earning combination.

	TryBl(dm, p);
}
