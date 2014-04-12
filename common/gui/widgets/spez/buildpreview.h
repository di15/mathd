

#ifndef BUILDPREVIEW_H
#define BUILDPREVIEW_H

#include "../../widget.h"
#include "../viewportw.h"

#define MINIMAP_SIZE				100
#define BOTTOM_MID_PANEL_HEIGHT		200
#define MINIMAP_OFF					5

class BuildPreview : public Widget
{
public:
	BuildPreview(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

#if 0
	Text restext;
	Image leftinnerdiagblur;
	Image rightinnerdiagblur;
	Image innerbottom;
	Image lefthlineblur;
	Image righthlineblur;
	Image whitebg;
#endif

	Image left_outer_topleftcorner;
	Image left_outer;
	
	Image right_outer_toprightcorner;
	Image right_outer;
	
	Image middle_outer_top;
	Image middle_outer_bottom;
	
	ViewportW viewport;

	Image white_bg;

	void draw();
	void draw2();
	void reframe();
	bool prelbuttonup(bool moved);
	bool lbuttonup(bool moved);
	bool prelbuttondown();
	bool lbuttondown();
	void premousemove();
	bool mousemove();
	bool prerbuttonup(bool moved);
	bool rbuttonup(bool moved);
	bool prerbuttondown();
	bool rbuttondown();
	bool keyup(int k);
	bool keydown(int k);
	bool charin(int k);
	bool mousewheel(int delta);
	void frameupd();
};

#endif