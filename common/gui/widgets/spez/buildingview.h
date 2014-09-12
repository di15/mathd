#ifndef BUILDINGVIEW_H
#define BUILDINGVIEW_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../../sim/selection.h"
#include "../windoww.h"

class BuildingView : public WindowW
{
public:
	BuildingView(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ev);
	//void frameupd();
	void regen(Selection* sel);
};

#endif
