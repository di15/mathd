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

void ConstructionView::regen(Selection* sel)
{
	RichText bname;
	int* prodprice;


	Player* py = &g_player[g_curP];

	if(sel->buildings.size() > 0)
	{
		int bi = *sel->buildings.begin();
		Building* b = &g_building[bi];
		BlType* t = &g_bltype[b->type];

		py->bptype = b->type;
		prodprice = b->prodprice;

		bname = RichText(UString(t->name));
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
