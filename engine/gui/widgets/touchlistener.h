#ifndef TOUCHLISTENER_H
#define TOUCHLISTENER_H

#include "../widget.h"

class TouchListen : public Widget
{
public:
	TouchListen();
	TouchListen(Widget* parent, void (*reframef)(Widget* thisw), void (*click2)(int p), void (*overf)(int p), void (*out)(), int parm);

	void inev(InEv* ie);
};

#endif
