

#include "player.h"
#include "resource.h"
#include "building.h"
#include "ai.h"
#include "chat.h"
#include "gui.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "sound.h"

CPlayer g_player[PLAYERS];
int g_localP = 0;
bool g_diplomacy[PLAYERS][PLAYERS];
float g_pColor[PLAYERS][4];	//player colors
int g_selP = 0;

const char* activitystr[] =
{
	"none",
	"full",
	"nobuildb",
	"nobuildu",
	"nobuildbu"
};

const char* facnamestr[] =
{
	"Trigear",
	"GLeaf",
	"M-K",
	"Dios",
	"FStates",
	"B&B"
};

const char* facfullname[] =
{
	"Trigear Corp.",
	"Green Leaf Group",
	"Mendel-Kruppeson",
	"Dios Corp.",
	"The Free States",
	"Bits & Bytes Inc."
};

const char* faclogofile[] =
{
	"gui\\factions\\player.png",
	"gui\\factions\\greenleaf.png",
	"gui\\factions\\mendelkruppeson.png",
	"gui\\factions\\dios.png",
	"gui\\factions\\freestates.png",
	"gui\\factions\\bitsbytes.png"
};

const float facowncolor[][4] =
{
	{0, 0, 1, 1},
	{0, 1, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 1, 1},
	{0, 1, 1, 1},
	{1, 1, 0, 1}
};

void MakePeace(int a, int b)
{
	if(!g_diplomacy[a][b])
	{
		g_diplomacy[a][b] = true;
		g_diplomacy[b][a] = true;

		char msg[128];
		//sprintf(msg, "%s and %s are at peace.", facfullname[a], facfullname[b]);
		sprintf(msg, "%s and %s are at peace.", facnamestr[a], facnamestr[b]);
		Chat(msg);
	}
}

void MakeWar(int a, int b)
{
	if(g_diplomacy[a][b])
	{
		g_diplomacy[a][b] = false;
		g_diplomacy[b][a] = false;
		
		char msg[128];
		//sprintf(msg, "%s and %s are at war.", facfullname[a], facfullname[b]);
		sprintf(msg, "%s and %s are at war.", facnamestr[a], facnamestr[b]);
		Chat(msg);

		if(a == g_localP || b == g_localP)
			WeAUA();
	}
}

void PlayerColor(int p, float r, float g, float b, float a)
{
	g_pColor[p][0] = r;
	g_pColor[p][1] = g;
	g_pColor[p][2] = b;
	g_pColor[p][3] = a;
}

void InitPlayers()
{
	PlayerColor(0,	0, 0, 1, 1);
	PlayerColor(1,	1, 0, 0, 1);
}

void UpdatePlayers()
{
	//if(rand()%5 == 1 && g_player[g_localP].activity != ACTIVITY_NONE)
	{
	//	GameOver("kajsdlkalksd", g_localP);
	}

	for(int i=0; i<PLAYERS; i++)
	{
		CPlayer* p = &g_player[i];
		/*
		if(i == g_localP)
		{
			if(p->truckh.size() > 0)
			{
				CCycleH *ch = &p->truckh[ p->truckh.size()-1 ];
		
				if(ch->produced[CURRENC] > 0)
					Chat("truck +");
			}
		}*/

		if(i != g_localP && p->activity == ACTIVITY_NONE)
			continue;

		p->cycleframes ++;

		if(p->cycleframes >= CYCLE_FRAMES)
		{
			AdjustPQuotas(i);
			p->cycleframes = 0;
			p->truckh.push_back(p->recentth);
			p->recentth.reset();
			RDelta(i);
		}
	}
}

void TempPlayers()
{
	float uprice[UNIT_TYPES];

	for(int i=0; i<UNIT_TYPES; i++)
	{
		uprice[i] = 0;
		CUnitType* ut = &g_unitType[i];

		for(int j=0; j<RESOURCES; j++)
		{
			//float multiplier = 0.01f;
			float multiplier = 0.10f;

			if(j == LABOUR)
				//multiplier = 0.02f;
				multiplier = 0.20f;

			uprice[i] += ut->cost[j] * multiplier;
		}
		
		//uprice[i] += 0.1f;
		uprice[i] += 1.0f;
	}
	
	//uprice[CARLYLE] += 0.05f;
	//uprice[MECH] += 0.1f;
	uprice[CARLYLE] += 0.5f;
	uprice[MECH] += 1.0f;
	
	CPlayer* p;
	for(int i=0; i<PLAYERS; i++)
	{
		p = &g_player[i];

		p->conwage = 0.05f;
		p->truckwage = 0.05f;
		for(int j=0; j<RESOURCES; j++)
			//p->price[j] = 0.01f;
			p->price[j] = 0.20f;
		for(int j=0; j<BUILDING_TYPES; j++)
			//p->wage[j] = 0.02f;
			p->wage[j] = 0.10f;
	
		memcpy(p->uprice, uprice, sizeof(float)*UNIT_TYPES);
	}
}

void LogTransx(const int player, const float moneych, const char* what)
{
	return;
	char msg[128];
	if(moneych > 0.0f)
		sprintf(msg, "%s +%0.2f/%0.2f %s", facnamestr[player], moneych, g_player[player].global[CURRENC], what);
	else
		sprintf(msg, "%s %0.2f/%0.2f %s", facnamestr[player], moneych, g_player[player].global[CURRENC], what);
	Chat(msg);
	g_log<<msg<<endl;
	g_log.flush();
}

void ResetPlayers()
{
	float uprice[UNIT_TYPES];

	for(int i=0; i<UNIT_TYPES; i++)
	{
		uprice[i] = 0;
		CUnitType* ut = &g_unitType[i];

		for(int j=0; j<RESOURCES; j++)
		{
			//float multiplier = 0.01f;
			float multiplier = 0.10f;

			if(j == LABOUR)
				//multiplier = 0.02f;
				multiplier = 0.20f;

			uprice[i] += ut->cost[j] * multiplier;
		}
		
		//uprice[i] += 0.1f;
		uprice[i] += 1.0f;
	}
	
	//uprice[CARLYLE] += 0.05f;
	//uprice[MECH] += 0.1f;
	uprice[CARLYLE] += 0.5f;
	uprice[MECH] += 1.0f;

	g_localP = 0;

	CPlayer* p;
	for(int i=0; i<PLAYERS; i++)
	{
		p = &g_player[i];
		p->on = false;
		p->username = "";
		Zero(p->global);
		Zero(p->local);
		for(int j=0; j<RESOURCES; j++)
		{
			p->reschange[j] = 0;

			if(g_resource[j].capacity)
				continue;

			p->global[j] = 100;
		}
		//g_player[i].global[LABOUR] = 0;
		//p->global[ORE] = 0;
		Zero(p->reschange);
		Zero(p->occupied);
		p->conwage = 0.05f;
		p->truckwage = 0.01f;
		p->transportprice = 0.02f;
		for(int j=0; j<RESOURCES; j++)
			//p->price[j] = 0.01f;
			p->price[j] = 0.20f;
		p->price[CEMENT] = 0.01f;
		p->price[STONE] = 0.01f;
		p->price[CRUDE] = 0.01f;
		p->price[ZETROL] = 0.01f;
		p->price[HOUSING] = 0.08f;
		for(int j=0; j<BUILDING_TYPES; j++)
			//p->wage[j] = 0.02f;
			p->wage[j] = 0.10f;
		p->wage[REACTOR] = 0.02f;

		for(int j=0; j<PLAYERS; j++)
		{
			//MakePeace(i, j);
			g_diplomacy[i][j] = true;
			g_diplomacy[j][i] = true;
		}

		if(i == g_localP)
			p->ai = false;
		else
			p->ai = true;

		p->activity = ACTIVITY_FULL;
		p->bbframesago = BUILDSTUFF_DELAY;
		p->buframesago = BUILDUNITS_DELAY;

		for(int j=0; j<RESOURCES; j++)
		{
			p->priceh[j].clear();
			p->recentph[j].earnings = 0;
			p->recentph[j].expenditures = 0;
			p->recentph[j].price = p->price[j];
		}

		p->truckh.clear();
		p->recentth.reset();

		p->priceadjframes = 0;
		p->cycleframes = 0;

		memcpy(p->uprice, uprice, sizeof(float)*UNIT_TYPES);
	}

	//MakeWar(0, 1);
}

void Victory()
{
	OpenAnotherView("victory");
}

void CheckVictory()
{
	for(int i=0; i<PLAYERS; i++)
	{
		if(i == g_localP)
			continue;

		if(g_player[i].activity != ACTIVITY_NONE)
			return;
	}

	Victory();
}

void GameOver(const char* reason, int player)
{
	CPlayer* p = &g_player[player];

	if(p->activity == ACTIVITY_NONE)
		return;

	p->activity = ACTIVITY_NONE;

	if(player != g_localP)
	{
		for(int i=0; i<BUILDINGS; i++)
		{
			CBuilding* b = &g_building[i];

			if(!b->on)
				continue;

			for(int j=0; j<b->produ.size(); j++)
			{
				ManufJob* mj = &b->produ[j];

				if(mj->owner == player)
				{
					//b->produ.erase( b->produ.begin() + j );
					b->DequeueU(mj->utype, mj->owner);
					j--;
				}
			}

			if(b->owner == player)
			{
				b->forsale = true;
				b->saleprice = 0.00f;
			}
		}

		for(int i=0; i<UNITS; i++)
		{
			CUnit* u = &g_unit[i];

			if(!u->on)
				continue;

			if(u->type == LABOURER)
				continue;

			if(u->owner != player)
				continue;

			if(u->type == TRUCK)
			{
				for(int j=PLAYERS; j>=0; j--)
				{
					if(g_player[j].activity != ACTIVITY_NONE)
					{
						u->owner = j;
						break;
					}
				}
			}
			else
				u->on = false;
		}

		for(int x=0; x<g_hmap.m_widthX; x++)
			for(int z=0; z<g_hmap.m_widthZ; z++)
			{
				CRoad* road = RoadAt(x, z);

				if(road->on && !road->finished && road->owner == player)
				{
					//road->on = false;
					road->destroy();
					TypeRoadsAround(x, z);
				}

				CPowerline* powl = PowlAt(x, z);

				if(powl->on && !powl->finished && powl->owner == player)
				{
					//powl->on = false;
					powl->destroy();
					TypePowerlinesAround(x, z);
				}

				CPipeline* pipe = PipeAt(x, z);

				if(pipe->on && !pipe->finished && pipe->owner == player)
				{
					//pipe->on = false;
					pipe->destroy();
					TypePipelinesAround(x, z);
				}
			}

		CheckVictory();
	}

	if(player == g_localP)
	{
		LastNum("gameover 1");
		g_gameover = true;
		OpenAnotherView("game over");
		CView* v = g_GUI.getview("game over");
		Widget* w = v->getwidget("reason", TEXT);
		w->changetext(reason);
	}
}

void Bankrupt(int player, const char* reason)
{
	CPlayer* p = &g_player[player];

	if(player == g_localP)
	{
		GameOver("You've gone bankrupt", player);
	}
	else if(p->activity != ACTIVITY_NONE)
	{
		LogTransx(player, 0.0f, "BANKRUPT");

		char msg[128];
		sprintf(msg, "%s has gone bankrupt", facnamestr[player]);

		char add[64];

		if(reason[0] != '\0')
			sprintf(add, " (reason: %s).", reason);
		else
			sprintf(add, ".");

		strcat(msg, add);

		Chat(msg);
		GameOver("", player);
	}
}

void CheckGameOver(int who)
{
	CUnit* u;

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hp <= 0.0f)
			continue;

		if(u->owner != who)
			continue;

		return;
	}

	CBuilding* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(b->hp <= 0.0f)
			continue;

		if(b->owner != who)
			continue;

		return;
	}

	GameOver("All units and buildings destroyed", who);
}
