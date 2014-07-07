
#include "labourer.h"
#include "../platform.h"
#include "../econ/demand.h"
#include "unittype.h"
#include "player.h"

#if 0
void UpdFoodDem(Unit* u)
{
	Demand* foodd = NULL;

	for(auto diter = u->demands.begin(); diter != u->demands.end(); diter++)
	{
		Demand* d = *diter;

		if(d->type == DEMAND_FORRES && d->subtype == RES_RETFOOD)
		{
			foodd = d;
			break;
		}
	}

	if(!foodd)
	{
		Demand d;
		d.type = DEMAND_FORRES;
		d.subtype = RES_RETFOOD;
		d.alt = false;
		g_demand.push_back(d);
		foodd = &*g_demand.rbegin();
	}

	UnitT* ut = &g_utype[u->type];
	unsigned char curry = g_player[u->owner].currencyres;

	foodd->cmpos = u->cmpos;
	foodd->cmdist = (u->belongings[RES_RETFOOD] - 1) / ut->cmspeed;
	foodd->askprice = u->belongings[curry] / 3;
	foodd->askcurry = curry;
	//foodd->alt = false;
}

void UpdHousDem(Unit* u)
{
	Demand* housd = NULL;

	for(auto diter = u->demands.begin(); diter != u->demands.end(); diter++)
	{
		Demand* d = *diter;

		if(d->type == DEMAND_FORRES && d->subtype == RES_HOUSING)
		{
			housd = d;
			break;
		}
	}

	if(!housd)
	{
		Demand d;
		d.type = DEMAND_FORRES;
		d.subtype = RES_HOUSING;
		d.subtype2 = HOUSING_NONSPECIFIC;
		d.alt = false;
		g_demand.push_back(d);
		housd = &*g_demand.rbegin();
	}

	UnitT* ut = &g_utype[u->type];
	unsigned char curry = g_player[u->owner].currencyres;

	housd->cmpos = u->cmpos;
	housd->cmdist = (STARTING_LABOUR - u->belongings[RES_LABOUR]) / ut->cmspeed;
	housd->askprice = u->belongings[curry] / 3;
	housd->askcurry = curry;
}

void UpdateLabourer(Unit* u)
{
	UpdFoodDem(u);
	UpdHousDem(u);
}

#endif

void UpdateLabourer(Unit* u)
{

}
