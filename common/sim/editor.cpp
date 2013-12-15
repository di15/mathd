

#include "editor.h"
#include "map.h"
#include "main.h"
#include "image.h"
#include "gui.h"
#include "menu.h"
#include "tileset.h"
#include "player.h"
#include "model.h"
#include "selection.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "save.h"
#include "unit.h"
#include "physics.h"
#include "water.h"
#include "font.h"
#include "script.h"
#include "chat.h"
#include "scenery.h"

int g_edTool = EDTOOL::NOTOOL;
int g_selU;

float strToFloat(const char *s)
{
    float x;
    istringstream iss(s);
    iss >> x;
    return x;
}

void Change_EdPRes(int param)
{
	CPlayer* p = &g_player[g_selP];
	CView* v = g_GUI.getview("ed pmore");
	CWidget* eb = v->getwidget(g_resource[param].name, EDITBOX);
	p->global[param] = strToFloat(eb->value.c_str());
}

void Change_EdPWage(int param)
{
	CPlayer* p = &g_player[g_selP];
	CView* v = g_GUI.getview("ed pmore2");
	if(param >= 0)
	{
		CWidget* eb = v->getwidget(g_buildingType[param].name, EDITBOX);
		p->wage[param] = strToFloat(eb->value.c_str());
	}
	else if(param == -1)
	{
		CWidget* eb = v->getwidget("truck wage", EDITBOX);
		p->truckwage = strToFloat(eb->value.c_str());
	}
	else if(param == -3)
	{
		CWidget* eb = v->getwidget("construction wage", EDITBOX);
		p->conwage = strToFloat(eb->value.c_str());

		//char msg[128];
		//sprintf(msg, "set %f", p->conwage);
		//Chat(msg);
	}
}

void Change_EdPPrice(int param)
{
	CPlayer* p = &g_player[g_selP];
	CView* v = g_GUI.getview("ed pmore3");

	if(param >= 0)
	{
		CWidget* eb = v->getwidget(g_resource[param].name, EDITBOX);
		p->price[param] = strToFloat(eb->value.c_str());
	}
	else if(param == -2)
	{
		CWidget* eb = v->getwidget("transport price", EDITBOX);
		p->transportprice = strToFloat(eb->value.c_str());
	}
}

void Click_ClosePMore()
{
	CloseView("ed pmore");
	CloseView("ed pmore2");
	CloseView("ed pmore3");
}

void Click_EdPMore()
{
	Click_ClosePMore();
	OpenAnotherView("ed pmore");

	CView* v = g_GUI.getview("ed pmore");
	
	CResource* r;
	int num = 0;
	for(int i=0; i<RESOURCES; i++)
	{
		r = &g_resource[i];

		if(r->capacity)
			continue;

		CWidget* eb = v->getwidget(r->name, EDITBOX);

		char value[32];
		if(i == CURRENC)
			sprintf(value, "%.2f", (float)g_player[g_selP].global[i]);
		else
			sprintf(value, "%d", (int)g_player[g_selP].global[i]);

		eb->changevalue(value);
	}
}

void Click_EdPMore2()
{
	Click_ClosePMore();
	OpenAnotherView("ed pmore2");

	CView* v = g_GUI.getview("ed pmore2");
	int num = 0;
	for(int i=0; i<BUILDING_TYPES; i++)
	{
		if(g_buildingType[i].input[LABOUR] <= 0.0f)
			continue;

		CWidget* eb = v->getwidget(g_buildingType[i].name, EDITBOX);

		char value[32];
		sprintf(value, "%.2f", (float)g_player[g_selP].wage[i]);

		eb->changevalue(value);
	}

	CWidget* eb = v->getwidget("truck wage", EDITBOX);
	char value[32];
	sprintf(value, "%.2f", (float)g_player[g_selP].truckwage);
	eb->changevalue(value);

	eb = v->getwidget("construction wage", EDITBOX);
	sprintf(value, "%.2f", (float)g_player[g_selP].conwage);
	eb->changevalue(value);

	//char msg[128];
	//sprintf(msg, "show %f", g_player[g_selP].conwage);
	//Chat(msg);
}

void Click_EdPMore3()
{
	Click_ClosePMore();
	OpenAnotherView("ed pmore3");

	CView* v = g_GUI.getview("ed pmore3");
	
	CResource* r;
	int num = 0;
	for(int i=0; i<RESOURCES; i++)
	{
		if(i == CURRENC)
			continue;
		if(i == LABOUR)
			continue;

		r = &g_resource[i];

		CWidget* eb = v->getwidget(r->name, EDITBOX);

		char value[32];
		sprintf(value, "%.2f", (float)g_player[g_selP].price[i]);

		eb->changevalue(value);
	}

	CWidget* eb = v->getwidget("transport price", EDITBOX);
	char value[32];
	sprintf(value, "%.2f", (float)g_player[g_selP].transportprice);
	eb->changevalue(value);
	
}

void Click_EdSave()
{
	OPENFILENAME ofn;
	
	char filepath[MAX_PATH+1];
	
	ZeroMemory( &ofn , sizeof( ofn));

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrFile = filepath;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return;

	SaveMap(filepath);
}

void Click_EdLoad()
{	
	OPENFILENAME ofn;
	
	char filepath[MAX_PATH+1];
	
	ZeroMemory( &ofn , sizeof( ofn));

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filepath;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
		return;

	SelectionFunc = NULL;
	LoadMap(filepath);

	Change_EdCat();
	ReGUIScript();
}

void Click_Raise()
{
	g_edTool = EDTOOL::RAISE;
}

void Click_Lower()
{
	g_edTool = EDTOOL::LOWER;
}

void Click_Spread()
{
	g_edTool = EDTOOL::SPREAD;
}

void Click_TileType(int param)
{
	g_edTool = EDTOOL::PLACETILE;
	g_selTS = param;
}

void AfterHeight(int minx, int maxx, int minz, int maxz)
{
	int minx2 = max(0, minx-1);
	int minz2 = max(0, minz-1);
	int z;
	int maxx3 = min(g_hmap.m_widthX-1, maxx);
	int maxz3 = min(g_hmap.m_widthZ-1, maxz);

	for(int x=minx2; x<=maxx3; x++)
		for(z=minz2; z<=maxz3; z++)
		{
			if(RoadAt(x, z)->on && !RoadLevel(x, z, x, z))
				RoadAt(x, z)->on = false;
			TypeRoad(x, z);
			TypeRoadsAround(x, z);

			if(x+1 < g_hmap.m_widthX)
			{
				if(RoadAt(x+1, z)->on && !RoadLevel(x+1, z, x+1, z))
					RoadAt(x+1, z)->on = false;
				TypeRoad(x+1, z);
				TypeRoadsAround(x+1, z);

				if(z+1 < g_hmap.m_widthZ)
				{
					if(RoadAt(x+1, z+1)->on && !RoadLevel(x+1, z+1, x+1, z+1))
						RoadAt(x+1, z+1)->on = false;
					TypeRoad(x+1, z+1);
					TypeRoadsAround(x+1, z+1);
				}
			}
			else if(z+1 < g_hmap.m_widthZ)
			{
				if(RoadAt(x, z+1)->on && !RoadLevel(x, z+1, x, z+1))
					RoadAt(x, z+1)->on = false;
				TypeRoad(x, z+1);
				TypeRoadsAround(x, z+1);
			}
			
			float h0 = g_hmap.getheight(x, z);
			float h1 = g_hmap.getheight(x+1, z);
			float h2 = g_hmap.getheight(x, z+1);
			float h3 = g_hmap.getheight(x+1, z+1);
			float h4 = h0;
			float h5 = h0;
			float h6 = h0;
			if(x > 0)
			{
				h4 = g_hmap.getheight(x-1, z);
				if(z > 0)
				{
					h5 = g_hmap.getheight(x-1, z-1);
				}
			}
			else if(z > 0)
			{
				h6 = g_hmap.getheight(x, z);
			}

			if(h0 <= WATER_HEIGHT || h1 <= WATER_HEIGHT || h2 <= WATER_HEIGHT || h3 <= WATER_HEIGHT ||
				h4 <= WATER_HEIGHT || h5 <= WATER_HEIGHT || h6 <= WATER_HEIGHT)
			{
				PowlAt(x, z)->on = false;
				PipeAt(x, z)->on = false;
				
				if(x+1 < g_hmap.m_widthX)
				{
					PowlAt(x+1, z)->on = false;
					PipeAt(x+1, z)->on = false;

					if(z+1 < g_hmap.m_widthZ)
					{
						PowlAt(x+1, z+1)->on = false;
						PipeAt(x+1, z+1)->on = false;
					}
				}
				else if(z+1 < g_hmap.m_widthZ)
				{
					PowlAt(x, z+1)->on = false;
					PipeAt(x, z+1)->on = false;
				}
			}

			TypePowerline(x, z);
			TypePowerlinesAround(x, z);
			TypePipeline(x, z);
			TypePipelinesAround(x, z);
				
			if(x+1 < g_hmap.m_widthX)
			{
				TypePowerline(x+1, z);
				TypePowerlinesAround(x+1, z);
				TypePipeline(x+1, z);
				TypePipelinesAround(x+1, z);

				if(z+1 < g_hmap.m_widthZ)
				{
					TypePowerline(x+1, z+1);
					TypePowerlinesAround(x+1, z+1);
					TypePipeline(x+1, z+1);
					TypePipelinesAround(x+1, z+1);
				}
			}
			else if(z+1 < g_hmap.m_widthZ)
			{
				TypePowerline(x, z+1);
				TypePowerlinesAround(x, z+1);
				TypePipeline(x, z+1);
				TypePipelinesAround(x, z+1);
			}
		}

	
	int maxx2 = min(g_hmap.m_widthX, maxx+1);
	int maxz2 = min(g_hmap.m_widthZ, maxz+1);

	float fminx = minx2 * TILE_SIZE;
	float fminz = minz2 * TILE_SIZE;
	float fmaxx = maxx2 * TILE_SIZE;
	float fmaxz = maxz2 * TILE_SIZE;

	CUnit* u;
	Camera* c;
	Vec3f pos;
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		c = &u->camera;
		pos = c->Position();

		if(pos.x >= fminx && pos.x <= fmaxx && pos.z >= fminz && pos.z <= fmaxz)
		{
			pos.y = Bilerp(pos.x, pos.z);
			c->MoveTo(pos);
		}

		if(u->Collides())
		{
			u->destroy();
			u->on = false;
		}
	}

	for(int i=0; i<SCENERY; i++)
	{
		Scenery* s = &g_scenery[i];

		if(!s->on)
			continue;

		if(s->pos.x >= fminx && s->pos.x <= fmaxx && s->pos.z >= fminz && s->pos.z <= fmaxz)
		{
			s->pos.y = Bilerp(s->pos.x, s->pos.z);

			if(s->pos.y <= WATER_HEIGHT)
				s->on = false;
		}
	}

	CBuilding* b;
	CBuildingType* t;
	float hwx = (fmaxx+fminx)/2.0f;
	float cx = (fmaxx-fminx)/2.0f;
	float hwz = (fmaxz+fminz)/2.0f;
	float cz = (fmaxz-fminz)/2.0f;
	float hwx2, hwz2;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		t = &g_buildingType[b->type];
		pos = b->pos;
		hwx2 = t->widthX*TILE_SIZE/2.0f;
		hwz2 = t->widthZ*TILE_SIZE/2.0f;

		if(fabs(pos.x-cx) < hwx+hwx2 && fabs(pos.z-cz) < hwz+hwz2)
		{
			pos.y = Bilerp(pos.x, pos.z);
			b->pos = pos;

			if(!BuildingLevel(b->type, pos))
			{
				b->destroy();
				b->on = false;
				RePipe();
				RePow();
			}
		}
	}
}

void Reheight()
{
	int minx, maxx, minz, maxz;
	MinMaxCorners(minx, maxx, minz, maxz);

	float change = 0;

	if(g_edTool == EDTOOL::RAISE)
		change = 1;
	else if(g_edTool == EDTOOL::LOWER)
		change = -1;

	int z;
	for(int x=minx; x<=maxx; x++)
		for(z=minz; z<=maxz; z++)
		{
			g_hmap.changeheight(x, z, change);
		}

	g_hmap.remesh();
	AfterHeight(minx, maxx, minz, maxz);
}

void Spread()
{
	int minx, maxx, minz, maxz;
	MinMaxCorners(minx, maxx, minz, maxz);

	int x1, x2, z1, z2;
	StartEndCorners(x1, x2, z1, z2);

	float height = g_hmap.getheight(x1, z1);

	int z;
	for(int x=minx; x<=maxx; x++)
		for(z=minz; z<=maxz; z++)
		{
			g_hmap.setheight(x, z, height);
		}

	g_hmap.remesh();
	AfterHeight(minx, maxx, minz, maxz);
}

void Destroy()
{
	//g_canselect = true;
	Selection();
	//g_canselect = false;

	if(g_selectType == SELECT_BUILDING)
	{
		CBuilding* b;
		for(int i=0; i<g_selection.size(); i++)
		{
			b = &g_building[ g_selection[i] ];
			b->destroy();
			b->on = false;
		}
	}
	else if(g_selectType == SELECT_UNIT)
	{
		CUnit* u;
		for(int i=0; i<g_selection.size(); i++)
		{
			u = &g_unit[ g_selection[i] ];
			u->destroy();
			u->on = false;
		}
	}
	else if(g_selectType == SELECT_ROAD)
	{
		CRoad* r;
		POINT point;
		for(int i=0; i<g_selection2.size(); i++)
		{
			point = g_selection2[i];
			r = RoadAt(point.x, point.y);
			r->destroy();
			r->on = false;
			TypeRoadsAround(point.x, point.y);
		}
	}
	else if(g_selectType == SELECT_POWERLINE)
	{
		CPowerline* p;
		POINT point;
		for(int i=0; i<g_selection2.size(); i++)
		{
			point = g_selection2[i];
			p = PowlAt(point.x, point.y);
			p->destroy();
			p->on = false;
			TypePowerlinesAround(point.x, point.y);
		}
	}
	else if(g_selectType == SELECT_PIPELINE)
	{
		CPipeline* p;
		POINT point;
		for(int i=0; i<g_selection2.size(); i++)
		{
			point = g_selection2[i];
			p = PipeAt(point.x, point.y);
			p->destroy();
			p->on = false;
			TypePipelinesAround(point.x, point.y);
		}
	}
	else if(g_selectType == SELECT_SCENERY)
	{
		Scenery* s;
		for(int i=0; i<g_selection.size(); i++)
		{
			s = &g_scenery[ g_selection[i] ];
			s->on = false;
		}
	}

	g_selection.clear();
	g_selection2.clear();
}

void PlaceUnit()
{
	int u = PlaceUnit(g_selU, g_selP, g_vMouse);
	if(g_selU == LABOURER)
	{
		g_unit[u].ResetLabourer();
	}
	
	//if(g_unit[u].Collides2())
	if(g_unit[u].Collides2(false))
		g_unit[u].on = false;
}

void EdApply()
{
}

void EdApplyUp()
{
	if(g_edTool == EDTOOL::PLACETILE)
	{
		Retile();
	}
	else if(g_edTool == EDTOOL::RAISE || g_edTool == EDTOOL::LOWER)
		Reheight();
	else if(g_edTool == EDTOOL::SPREAD)
		Spread();
	else if(g_edTool == EDTOOL::DESTROY)
	{
		Destroy();
	}
	else if(g_edTool == EDTOOL::PLACEUNIT)
		PlaceUnit();
	else if(g_edTool == PLACESCENERY)
		PlaceScenery();
}

void Change_EdUnitType()
{
	g_selU = g_GUI.getview("ed units")->getwidget("unit type", DROPDOWN)->selected;
	g_edTool = EDTOOL::PLACEUNIT;
}

void Change_EdCat()
{
	int selected = g_GUI.getview("editor")->getwidget("edcat", DROPDOWN)->selected;

	g_edTool = NOTOOL;
	SelectionFunc = NULL;
	g_scT = -1;
	
	CloseView("ed build selector");
	CloseView("editor selp");
	CloseView("tiles");
	CloseView("ed units");
	CloseView("ed inspector");
	CloseView("ed players");
	Click_ClosePMore();
	CloseView("ed script");
	CloseView("ed trigger");
	CloseView("ed effect");
	CloseCondition();
	CloseView("name unique");
	CloseEffect();
	CloseView("ed scenery");

	if(selected == 0)	//Tiles
	{
		g_canselect = false;
		OpenAnotherView("tiles");
	}
	else if(selected == 1)	//Buildings
	{
		g_canselect = false;
		OpenAnotherView("ed build selector");
		OpenAnotherView("editor selp");
	}
	else if(selected == 2)	//Units
	{
		OpenAnotherView("ed units");
		OpenAnotherView("editor selp");
	}
	else if(selected == 3)	//Inspector
	{
		OpenAnotherView("ed inspector");
	}
	else if(selected == 4)	//Players
	{
		OpenAnotherView("ed players");
		OpenAnotherView("editor selp");
	}
	else if(selected == 5)	//Scripts
	{
		OpenAnotherView("ed script");
	}
	else if(selected == 6)	//Scenery
	{
		OpenAnotherView("ed scenery");
		g_edTool = PLACESCENERY;
	}
}

void Change_Activity()
{
	int act = g_GUI.getview("ed players")->getwidget("activity", DROPDOWN)->selected;
	g_player[g_selP].activity = act;
}

void Change_SelP()
{
	g_selP = g_GUI.getview("editor selp")->getwidget("player", DROPDOWN)->selected;

	CView* edplayers = g_GUI.getview("ed players");
	edplayers->getwidget("activity", DROPDOWN)->selected = g_player[g_selP].activity;
	CWidget* logo = edplayers->getwidget("logo", IMAGE);
	CreateTexture(logo->tex, faclogofile[g_selP], true);
}

void Change_ScT()
{
	g_edTool = PLACESCENERY;
	g_scT = g_GUI.getview("ed scenery")->getwidget("scenery type", DROPDOWN)->selected;
}

void Click_Destroy()
{
	g_canselect = true;
	g_edTool = EDTOOL::DESTROY;
}