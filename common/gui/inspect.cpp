

#include "main.h"
#include "gui.h"
#include "menu.h"
#include "font.h"
#include "resource.h"
#include "building.h"
#include "unit.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "player.h"
#include "selection.h"
#include "script.h"
#include "chat.h"
#include "inspect.h"
#include "transaction.h"
#include "bgraph.h"

void AddGrayBGFr(vector<Widget> *wv, int left, int top, int right, int bottom)
{
	int midheight = bottom - top - 32 - 23;

	Widget image;

	image.Image("gui\\grayframes\\grayfbg.png",				left + 23,						top + 17,					right - 21,					bottom - 18,					1,1,1,1,		0, 0, ((right-21)-(left+23))/46.0f, ((bottom-18)-(top+17))/46.0f);
	wv->push_back(image);

	image.Image("gui\\grayframes\\grayftopleft.png",			left,							top,						left + 21 + 1-1,			top + 17 + 1-1,					1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\grayframes\\grayftop.png",				left + 21,						top,						right - 20,					top + 17 + 1-1,					1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\grayframes\\grayftopright.png",			right - 20,						top,						right,						top + 17 + 1-1,					1,1,1,1);
	wv->push_back(image);
	
	image.Image("gui\\grayframes\\grayfleft.png",				left,							top + 17,					left + 23,					bottom - 18,					1,1,1,1,		0, 0, 1, ((bottom-18)-(top+17))/98.0f);
	wv->push_back(image);
	
	image.Image("gui\\grayframes\\grayfright.png",				right - 21,						top + 17,					right,						bottom - 18,					1,1,1,1,		0, 0, 1, ((bottom-18)-(top+17))/80.0f);
	wv->push_back(image);
	
	image.Image("gui\\grayframes\\grayfbottomleft.png",		left,							bottom - 18 - 1+1,			left + 22,					bottom,							1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\grayframes\\grayfbottom.png",			left + 22,						bottom - 18 - 1+1,			right - 26,					bottom,							1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\grayframes\\grayfbottomright.png",		right - 26,						bottom - 18 - 1+1,			right,						bottom,							1,1,1,1);
	wv->push_back(image);
}

void AddFrame(vector<Widget> *wv, int left, int top, int right, int bottom, int leftoff, int rightoff, int titlewidth, int undertextw)
{
	int midheight = bottom - top - 60 - 52;
	
	Widget image;

	image.Image("gui\\frames\\topleft.png",			left - leftoff,						top,						left + 29 - leftoff + 1-1,			top + 52 + 1-1,					1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\topleftraising.png",		left + 29 - leftoff,				top,						left + 29 + 31 - leftoff,			top + 52 + 1-1,					1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\topraised.png",			left + 29 + 31 - leftoff,			top,						right - 34 - 35 + rightoff,			top + 52 + 1-1,					1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\toprightlowering.png",	right - 34 - 35 + rightoff,			top,						right - 34 + rightoff,				top + 52 + 1-1,					1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\topright.png",			right - 34 + rightoff,				top,						right + rightoff,					top + 52 + 1-1,					1,1,1,1);
	wv->push_back(image);
	
	image.Image("gui\\frames\\leftleftborder.png",		left - leftoff,						top + 52,					left + 14 - leftoff,				bottom - 60,					1,1,1,1,		0, 0, 1, ((bottom-60)-(top+52))/170.0f);
	wv->push_back(image);
	image.Image("gui\\frames\\lefttopborder.png",		left + 14 - leftoff,				top + 52,					left + 14 + 16 - leftoff,			bottom - 60 - midheight/2,		1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\leftbottomborder.png",	left + 14 - leftoff,				bottom - 60 - midheight/2,	left + 14 + 16 - leftoff,			bottom - 60,					1,1,1,1);
	wv->push_back(image);
	
	image.Image("gui\\frames\\rightrightborder.png",	right - 20 + rightoff,				top + 52,					right + rightoff,					bottom - 60,					1,1,1,1,		0, 0, 1, ((bottom-60)-(top+52))/170.0f);
	wv->push_back(image);
	image.Image("gui\\frames\\righttopborder.png",		right - 20 - 17 + rightoff,			top + 52,					right - 20 + rightoff,				top + 52 + midheight/2,			1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\rightbottomborder.png",	right - 20 - 17 + rightoff,			top + 52 + midheight/2,		right - 20 + rightoff,				bottom - 60,					1,1,1,1);
	wv->push_back(image);

	image.Image("gui\\frames\\bottomleftcorner.png",	left - leftoff,						bottom - 60 - 1+1,			left + 35 - leftoff,				bottom,							1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\bottomleftlowering.png", left + 35 - leftoff,					bottom - 60 - 1+1,			left + 35 + 32 - leftoff,			bottom,							1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\bottomlowered.png",		left + 35 + 32 - leftoff,			bottom - 60 - 1+1,			right - 35 - 30 + rightoff,			bottom,							1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\bottomrightraising.png",	right - 35 - 30 + rightoff,			bottom - 60 - 1+1,			right - 35 + rightoff,				bottom,							1,1,1,1);
	wv->push_back(image);
	image.Image("gui\\frames\\bottomrightcorner.png",	right - 35 + rightoff,				bottom - 60 - 1+1,			right + rightoff,					bottom,							1,1,1,1);
	wv->push_back(image);
}

void RePrice(int res)
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	CResource* r = &g_resource[res];
	CPlayer* p = &g_player[b->owner];
	float pricef = p->price[res];
	
	CView* v = g_GUI.getview("binspect");
	Widget* mw = &v->widget[1];

	char textname[32];
	sprintf(textname, "%s price", r->name);
	Widget* w = mw->getsubwidg(textname, TEXTBOX);

	char pricestr[32];
	sprintf(pricestr, "%c%0.2f / %c", g_resource[CURRENC].icon, p->price[res], g_resource[res].icon);
	w->changetext(pricestr);
}

void ReUPrice(int unit)
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	CUnitType* t = &g_unitType[unit];
	CPlayer* p = &g_player[b->owner];
	float pricef = p->uprice[unit];
	
	CView* v = g_GUI.getview("binspect");
	Widget* mw = &v->widget[1];

	char textname[32];
	sprintf(textname, "%s price", t->name);
	Widget* w = mw->getsubwidg(textname, TEXTBOX);

	char pricestr[32];
	sprintf(pricestr, "%c%0.2f", g_resource[CURRENC].icon, p->uprice[unit]);
	w->changetext(pricestr);
}

void Click_PlusPrice(int res)
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	CResource* r = &g_resource[res];
	CPlayer* p = &g_player[b->owner];
	p->price[res] += 0.01f;
	RePrice(res);
	OnPriceSet(res, p->price[res]);
}

void Click_MinusPrice(int res)
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	CResource* r = &g_resource[res];
	CPlayer* p = &g_player[b->owner];
	if(p->price[res] >= 0.01f)
	{
		p->price[res] -= 0.01f;
		RePrice(res);
		OnPriceSet(res, p->price[res]);
	}
}

void Click_PlusUPrice(int unit)
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	CPlayer* p = &g_player[b->owner];
	p->uprice[unit] += 0.01f;
	ReUPrice(unit);
	//OnUPriceSet(unit, p->uprice[unit]);
}

void Click_MinusUPrice(int unit)
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	CPlayer* p = &g_player[b->owner];
	if(p->uprice[unit] >= 0.01f)
	{
		p->uprice[unit] -= 0.01f;
		ReUPrice(unit);
		//OnUPriceSet(unit, p->uprice[unit]);
	}
}

void UpdateQueueCount(int utindex)
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];
	CView* v = g_GUI.getview("manufunits");
	CUnitType* ut = &g_unitType[utindex];
	
	int numinprod = 0;
	int allpinprod = 0;
	for(int j=0; j<b->produ.size(); j++)
	{
		ManufJob mj = b->produ[j];
		if(mj.utype == utindex && mj.owner == g_localP)
			numinprod++;
		if(mj.utype == utindex)
			allpinprod++;
	}

	if(v->widget.size() >= 2)
	{
		Widget* mw = &v->widget[1];
		char numboxname[64];
		sprintf(numboxname, "num %s", ut->name);

		Widget* numbox = mw->getsubwidg(numboxname, TEXTBOX);

		if(numbox != NULL)
		{
			char numboxvalue[64];
			sprintf(numboxvalue, "%d queued (%d total)", numinprod, allpinprod);

			numbox->changetext(numboxvalue);
		}
	}

	v = g_GUI.getview("binspect");
	
	if(v->widget.size() >= 2)
	{
		Widget* mw = &v->widget[1];

		char numboxname[64];
		sprintf(numboxname, "num %s", ut->name);
		char numboxvalue[32];
		sprintf(numboxvalue, "%d (%d)", numinprod, allpinprod);
		Widget* numbox = mw->getsubwidg(numboxname, TEXTBOX);
		numbox->changetext(numboxvalue);
	}
}

void UpdatePQuota()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];
	CView* v = g_GUI.getview("binspect");

	if(v->widget.size() >= 2)
	{
		Widget* mw = &v->widget[1];

		CBuildingType* t = &g_buildingType[b->type];
		
		char pquotastr[64];
		pquotastr[0] = '\0';
		for(int i=0; i<RESOURCES; i++)
		{
			char add[32];
			if(t->output[i] <= 0.0f)
				continue;

			float rquota = b->prodquota * t->output[i];
			sprintf(add, "%c%d ", g_resource[i].icon, (int)rquota);
			strcat(pquotastr, add);
		}
		Widget* numbox = mw->getsubwidg("quota str", TEXTBOX);
		numbox->changetext(pquotastr);
	}
}

void UpdateBWage()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];
	CView* v = g_GUI.getview("binspect");

	if(v->widget.size() >= 2)
	{
		Widget* mw = &v->widget[1];

		CBuildingType* t = &g_buildingType[b->type];
		
		char wagestr[64];
		CPlayer* p = &g_player[b->owner];
		sprintf(wagestr, "%c%0.2f/%c", g_resource[CURRENC].icon, p->wage[b->type], g_resource[LABOUR].icon);
		Widget* numbox = mw->getsubwidg("wage str", TEXTBOX);
		numbox->changetext(wagestr);
	}
}

void UpdateStockStr()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];
	CView* v = g_GUI.getview("binspect");

	if(v->widget.size() >= 2)
	{
		Widget* mw = &v->widget[1];

		CBuildingType* bt = &g_buildingType[b->type];
		CPlayer* p = &g_player[b->owner];

		char stockstr[64];
		stockstr[0] = '\0';
		for(int i=0; i<RESOURCES; i++)
		{
			if(b->stock[i] <= 0.0f && bt->output[i] <= 0.0f)
				continue;
			char add[32];
			add[0] = '\0';
			if(g_resource[i].capacity)
			{
				if(i == HOUSING)
				{
					sprintf(add, "%c%d/%d ", g_resource[i].icon, (int)b->occupier.size(), (int)(b->prodquota*bt->output[i]));
				}
				else
				{
					if(bt->output[i] > 0.0f)
					{
						int consumption = (int)b->recenth.consumed[i];
						int production = (int)b->recenth.produced[i];
						if(b->cycleh.size() > 0)
						{
							consumption = b->cycleh[ b->cycleh.size()-1 ].consumed[i];
							production = b->cycleh[ b->cycleh.size()-1 ].produced[i];
						}
						sprintf(add, "%c%d/%d ", g_resource[i].icon, consumption, production);
					}
					else if(b->stock[i] > 0.0f)
					{
						sprintf(add, "%c%d ", g_resource[i].icon, (int)b->stock[i]);
					}
				}
			}
			else
			{
				char change[32];
				float changef = 0;
				/*if(i == CURRENC)
				{
					changef -= b->prodquota * bt->input[LABOUR] * p->wage[b->type];

					sprintf(add, "%c%0.2f%s/%c ", g_resource[i].icon, (int)b->stock[i], change, TIME_CHAR);
				}
				else*/
				if(i == LABOUR)
				{
					sprintf(add, "%c%d ", g_resource[i].icon, (int)b->stock[i]);
				}
				else
				{
					changef += b->prodquota * bt->output[i];
					if(changef >= 0.0f)
						sprintf(change, "+%d", (int)changef);
					else
						sprintf(change, "%d", (int)changef);

					char globstr[32];
					globstr[0] = '\0';
					if(p->global[i] > 0.0f)
						sprintf(globstr, "(%d)", (int)p->global[i]);

					if(bt->output[i] > 0.0f)
						//sprintf(add, "%c%d%s/%c ", g_resource[i].icon, (int)b->stock[i], change, TIME_CHAR);
						sprintf(add, "%c%d%s%s/%c ", g_resource[i].icon, (int)b->stock[i], globstr, change, TIME_CHAR);
					//else if(b->stock[i] > 0.0f)
					else if(b->stock[i]+p->global[i] > 0.0f)
						//sprintf(add, "%c%d ", g_resource[i].icon, (int)b->stock[i]);
						sprintf(add, "%c%d%s ", g_resource[i].icon, (int)b->stock[i], globstr);
				}
			}
			strcat(stockstr, add);
		}

		Widget* w = mw->getsubwidg("stock", TEXTBOX);
		w->changetext(stockstr);
	}
}

void UpdateBForSale()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];
	CView* v = g_GUI.getview("binspect");

	if(v->widget.size() >= 2)
	{
		Widget* mw = &v->widget[1];

		CBuildingType* bt = &g_buildingType[b->type];

		/*
		Widget* w = mw->getsubwidg("stock", TEXTBOX);
		w->changetext(stockstr);
		*/
		
		Widget* salebutton = mw->getsubwidg("sale button", BUTTON);
		Widget* saleprice = mw->getsubwidg("sale price", TEXTBOX);

		if(b->forsale)
		{
			char pricestr[32];
			sprintf(pricestr, "For sale: %c%0.2f", g_resource[CURRENC].icon, b->saleprice);
			saleprice->changetext(pricestr);

			if(b->owner == g_localP)
			{
				salebutton->changetext("Change");
			}
			else
			{
				salebutton->changetext("Buy");
			}
		}
		else
		{
			if(b->owner == g_localP)
			{
				saleprice->changetext(" ");
				salebutton->changetext("Put for sale");
			}
		}
	}
}

void BForSalePrice()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];

	CView* v = g_GUI.getview("put for sale");
	Widget* pricebox = v->getwidget("price", TEXT);

	char pricestr[32];
	sprintf(pricestr, "%c%0.2f", g_resource[CURRENC].icon, b->saleprice);
	pricebox->changetext(pricestr);
}

void Click_BForSale()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];

	CloseView("binspect");

	if(b->forsale)
	{
		if(b->owner == g_localP)
		{
			//salebutton->changetext("Change");
			OpenAnotherView("put for sale");
			BForSalePrice();
		}
		else
		{
			// Buy...
			//salebutton->changetext("Buy");

			if(b->trybuy(g_localP))
				CloseView("binspect");
		}
	}
	else
	{
		if(b->owner == g_localP)
		{
			//saleprice->changetext(" ");
			//salebutton->changetext("Put for sale");
			OpenAnotherView("put for sale");
			BForSalePrice();
		}
	}
}

void Click_PlusPQuota()
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	b->prodquota += 1.0f;
	UpdatePQuota();
	UpdateStockStr();
}

void Click_MinusPQuota()
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	if(b->prodquota >= 1.00f)
	{
		b->prodquota -= 1.0f;
		UpdatePQuota();
		UpdateStockStr();
	}
}

void Click_PlusBWage()
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	CPlayer* p = &g_player[b->owner];
	p->wage[b->type] += 0.01f;
	UpdateBWage();
}

void Click_MinusBWage()
{
	int bui = g_selection[0];
	CBuilding* b = &g_building[bui];
	CPlayer* p = &g_player[b->owner];
	if(p->wage[b->type] >= 0.01f)
	{
		p->wage[b->type] -= 0.01f;
		UpdateBWage();
	}
}

void Click_ManufUnit(int utindex)
{
	CBuilding* b = &g_building[g_selection[0]];
	b->QueueU(utindex, g_localP);
	UpdateQueueCount(utindex);
}

void Click_CancelManuf(int utindex)
{
	CBuilding* b = &g_building[g_selection[0]];
	b->DequeueU(utindex, g_localP);
	UpdateQueueCount(utindex);
}

void FillBManuf(int building)
{
	CView* v = g_GUI.getview("manufunits");
	Widget* mw = &v->widget[1];
	
	mw->pos[0] = 0;
	mw->pos[2] = HSCROLLER_SPACING;

	mw->subwidg.clear();

	CBuilding* b = &g_building[building];
	CBuildingType* bt = &g_buildingType[b->type];

	for(int i=0; i<bt->buildable.size(); i++)
	{
		int utindex = bt->buildable[i];
		CUnitType* ut = &g_unitType[utindex];
		
		AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+HSCROLLER_OPTIONW, mw->pos[3]);
		Widget usprite;
		usprite.Image(ut->sprite, mw->pos[2]+20, mw->pos[1]+20, mw->pos[2]+20+HSCROLLER_OPTIONW-20-20, mw->pos[1]+20+HSCROLLER_OPTIONW-20-20, 1, 1, 1, 1.0f);
		mw->subwidg.push_back(usprite);
		
		Widget namebox;
		namebox.TextBox("uname", ut->name, MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
		mw->subwidg.push_back(namebox);

		Widget numbox;
		char numboxname[64];
		sprintf(numboxname, "num %s", ut->name);
		numbox.TextBox(numboxname, "0 queued", MAINFONT8, mw->pos[2]+10, mw->pos[1]+10+15, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
		mw->subwidg.push_back(numbox);
		UpdateQueueCount(utindex);

		Widget costbox;
		char costboxname[64];
		sprintf(costboxname, "cost %s", ut->name);
		char coststr[128];
		coststr[0] = '\0';
		if(b->owner == g_localP)
		{
			for(int j=0; j<RESOURCES; j++)
			{
				if(ut->cost[j] > 0.0f)
				{
					char add[16];
					sprintf(add, "%c%d ", g_resource[j].icon, (int)ut->cost[j]);
					strcat(coststr, add);
				}
			}
		}
		else
		{
			CPlayer* p = &g_player[b->owner];
			char add[16];
			sprintf(add, "%c%0.2f", g_resource[CURRENC].icon, p->uprice[utindex]);
			strcat(coststr, add);
		}
		costbox.TextBox(costboxname, coststr, MAINFONT8, mw->pos[2]+10, mw->pos[1]+10+15*2, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3], 1, 1, 1, 1.0f);
		mw->subwidg.push_back(costbox);

		Widget button;
		if(b->owner == g_localP)
			button.Button("gui\\transp.png", "Manufacture", MAINFONT8, mw->pos[2]+HSCROLLER_OPTIONW-10-140, mw->pos[3]-10*2-44*2, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3]-10*2-44*1, &Click_ManufUnit, NULL, NULL, utindex);
		else
			button.Button("gui\\transp.png", "Order", MAINFONT8, mw->pos[2]+HSCROLLER_OPTIONW-10-140, mw->pos[3]-10*2-44*2, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3]-10*2-44*1, &Click_ManufUnit, NULL, NULL, utindex);
		mw->subwidg.push_back(button);
		
		button.Button("gui\\transp.png", "Cancel", MAINFONT8, mw->pos[2]+HSCROLLER_OPTIONW-10-140, mw->pos[3]-10*1-44*1, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3]-10*1-44*0, &Click_CancelManuf, NULL, NULL, utindex);
		mw->subwidg.push_back(button);
		
		mw->pos[2] += HSCROLLER_OPTIONW;
		mw->pos[2] += HSCROLLER_SPACING;
	}
}

void Click_ManufUnits(int dummy)
{
	CloseView("binspect");
	OpenAnotherView("manufunits");
	FillBManuf(g_selection[0]);
}

// open resource stock graph
void Click_BGraph()
{
	CloseView("binspect");
	OpenAnotherView("bgraph");
	FillBGraph();
}

void FillBInspect(int building)
{
	CView* v = g_GUI.getview("binspect");
	Widget* mw = &v->widget[1];

	mw->pos[0] = 0;
	mw->pos[2] = HSCROLLER_SPACING;

	mw->subwidg.clear();

	AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+HSCROLLER_OPTIONW, mw->pos[3]);
	CBuilding* b = &g_building[building];
	CBuildingType* bt = &g_buildingType[b->type];
	CPlayer* p = &g_player[b->owner];
	Widget bsprite;
	bsprite.Image(bt->sprite, mw->pos[2]+20, mw->pos[1]+20, mw->pos[2]+20+HSCROLLER_OPTIONW-20-20, mw->pos[1]+20+HSCROLLER_OPTIONW-20-20, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(bsprite);
	Widget namebox;
	//title.Text(name, name, MAINFONT8, mw->pos[0]+5, mw->pos[1], true);
	namebox.TextBox("bname", bt->name, MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(namebox);
	mw->pos[2] += HSCROLLER_OPTIONW;
	mw->pos[2] += HSCROLLER_SPACING;

	AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+HSCROLLER_OPTIONW, mw->pos[3]);
	Widget faclogo;
	faclogo.Image(faclogofile[b->owner], mw->pos[2]+20, mw->pos[1]+20, mw->pos[2]+20+HSCROLLER_OPTIONW-20-20, mw->pos[1]+20+HSCROLLER_OPTIONW-20-20, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(faclogo);
	namebox.TextBox("facname", facfullname[b->owner], MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(namebox);
	mw->pos[2] += HSCROLLER_OPTIONW;
	mw->pos[2] += HSCROLLER_SPACING;

	AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+HSCROLLER_OPTIONW, mw->pos[3]);
	namebox.TextBox("stockt", "Stock", MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(namebox);
	Widget textbox;
	textbox.TextBox("stock", " ", MAINFONT8, mw->pos[2]+10, mw->pos[1]+30, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3], 1, 1, 1, 1.0f);
	mw->subwidg.push_back(textbox);
	UpdateStockStr();
	Widget graphbutt;
	graphbutt.Button("gui\\transp.png", "Stock Graph", MAINFONT8, mw->pos[2]+10, mw->pos[3]-20-44, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3]-20, &Click_BGraph, NULL, NULL);
	mw->subwidg.push_back(graphbutt);
	mw->pos[2] += HSCROLLER_OPTIONW;
	mw->pos[2] += HSCROLLER_SPACING;
	
	if(
		//b->owner == g_localP && 
		b->finished)
	{
		int vspace = mw->pos[3]-30 - (mw->pos[1]+30);
		int hspace = HSCROLLER_OPTIONW+50;
		int vsppp = 54;	//vertical space per price
		int numprices = 0;
		for(int i=0; i<RESOURCES; i++)
			if(bt->output[i] > 0.0f)
				numprices++;
		int numrows = vspace/vsppp;
		int numcols = ceil((float)numprices/(float)numrows);
		AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+hspace*numcols+(hspace+HSCROLLER_SPACING)*bt->buildable.size(), mw->pos[3]);
		namebox.TextBox("prices", "Prices", MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3], 1, 1, 1, 1.0f);
		mw->subwidg.push_back(namebox);
		int col = 0;
		int row = 0;
		CFont* f = &g_font[MAINFONT8];
		for(int i=0; i<RESOURCES; i++)
		{
			if(bt->output[i] <= 0.0f)
				continue;
			char pricestr[32];
			sprintf(pricestr, "%c%0.2f / %c", g_resource[CURRENC].icon, p->price[i], g_resource[i].icon);
			char textname[32];
			sprintf(textname, "%s price", g_resource[i].name);
			textbox.TextBox(textname, pricestr, MAINFONT8, mw->pos[2]+20, mw->pos[1]+30+row*vsppp+44/2-f->gheight/2, mw->pos[2]+hspace-20, mw->pos[1]+30+(row+1)*vsppp, 1, 1, 1, 1.0f);
			mw->subwidg.push_back(textbox);
			if(b->owner == g_localP)
			{
				Widget butt;
				butt.Button("gui\\plus.png", "", MAINFONT8, mw->pos[2]+hspace-20-44*2, mw->pos[1]+30+row*vsppp, mw->pos[2]+hspace-20-44*1, mw->pos[1]+30+row*vsppp+44, &Click_PlusPrice, NULL, NULL, i);
				mw->subwidg.push_back(butt);
				butt.Button("gui\\minus.png", "", MAINFONT8, mw->pos[2]+hspace-20-44*1, mw->pos[1]+30+row*vsppp, mw->pos[2]+hspace-20-44*0, mw->pos[1]+30+row*vsppp+44, &Click_MinusPrice, NULL, NULL, i);
				mw->subwidg.push_back(butt);
			}
			row++;
			if(row >= numrows)
			{
				col++;
				row = 0;
				mw->pos[2] += hspace;
			}
		}
	
		mw->pos[2] += hspace;
		mw->pos[2] += HSCROLLER_SPACING;

		if(b->owner == g_localP)
		{
			for(int i=0; i<bt->buildable.size(); i++)
			{
				int utindex = bt->buildable[i];
				CUnitType* ut = &g_unitType[utindex];

				Widget usprite;
				usprite.Image(ut->sprite, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[1]+HSCROLLER_OPTIONW-10, 1, 1, 1, 1.0f);
				mw->subwidg.push_back(usprite);

				char coststr[64];
				coststr[0] = '\0';
				for(int j=0; j<RESOURCES; j++)
				{
					if(ut->cost[j] <= 0.0f)
						continue;
					char add[32];
					sprintf(add, "%c%d ", g_resource[j].icon, (int)ut->cost[j]);
					strcat(coststr, add);
				}
			
				char textname[32];
			
				sprintf(textname, "%s name", ut->name);
				textbox.TextBox(textname, ut->name, MAINFONT8, mw->pos[2]+20, mw->pos[1]+10, mw->pos[2]+hspace-20, mw->pos[1]+10+50, 1, 1, 1, 1.0f);
				mw->subwidg.push_back(textbox);
			
				sprintf(textname, "%s cost", ut->name);
				textbox.TextBox(textname, coststr, MAINFONT8, mw->pos[2]+20, mw->pos[1]+30, mw->pos[2]+hspace-20, mw->pos[1]+30+50, 1, 1, 1, 1.0f);
				mw->subwidg.push_back(textbox);

				CPlayer* p = &g_player[b->owner];
				char pricestr[32];
				sprintf(pricestr, "%c%0.2f", g_resource[CURRENC].icon, p->uprice[utindex]);
				sprintf(textname, "%s price", ut->name);
				textbox.TextBox(textname, pricestr, MAINFONT8, mw->pos[2]+20, mw->pos[1]+30+1*vsppp, mw->pos[2]+hspace-20-44*2, mw->pos[1]+30+2*vsppp, 1, 1, 1, 1.0f);
				mw->subwidg.push_back(textbox);

				Widget butt;
				butt.Button("gui\\plus.png", "", MAINFONT8, mw->pos[2]+hspace-20-44*2, mw->pos[1]+30+1*vsppp, mw->pos[2]+hspace-20-44*1, mw->pos[1]+30+1*vsppp+44, &Click_PlusUPrice, NULL, NULL, utindex);
				mw->subwidg.push_back(butt);
				butt.Button("gui\\minus.png", "", MAINFONT8, mw->pos[2]+hspace-20-44*1, mw->pos[1]+30+1*vsppp, mw->pos[2]+hspace-20-44*0, mw->pos[1]+30+1*vsppp+44, &Click_MinusUPrice, NULL, NULL, utindex);
				mw->subwidg.push_back(butt);

				mw->pos[2] += hspace;
				mw->pos[2] += HSCROLLER_SPACING;
			}
		
			if(bt->maxprodquota > 1.0f)
			{
				AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+hspace, mw->pos[3]);
				namebox.TextBox("production quota", "Production quota", MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3], 1, 1, 1, 1.0f);
				mw->subwidg.push_back(namebox);

				char quotastr[64];
				sprintf(quotastr, " ");
				textbox.TextBox("quota str", quotastr, MAINFONT8, mw->pos[2]+20, mw->pos[1]+30, mw->pos[2]+hspace-20, mw->pos[1]+10+50, 1, 1, 1, 1.0f);
				mw->subwidg.push_back(textbox);
				UpdatePQuota();

				Widget butt;
				butt.Button("gui\\plus.png", "", MAINFONT8, mw->pos[2]+hspace-20-44*2, mw->pos[1]+30+1*vsppp, mw->pos[2]+hspace-20-44*1, mw->pos[1]+30+1*vsppp+44, &Click_PlusPQuota, NULL, NULL);
				mw->subwidg.push_back(butt);
				butt.Button("gui\\minus.png", "", MAINFONT8, mw->pos[2]+hspace-20-44*1, mw->pos[1]+30+1*vsppp, mw->pos[2]+hspace-20-44*0, mw->pos[1]+30+1*vsppp+44, &Click_MinusPQuota, NULL, NULL);
				mw->subwidg.push_back(butt);
		
				mw->pos[2] += hspace;
				mw->pos[2] += HSCROLLER_SPACING;
			}

			if(bt->input[LABOUR] > 0.0f)
			{
				AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+hspace, mw->pos[3]);
				namebox.TextBox("wage title", "Wage", MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3], 1, 1, 1, 1.0f);
				mw->subwidg.push_back(namebox);
			
				char wagestr[64];
				sprintf(wagestr, " ");
				textbox.TextBox("wage str", wagestr, MAINFONT8, mw->pos[2]+20, mw->pos[1]+30, mw->pos[2]+hspace-20, mw->pos[1]+10+50, 1, 1, 1, 1.0f);
				mw->subwidg.push_back(textbox);
				UpdateBWage();
				
				Widget butt;
				butt.Button("gui\\plus.png", "", MAINFONT8, mw->pos[2]+hspace-20-44*2, mw->pos[1]+30+1*vsppp, mw->pos[2]+hspace-20-44*1, mw->pos[1]+30+1*vsppp+44, &Click_PlusBWage, NULL, NULL);
				mw->subwidg.push_back(butt);
				butt.Button("gui\\minus.png", "", MAINFONT8, mw->pos[2]+hspace-20-44*1, mw->pos[1]+30+1*vsppp, mw->pos[2]+hspace-20-44*0, mw->pos[1]+30+1*vsppp+44, &Click_MinusBWage, NULL, NULL);
				mw->subwidg.push_back(butt);

				mw->pos[2] += hspace;
				mw->pos[2] += HSCROLLER_SPACING;
			}
		}
	}

	if((b->owner == g_localP || g_diplomacy[g_localP][b->owner]) && b->finished && bt->buildable.size() > 0)
	{
		float vspace = mw->pos[3]-30 - (mw->pos[1]+30);
		int numbs = bt->buildable.size();
		int vsppu = 54;	//vertical space per unit
		float hspace = 54;
		int numrows = vspace/(vsppu+10);
		int numcols = ceil((float)numbs/(float)numrows);
		int totalw = max(120, (hspace+10)*numcols+10);
		Widget touchlistener;
		touchlistener.TouchListener(mw->pos[2], mw->pos[1], mw->pos[2]+totalw, mw->pos[3], &Click_ManufUnits, NULL, NULL, 0);
		mw->subwidg.push_back(touchlistener);
		AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+totalw, mw->pos[3]);

		if(b->owner == g_localP)
			namebox.TextBox("manufacture", "Manufacture", MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+totalw-10, mw->pos[3], 1, 1, 1, 1.0f);
		else
			namebox.TextBox("manufacture", "Order", MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+totalw-10, mw->pos[3], 1, 1, 1, 1.0f);
		mw->subwidg.push_back(namebox);

		int col = 0;
		int row = 0;
		for(int i=0; i<bt->buildable.size(); i++)
		{
			int utindex = bt->buildable[i];
			CUnitType* ut = &g_unitType[utindex];
			Widget usprite;
			usprite.Image(ut->sprite, mw->pos[2]+10, mw->pos[1]+(vsppu+10)*row+30, mw->pos[2]+hspace+10, mw->pos[1]+(vsppu)*(row+1)+10*row+30, 1, 1, 1, 1.0f);
			mw->subwidg.push_back(usprite);

			int numinprod = 0;

			for(int j=0; j<b->produ.size(); j++)
			{
				ManufJob mj = b->produ[j];
				if(mj.utype == utindex && mj.owner == g_localP)
					numinprod ++;
			}

			Widget numbox;
			char numboxname[64];
			sprintf(numboxname, "num %s", ut->name);
			numbox.TextBox(numboxname, "0", MAINFONT8, mw->pos[2]+10, mw->pos[1]+vsppu*row+30+10, mw->pos[2]+hspace-10, mw->pos[1]+vsppu*(row+1)+30+10, 1, 1, 1, 1.0f);
			mw->subwidg.push_back(numbox);
			UpdateQueueCount(utindex);

			row++;
			if(row >= numrows)
			{
				col++;
				row = 0;
				mw->pos[2] += hspace+10;
			}
		}

		mw->pos[2] += hspace;
		mw->pos[2] += HSCROLLER_SPACING;
	}

	if(b->owner == g_localP || b->forsale)
	{
		//int hspace = HSCROLLER_OPTIONW+50;
		int hspace = HSCROLLER_OPTIONW;
		
		AddGrayBGFr(&mw->subwidg, mw->pos[2], mw->pos[1], mw->pos[2]+hspace, mw->pos[3]);

		Widget numbox;
		numbox.TextBox("sale price", " ", MAINFONT8, mw->pos[2]+10, mw->pos[1]+30, mw->pos[2]+hspace-10, mw->pos[1]+30+40, 1, 1, 1, 1.0f);
		mw->subwidg.push_back(numbox);

		Widget butt;
		butt.Button("sale button", "gui\\transp.png", "Put for sale", MAINFONT8, mw->pos[2]+10, mw->pos[1]+30+40, mw->pos[2]+hspace-10, mw->pos[1]+30+40+44, &Click_BForSale, NULL, NULL);
		mw->subwidg.push_back(butt);
		UpdateBForSale();

		mw->pos[2] += hspace;
		mw->pos[2] += HSCROLLER_SPACING;
	}

	/*
	Widget optbg;
	optbg.Image("gui\\buildbg.png", mw->pos[2], mw->pos[1], mw->pos[2]+HSCROLLER_OPTIONW, mw->pos[3], 1, 1, 1, 1.0f);
	mw->subwidg.push_back(optbg);

	Widget touchl;
	touchl.TouchListener(mw->pos[2], mw->pos[1], mw->pos[2]+HSCROLLER_OPTIONW, mw->pos[3], &Click_BInspect, NULL, NULL, BINSPECT_);
	mw->subwidg.push_back(touchl);

	Widget bsprite;
	bsprite.Image(graphic, mw->pos[2]+20, mw->pos[1]+20, mw->pos[2]+20+75, mw->pos[1]+20+75, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(bsprite);

	Widget namebox;
	//title.Text(name, name, MAINFONT8, mw->pos[0]+5, mw->pos[1], true);
	namebox.TextBox(name, name, MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(namebox);

	Widget descbox;
	char descall[1024];
	sprintf(descall, "%s", desc);

	if(stricmp(desc, "") != 0)
		strcat(descall, "\n\n");
	
	strcat(descall, "Cost: ");
	for(int i=0; i<RESOURCES; i++)
	{
		if(cost[i] <= 0.0f)
			continue;

		char addt[64];
		sprintf(addt, "%c%d ", g_resource[i].icon, (int)cost[i]);
		strcat(descall, addt);
	}

	strcat(descall, "\n");

	bool hasinout = false;

	if(inp != NULL && outp != NULL)
	{
		for(int i=0; i<RESOURCES; i++)
		{
			if(inp[i] > 0.0f || outp[i] > 0.0f)
			{
				hasinout = true;
				break;
			}
		}
	}
	
	if(hasinout)
	{
		strcat(descall, "Input: ");
		for(int i=0; i<RESOURCES; i++)
		{
			if(inp[i] <= 0.0f)
				continue;

			char addt[64];
			sprintf(addt, "%c%d ", g_resource[i].icon, (int)inp[i]);
			strcat(descall, addt);
		}
		strcat(descall, "\n");

		strcat(descall, "Output: ");
		for(int i=0; i<RESOURCES; i++)
		{
			if(outp[i] <= 0.0f)
				continue;

			char addt[64];
			sprintf(addt, "%c%d ", g_resource[i].icon, (int)outp[i]);
			strcat(descall, addt);
		}
	}
	
	//descbox.TextBox("desc", descall, MAINFONT8, mw->pos[2]+10, mw->pos[1]+20+HSCROLLER_OPTIONW-30-20, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3]-10, 1, 1, 1, 1.0f);
	descbox.TextBox("desc", descall, MAINFONT8, mw->pos[2]+10, mw->pos[1]+20+75, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3]-10, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(descbox);
	*/
	
	//mw->pos[2] += HSCROLLER_OPTIONW;
	//mw->pos[2] += HSCROLLER_SPACING;
}

void Click_OutPutSale(int dummy)
{
	CloseView("put for sale");
	OpenAnotherView("binspect");
	FillBInspect(g_selection[0]);
}

void Click_PlusSaleP()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];

	if(b->owner != g_localP)
		return;

	b->saleprice += 0.01f;

	BForSalePrice();
}

void Click_MinusSaleP()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];

	if(b->owner != g_localP)
		return;

	if(b->saleprice >= 0.01f)
	{
		b->saleprice -= 0.01f;
		BForSalePrice();
	}
}

void Click_TakeDownSale()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];
	
	if(b->owner != g_localP)
		return;

	b->forsale = false;
	CloseView("put for sale");
	OpenAnotherView("binspect");
	FillBInspect(g_selection[0]);
}

void Click_PutForSale()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];
	
	if(b->owner != g_localP)
		return;

	b->forsale = true;
	CloseView("put for sale");
	OpenAnotherView("binspect");
	FillBInspect(g_selection[0]);
}

void Click_OutBGraph()
{
	CloseView("bgraph");
	FillBInspect(g_selection[0]);
	OpenAnotherView("binspect");
}

void MakeBInspect()
{
	AddView("binspect");
	AddImage("gui\\backg\\white.jpg",				14 + 16 - 7,						g_height - 242,					g_width - 20 + 11,				g_height - 60,				1,1,1,1.0f);
	AddHScroller(g_height - 242, g_height - 60, NULL, &Click_OutBInspect);
	AddFrame(0, g_height - 242 - 52, g_width, g_height, 7, 11, 200, 200);

	AddView("manufunits");
	AddImage("gui\\backg\\white.jpg",				14 + 16 - 7,						g_height - 242,					g_width - 20 + 11,				g_height - 60,				1,1,1,1.0f);
	AddHScroller(g_height - 242, g_height - 60, NULL, &Click_OutManufUnits);
	AddFrame(0, g_height - 242 - 52, g_width, g_height, 7, 11, 200, 200);
	
	AddView("bgraph");
	AddImage("gui\\backg\\white.jpg",				14 + 16 - 7,						g_height - 242,					g_width - 20 + 11,				g_height - 60,				1,1,1,1.0f);
	AddHScroller(g_height - 242, g_height - 60, NULL, &Click_OutBGraph);
	AddInsDraw(&DrawBGraph);
	AddFrame(0, g_height - 242 - 52, g_width, g_height, 7, 11, 200, 200);

	AddView("put for sale");
	AddTouchListener(0, 0, g_width, g_height, &Click_OutPutSale, NULL, NULL, -1);
	AddGrayBGFr(g_width/2-125, g_height/2-75, g_width/2+125, g_height/2+75);
	AddText("price", "$0.50", MAINFONT8, g_width/2-110, g_height/2-44);
	AddButton("gui\\plus.png", "", MAINFONT8, g_width/2+100-44*2, g_height/2-50, g_width/2+100-44*1, g_height/2-50+44, &Click_PlusSaleP, NULL, NULL);
	AddButton("gui\\minus.png", "", MAINFONT8, g_width/2+100-44*1, g_height/2-50, g_width/2+100-44*0, g_height/2-50+44, &Click_MinusSaleP, NULL, NULL);
	AddButton("gui\\transp.png", "Put for sale", MAINFONT8, g_width/2-100, g_height/2-44+50, g_width/2, g_height/2+50, &Click_PutForSale, NULL, NULL);
	AddButton("gui\\transp.png", "Take down", MAINFONT8, g_width/2, g_height/2-44+50, g_width/2+100, g_height/2+50, &Click_TakeDownSale, NULL, NULL);
}