

#ifndef RESTICKER_H
#define RESTICKER_H

#include "../../widget.h"

class ResTicker : public Widget
{
public:
	ResTicker(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

	Text restext;
	Image leftinnerdiagblur;
	Image rightinnerdiagblur;
	Image innerbottom;
	Image lefthlineblur;
	Image righthlineblur;
	Image whitebg;

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