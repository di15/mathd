

#ifndef BOTTOMPANEL_H
#define BOTTOMPANEL_H

#include "../../widget.h"
#include "../viewportw.h"

#define MINIMAP_SIZE				100
#define BOTTOM_MID_PANEL_HEIGHT		200
#define MINIMAP_OFF					5

class BottomPanel : public Widget
{
public:
	BottomPanel(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

#if 0
	Text restext;
	Image leftinnerdiagblur;
	Image rightinnerdiagblur;
	Image innerbottom;
	Image lefthlineblur;
	Image righthlineblur;
	Image whitebg;
#endif

	Image left_outer_toprightcorner;
	Image left_outer_top;
	ViewportW left_minimap;
	
	Image right_outer_topleftcorner;
	Image right_outer_top;

	Image middle_outer_top;

	Image white_bg;

	Button bottomright_button[9];
	bool bottomright_button_on[9];

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