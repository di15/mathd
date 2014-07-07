
#include "demand.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/sim.h"
#include "../sim/labourer.h"
#include "../utils.h"

int g_rdem[RESOURCES];	//resource demands
int g_udem[UNIT_TYPES];	//unit demands
int g_bdem[BUILDING_TYPES];	//building demands

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

#define MAX_REQ		1000

void AddReq(int rtype, int reqamount, int depth)
{
	if(depth > MAX_REQ)
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
	int reqnb = max(1, Ceili(reqamount, t->output[rtype]));

	for(int i=0; i<RESOURCES; i++)
	{
		int reqconmat = (t->conmat[i] + t->input[i]) * reqnb;
		int sup = CalcSup(i);
		sup -= g_rdem[i];


		if(reqconmat > 0)
			AddReq(i, reqconmat, depth+1);
	}


}

#define AVG_DIST		(TILE_SIZE*6)
#define CYCLE_FRAMES	(SIM_FRAME_RATE*60)	

void CalcDem()
{
	Zero(g_rdem);
	for(int i=0; i<UNIT_TYPES; i++)
		g_udem[i] = 0;
	for(int i=0; i<BUILDING_TYPES; i++)
		g_bdem[i] = 0;

	int nlab = CountU(UNIT_LABOURER);

	g_rdem[RES_RETFOOD] += LABOURER_FOODCONSUM * CYCLE_FRAMES;

	AddReq(RES_RETFOOD, LABOURER_FOODCONSUM * CYCLE_FRAMES, 0);
}

