
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

int CalcOutlet(int rtype, int pyi)
{

}

int CalcSup(int rtype)
{
	int supply = 0;

	for(int i=0; i<PLAYERS; i++)
	{

	}

	for(int i=0; i<BUILDING_TYPES; i++)
	{
		BuildingT* t = &g_bltype[i];

		if(t->output[rtype] <= 0)
			continue;
	}
}

#define MAX_REQ		50

void AddReq(DemTree* dm, std::list<DemNode*>* nodes, DemNode* parent, int rtype, int ramt, int depth)
{
	if(depth > MAX_REQ)
		return;

	int remain = ramt;

	for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter++)
	{
		DemsAtB* demb = *biter;
		BuildingT* bt = &g_bltype[demb->btype];

		if(bt->output[rtype] <= 0)
			continue;

		int capleft = bt->output[rtype];
		capleft -= demb->supplying[rtype];

		if(capleft <= 0)
			continue;

		int suphere = imin(capleft, remain);
		remain -= suphere;

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
		demb->supplying[rtype] += suphere;

		if(newprodlevel > demb->prodratio)
		{
			int extraprodlev = newprodlevel - demb->prodratio;

			for(int ri=0; ri<RESOURCES; ri++)
			{
				if(bt->input[ri] <= 0)
					continue;

				int rreq = imax(1, extraprodlev * bt->input[ri] / RATIO_DENOM);

				AddReq(dm, &demb->proddems, demb, ri, rreq, depth+1);
			}
		}

		if(remain <= 0)
			return;
	}

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

		BuildingT* bt = &g_bltype[demb->btype];

		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->conmat[ri] <= 0)
				continue;

			AddReq(dm, &demb->condems, demb, ri, bt->conmat[ri], depth+1);
			demb->condem[ri] += bt->conmat[ri];
		}

		// TO DO: requisites for production, calc prodratio

		demb->prodratio = imin(RATIO_DENOM, remain * RATIO_DENOM / bt->output[rtype]);
		
		int prodamt = bt->output[rtype] * demb->prodratio / RATIO_DENOM;
		demb->supplying[rtype] += prodamt;
		
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

		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			int rreq = bt->input[ri] * demb->prodratio / RATIO_DENOM;
			AddReq(dm, &demb->proddems, demb, ri, rreq, depth+1);
		}

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

#define AVG_DIST		(TILE_SIZE*6)
#define CYCLE_FRAMES	(SIM_FRAME_RATE*60)	

void CalcDem()
{
	g_demtree.free();
	AddBl(&g_demtree);
	BlConReq(&g_demtree);

	int nlab = CountU(UNIT_LABOURER);

	AddReq(&g_demtree, &g_demtree.nodes, NULL, RES_RETFOOD, LABOURER_FOODCONSUM * CYCLE_FRAMES, 0);
}

