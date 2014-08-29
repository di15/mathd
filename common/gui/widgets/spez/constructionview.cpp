#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../dropdowns.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "constructionview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../../../game/gui/gviewport.h"
#include "../../../sim/building.h"
#include "../../../sim/buildingtype.h"
#include "../../../sim/road.h"
#include "../../../sim/crpipe.h"
#include "../../../sim/powl.h"
#include "../../../sim/player.h"
#include "buildpreview.h"
#include "../../icon.h"

//resize construction view column item
void Resize_CV_Cl(Widget* thisw)
{
	Widget* parw = thisw->m_parent;
	int col;
	int row;
	sscanf(thisw->m_name.c_str(), "%d %d", &col, &row);
	
	thisw->m_pos[0] = parw->m_pos[0] + 150*row;
	thisw->m_pos[1] = parw->m_pos[1] + 20*col + 60;
	thisw->m_pos[2] = imin(parw->m_pos[0] + 150*(row+1), parw->m_pos[2]);
	thisw->m_pos[3] = imin(parw->m_pos[1] + 20*(col+1) + 60, parw->m_pos[3]);
}

// change construction view construction wage
void Change_CV_CW(unsigned int key, unsigned int scancode, bool down)
{
	Player* py = &g_player[g_curP];
	GUI* gui = &py->gui;
	ConstructionView* cv = (ConstructionView*)gui->get("construction view");
	EditBox* cw = (EditBox*)cv->get("0 1");
	std::string sval = cw->m_value.rawstr();
	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	Selection* sel = &py->sel;

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	if(sel->buildings.size() > 0)
	{
		int bi = *sel->buildings.begin();
		Building* b = &g_building[bi];
		BlType* t = &g_bltype[b->type];
		b->conwage = ival;
	}
#if 0
	else if(sel->roads.size() > 0)
	{
		py->bptype = BUILDING_ROAD;
		conmat = g_roadcost;
		qty = sel->roads.size();
		Vec2i xz = *sel->roads.begin();
		RoadTile* road = RoadAt(xz.x, xz.y);
		maxcost = road->maxcost;
	}
	else if(sel->powls.size() > 0)
	{
		py->bptype = BUILDING_POWL;
		conmat = g_powlcost;
		qty = sel->powls.size();
		Vec2i xz = *sel->powls.begin();
		PowlTile* powl = PowlAt(xz.x, xz.y);
		maxcost = powl->maxcost;
	}
	else if(sel->crpipes.size() > 0)
	{
		py->bptype = BUILDING_CRPIPE;
		qty = sel->crpipes.size();
		conmat = g_crpipecost;
		Vec2i xz = *sel->crpipes.begin();
		CrPipeTile* crpipe = CrPipeAt(xz.x, xz.y);
		maxcost = crpipe->maxcost;
	}
#endif
}

ConstructionView::ConstructionView(Widget* parent, const char* n, void (*reframef)(Widget* thisw), void (*movefunc)(), void (*cancelfunc)(), void (*proceedfunc)(), void (*estimatefunc)()) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_CONSTRUCTIONVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	this->movefunc = movefunc;
	this->cancelfunc = cancelfunc;
	this->proceedfunc = proceedfunc;
	this->estimatefunc = estimatefunc;

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void ConstructionView::regen(Selection* sel)
{
	int* conmat = NULL;
	int qty = -1;
	int* maxcost = NULL;
	RichText bname;
	int conwage;

	Player* py = &g_player[g_curP];

	if(sel->buildings.size() > 0)
	{
		int bi = *sel->buildings.begin();
		Building* b = &g_building[bi];
		BlType* t = &g_bltype[b->type];

		conmat = t->conmat;
		py->bptype = b->type;
		maxcost = b->maxcost;
		bname = RichText(UString(t->name));
		conwage = b->conwage;
	}
#if 0
	else if(sel->roads.size() > 0)
	{
		py->bptype = BUILDING_ROAD;
		conmat = g_roadcost;
		qty = sel->roads.size();
		Vec2i xz = *sel->roads.begin();
		RoadTile* road = RoadAt(xz.x, xz.y);
		maxcost = road->maxcost;
	}
	else if(sel->powls.size() > 0)
	{
		py->bptype = BUILDING_POWL;
		conmat = g_powlcost;
		qty = sel->powls.size();
		Vec2i xz = *sel->powls.begin();
		PowlTile* powl = PowlAt(xz.x, xz.y);
		maxcost = powl->maxcost;
	}
	else if(sel->crpipes.size() > 0)
	{
		py->bptype = BUILDING_CRPIPE;
		qty = sel->crpipes.size();
		conmat = g_crpipecost;
		Vec2i xz = *sel->crpipes.begin();
		CrPipeTile* crpipe = CrPipeAt(xz.x, xz.y);
		maxcost = crpipe->maxcost;
	}
#endif

	freech();

	m_subwidg.push_back(new ViewportW(this, "viewport", Resize_BP_VP, &DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, VIEWPORT_ENTVIEW));
	m_subwidg.push_back(new Text(this, "title", bname, MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));

	int col = 0;

	char colname[32];
	sprintf(colname, "%d %d", col, 0);
	m_subwidg.push_back(new Text(this, colname, RichText(UString("Construction Wage")) + RichText(RichPart(RICHTEXT_ICON, ICON_LABOUR)) + RichText(UString(":")), MAINFONT16, Resize_CV_Cl, true, 0.7f, 0.9f, 0.3f, 1));

	sprintf(colname, "%d %d", col, 1);
	char cwstr[32];
	sprintf(cwstr, "%d", conwage);
	m_subwidg.push_back(new EditBox(this, colname, RichText(UString(cwstr)), MAINFONT16, Resize_CV_Cl, false, 6, Change_CV_CW, NULL, -1));

	reframe();
}
