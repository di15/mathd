#include "demand.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/sim.h"
#include "../sim/labourer.h"
#include "../utils.h"
#include "utility.h"
#include "../math/fixmath.h"
#include "../sim/build.h"
#include "../debug.h"

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

#define MAX_REQ		10

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
		if(!rdem) OutOfMem(__FILE__, __LINE__);
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
}

void AddBl(DemTree* dm)
{
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		DemsAtB* demb = new DemsAtB();
		if(!demb) OutOfMem(__FILE__, __LINE__);

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

void AddU(DemTree* dm, Unit* u, DemsAtU** retdemu)
{
#if 0
	int ui;
	int utype;
	std::list<DemNode*> manufdems;	// (RDemNode)
	std::list<DemNode*> consumdems;	// (RDemNode)
	DemNode* opup;	//operator/driver (DemsAtU)
	int prodratio;
	int timeused;
	int totaldem[RESOURCES];
#endif

	DemsAtU* demu = new DemsAtU;
	if(!demu) OutOfMem(__FILE__, __LINE__);

	demu->ui = u - g_unit;
	demu->utype = u->type;

	dm->supupcopy.push_back(demu);
	*retdemu = demu;
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
void LabDemH(DemTree* dm, Unit* u, int* fundsleft, DemsAtU* pardemu)
{
	if(*fundsleft <= 0)
		return;

	// Housing
	RDemNode* homedem = new RDemNode;
	if(!homedem) OutOfMem(__FILE__, __LINE__);
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
		//altbid->minutil = -1;	//any util

		int util = PhUtil(homepr, homedist);
		altbid->minutil = util;

		*fundsleft -= homepr;
	}
	else
	{
		// If there are alternatives/competitors
		DemsAtB* bestdemb = NULL;
		RDemNode bestr;
		bestr.bi = -1;
		int bestutil = -1;

		//for(int bi=0; bi<BUILDINGS; bi++)
		for(auto biter=dm->supbpcopy.begin(); biter!=dm->supbpcopy.end(); biter++)
		{
			DemsAtB* demb = (DemsAtB*)*biter;
			int bi = demb->bi;
			BlType* bt = NULL;
			Vec2i bcmpos;
			Vec2i btpos;
			int marginpr;

			if(bi < 0)
			{
				bt = &g_bltype[demb->btype];
				bcmpos = demb->bid.cmpos;
				btpos = demb->bid.tpos;
				//TO DO: this might not be for this output r
				marginpr = demb->bid.marginpr;
			}
			else
			{
				Building* b = &g_building[bi];

				if(!b->on)
					continue;

				//if(!b->finished)
				//	continue;

				bt = &g_bltype[b->type];
				bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
				btpos = b->tilepos;
				marginpr = b->prodprice[RES_HOUSING];
			}

			if(bt->output[RES_HOUSING] <= 0)
				continue;

			//NOTE: remember to change this back when labourers take up housing and
			//are constructed and ai's don't keep placing new ones
			//int stockqty = b->stocked[RES_HOUSING] - demb->supplying[RES_HOUSING];	//this is proper
			int stockqty = bt->output[RES_HOUSING] - demb->supplying[RES_HOUSING];	//temporary, not correct

			if(stockqty <= 0)
				continue;

#if 0
			char msg2[128];
			sprintf(msg2, "fundsleft%d margpr%d", *fundsleft, marginpr);
			InfoMessage("funds b", msg2);
#endif

			if(marginpr > *fundsleft)
				continue;

			int cmdist = Magnitude(bcmpos - u->cmpos);
			int thisutil = PhUtil(marginpr, cmdist);
#if 1
			char msg[128];
			sprintf(msg, "thisu%d margpr%d", thisutil, marginpr);
			InfoMessage("util b", msg);
#endif

			if(thisutil <= bestutil && bestutil >= 0)
				continue;

			bestutil = thisutil;

			bestr.bi = bi;
			bestr.btype = bt - g_bltype;
			bestr.rtype = RES_HOUSING;
			bestr.ramt = 1;
			bestr.bid.minbid = 1 * marginpr;
			bestr.bid.maxbid = 1 * marginpr;
			bestr.bid.tpos = btpos;
			bestr.bid.cmpos = bcmpos;
			bestr.bid.maxdist = cmdist;
			bestr.bid.minutil = thisutil;

			bestdemb = demb;
		}

		if(!bestdemb)
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
			InfoMessage("found bestdemb", "found bestdemb");

			// mark building consumption, so that other lab's consumption goes elsewhere
			bestdemb->supplying[RES_HOUSING] += bestr.ramt;

			//Just need to be as good or better than the last competitor.
			//Thought: but then that might not get all of the potential market.
			//So what to do?
			//Answer for now: it's probably not likely that more than one
			//shop will be required, so just get the last one.
			//Better answer: actually, the different "layers" can be
			//segmented, to create a demand for each with the associated
			//market/bid/cash.

			*homedem = bestr;

			//Don't subtract anything from fundsleft yet, purely speculative
		}
	}

	//homedem->bi = -1;
	homedem->supbp = NULL;
	homedem->btype = -1;
	homedem->parent = pardemu;
	homedem->rtype = RES_HOUSING;
	homedem->ramt = 1;
	homedem->ui = -1;
	homedem->utype = -1;
	homedem->demui = u - g_unit;
	//dm->nodes.push_back(homedem);
	dm->rdemcopy.push_back(homedem);
}

// Food demand, bare necessity
void LabDemF(DemTree* dm, Unit* u, int* fundsleft, DemsAtU* pardemu)
{
	// Food
	// Which shop will the labourer shop at?
	// There might be more than one, if
	// supplies are limited or if money is still
	// left over.
	std::list<RDemNode> alts;	//alternatives
	//int fundsleft = u->belongings[RES_FUNDS];
	int fundsleft2 = *fundsleft;	//fundsleft2 includes subtractions from speculative, non-existent food suppliers, which isn't supposed to be subtracted from real funds
	//int reqfood = CYCLE_FRAMES * LABOURER_FOODCONSUM;
	int reqfood = CYCLE_FRAMES/SIM_FRAME_RATE * LABOURER_FOODCONSUM;

	if(*fundsleft <= 0)
		return;

	bool changed = false;

	// If there are alternatives/competitors
	do
	{
		DemsAtB* bestdemb = NULL;
		RDemNode best;
		best.bi = -1;
		changed = false;
		int bestutil = -1;
		int bestaffordqty = 0;

		//for(int bi=0; bi<BUILDINGS; bi++)
		for(auto biter=dm->supbpcopy.begin(); biter!=dm->supbpcopy.end(); biter++)
		{
			DemsAtB* demb = (DemsAtB*)*biter;
			int bi = demb->bi;
			BlType* bt = NULL;
			Vec2i bcmpos;
			Vec2i btpos;
			int marginpr;

			if(bi < 0)
			{
				bt = &g_bltype[demb->btype];
				bcmpos = demb->bid.cmpos;
				btpos = demb->bid.tpos;
				//TO DO: this might not be for this r output
				marginpr = demb->bid.marginpr;
			}
			else
			{
				Building* b = &g_building[bi];

				if(!b->on)
					continue;

				//if(!b->finished)
				//	continue;

				bt = &g_bltype[b->type];
				bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
				btpos = b->tilepos;
				marginpr = b->prodprice[RES_RETFOOD];
			}

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

			//TO DO:
			//int stockqty = b->stocked[RES_RETFOOD] - demb->supplying[RES_RETFOOD];
			int stockqty = bt->output[RES_RETFOOD] - demb->supplying[RES_RETFOOD];

			if(stockqty <= 0)
				continue;

			int cmdist = Magnitude(bcmpos - u->cmpos);
			int thisutil = PhUtil(marginpr, cmdist);

			if(thisutil <= bestutil && bestutil >= 0)
				continue;

#if 0
			if(thisutil <= 0)
			{
				InfoMessage("blah", "thisutil <= 0");
			}
#endif

			bestutil = thisutil;

			int reqqty = imin(stockqty, reqfood);
			int affordqty = imin(reqqty, fundsleft2 / marginpr);
			stockqty -= affordqty;
			//reqfood -= affordqty;

			if(affordqty <= 0)
				continue;

			changed = true;

			bestaffordqty = affordqty;

			best.bi = bi;
			best.btype = bt - g_bltype;
			best.rtype = RES_RETFOOD;
			best.ramt = affordqty;
			int maxrev = imin(fundsleft2, affordqty * marginpr);
			best.bid.minbid = maxrev;
			best.bid.maxbid = maxrev;
			best.bid.tpos = btpos;
			best.bid.cmpos = bcmpos;
			best.bid.maxdist = cmdist;
			best.bid.minutil = thisutil;

			bestdemb = demb;
		}

		if(!bestdemb)
			break;

		reqfood -= bestaffordqty;

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
		if(!rdem) OutOfMem(__FILE__, __LINE__);
		*rdem = best;
		rdem->parent = pardemu;
		//dm->nodes.push_back(rdem);
		dm->rdemcopy.push_back(rdem);

#if 1
		//if(ri == RES_HOUSING)
		//if(ri == RES_RETFOOD)
		{
			int pi = -1;
			int ri = RES_RETFOOD;
			int bi = rdem->bi;
			char msg[512];
			int margpr = rdem->bid.marginpr;
			int cmdist = -1;

			if(bi >= 0)
			{
				Building* b = &g_building[bi];
				pi = b->owner;
				margpr = b->prodprice[ri];
			}
			
			sprintf(msg, "found food p%d %s b%d margpr%d minutil%d ramt%d cmdist%d", pi, g_resource[ri].name.c_str(), bi, margpr, rdem->bid.minutil, rdem->ramt, cmdist);
			InfoMessage("ff", msg);
		}
#endif

	} while(changed && fundsleft2 > 0);

	//I now see that fundsleft has to be subtracted too
	*fundsleft = fundsleft2;

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
		if(!demremain) OutOfMem(__FILE__, __LINE__);
		demremain->parent = pardemu;
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
		//dm->nodes.push_back(demremain);
		dm->rdemcopy.push_back(demremain);

		*fundsleft -= fundsleft2;
	}

	// If there is any money not spent on available food
	// Possible demand for more food (luxury) in LabDemF2();
}

// Food demand, luxury
void LabDemF2(DemTree* dm, Unit* u, int* fundsleft, DemsAtU* pardemu)
{
	// Food
	// Which shop will the labourer shop at?
	// There might be more than one, if
	// supplies are limited or if money is still
	// left over.
	std::list<RDemNode> alts;	//alternatives
	//int fundsleft = u->belongings[RES_FUNDS];
	int fundsleft2 = *fundsleft;	//fundsleft2 includes subtractions from speculative, non-existent food suppliers, which isn't supposed to be subtracted from real funds

	if(*fundsleft <= 0)
		return;

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
			BlType* bt = NULL;
			Vec2i bcmpos;
			Vec2i btpos;
			int marginpr;

			if(bi < 0)
			{
				bt = &g_bltype[demb->btype];
				bcmpos = demb->bid.cmpos;
				btpos = demb->bid.tpos;
				//TO DO: this might not be for this output r
				marginpr = demb->bid.marginpr;
			}
			else
			{
				Building* b = &g_building[bi];

				if(!b->on)
					continue;

				//if(!b->finished)
				//	continue;

				bt = &g_bltype[b->type];
				bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
				btpos = b->tilepos;
				marginpr = b->prodprice[RES_RETFOOD];
			}

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

			//TO DO:
			//int stockqty = b->stocked[RES_RETFOOD] - demb->supplying[RES_RETFOOD];
			int stockqty = bt->output[RES_RETFOOD] - demb->supplying[RES_RETFOOD];

			if(stockqty <= 0)
				continue;

			int cmdist = Magnitude(bcmpos - u->cmpos);
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
			best.btype = bt - g_bltype;
			best.rtype = RES_RETFOOD;
			best.ramt = luxuryqty;
			best.bid.minbid = 0;
			int maxrev = imin(fundsleft2, luxuryqty * marginpr);
			best.bid.maxbid = maxrev;
			best.bid.tpos = btpos;
			best.bid.cmpos = bcmpos;
			best.bid.maxdist = cmdist;
			best.bid.minutil = thisutil;

			bestdemb = demb;
		}

		if(!bestdemb)
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
		if(!rdem) OutOfMem(__FILE__, __LINE__);
		*rdem = best;
		rdem->parent = pardemu;
		//dm->nodes.push_back(rdem);
		dm->rdemcopy.push_back(rdem);

	} while(changed && fundsleft2 > 0);

	//I now see that fundsleft has to be subtracted too
	*fundsleft = fundsleft2;

	if(fundsleft2 <= 0)
		return;

	// If there is any money not spent on available food
	// Possible demand for more food (luxury) in LabDemF2();

	RDemNode* demremain = new RDemNode;
	if(!demremain) OutOfMem(__FILE__, __LINE__);
	demremain->parent = pardemu;
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
	//dm->nodes.push_back(demremain);
	dm->rdemcopy.push_back(demremain);

	*fundsleft -= fundsleft2;
}

// Electricity demand, bare necessity
void LabDemE(DemTree* dm, Unit* u, int* fundsleft, DemsAtU* pardemu)
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

	if(*fundsleft <= 0)
		return;

	bool changed = false;

	// If there are alternatives/competitors
	do
	{
		DemsAtB* bestdemb = NULL;
		RDemNode best;
		best.bi = -1;
		changed = false;
		int bestutil = -1;
		int bestaffordqty = 0;

		//for(int bi=0; bi<BUILDINGS; bi++)
		for(auto biter=dm->supbpcopy.begin(); biter!=dm->supbpcopy.end(); biter++)
		{
			DemsAtB* demb = (DemsAtB*)*biter;
			int bi = demb->bi;
			BlType* bt = NULL;
			Vec2i bcmpos;
			Vec2i btpos;
			int marginpr;

			if(bi < 0)
			{
				bt = &g_bltype[demb->btype];
				bcmpos = demb->bid.cmpos;
				btpos = demb->bid.tpos;
				//TO DO: this might not be for this output r
				marginpr = demb->bid.marginpr;
			}
			else
			{
				Building* b = &g_building[bi];

				if(!b->on)
					continue;

				//if(!b->finished)
				//	continue;

				bt = &g_bltype[b->type];
				bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
				btpos = b->tilepos;
				marginpr = b->prodprice[RES_ENERGY];
			}

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

			//TO DO:
			//int stockqty = b->stocked[RES_ENERGY] - demb->supplying[RES_ENERGY];
			int stockqty = bt->output[RES_ENERGY] - demb->supplying[RES_ENERGY];

			//int cmdist = Magnitude(b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2) - u->cmpos);
			int thisutil = GlUtil(marginpr);

			if(thisutil <= bestutil && bestutil >= 0)
				continue;

			bestutil = thisutil;

			int reqqty = imin(stockqty, reqelec);
			int affordqty = imin(reqqty, fundsleft2 / marginpr);
			stockqty -= affordqty;
			//reqelec -= affordqty;

			if(affordqty <= 0)
				continue;

			changed = true;

			bestaffordqty = affordqty;

			best.bi = bi;
			best.btype = bt - g_bltype;
			best.rtype = RES_ENERGY;
			best.ramt = affordqty;
			int maxrev = imin(fundsleft2, affordqty * marginpr);
			best.bid.minbid = maxrev;
			best.bid.maxbid = maxrev;
			best.bid.tpos = btpos;
			best.bid.cmpos = bcmpos;
			best.bid.maxdist = -1;	//any distance
			best.bid.minutil = thisutil;

			bestdemb = demb;
		}

		if(!bestdemb)
			break;

		reqelec -= bestaffordqty;

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
		if(!rdem) OutOfMem(__FILE__, __LINE__);
		*rdem = best;
		rdem->parent = pardemu;
		//dm->nodes.push_back(rdem);
		dm->rdemcopy.push_back(rdem);

	} while(changed && fundsleft2 > 0);

	
	//I now see that fundsleft has to be subtracted too
	*fundsleft = fundsleft2;

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
		if(!demremain) OutOfMem(__FILE__, __LINE__);
		demremain->parent = pardemu;
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
		//dm->nodes.push_back(demremain);
		dm->rdemcopy.push_back(demremain);

		*fundsleft -= fundsleft2;
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
		if(!newdem) OutOfMem(__FILE__, __LINE__);

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
		if(!newdem) OutOfMem(__FILE__, __LINE__);

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
			if(!newdem) OutOfMem(__FILE__, __LINE__);

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
		if(!newrdem) OutOfMem(__FILE__, __LINE__);

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

void LinkPar(DemTree* orig, DemTree* copy, DemNode* olddem, DemNode* newdem)
{
	if(!olddem->parent)
		return;

	int bi = DemIndex(olddem->parent, orig->supbpcopy);
	if(bi >= 0)
	{
		newdem->parent = DemAt(copy->supbpcopy, bi);
		return;
	}

	int ui = DemIndex(olddem->parent, orig->supupcopy);
	if(ui >= 0)
	{
		newdem->parent = DemAt(copy->supupcopy, ui);
		return;
	}

	int ri = DemIndex(olddem->parent, orig->rdemcopy);
	if(ri >= 0)
	{
		newdem->parent = DemAt(copy->rdemcopy, ri);
		return;
	}

	for(int ctype=0; ctype<CONDUIT_TYPES; ctype++)
	{
		int ci = DemIndex(olddem->parent, orig->codems[ctype]);
		if(ci >= 0)
		{
			newdem->parent = DemAt(copy->codems[ctype], ci);
			return;
		}
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

		LinkPar(orig, copy, olddem, newdem);
		LinkDems(olddem->condems, newdem->condems, orig->rdemcopy, copy->rdemcopy);
		LinkDems(olddem->proddems, newdem->proddems, orig->rdemcopy, copy->rdemcopy);
		LinkDems(olddem->manufdems, newdem->manufdems, orig->rdemcopy, copy->rdemcopy);

		for(int ctype=0; ctype<CONDUIT_TYPES; ctype++)
			LinkDems(olddem->cddems[ctype], newdem->cddems[ctype], orig->codems[ctype], copy->codems[ctype]);
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

		//g_log<<"oldcd"<<olddem->consumdems.size()<<" newcd"<<newdem->consumdems.size()<<std::endl;
		//g_log.flush();

		LinkPar(orig, copy, olddem, newdem);
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

	for(int ctype=0; ctype<CONDUIT_TYPES; ctype++)
	{
		auto oditer=orig->codems[ctype].begin();
		auto cditer=copy->codems[ctype].begin();
		for(;
						oditer!=orig->codems[ctype].end() && cditer!=copy->codems[ctype].end();
						oditer++, cditer++)
		{
			CdDem* olddem = (CdDem*)*oditer;
			CdDem* newdem = (CdDem*)*cditer;

			LinkPar(orig, copy, olddem, newdem);
			LinkDems(olddem->condems, newdem->condems, orig->rdemcopy, copy->rdemcopy);
		}
	}
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

	auto oditer=orig->rdemcopy.begin();
	auto cditer=copy->rdemcopy.begin();
	for(;
	                oditer!=orig->rdemcopy.end() && cditer!=copy->rdemcopy.end();
	                oditer++, cditer++)
	{
		RDemNode* olddem = (RDemNode*)*oditer;
		RDemNode* newdem = (RDemNode*)*cditer;

		LinkPar(orig, copy, olddem, newdem);
		newdem->supbp = (DemsAtB*)DemAt(copy->supbpcopy, DemIndex(olddem->supbp, orig->supbpcopy));
		newdem->supup = (DemsAtU*)DemAt(copy->supupcopy, DemIndex(olddem->supup, orig->supupcopy));
		newdem->opup = (DemsAtU*)DemAt(copy->supupcopy, DemIndex(olddem->opup, orig->supupcopy));
	}
}

void LinkNodes(DemTree* orig, DemTree* copy)
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
	std::list<RDemNode*> rdemcopy;	//master copy, this one will be freed
	int pyrsup[PLAYERS][RESOURCES];	//player global res supplying
#endif

	char msg[128];
	char msg2[128];

#if 0
	sprintf(msg, "orig %db %du %dr %dc", (int)orig->supbpcopy.size(), (int)orig->supupcopy.size(), (int)orig->rdemcopy.size(), (int)(orig->codems[0].size()+orig->codems[1].size()+orig->codems[2].size()));
	sprintf(msg2, "copy %db %du %dr %dc", (int)copy->supbpcopy.size(), (int)copy->supupcopy.size(), (int)copy->rdemcopy.size(), (int)(copy->codems[0].size()+copy->codems[1].size()+copy->codems[2].size()));
	g_log<<msg<<std::endl;
	g_log<<msg2<<std::endl;
	g_log.flush();
#endif

	DupDemB(orig, copy);
	DupDemU(orig, copy);
	DupDemCo(orig, copy);
	DupRDem(orig, copy);
	LinkDemB(orig, copy);
	LinkDemU(orig, copy);
	LinkDemCo(orig, copy);
	LinkDemR(orig, copy);
	LinkNodes(orig, copy);

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

//combine cost compositions of req input resources
//to give single costcompo list that shows how much it costs
//to produce output for any amount
void CombCo(int btype, Bid* bid, int rtype, int ramt)
{
	//combine the res compos into one costcompo

	std::list<CostCompo> rcostco[RESOURCES];
	std::list<CostCompo>::iterator rcoiter[RESOURCES];
	BlType* bt = &g_bltype[btype];

	//leave it like this for now
	//not counting fixed or transport costs
	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		//Each item in the list for resource type #ri indicates how much
		//that amount of the resource will cost to obtain.
		CostCompo rco;

		rco.fixcost = 0;
		rco.margcost = 0;
		rco.ramt = bt->input[ri];
		rco.transpcost = 0;

		rcostco[ri].push_back(rco);
	}

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		//Each item in the list for resource type #ri indicates how much
		//that amount of the resource will cost to obtain.
		rcoiter[ri] = rcostco[ri].begin();
	}

	int stepcounted[RESOURCES];
	Zero(stepcounted);

	int remain = ramt;

	// determine cost for building's output production
	while(remain > 0)
	{
#ifdef DEBUGDEM
		g_log<<"\t\t CheckBlType remain="<<remain<<std::endl;
		g_log.flush();
#endif

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

			//if at end
			if(rco == rcostco[ri].end())
				break;

			stepleft[ri] = rco->ramt - stepcounted[ri];

#ifdef DEBUGDEM
			g_log<<"\t\t\t ri="<<ri<<" stepleft("<<stepleft[ri]<<") = rco->ramt("<<rco->ramt<<") - stepcounted[ri]("<<stepcounted[ri]<<")"<<std::endl;
			g_log.flush();
#endif

			prodstep[ri] = stepleft[ri] * RATIO_DENOM / bt->input[ri];

#ifdef DEBUGDEM
			g_log<<"\t\t\t ri prodstep="<<prodstep[ri]<<" bt->input[ri]="<<bt->input[ri]<<std::endl;
			g_log.flush();
#endif
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

#ifdef DEBUGDEM
		g_log<<"\t\t minstepr="<<minstepr<<std::endl;
		g_log.flush();
#endif

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

#ifdef DEBUGDEM
		g_log<<"\t\t sub="<<nextco.ramt<<" prodstep[minstepr]="<<prodstep[minstepr]<<" bt->output[rtype]="<<bt->output[rtype]<<std::endl;
		g_log.flush();
#endif

		remain -= nextco.ramt;

		bid->costcompo.push_back(nextco);
	}
}

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
	if(!demb) OutOfMem(__FILE__, __LINE__);

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

	//TO DO: in the future, combco should figure out cost of transport and input resources
	CombCo(btype, bid, rtype, ramt);

	demb->bid = *bid;

	*success = true;
	*retdemb = demb;

	// TO DO
}

//max profit
bool MaxPro(std::list<CostCompo>& costco, int pricelevel, int demramt, int* proramt, int maxbudget, int* bestrev, int* bestprofit)
{
	//int bestprofit = -1;
	*bestprofit = -1;
	*bestrev = -1;
	int bestramt = 0;
	//*ramt = 0;

	// if we must begin from the beginning of costco, and end at any point after,
	//what is the maximum profit we can reach?
	//we must end at the certain point only, because later marginals might depend on
	//fixed cost of previous (of transport eg.)

	int bestlim = -1;
	int currlim = costco.size();

#ifdef DEBUGDEM
	g_log<<"\t\t costco.size()="<<costco.size()<<std::endl;
	g_log.flush();
#endif

	while(currlim >= 1)
	{
		int currev = 0;
		int curprofit = 0;
		int curramt = 0;
		int i = 0;
		for(auto citer=costco.begin(); citer!=costco.end() && i<currlim; citer++, i++)
		{
			int subprofit = pricelevel * citer->ramt - (citer->fixcost + citer->margcost * citer->ramt);
			int subrevenue = pricelevel * citer->ramt;

			if(currev + subrevenue >= maxbudget)
			{
				//budget left
				int budgleft = maxbudget - currev;
				int inchmore = budgleft / pricelevel;	//how much more ramt?
				
				subprofit = pricelevel * inchmore - (citer->fixcost + citer->margcost * citer->ramt);
				subrevenue = pricelevel * inchmore;

				currev += subrevenue;
				curprofit += subprofit;
				curramt += inchmore;
				break;
			}

			currev += subrevenue;
			curprofit += subprofit;
			curramt += citer->ramt;
		}

#ifdef DEBUGDEM
	g_log<<"\t\t\t currlim="<<currlim<<" curprofit="<<curprofit<<std::endl;
	g_log.flush();
#endif

		if(bestlim < 0 || curprofit > *bestprofit)
		{
			*bestprofit = curprofit;
			*bestrev = currev;
			bestramt = curramt;
			bestlim = currlim;
		}

		currlim--;
	}

	*proramt += bestramt;
	return *bestprofit > 0;
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
	int maxbudg = 0;	//max budget of consumer(s)
	Resource* r = &g_resource[ri];

	std::list<RDemNode*> rdns;

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
		//actually, it only matters if it's supplied 
		//by the same player, otherwise it's a competitor.
		if(rdn->bi >= 0)
		{
			Building* b = &g_building[rdn->bi];
			Player* p2 = &g_player[b->owner];

			//if it's owned by us, we don't want to compete with it.
			//it's better than to just adjust the price instead of wasting on another building.
			if(p2 == p)
				continue;
		}

		//int requtil = rdn->bid.minutil+1;
		int requtil = rdn->bid.minutil;

		if(requtil >= MAX_UTIL)
			continue;

		//int cmdist = Magnitude(Vec2i(x,z)*TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2 - rdn->bid.cmpos);

		Vec2i demcmpos;

		if(!DemCmPos(rdn->parent, &demcmpos))
			continue;

		int cmdist = Magnitude(Vec2i(x,z)*TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2 - demcmpos);
		int maxpr = 0;
		int maxrev = 0;
		
		//unecessary? might be necessary if minutil is -1 (any util) and there's only a budget constraint
		if(requtil < 0)
		{
			maxrev = rdn->bid.maxbid;
			maxpr = maxrev;
		}
		else
		{
			maxpr = r->physical ? InvPhUtilP(requtil, cmdist) : InvGlUtilP(requtil);
			
			if(maxpr <= 0)
				continue;

			while((r->physical ? PhUtil(maxpr, cmdist) : GlUtil(maxpr)) < requtil)
				maxpr--;
			
			if(maxpr <= 0)
				continue;

			maxrev = maxpr * rdn->ramt;
		}

#if 0
		char msg[128];
		sprintf(msg, "maxpr%d requtil%d", maxpr, requtil);
		InfoMessage("sea", msg);
#endif
		
#if 1
		//if(ri == RES_HOUSING)
		{
			g_log<<"dem "<<g_resource[ri].name<<" rdn->ramt="<<rdn->ramt<<" maxpr="<<maxpr<<" maxbid="<<rdn->bid.maxbid<<" rdn->bid.minutil="<<rdn->bid.minutil<<std::endl;
			g_log.flush();
		}
#endif

#if 1
		if(ri == RES_HOUSING)
		{
			g_log<<"housing maxpr="<<maxpr<<std::endl;
			g_log.flush();
		}
#endif

		if(maxpr <= 0)
			continue;

		maxramt += rdn->ramt;
		maxbudg += rdn->bid.maxbid;

		rdns.push_back(rdn);

#if 0
		if(totalmaxrev > 100)
		{
			g_log<<"totalmaxrev > 100 = "<<totalmaxrev<<" rdn->bid.maxbid="<<rdn->bid.maxbid<<std::endl;

			for(auto rditer=rdns.begin(); rditer!=rdns.end(); rditer++)
			{
				RDemNode* rd = *rditer;
				Vec2i pardempos;
				if(DemCmPos(rd->parent, &pardempos))
					g_log<<"\tpardempos = "<<pardempos.x<<","<<pardempos.y<<std::endl;
				else
					g_log<<"\t?pos"<<std::endl;
			}

			g_log.flush();
		}
#endif

		rdn->bid.marginpr = maxpr;
		rdn->bid.maxbid = maxrev;
		//rdn->bid.minbid = maxrev;

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
	DemTree bestbldm;

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

#ifdef DEBUGDEM
		g_log<<"\t zx "<<z<<","<<x<<" calling CheckBlType "<<bt->name<<std::endl;
		g_log.flush();
#endif

		CheckBlType(&bldm, p, btype, ri, maxramt, Vec2i(x,z), &bltybid, &blmaxr, success, &demb);

#ifdef DEBUGDEM
		g_log<<"\t zx "<<z<<","<<x<<" /fini calling CheckBlType"<<bt->name<<std::endl;
		g_log.flush();
#endif

		if(!*success)
			continue;

#ifdef DEBUGDEM
		g_log<<"\t zx "<<z<<","<<x<<" /fini calling CheckBlType success "<<bt->name<<std::endl;
		g_log.flush();
#endif

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
				if(leastnext < 0 || ((*diter)->bid.marginpr < leastnext && (*diter)->bid.marginpr > prevprc))
					leastnext = (*diter)->bid.marginpr;

#ifdef DEBUGDEM
			g_log<<"\t zx "<<z<<","<<x<<" try price "<<leastnext<<" from "<<prevprc<<std::endl;
			g_log.flush();
#endif

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

			int currev;
			int curprofit;
			//find max profit based on cost composition and price
			MaxPro(bid->costcompo, leastnext, demramt, &proramt, maxbudg, &currev, &curprofit);

			//int ofmax = Ceili(proramt * RATIO_DENOM, bestmaxr);	//how much of max demanded is
			//curprofit += ofmax * bestrecur / RATIO_DENOM;	//bl recurring costs, scaled to demanded qty

#ifdef DEBUGDEM
			g_log<<"\t\t\t curprofit = "<<curprofit<<" proramt="<<proramt<<" demramt="<<demramt<<std::endl;
			g_log.flush();
#endif

			//if(curprofit > maxbudg)
			//	curprofit = maxbudg;

			if(curprofit <= 0)
				continue;

			if(curprofit <= bestprofit && bestbtype >= 0)
				continue;

#ifdef DEBUGDEM
			g_log<<"\t\t\t profit success"<<std::endl;
			g_log.flush();
#endif

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

			demb->bid.marginpr = leastnext;
			demb->bid.maxbid = bestprofit;
			demb->bid.costcompo = bltybid.costcompo;

			dupdm = true;	//expensive op
		}

		if(!dupdm)
			continue;

		//if(*recurp > totalmaxrev)
		//	*recurp = totalmaxrev;

		*success = true;

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

			//NOTE: need to get equivalent rdem from copied DemTree, NOT original

			//AddInf(&bldm, bestdemb->cddems, bestdemb, rdem, ri, rdem->ramt, 0, success);
		}

		//add infrastructure to supplier
		//AddInf(dm, nodes, parent, *biter, rtype, ramt, depth, success);

		bestbldm.free();
		DupDT(&bldm, &bestbldm);
	}

	if(bestbtype < 0)
		return;

	dm->free();
	DupDT(&bestbldm, dm);
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
				char msg[128];
				sprintf(msg, "\t\t1\tstart %x,%d", x,z);
				CheckMem(__FILE__, __LINE__, "\t\t1\t");
#ifdef DEBUGDEM
				g_log<<"zx "<<z<<","<<x<<std::endl;
				g_log.flush();
#endif

#if 0
				char msg[1024];
				int cds = 0;
				for(int ctype=0; ctype<CONDUIT_TYPES; ctype++)
					cds += dm->codems[ctype].size();
				sprintf(msg, "r%d b%d u%d c%d", (int)dm->rdemcopy.size(), (int)dm->supbpcopy.size(), (int)dm->supupcopy.size(), cds);
				LastNum(msg);
#endif

				DemTree thisdm;
				DupDT(dm, &thisdm);

#ifdef DEBUGDEM
				g_log<<"zx "<<z<<","<<x<<" /fini dupdt"<<std::endl;
				g_log.flush();
#endif

				RDemNode tile;

				int recurp = 0;
				int fixc = 0;
				bool subsuccess;
				CheckBlTile(&thisdm, p, ri, &tile, x, z, &fixc, &recurp, &subsuccess);

#ifdef DEBUGDEM
				g_log<<"zx "<<z<<","<<x<<" /fini CheckBlTile"<<std::endl;
				g_log.flush();
#endif

				if(!subsuccess)
					continue;

				if((bestrecurp < 0 || recurp > bestrecurp) && recurp > 0)
				{
					*success = subsuccess ? true : *success;

#ifdef DEBUGDEM
					BlType* bt = &g_bltype[((DemsAtB*)(*thisdm.supbpcopy.rbegin()))->btype];
					g_log<<"zx "<<z<<","<<x<<" success dupdt"<<bt->name<<std::endl;
					g_log.flush();
#endif

					bestfixc = fixc;
					bestrecurp = recurp;
					CheckMem(__FILE__, __LINE__, "\t\t1\t");
					bestbldm.free();
					CheckMem(__FILE__, __LINE__, "\t\t\t2\t");
					DupDT(&thisdm, &bestbldm);

#ifdef DEBUGDEM
					g_log<<"zx "<<z<<","<<x<<" /fini success dupdt"<<std::endl;
					g_log.flush();
#endif
				}
				
				char msg2[128];
				sprintf(msg2, "\t\t1\tend%x,%d", x,z);
				CheckMem(__FILE__, __LINE__, "\t\t2\t");
			}

		// TODO ...

	}

	//TODO: ...

#ifdef DEBUGDEM
	g_log<<"bestrecurp = "<<bestrecurp<<std::endl;
	g_log.flush();
#endif

	//if no profit can be made
	if(bestrecurp <= 0)
		return;

	dm->free();
	DupDT(&bestbldm, dm);
	*fixcost = bestfixc;
	*recurprof = bestrecurp;
}

// given a parent of an rdem node, get its position
// for eg getting distance to demander from supplier bi
bool DemCmPos(DemNode* pardem, Vec2i* demcmpos)
{
	if(!pardem)
		return false;

	if(pardem->demtype == DEM_UNODE)
	{
		DemsAtU* demu = (DemsAtU*)pardem;

		if(demu->ui >= 0)
		{
			Unit* u = &g_unit[demu->ui];
			*demcmpos = u->cmpos;
			return true;
		}
		else
		{
			*demcmpos = demu->bid.cmpos;
			return true;
		}
	}

	if(pardem->demtype == DEM_BNODE)
	{
		DemsAtB* demb = (DemsAtB*)pardem;

		if(demb->bi >= 0)
		{
			Building* b = &g_building[demb->bi];
			*demcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
			return true;
		}
		else
		{
			*demcmpos = demb->bid.cmpos;
			return true;
		}
	}

	if(pardem->demtype == DEM_CDNODE)
	{
		CdDem* demcd = (CdDem*)pardem;
		ConduitType* ct = &g_cotype[demcd->cdtype];

		if(demcd->tpos.x >= 0 && demcd->tpos.y >= 0)
		{
			*demcmpos = demcd->tpos * TILE_SIZE + ct->physoff;
			return true;
		}
		else
		{
			*demcmpos = demcd->bid.cmpos;
			return true;
		}
	}

	return false;
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

	//return;

	// Point #2 - building for primary demands

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != UNIT_LABOURER)
			continue;

		int fundsleft = u->belongings[RES_FUNDS];

		DemsAtU* demu = NULL;

		AddU(dm, u, &demu);

		LabDemF(dm, u, &fundsleft, demu);
		LabDemH(dm, u, &fundsleft, demu);
		LabDemE(dm, u, &fundsleft, demu);
		LabDemF2(dm, u, &fundsleft, demu);
	}

	//return;

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

	//return;

	CheckMem(__FILE__, __LINE__, "1\t");
	CheckBl(&bldm, p, &fixcost, &recurprof, &success);	//check if there's any profitable building opp
	CheckMem(__FILE__, __LINE__, "\t2\t");

	//return;

	//if(recurprof > 0)
	if(success)
	{
#ifdef DEBUGDEM
		BlType* bt = &g_bltype[((DemsAtB*)(*bldm.supbpcopy.rbegin()))->btype];
		g_log<<"suc pi "<<pi<<" "<<bt->name<<std::endl;
		g_log.flush();
		//InfoMessage("suc", "suc");
#endif
		dm->free();
		DupDT(&bldm, dm);
	}
#ifdef DEBUGDEM
	else
	{
		g_log<<"fail pi "<<pi<<std::endl;
		g_log.flush();
		//InfoMessage("f", "f");
	}
#endif

	//TO DO: build infrastructure demanded too
}
