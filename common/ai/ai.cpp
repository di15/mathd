
#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"

void UpdateAI()
{
	CalcDem();

	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		if(!p->on)
			continue;

		if(!p->ai)
			continue;

		UpdateAI(p);
	}
}

void Build(Player* p)
{

}

void Manuf(Player* p)
{
}

void AdjustPrices(Player* p)
{
}

void UpdateAI(Player* p)
{
	Build(p);
	Manuf(p);
	AdjustPrices(p);
}