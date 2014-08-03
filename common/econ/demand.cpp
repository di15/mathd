#include "demand.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/sim.h"
#include "../sim/labourer.h"
#include "../utils.h"
#include "utility.h"
#include "../math/fixmath.h"
#include "../sim/build.h"

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
First parent is always a DemsAtB, while parent2 might be DemsAtU or DemsAtB
Maybe change so that it's clear which one is supplier and which demander?
*/
void AddInf(DemTree* dm, std::list<DemNode*>* cdarr, DemNode* parent, DemNode* parent2, int rtype, int ramt, int depth, bool* success)
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

//best actual (not just proposed) supplier
DemsAtB* BestAcSup(DemTree* dm, Vec2i demtpos, Vec2i demcmpos, int rtype)
{
    DemsAtB* bestdemb = NULL;
    int bestutil = -1;
    Resource* r = &g_resource[rtype];

    for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter++)
    {
        DemsAtB* demb = (DemsAtB*)*biter;
        BlType* bt = &g_bltype[demb->btype];

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

        Vec2i btpos;
        int margpr;

        //proposed bl?
        if(demb->bi < 0)
        {
            btpos = demb->bid.tpos;
            margpr = demb->bid.marginpr;
        }
        //actual bl's, rather than proposed
        else
        {
            Building* b = &g_building[demb->bi];
            btpos = b->tilepos;
            margpr = b->prodprice[rtype];
        }

        //check if distance is better or if there's no best yet

        Vec2i bcmpos = btpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

        int dist = Magnitude(bcmpos - demcmpos);

        //if(dist > bestdist && bestdemb)
        //	continue;

        int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

        if(util > bestutil && bestdemb)
            continue;

        bestdemb = demb;
        bestutil = util;
    }

	return bestdemb;
}
//best proposed supplier
DemsAtB* BestPrSup(DemTree* dm, Vec2i demtpos, Vec2i demcmpos, int rtype)
{
    DemsAtB* bestdemb = NULL;
    int bestutil = -1;
    Resource* r = &g_resource[rtype];

    //include proposed demb's in search if all actual b's used up
    for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter++)
    {
        DemsAtB* demb = (DemsAtB*)*biter;
        BlType* bt = &g_bltype[demb->btype];

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

        //only search for proposed bl's in this loop
        if(demb->bi >= 0)
            continue;

        //check if distance is better or if there's no best yet
        Building* b = &g_building[demb->bi];

        Vec2i btpos = b->tilepos;
        Vec2i bcmpos = btpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

        int dist = Magnitude(bcmpos - demcmpos);

        //if(dist > bestdist && bestdemb)
        //	continue;

        int margpr = b->prodprice[rtype];

        int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

        if(util > bestutil && bestdemb)
            continue;

        bestdemb = demb;
        bestutil = util;
    }

	return bestdemb;
}

//add requisite resource demand
//try to match to suppliers
void AddReq(DemTree* dm, Player* p, std::list<DemNode*>* nodes, DemNode* parent, int rtype, int ramt, Vec2i demtpos, Vec2i demcmpos, int depth, bool* success)
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

	*success = true;

	Resource* r = &g_resource[rtype];

#if 0
	Vec2i demtpos = Vec2i(g_hmap.m_widthx, g_hmap.m_widthz)/2;
	Vec2i demcmpos = demtpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	DemsAtB* pardemb = NULL;
	DemsAtU* pardemu = NULL;
	Unit* paru = NULL;

	if(parent)
	{
		if(parent->demtype == DEM_BNODE)
		{
			pardemb = (DemsAtB*)parent;
			demtpos = pardemb->tilepos;
			demcmpos = demtpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		}
		else if(parent->demtype == DEM_UNODE)
		{
			pardemu = (DemsAtU*)parent;

			if(pardemu->ui >= 0)
			{
				paru = &g_unit[paru->ui];
				demcmpos = paru->cmpos;
				demtpos = demcmpos / TILE_SIZE;
			}
		}
	}
#endif

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

	//commented out this part
	//leave it up to repeating CheckBl calls to match all r dems
#if 1
	while(rremain > 0)
	{
		DemsAtB* bestdemb = NULL;
		int bestutil = -1;

		//bestac
		bestdemb = BestAcSup(dm, demtpos, demcmpos, rtype);

		//best prop
		if(!bestdemb)
			bestdemb = BestPrSup(dm, demtpos, demcmpos, rtype);

		//if there's still no supplier, propose one to be made
		//and set that supplier as bestdemb
		if(!bestdemb)
		{
			//Leave it up to another call to CheckBl to match these up
			//Do nothing in this function
		}

		//if no proposed bl can be placed, return
		//if(!bestdemb)
		//	return;
		//actually, add rdem to dm->rdemcopy with minutil=-1 and let it
		//be matched by the next call to CheclBl


        RDemNode* rdem = new RDemNode;
        rdem->parent = parent;
        rdem->rtype = rtype;
        rdem->ui = -1;
        rdem->utype = -1;
        // TO DO: unit transport

		//if bestdemb found, subtract usage, add its reqs, add conduits, etc.

		if(bestdemb)
		{
			DemsAtB* demb = bestdemb;
			BlType* bt = &g_bltype[demb->btype];

			if(bt->output[rtype] <= 0)
				continue;

			int capleft = bt->output[rtype];
			capleft -= demb->supplying[rtype];

			int suphere = imin(capleft, rremain);
			rremain -= suphere;

	#ifdef DEBUG
			g_log<<"\tsuphere "<<suphere<<" remain "<<remain<<std::endl;
			g_log.flush();
	#endif

			rdem->bi = demb->bi;
			rdem->btype = demb->btype;
			rdem->supbp = demb;
			rdem->ramt = suphere;

			int margpr = -1;

			//for actual bl's
			if(demb->bi >= 0)
			{
				Building* b = &g_building[demb->bi];
				margpr = b->prodprice[rtype];
			}
			//for proposed bl's
			else
			{
				//this bid price might be for any one of the output res's
				//this might not be correct (for mines for eg)
				margpr = demb->bid.marginpr;
			}

			//distance between demanding bl that called this func and supl bl (in demb)
			int cmdist = cmdist = Magnitude(demcmpos - demb->bid.cmpos);

			int util = r->physical ? PhUtil(margpr, cmdist) : GlUtil(margpr);
        	rdem->bid.minutil = util;

			nodes->push_back(rdem);
			dm->rdemcopy.push_back(rdem);

			//int producing = bt->output[rtype] * demb->prodratio / RATIO_DENOM;
			//int overprod = producing - demb->supplying[rtype] - suphere;

			int newprodlevel = (demb->supplying[rtype] + suphere) * RATIO_DENOM / bt->output[rtype];
			newprodlevel = imax(1, newprodlevel);
			demb->supplying[rtype] += suphere;

	#ifdef DEBUG
			g_log<<"suphere"<<suphere<<" of total"<<demb->supplying[rtype]<<" of remain"<<remain<<" of res "<<g_resource[rtype].name<<" newprodlevel "<<demb->prodratio<<" -> "<<newprodlevel<<std::endl;
			g_log.flush();
	#endif

			//if prodlevel increased for this supplier,
			//add the requisite r dems
			if(newprodlevel > demb->prodratio)
			{
				int extraprodlev = newprodlevel - demb->prodratio;
				int oldprodratio = demb->prodratio;
				demb->prodratio = newprodlevel;

				Vec2i suptpos = demb->bid.tpos;
				Vec2i supcmpos = demb->bid.cmpos;

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

					bool subsuccess;
					AddReq(dm, p, &demb->proddems, demb, ri, rreq, suptpos, supcmpos, depth+1, &subsuccess);
				}
			}

			bool subsuccess;
			//add infrastructure to supplier
			AddInf(dm, bestdemb->cddems, parent, bestdemb, rtype, ramt, depth, &subsuccess);

			//this information might be important for building placement
			*success = subsuccess ? *success : false;
		}
		else	//no supplier bl found, so leave it up to the next call to CheckBl to match this rdem
		{
			rdem->bi = -1;
			rdem->btype = -1;
        	rdem->ramt = rremain;
        	rremain = 0;
        	rdem->bid.minutil = -1;
			nodes->push_back(rdem);
			dm->rdemcopy.push_back(rdem);
		}
        if(rremain <= 0)
            return;
	}
#endif

#if 0
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
		BlType* t = &g_bltype[i];

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

	BlType* t = &g_bltype[leastplb];
	//int reqnb = imax(1, Ceili(ramt, t->output[rtype]));

	//get all the req rtype supplied with proposed suppliers
	do
	{
		//check all tiles,bltypes for best supplying

		for(int x=0; x<g_hmap.m_widthx; x++)
		{
			for(int z=0; z<g_hmap.m_widthz; z++)
			{

			}
		}

		//CheckTile
#if 0
		DemsAtB* demb = new DemsAtB();

		demb->parent = NULL;
		demb->prodratio = 0;
		Zero(demb->supplying);
		Zero(demb->condem);
		demb->bi = -1;
		demb->btype = leastplb;

		dm->supbpcopy.push_back(demb);

		// need to set this before req of conmat because of possible recursive loop

		BlType* bt = &g_bltype[demb->btype];

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
		dm->rdemcopy.push_back(rdem);

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
#endif
	}while(rremain > 0);
#endif

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
		demb->bid.tpos = b->tilepos;
		demb->bid.cmpos = b->tilepos*TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

		dm->supbpcopy.push_back(demb);
	}
}

void BlConReq(DemTree* dm, Player* curp)
{
	bool success;

	for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter++)
	{
		DemsAtB* demb = (DemsAtB*)*biter;
		const int bi = demb->bi;

		BlType* bt = NULL;
		int conmat[RESOURCES];
		bool finished = false;

		Vec2i tpos;
		Vec2i cmpos;

		if(bi >= 0)
		{
			Building* b = &g_building[bi];
			bt = &g_bltype[b->type];
			memcpy(conmat, b->conmat, sizeof(int)*RESOURCES);
			finished = b->finished;
			tpos = b->tilepos;
			cmpos = tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		}
		else
		{
			bt = &g_bltype[demb->btype];
			memcpy(conmat, demb->condem, sizeof(int)*RESOURCES);
			finished = false;
			tpos = demb->bid.tpos;
			cmpos = demb->bid.cmpos;
		}

		if(!finished)
		{
			for(int i=0; i<RESOURCES; i++)
			{
				const int req = bt->conmat[i] - conmat[i];
				if(req > 0)
					AddReq(dm, curp, &demb->condems, *biter, i, req, tpos, cmpos, 0, &success);
			}
		}
		//else finished construction
		else
		{
			//Don't need to do anything then
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
	BlConReq(&g_demtree, NULL);

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

	//AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_HOUSING, nlab, 0);
	//AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_RETFOOD, LABOURER_FOODCONSUM * CYCLE_FRAMES, 0);
	//AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_ENERGY, nlab * LABOURER_ENERGYCONSUM, 0);
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
			DemsAtB* demb = (DemsAtB*)*biter;
			int bi = demb->bi;
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			//if(!b->finished)
			//	continue;

			BlType* bt = &g_bltype[b->type];

			if(bt->output[RES_HOUSING] <= 0)
				continue;

			int stockqty = b->stocked[RES_HOUSING] - demb->supplying[RES_HOUSING];

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

			bestdemb = demb;
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
	dm->rdemcopy.push_back(homedem);
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
			DemsAtB* demb = (DemsAtB*)*biter;
			int bi = demb->bi;
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			//if(!b->finished)
			//	continue;

			BlType* bt = &g_bltype[b->type];

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
			int stockqty = b->stocked[RES_RETFOOD] - demb->supplying[RES_RETFOOD];

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

			bestdemb = demb;
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
		dm->rdemcopy.push_back(rdem);

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
		dm->rdemcopy.push_back(demremain);
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
			DemsAtB* demb = (DemsAtB*)*biter;
			int bi = demb->bi;
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			//if(!b->finished)
			//	continue;

			BlType* bt = &g_bltype[b->type];

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
			int stockqty = b->stocked[RES_RETFOOD] - demb->supplying[RES_RETFOOD];

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

			bestdemb = demb;
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
		dm->rdemcopy.push_back(rdem);

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
	dm->rdemcopy.push_back(demremain);
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
			DemsAtB* demb = (DemsAtB*)*biter;
			int bi = demb->bi;
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			//if(!b->finished)
			//	continue;

			BlType* bt = &g_bltype[b->type];

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
			int stockqty = b->stocked[RES_ENERGY] - demb->supplying[RES_ENERGY];

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

			bestdemb = demb;
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
		dm->rdemcopy.push_back(rdem);

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
		dm->rdemcopy.push_back(demremain);
	}

	// If there is any money not spent on available electricity
	// Possible demand for more electricity (luxury) in LabDemE2();
}

#if 0
//See how much construction of bl, roads+infrastructure will cost, if location is suitable
//If it is, all the proposed construction will be added to the demtree
bool CheckBl(DemTree* dm, Player* p, RDemNode* pt, int* concost)
{
	//pt is the tile with bid and bltype

	return false;
}
#endif

//Duplicate demb's of demtree, without dem lists yet
void DupDemB(DemTree* orig, DemTree* copy)
{
#if 0
	int bi;
	int btype;
	std::list<DemNode*> condems;	//construction material
	std::list<DemNode*> proddems;	//production input raw materials
	std::list<DemNode*> manufdems;	//manufacturing input raw materials
	std::list<CdDem*> cddems[CONDUIT_TYPES];
	int prodratio;
	int condem[RESOURCES];
	int supplying[RESOURCES];
#endif
#if 0	//DemNode
	DemNode* parent;
	Bid bid;	//reflects the maximum possible gouging price
	int profit;
#endif

	for(auto diter=orig->supbpcopy.begin(); diter!=orig->supbpcopy.end(); diter++)
	{
		DemsAtB* olddem = (DemsAtB*)*diter;
		DemsAtB* newdem = new DemsAtB;

		newdem->bid = olddem->bid;
		newdem->profit = olddem->profit;

		newdem->bi = olddem->bi;
		newdem->btype = olddem->btype;
		newdem->prodratio = olddem->prodratio;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			newdem->condem[ri] = olddem->condem[ri];
			newdem->supplying[ri] = olddem->supplying[ri];
		}

		copy->supbpcopy.push_back(newdem);
	}
}


//Duplicate demb's of demtree, without dem lists yet
void DupDemU(DemTree* orig, DemTree* copy)
{
//DemsAtU
#if 0
	int ui;
	int utype;
	std::list<RDemNode*> manufdems;
	std::list<RDemNode*> consumdems;
	DemsAtU* opup;	//operator/driver
	int prodratio;
	int timeused;
	int totaldem[RESOURCES];
#endif
#if 0	//DemNode
	DemNode* parent;
	Bid bid;	//reflects the maximum possible gouging price
	int profit;
#endif

	for(auto diter=orig->supupcopy.begin(); diter!=orig->supupcopy.end(); diter++)
	{
		DemsAtU* olddem = (DemsAtU*)*diter;
		DemsAtU* newdem = new DemsAtU;

		newdem->bid = olddem->bid;
		newdem->profit = olddem->profit;

		newdem->ui = olddem->ui;
		newdem->utype = olddem->utype;
		newdem->prodratio = olddem->prodratio;
		newdem->timeused = olddem->timeused;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			newdem->totaldem[ri] = olddem->totaldem[ri];
		}

		copy->supupcopy.push_back(newdem);
	}
}

//Duplicate demb's of demtree, without dem lists yet
void DupDemCo(DemTree* orig, DemTree* copy)
{
//CdDem
#if 0
	int cdtype;
	Vec2i tpos;
	std::list<RDemNode*> condems;
#endif

#if 0	//DemNode
	DemNode* parent;
	Bid bid;	//reflects the maximum possible gouging price
	int profit;
#endif

	for(int ctype=0; ctype<CONDUIT_TYPES; ctype++)
	{
		for(auto diter=orig->codems[ctype].begin(); diter!=orig->codems[ctype].end(); diter++)
		{
			CdDem* olddem = (CdDem*)*diter;
			CdDem* newdem = new CdDem;

			newdem->bid = olddem->bid;
			newdem->profit = olddem->profit;

			newdem->cdtype = olddem->cdtype;
			newdem->tpos = olddem->tpos;

			copy->codems[ctype].push_back(newdem);
		}
	}
}

//Duplicate dem nodes, but don't link yet
void DupRDem(DemTree* orig, DemTree* copy)
{
	//	RDemNode
#if 0
	int rtype;
	int ramt;
	int btype;
	int bi;
	int utype;
	int ui;
	int demui;
	DemsAtB* supbp;
	DemsAtU* supup;
	DemsAtU* opup;
#endif

#if 0	//DemNode
	DemNode* parent;
	Bid bid;	//reflects the maximum possible gouging price
	int profit;
#endif

	for(auto diter=orig->rdemcopy.begin(); diter!=orig->rdemcopy.end(); diter++)
	{
		DemNode* olddem = *diter;

		if(olddem->demtype != DEM_RNODE)
			continue;

		RDemNode* oldrdem = (RDemNode*)olddem;
		RDemNode* newrdem = new RDemNode;

		newrdem->bid = oldrdem->bid;
		newrdem->profit = oldrdem->profit;

		newrdem->rtype = oldrdem->rtype;
		newrdem->ramt = oldrdem->ramt;
		newrdem->btype = oldrdem->btype;
		newrdem->bi = oldrdem->bi;
		newrdem->utype = oldrdem->utype;
		newrdem->ui = oldrdem->ui;
		newrdem->demui = oldrdem->demui;

		copy->rdemcopy.push_back(newrdem);
	}
}

int DemIndex(DemNode* pdem, std::list<DemNode*>& list)
{
	if(!pdem)
		return -1;

	int di = 0;
	for(auto diter=list.begin(); diter!=list.end(); diter++, di++)
	{
		if(*diter == pdem)
			return di;
	}

	return -1;
}

DemNode* DemAt(std::list<DemNode*>& list, int seekdi)
{
	if(seekdi < 0)
		return NULL;

	int di = 0;
	for(auto diter=list.begin(); diter!=list.end(); diter++, di++)
	{
		if(di == seekdi)
			return *diter;
	}

	return NULL;
}

void LinkDems(std::list<DemNode*>& olist, std::list<DemNode*>& nlist, std::list<DemNode*>& osearch, std::list<DemNode*>& nsearch)
{
	for(auto oditer=olist.begin(); oditer!=olist.end(); oditer++)
	{
		DemNode* olddem = *oditer;
		int cdi = DemIndex(olddem, osearch);
		DemNode* newdem = DemAt(nsearch, cdi);
		nlist.push_back(newdem);
	}
}

//Link dem nodes
void LinkDemB(DemTree* orig, DemTree* copy)
{
#if 0	//DemNode
	DemNode* parent;
	Bid bid;	//reflects the maximum possible gouging price
	int profit;
#endif
#if 0
	int bi;
	int btype;
	std::list<RDemNode*> condems;	//construction material
	std::list<RDemNode*> proddems;	//production input raw materials
	std::list<RDemNode*> manufdems;	//manufacturing input raw materials
	std::list<CdDem*> cddems[CONDUIT_TYPES];
	int prodratio;
	int condem[RESOURCES];
	int supplying[RESOURCES];
#endif

	auto oditer=orig->supbpcopy.begin();
	auto cditer=copy->supbpcopy.begin();
	for(;
	oditer!=orig->supbpcopy.end() && cditer!=copy->supbpcopy.end();
	oditer++, cditer++)
	{
		DemsAtB* olddem = (DemsAtB*)*oditer;
		DemsAtB* newdem = (DemsAtB*)*cditer;

		LinkDems(olddem->condems, newdem->condems, orig->rdemcopy, copy->rdemcopy);
		LinkDems(olddem->proddems, newdem->proddems, orig->rdemcopy, copy->rdemcopy);
		LinkDems(olddem->manufdems, newdem->manufdems, orig->rdemcopy, copy->rdemcopy);

		for(int ctype=0; ctype<CONDUIT_TYPES; ctype++)
			LinkDems(olddem->cddems[ctype], newdem->cddems[ctype], orig->rdemcopy, copy->rdemcopy);
	}
}

//Link dem nodes
void LinkDemU(DemTree* orig, DemTree* copy)
{
#if 0	//DemNode
	DemNode* parent;
	Bid bid;	//reflects the maximum possible gouging price
	int profit;
#endif
#if 0
	int ui;
	int utype;
	std::list<RDemNode*> manufdems;
	std::list<RDemNode*> consumdems;
	DemsAtU* opup;	//operator/driver
	int prodratio;
	int timeused;
	int totaldem[RESOURCES];
#endif

#if 0
int DemIndex(DemNode* pdem, std::list<DemNode*>& list)
DemNode* DemAt(std::list<DemNode*>& list, int seekdi)
#endif

	auto oditer=orig->supupcopy.begin();
	auto cditer=copy->supupcopy.begin();
	for(;
	oditer!=orig->supupcopy.end() && cditer!=copy->supupcopy.end();
	oditer++, cditer++)
	{
		DemsAtU* olddem = (DemsAtU*)*oditer;
		DemsAtU* newdem = (DemsAtU*)*cditer;

		LinkDems(olddem->manufdems, newdem->manufdems, orig->rdemcopy, copy->rdemcopy);
		LinkDems(olddem->consumdems, newdem->consumdems, orig->rdemcopy, copy->rdemcopy);
		newdem->opup = DemAt(copy->supupcopy, DemIndex(olddem->opup, orig->supupcopy));
	}
}
//Link dem nodes
void LinkDemCo(DemTree* orig, DemTree* copy)
{
#if 0	//DemNode
	DemNode* parent;
	Bid bid;	//reflects the maximum possible gouging price
	int profit;
#endif
#if 0
	int cdtype;
	Vec2i tpos;
	std::list<RDemNode*> condems;
#endif
}

//Link dem nodes
void LinkDemR(DemTree* orig, DemTree* copy)
{
#if 0	//DemNode
	DemNode* parent;
	Bid bid;	//reflects the maximum possible gouging price
	int profit;
#endif
#if 0
	int rtype;
	int ramt;
	int btype;
	int bi;
	int utype;
	int ui;
	int demui;
	DemsAtB* supbp;
	DemsAtU* supup;
	DemsAtU* opup;
#endif
}

//Duplicate demtree
void DupDT(DemTree* orig, DemTree* copy)
{
#if 0
	std::list<DemNode*> nodes;
	std::list<DemsAtB*> supbpcopy;	//master copy, this one will be freed
	std::list<DemsAtU*> supupcopy;	//master copy, this one will be freed
	std::list<DemNode*> codems[CONDUIT_TYPES];	//conduit placements
	std::list<RDemNode*> rdemcopy;	//master copy, this one will be freed
	int pyrsup[PLAYERS][RESOURCES];	//player global res supplying
#endif

	DupDemB(orig, copy);
	DupDemU(orig, copy);
	DupDemCo(orig, copy);
	DupRDem(orig, copy);
	LinkDemB(orig, copy);
	LinkDemU(orig, copy);
	LinkDemCo(orig, copy);
	LinkDemR(orig, copy);

	for(int pi=0; pi<PLAYERS; pi++)
	{
		for(int ri=0; ri<RESOURCES; ri++)
		{
			copy->pyrsup[pi][ri] = orig->pyrsup[pi][ri];
		}
	}
}

/*
Hypothetical transport cost, create demand for insufficient transports
*/
void TranspCost(DemTree* dm, Player* p, Vec2i tfrom, Vec2i tto)
{

}

/*
Hypothetical costs of inputs and transport
Add calculated demands
Add infrastructure roads, powerlines, etc.
*/
#if 0
void CheckSups(DemTree* dm, Player* p, int rtype, int ramt, DemsAtB* demb, std::list<CostCompo>& costco)
{
	Resource* r = &g_resource[rtype];

	std::list<int> usedbi;

	int remain = ramt;

	while(remain > 0)
	{
		int bestbi = -1;
		int bestpr = -1;
		DemsAtB* bestsupb = NULL;
		int beststock = 0;
		int bestglob = 0;

		for(auto supbiter=dm->supbpcopy.begin(); supbiter!=dm->supbpcopy.end(); supbiter++)
		{
			DemsAtB* supb = (DemsAtB*)*supbiter;
			int bi = supb->bi;

			if(bi < 0)
				continue;

			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			//if(!b->finished)
			//	continue;

			BlType* bt = &g_bltype[b->type];

			if(bt->output[rtype] <= 0)
				continue;

			bool found = false;

			for(auto biter=usedbi.begin(); biter!=usedbi.end(); biter++)
			{
				biter++;

				if(*biter == bi)
				{
					found = true;
					break;
				}
			}

			if(found)
				continue;

			if(bestbi >= 0 && bestpr < b->prodprice[rtype])
				continue;

			int stockqty = bt->output[rtype] - supb->supplying[rtype];
			//int stockqty = b->stocked[rtype] - demb->supplying[rtype];

			int pi = b->owner;
			Player* p = &g_player[pi];
			int globqty = p->global[rtype] - dm->pyrsup[pi][rtype];

			if(stockqty + globqty < 0)
				continue;

			int transpcost = 0;

			if(r->physical)
			{

			}
			else
			{
			}

			bestbi = bi;
			bestpr = b->prodprice[rtype];
			bestsupb = supb;
			beststock = stockqty;
			bestglob = globqty;
		}

		if(bestbi < 0)
			break;

		int globconsum = imin(remain, bestglob);

		if(globconsum > 0)
		{
			remain -= globconsum;
			Building* b = &g_building[bestsupb->bi];
			int pi = b->owner;
			dm->pyrsup[pi][rtype] += globconsum;
		}

		int locconsum = imin(remain, beststock);

		if(locconsum > 0)
		{
			remain -= locconsum;
			bestdemb->supplying[rtype] += locconsum;
		}

		Building* b = &g_building[bestbi];

		CostCompo cc;
		cc.margcost = b->prodprice[rtype];
		cc.ramt = globconsum + locconsum;
	}
}
#endif

/*
Determine cost composition for proposed building production, create r dems
Also estimate transportation
Roads and infrastructure might not be present yet, create demand?
If not on same island, maybe create demand for overseas transport?
If no trucks are present, create demand?
*/
void CheckBlType(DemTree* dm, Player* p, int btype, int rtype, int ramt, Vec2i tpos, Bid* bid, int* blmaxr, bool* success, DemsAtB** retdemb)
{
	if(BuildingCollides(btype, tpos))
	{
		*success = false;
		return;
	}

	BlType* bt = &g_bltype[btype];

	int prodlevel = Ceili(RATIO_DENOM * ramt, bt->output[rtype]);

	if(prodlevel > RATIO_DENOM)
		prodlevel = RATIO_DENOM;

	if(ramt > bt->output[rtype])
		ramt = bt->output[rtype];

	*blmaxr = ramt;

	DemsAtB* demb = new DemsAtB;

	demb->parent = NULL;
	demb->prodratio = 0;
	Zero(demb->supplying);
	Zero(demb->condem);
	demb->bi = -1;
	demb->btype = btype;
	demb->prodratio = prodlevel;
	demb->supplying[rtype] = ramt;
	//demb->tilepos = tpos;
	bid->tpos = tpos;
	Vec2i cmpos = tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	bid->cmpos = cmpos;

	dm->supbpcopy.push_back(demb);

	std::list<CostCompo> rcostco[RESOURCES];

	*success = true;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		int reqr = Ceili(prodlevel * bt->input[ri], RATIO_DENOM);

		if(reqr <= 0)
			continue;

		//CheckSups(dm, p, ri, reqr, demb, rcostco[ri]);
		AddReq(dm, p, &demb->proddems, demb, ri, reqr, tpos, cmpos, 0, success);

		//if there's no way to build infrastructure to suppliers
		//then we shouldn't build this
		//but there might be no suppliers yet, so in that case let's say we can build it
		//if(!*success)
		//	return;
	}

	//combine the res compos into one costcompo

	std::list<CostCompo>::iterator rcoiter[RESOURCES];

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		rcoiter[ri] = rcostco[ri].begin();
	}

	int stepcounted[RESOURCES];
	Zero(stepcounted);

	int remain = ramt;

	// determine cost for building's output production
	while(remain > 0)
	{
		int prodstep[RESOURCES];	//production level based on stepleft and bl's output cap
		int stepleft[RESOURCES];	//how much is still left in this step of the list
		Zero(prodstep);
		Zero(stepleft);

		//determine how much is left in each step and the production level it would be equivalent to
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			auto& rco = rcoiter[ri];
			stepleft[ri] = rco->ramt - stepcounted[ri];
			prodstep[ri] = stepleft[ri] * RATIO_DENOM / bt->input[ri];
		}

		//find the lowest production level

		int minstepr = -1;

		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			if(minstepr < 0 || prodstep[ri] < prodstep[minstepr])
				minstepr = ri;
		}

		//if there's no such resource, there's something wrong
		if(minstepr < 0)
			break;

		//if we're at the end of the step, advance to next in list
		if(stepleft[minstepr] <= 0)
		{
			stepcounted[minstepr] = 0;

			auto& rco = rcoiter[minstepr];

			//if at end
			if(rco == rcostco[minstepr].end())
				break;

			rco++;

			//if at end
			if(rco == rcostco[minstepr].end())
				break;

			continue;
		}

		CostCompo nextco;
		nextco.fixcost = 0;
		nextco.transpcost = 0;

		//count fixed/transport cost if it hasn't already been counted
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			if(stepcounted[ri] > 0)
				continue;

			nextco.fixcost += rcoiter[ri]->fixcost;
			nextco.transpcost += rcoiter[ri]->transpcost;
		}

		int minstep = prodstep[minstepr];
		nextco.ramt = bt->output[rtype] * prodstep[minstepr] / RATIO_DENOM;
		nextco.margcost = 0;

		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			//int rstep = Ceili(minstep * bt->input[ri], RATIO_DENOM);
			int rstep = minstep * bt->input[ri] / RATIO_DENOM;
			rstep = imin(rstep, stepleft[ri]);
			stepcounted[ri] += rstep;
			nextco.margcost += rcoiter[ri]->margcost * rstep;
		}

		bid->costcompo.push_back(nextco);
	}

	demb->bid = *bid;

	*success = true;
	*retdemb = demb;

	// TO DO
}

//max profit
int MaxPro(std::list<CostCompo>& costco, int pricelevel, int demramt, int* proramt)
{
	int bestprofit = -1;
	int bestramt = 0;
	//*ramt = 0;

	// if we must begin from the beginning of costco, and end at any point after,
	//what is the maximum profit we can reach?
	//we must end at the certain point only, because later marginals might depend on
	//fixed cost of previous (of transport eg.)

	int bestlim = -1;
	int currlim = costco.size();

	while(currlim >= 1)
	{
		int curprofit = 0;
		int curramt = 0;
		int i = 0;
		for(auto citer=costco.begin(); citer!=costco.end() && i<currlim; citer++, i++)
		{
			int subprofit = pricelevel * citer->ramt - (citer->fixcost + citer->margcost * citer->ramt);

			curprofit += subprofit;
			curramt += citer->ramt;
		}

		if(bestlim < 0 || curprofit > bestprofit)
		{
			bestprofit = curprofit;
			bestramt = curramt;
			bestlim = currlim;
		}

		currlim--;
	}

	*proramt += bestramt;
	return bestprofit;
}

/*
Based on the dems in the tree, find the best price for this tile and res, if a profit can be generated
pt: tile info to return, including cost compo, bid, profit
*/
void CheckBlTile(DemTree* dm, Player* p, int ri, RDemNode* pt, int x, int z, int* fixc, int* recurp, bool* success)
{
	//list of r demands for this res type, with max revenue and costs for this tile
	std::list<DemNode*> rdems;	//RDemNode*
	int maxramt = 0;

	for(auto diter=dm->rdemcopy.begin(); diter!=dm->rdemcopy.end(); diter++)
	{
		DemNode* dn = *diter;

		if(dn->demtype != DEM_RNODE)
			continue;

		RDemNode* rdn = (RDemNode*)dn;

		if(rdn->rtype != ri)
			continue;

		//if(rdn->bi >= 0)
		//	continue;	//already supplied?
		//actually, no, we might be a competitor

		Resource* r = &g_resource[ri];

		int requtil = rdn->bid.minutil+1;

		if(requtil >= MAX_UTIL)
			continue;

		int cmdist = Magnitude(Vec2i(x,z)*TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2 - rdn->bid.cmpos);
		int maxpr = r->physical ? InvPhUtilP(requtil, cmdist) : InvGlUtilP(requtil);

		while((r->physical ? PhUtil(maxpr, cmdist) : GlUtil(maxpr)) < requtil)
			maxpr--;

		if(maxpr <= 0)
			continue;

		int maxrev = maxpr * rdn->ramt;

		if(rdn->bid.minutil < 0)
			maxrev = imin(maxrev, rdn->bid.maxbid);	//unecessary? might be necessary if minutil is -1 (any util) and there's only a budget constraint

		maxramt += rdn->ramt;

#if 0
		RDemNode proj;	//projected revenue
		proj.ramt = rdn->ramt;
		proj.bid.marginpr = maxpr;
		proj.bid.maxbid = maxrev;
		proj.bid.minbid = maxrev;
		//proj.btype = bestbtype;
		proj.dembi = rdn->dembi;
		proj.demui = rdn->demui;
		proj.bid.cmpos = rdn->
		rdems.push_back(proj);
#endif

		rdems.push_back(rdn);
	}

	if(rdems.size() <= 0)
		return;

	int bestbtype = -1;
	//int bestfix = -1;
	int bestmaxr = maxramt;
	int bestprofit = -1;
	DemsAtB* bestdemb = NULL;

	//TODO: variable cost of resource production and raw input transport
	//Try for all supporting bltypes
	for(int btype=0; btype<BUILDING_TYPES; btype++)
	{
		BlType* bt = &g_bltype[btype];

		if(bt->output[ri] <= 0)
			continue;

		DemTree bldm;
		DupDT(dm, &bldm);

		Bid bltybid;
		int blmaxr = maxramt;
		DemsAtB* demb = NULL;
		CheckBlType(&bldm, p, btype, ri, maxramt, Vec2i(x,z), &bltybid, &blmaxr, success, &demb);

		if(!*success)
			continue;

		//int bltyfix = 0;
		//int bltyrecur = 0;
		//int bestprc = -1;
		int prevprc = -1;
		bool dupdm = false;

		//evalute max projected revenue at tile and bltype
		//try all the price levels from smallest to greatest
		while(true)
		{
			int leastnext = prevprc;

			//while there's another possible price, see if it will generate more total profit

			for(auto diter=rdems.begin(); diter!=rdems.end(); diter++)
				if(((*diter)->bid.marginpr < leastnext && (*diter)->bid.marginpr > prevprc) || leastnext < 0)
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
				}

			//if demanded exceeds bl's max out
			if(demramt > blmaxr)
				demramt = blmaxr;

			int proramt = 0;	//how much is most profitable to produce in this case
			Bid* bid = &bltybid;

			//find max profit based on cost composition and price
			int curprofit = MaxPro(bid->costcompo, leastnext, demramt, &proramt);

			//int ofmax = Ceili(proramt * RATIO_DENOM, bestmaxr);	//how much of max demanded is
			//curprofit += ofmax * bestrecur / RATIO_DENOM;	//bl recurring costs, scaled to demanded qty

			if(curprofit <= bestprofit && bestbtype >= 0)
				continue;

			bestprofit = curprofit;
			*fixc = 0;	//TO DO: cost of building roads, infrast etc.
			*recurp = bestprofit;
			bestbtype = btype;
			bestmaxr = blmaxr;

			pt->bid.maxbid = bestprofit;
			pt->bid.marginpr = leastnext;
			pt->bid.tpos = Vec2i(x,z);
			pt->btype = bestbtype;
			pt->bid.costcompo = bltybid.costcompo;

			bestdemb = demb;

			dupdm = true;	//expensive op
		}

		if(!dupdm)
			continue;

		//TO DO: AddInf to all r dems of costcompo
		//need way to identify bl pos from rdem
		//and change rdem to specify supplied
        for(auto diter=rdems.begin(); diter!=rdems.end(); diter++)
        {
        	RDemNode* rdem = (RDemNode*)*diter;
            if(rdem->bid.marginpr < pt->bid.marginpr)
            	continue;

			DemNode* pardem = rdem->parent;

			if(!pardem)
				continue;



			AddInf(&bldm, bestdemb->cddems, bestdemb, rdem, ri, rdem->ramt, 0, success);
		}

		//add infrastructure to supplier
        //AddInf(dm, nodes, parent, *biter, rtype, ramt, depth, success);

		dm->free();
		DupDT(&bldm, dm);
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
void CheckBl(DemTree* dm, Player* p, int* fixcost, int* recurprof, bool* success)
{
	DemTree bestbldm;
	int bestfixc = -1;
	int bestrecurp = -1;
	*success = false;

	//For resources that must be transported physically
	//For non-physical res suppliers, distance
	//to clients doesn't matter, and only distance of
	//its suppliers and road+infrastructure costs
	//matter.
	for(int ri=0; ri<RESOURCES; ri++)
	{
		//Resource* r = &g_resource[ri];

		//if(!r->physical)
		//	continue;

		for(int z=0; z<g_hmap.m_widthz; z++)
			for(int x=0; x<g_hmap.m_widthx; x++)
			{
				DemTree thisdm;
				DupDT(dm, &thisdm);

				RDemNode tile;

				int recurp = 0;
				int fixc = 0;
				bool subsuccess;
				CheckBlTile(&thisdm, p, ri, &tile, x, z, &fixc, &recurp, &subsuccess);

				if(!subsuccess)
					continue;

				*success = subsuccess ? true : *success;

				if(bestrecurp < 0 || recurp > bestrecurp)
				{
					bestfixc = fixc;
					bestrecurp = recurp;
					bestbldm.free();
					DupDT(&thisdm, &bestbldm);
				}
			}

		// TODO ...

	}

	//TODO: ...

	//if no profit can be made
	if(bestrecurp <= 0)
		return;

	dm->free();
	DupDT(&bestbldm, dm);
	*fixcost = bestfixc;
	*recurprof = bestrecurp;
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

	dm->free();

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

	BlConReq(dm, p);

	// To do: inter-industry demand
	// TODO :...

	// TO DO: transport for constructions, for bl prods

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

	DemTree bldm;
	DupDT(dm, &bldm);
	int fixcost = 0;
	int recurprof = 0;
	bool success;
	CheckBl(&bldm, p, &fixcost, &recurprof, &success);	//check if there's any profitable building opp

	//if(recurprof > 0)
	if(success)
	{
		dm->free();
		DupDT(&bldm, dm);
	}

	//TO DO: build infrastructure demanded too
}
