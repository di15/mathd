

#include "main.h"
#include "resource.h"
#include "gui.h"
#include "player.h"
#include "building.h"
#include "selection.h"
#include "job.h"
#include "image.h"
#include "numpad.h"
#include "chat.h"

CResource g_resource[RESOURCES];
int g_insufficient;
int g_resEd;

void RDefine(int res, const char* n, char ico, bool phys, bool cap, bool glob, float red, float green, float blue, float alpha)
{
	CResource* r = &g_resource[res];
	strcpy(r->name, n);
	r->icon = ico;
	r->physical = phys;
	r->capacity = cap;
	r->global = glob;
	r->rgba[0] = red;
	r->rgba[1] = green;
	r->rgba[2] = blue;
	r->rgba[3] = alpha;
}

void Zero(float *r)
{
	for(int i=0; i<RESOURCES; i++)
		r[i] = 0;
}

bool ResB(int building, int res)
{
	CBuilding* b = &g_building[building];
	CBuildingType* t = &g_buildingType[b->type];
	if(t->input[res] > 0.0f)
		return true;
	if(t->output[res] > 0.0f)
		return true;
	return false;
}

bool TrySubtract(float* cost, float* global, float* stock, float* local, float* netch)
{
	float combined[RESOURCES];

	if(netch != NULL)
		Zero(netch);
	
	for(int i=0; i<RESOURCES; i++)
	{
		if(i != LABOUR && !g_resource[i].capacity)
			combined[i] = global[i];
		else
			combined[i] = 0;

		if(stock != NULL)
			combined[i] += stock[i];
		
		if(cost[i] > combined[i])
		{
			g_insufficient = i;
			return false;
		}
	}

	for(int i=0; i<RESOURCES; i++)
	{
		if(i == LABOUR)
		{
			stock[i] -= cost[i];
			
			if(netch != NULL)
				netch[i] += cost[i];
		}
		else if(cost[i] > global[i])
		{
			float deducted = global[i];
			global[i] = 0;
			stock[i] -= (cost[i] - deducted);

			if(!g_resource[i].capacity)
				local[i] -= (cost[i] - deducted);
			
			if(netch != NULL)
				netch[i] += cost[i];
		}
		else
		{
			global[i] -= cost[i];
			
			if(netch != NULL)
				netch[i] += cost[i];
		}
	}

	return true;
}

void InitResources()
{
	//void RDefine(char* n, char ico, bool phys, bool cap, bool glob,			red, green, blue, alpha)
	RDefine(CURRENC,		"Currency",			0x1,	false,	false,	false,	0.5f,	1,		0.5f,	1.0f);
	RDefine(LABOUR,			"Labour",			0x2,	false,	false,	false,	0.6f,	0.2f,	0.2f,	1.0f);
	RDefine(HOUSING,		"Housing",			0xf,	false,	true,	false,	0.8f,	0.2f,	0.8f,	1.0f);
	RDefine(CONSUMERGOODS,	"Consumer Goods",	0x3,	true,	false,	false,	1,		0,		1,		1.0f);
	RDefine(PRODUCE,		"Produce",			0x4,	true,	false,	false,	0.6f,	0.8f,	0.2f,	1.0f);
	RDefine(PRODUCTION,		"Production",		0x5,	true,	false,	false,	0.7f,	0.6f,	0.3f,	1.0f);
	RDefine(ORE,			"Ore",				0x6,	true,	false,	false,	0.6f,	0.3f,	0.1f,	1.0f);
	RDefine(METAL,			"Metal",			0x7,	true,	false,	false,	0.7f,	0.7f,	0.7f,	1.0f);
	RDefine(CRUDE,			"Crude",			0x17,	false,	false,	false,	0.0f,	0.0f,	0.0f,	1.0f);	//0x8
	RDefine(ZETROL,			"Zetrol",			0x18,	true,	false,	false,	0.2f,	0.2f,	0.3f,	1.0f);	//0x14
	RDefine(STONE,			"Stone",			0x12,	true,	false,	false,	0.4f,	0.4f,	0.4f,	1.0f);
	RDefine(CEMENT,			"Cement",			0xb,	true,	false,	false,	0.6f,	0.6f,	0.6f,	1.0f);
	RDefine(ELECTRICITY,	"Electricity",		0xc,	false,	true,	false,	1.0f,	1.0f,	0.0f,	1.0f);
	RDefine(URONIUM,		"Uronium",			0x19,	true,	false,	false,	0.2f,	1.0f,	0.2f,	1.0f);	//0xd
	RDefine(COHL,			"Cohl",				0xe,	true,	false,	false,	0.3f,	0.3f,	0.4f,	1.0f);
	RDefine(CHEMICALS,		"Chemicals",		0x1a,	true,	false,	false,	0.7f,	0.2f,	0.75f,	1.0f);	//0x10
	RDefine(ELECTRONICS,	"Electronics",		0x11,	true,	false,	false,	0.7f,	0.7f,	2.0f,	1.0f);
	RDefine(RESEARCH,		"Research",			0x16,	true,	false,	true,	0.2f,	0.7f,	0.7f,	1.0f);
	//0xf - housing
	//0x11 - electronics
	//0x12 - stone (was 0xa - \n)
	//0x9 - horizontal tab
	//0x13 - carriage return
	//0x14 - zetrol
	//0x15 - time char
	//0x16 - research

	//0x17 - crude
	//0x18 - zetrol
	//0x19 - uronium
	//0x1a - chemicals
}

void ResourceTicker()
{
	string full("");
	static float tick = 0;
	CPlayer* p = &g_player[g_localP];
	char part[32];
	CResource* r;

	for(int i=0; i<RESOURCES; i++)
	{
		r = &g_resource[i];

		if(i == LABOUR)
			continue;

		if(i != CURRENC)
			continue;

		//if(i == LABOUR)
		//	full.append("Housing");
		//else
		//	full.append(g_resource[i].name);
		full += r->icon;
		full.append(": ");
		
		if(i == CURRENC)
		{
			//sprintf(part, "$%.2f", p->global[i]);
			
			if(p->reschange[i] >= 0.0f)
			{
				//sprintf(part, "$%.2f +%.2f/%c", p->global[i], p->reschange[i], TIME_CHAR);
				sprintf(part, "%.2f +%.2f/%c", p->global[i], p->reschange[i], TIME_CHAR);
			}
			else
			{
				//sprintf(part, "$%.2f %.2f/%c", p->global[i], p->reschange[i], TIME_CHAR);
				sprintf(part, "%.2f %.2f/%c", p->global[i], p->reschange[i], TIME_CHAR);
			}
		}/*
		else
		{
			if(r->capacity)
			{
				sprintf(part, "%d/%d", (int)p->occupied[i], (int)(p->local[i] + p->global[i]));
			}
			else
			{
				if(p->reschange[i] >= 0.0f)
				{
					sprintf(part, "%d +%d/%c", (int)(p->local[i] + p->global[i]), (int)p->reschange[i], TIME_CHAR);
				}
				else
				{
					sprintf(part, "%d %d/%c", (int)(p->local[i] + p->global[i]), (int)p->reschange[i], TIME_CHAR);
				}
			}
		}*/

		full.append(part);
		full.append("  ");
	}

	if(tick >= full.size())
		tick = 0;

	//g_GUI.getview("game")->getwidget("resource", TEXT)->text = full.substr(tick, full.size()-tick) + full.substr(0, tick);
	g_GUI.getview("game")->getwidget("resource", TEXT)->text = full;

	tick += 0.25f;
}

void WageAccept()
{
	CloseNumPad();
	
	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CPlayer* p = &g_player[b->owner];
	p->wage[b->type] = g_numPadVal;
	//RedoLeftPanel();
	NewJob(GOINGTONORMJOB, g_selection[0], -1);
}

void NumPadWage()
{
	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CPlayer* p = &g_player[b->owner];
	//Chat("numpadexpenses");
	char before[4];
	char after[8];
	//sprintf(before, "%c", g_resource[RESOURCE::CURRENC].icon);
	sprintf(before, "$");
	//sprintf(after, " / %c", g_resource[RESOURCE::LABOUR].icon);
	sprintf(after, "");
	OpenNumPad(&WageAccept, &CloseNumPad, p->wage[b->type], true, before, after);
}

void PlusWage()
{
	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CPlayer* p = &g_player[b->owner];
	p->wage[b->type] += 0.01f;
	//RedoLeftPanel();
	NewJob(GOINGTONORMJOB, g_selection[0], -1);
}

void MinusWage()
{
	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CPlayer* p = &g_player[b->owner];
	p->wage[b->type] -= 0.01f;
	if(p->wage[b->type] < 0.0f)
		p->wage[b->type] = 0.0f;
	//RedoLeftPanel();
	NewJob(GOINGTONORMJOB, g_selection[0], -1);
}

void PlusTruckWage()
{
	int i = g_selection[0];
	CPlayer* p = &g_player[g_localP];
	p->truckwage += 0.01f;
	//RedoLeftPanel();
	NewJob(GOINGTOTRUCK, g_selection[0], -1);
}

void MinusTruckWage()
{
	int i = g_selection[0];
	CPlayer* p = &g_player[g_localP];
	p->truckwage -= 0.01f;
	if(p->truckwage < 0.0f)
		p->truckwage = 0.0f;
	//RedoLeftPanel();
	NewJob(GOINGTOTRUCK, g_selection[0], -1);
}

void PriceAccept()
{
	CloseNumPad();
	
	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CPlayer* p = &g_player[b->owner];
	p->price[g_resEd] = g_numPadVal;

	//RedoLeftPanel();
}

void NumPadPrice(int r)
{
	g_resEd = r;
	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CPlayer* p = &g_player[b->owner];
	//Chat("numpadprice");
	char before[4];
	char after[8];
	//sprintf(before, "%c", g_resource[RESOURCE::CURRENC].icon);
	sprintf(before, "$");
	//sprintf(after, " / %c", g_resource[r].icon);
	sprintf(after, "");
	//sprintf(after, " / %c", g_resource[r].icon);
	OpenNumPad(&PriceAccept, &CloseNumPad, p->price[r], true, before, after);
}

void PlusPrice(int r)
{
	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CPlayer* p = &g_player[b->owner];
	p->price[r] += 0.01f;
	//RedoLeftPanel();
}

void MinusPrice(int r)
{
	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CPlayer* p = &g_player[b->owner];
	p->price[r] -= 0.01f;
	if(p->price[r] < 0.0f)
		p->price[r] = 0.0f;
	//RedoLeftPanel();
}

/*
i		f(i == CURRENC)
		{
			//sprintf(part, "$%.2f", p->global[i]);
			
			if(p->reschange[i] >= 0.0f)
			{
				sprintf(part, "$%.2f +%.2f/%c", p->global[i], p->reschange[i], TIME_CHAR);
			}
			else
			{
				sprintf(part, "$%.2f %.2f/%c", p->global[i], p->reschange[i], TIME_CHAR);
			}
		}
		else
		{
			if(r->capacity)
			{
				sprintf(part, "%d/%d", (int)p->occupied[i], (int)(p->local[i] + p->global[i]));
			}
			else
			{
				if(p->reschange[i] >= 0.0f)
				{
					sprintf(part, "%d +%d/%c", (int)(p->local[i] + p->global[i]), (int)p->reschange[i], TIME_CHAR);
				}
				else
				{
					sprintf(part, "%d %d/%c", (int)(p->local[i] + p->global[i]), (int)p->reschange[i], TIME_CHAR);
				}
			}
			*/

void RDelta(int player)
{
	float reschange[RESOURCES];
	float local[RESOURCES];
	float occupied[RESOURCES];
	
	Zero(reschange);
	Zero(local);
	Zero(occupied);

	CPlayer* p = &g_player[player];

	if(p->truckh.size() > 0)
	{
		CCycleH *ch = &p->truckh[ p->truckh.size()-1 ];
		
		reschange[CURRENC] += ch->produced[CURRENC];
		reschange[CURRENC] -= ch->consumed[CURRENC];

		// TO DO: might want to add resources purchased for construction or something
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		//if(!b->finished)
		//	continue;

		if(b->owner != player)
			continue;

		if(b->cycleh.size() > 0)
		{
			CCycleH *ch = &b->cycleh[ b->cycleh.size()-1 ];

			for(int j=0; j<RESOURCES; j++)
			{
				CResource* r = &g_resource[j];

				if(j == CURRENC)
				{
					reschange[CURRENC] += ch->produced[CURRENC];
					reschange[CURRENC] -= ch->consumed[CURRENC];
				}
				else if(r->capacity)
				{
					local[j] += ch->produced[j];
					occupied[j] += ch->consumed[j];
				}
				else
				{
					reschange[j] += ch->produced[j];
					reschange[j] -= ch->consumed[j];
				}
			}
		}
	}
	
	memcpy(p->reschange, reschange, sizeof(float)*RESOURCES);
	memcpy(p->local, local, sizeof(float)*RESOURCES);
	memcpy(p->occupied, occupied, sizeof(float)*RESOURCES);
}