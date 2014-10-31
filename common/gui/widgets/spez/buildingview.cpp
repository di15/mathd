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
#include "buildingview.h"
#include "buildpreview.h"
#include "../../icon.h"

//resize building view column item
void Resize_BV_Cl(Widget* thisw)
{
	Widget* parw = thisw->m_parent;
	int col;
	int row;
	sscanf(thisw->m_name.c_str(), "%d %d", &col, &row);
	
	thisw->m_pos[0] = parw->m_pos[0] + 64*(imax(row-1,0));
	thisw->m_pos[1] = parw->m_pos[1] + 20*col + 60;
	thisw->m_pos[2] = imin(parw->m_pos[0] + 150 + 64*(row+1), parw->m_pos[2]);
	thisw->m_pos[3] = imin(parw->m_pos[1] + 20*(col+1) + 60, parw->m_pos[3]);

	if(row > 0)
		thisw->m_pos[0] += 150;
}

// change building view prod price
void Change_BV_PP(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_curP];
	GUI* gui = &py->gui;
	BuildingView* bv = (BuildingView*)gui->get("building view");
	Selection* sel = &py->sel;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int col = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(ri >= parm)
			break;
		if(bt->output[ri] <= 0)
			continue;
		col++;
	}

	char wn[32];
	sprintf(wn, "%d 1", col);
	EditBox* cw = (EditBox*)bv->get(wn);
	std::string sval = cw->m_value.rawstr();
	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	b->prodprice[parm] = ival;
}

BuildingView::BuildingView(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_BUILDINGVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void BuildingView::regen(Selection* sel)
{
	RichText bname;
	int* prodprice;

	Player* py = &g_player[g_curP];
	BlType* bt;
	Building* b;

	if(sel->buildings.size() > 0)
	{
		int bi = *sel->buildings.begin();
		b = &g_building[bi];
		bt = &g_bltype[b->type];

		py->bptype = b->type;
		prodprice = b->prodprice;

		bname = RichText(UString(bt->name));
	}
#if 0
	else if(sel->roads.size() > 0)
	{
		py->bptype = BL_ROAD;
		conmat = g_roadcost;
		qty = sel->roads.size();
		Vec2i xz = *sel->roads.begin();
		RoadTile* road = RoadAt(xz.x, xz.y);
		maxcost = road->maxcost;
	}
	else if(sel->powls.size() > 0)
	{
		py->bptype = BL_POWL;
		conmat = g_powlcost;
		qty = sel->powls.size();
		Vec2i xz = *sel->powls.begin();
		PowlTile* powl = PowlAt(xz.x, xz.y);
		maxcost = powl->maxcost;
	}
	else if(sel->crpipes.size() > 0)
	{
		py->bptype = BL_CRPIPE;
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

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;

		Resource* r = &g_resource[ri];

		char colname[32];
		sprintf(colname, "%d %d", col, 0);
		RichText label;

		m_subwidg.push_back(new Text(this, colname, RichText(UString("Price of ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString(r->name.c_str())) + RichText(UString(":")), MAINFONT16, Resize_BV_Cl, true, 0.7f, 0.9f, 0.3f, 1));

		sprintf(colname, "%d %d", col, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", b->prodprice[ri]);
		m_subwidg.push_back(new EditBox(this, colname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl, false, 6, Change_BV_PP, NULL, ri));

		col++;
	}


	reframe();
}
