#include "demand.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/sim.h"
#include "../sim/labourer.h"
#include "../utils.h"

DemTree g_demtree;

DemNode::DemNode()
{
	demtype = DEM_NODE;
	parent = NULL;
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

	if(r->physical)
	{
		std::list<RoadDem*> roads;
	}
	else
	{//offsetof(Building,roadnetw);

	}
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

	int remain = ramt;
	
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

		int suphere = imin(capleft, remain);
		remain -= suphere;
		
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

		if(remain <= 0)
			return;
	}

	if(remain <= 0)
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
		
		demb->prodratio = remain * RATIO_DENOM / bt->output[rtype];
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

		remain -= prodamt;
	}while(remain > 0);

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

void CalcDem()
{
	g_demtree.free();
	AddBl(&g_demtree);
	BlConReq(&g_demtree);

	int nlab = CountU(UNIT_LABOURER);
	
	AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_HOUSING, nlab, 0);
	AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_RETFOOD, LABOURER_FOODCONSUM * CYCLE_FRAMES, 0);
	AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_ENERGY, nlab * LABOURER_ENERGYCONSUM, 0);
}

