



#ifndef GUI_H
#define GUI_H

#include "../platform.h"
#include "../math/3dmath.h"
#include "view.h"
#include "widgets/image.h"
#include "widgets/barbutton.h"
#include "widgets/button.h"
#include "widgets/checkbox.h"
#include "widgets/dropdowns.h"
#include "widgets/editbox.h"
#include "widgets/insdraw.h"
#include "widgets/link.h"
#include "widgets/listbox.h"
#include "widgets/text.h"
#include "widgets/textarea.h"
#include "widgets/textblock.h"
#include "widgets/touchlistener.h"
#include "widgets/frame.h"
#include "widgets/viewportw.h"

class GUI
{
public:
	list<View> view;
	void (*keyupfunc[256])();
	void (*keydownfunc[256])();
	void (*anykeyupfunc)(int k);
	void (*anykeydownfunc)(int k);
	void (*mousemovefunc)();
	void (*lbuttondownfunc)();
	void (*lbuttonupfunc)();
	void (*rbuttondownfunc)();
	void (*rbuttonupfunc)();
	void (*mbuttondownfunc)();
	void (*mbuttonupfunc)();
	void (*mousewheelfunc)(int delta);

	GUI()
	{
		for(int i=0; i<256; i++)
		{
			keyupfunc[i] = NULL;
			keydownfunc[i] = NULL;
		}

		mousemovefunc = NULL;
	}

	void frameupd()
	{
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened)
				i->frameupd();
	}

	View* getview(char* name)
	{
		for(auto i=view.begin(); i!=view.end(); i++)
			if(_stricmp(i->name.c_str(), name) == 0)
				return &*i;

		return NULL;
	}

	void assignMouseWheel(void (*wheel)(int delta))
	{
		mousewheelfunc = wheel;
	}

	void assignLButton(void (*down)(), void (*up)())
	{
		lbuttondownfunc = down;
		lbuttonupfunc = up;
	}

	void assignRButton(void (*down)(), void (*up)())
	{
		rbuttondownfunc = down;
		rbuttonupfunc = up;
	}

	void assignMButton(void (*down)(), void (*up)())
	{
		mbuttondownfunc = down;
		mbuttonupfunc = up;
	}

	void assignMouseMove(void (*mouse)())
	{
		mousemovefunc = mouse;
	}

	void assignKey(int i, void (*down)(), void (*up)())
	{
		keydownfunc[i] = down;
		keyupfunc[i] = up;
	}

	void assignAnyKey(void (*down)(int k), void (*up)(int k))
	{
		anykeydownfunc = down;
		anykeyupfunc = up;
	}

	void mousewheel(int delta)
	{
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->mousewheel(delta))
				return;	// intercept mouse event

		if(mousewheelfunc != NULL)
			mousewheelfunc(delta);
	}

	void lbuttondown()
	{
		g_keyintercepted = false;

		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->prelbuttondown())
			{
				g_keyintercepted = true;
				return;	// intercept mouse event
			}
		
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->lbuttondown())
			{
				g_keyintercepted = true;
				return;	// intercept mouse event
			}

		if(lbuttondownfunc != NULL)
			lbuttondownfunc();
	}
	void lbuttonup(bool moved)
	{
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->prelbuttonup(moved))
				return;	// intercept mouse event
		
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->lbuttonup(moved))
				return;	// intercept mouse event

		if(lbuttonupfunc != NULL)
			lbuttonupfunc();
	}

	void rbuttondown()
	{
		
	//g_log<<"r down gui"<<endl;

		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->prerbuttondown())
				return;	// intercept mouse event
		
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->rbuttondown())
				return;	// intercept mouse event

		if(rbuttondownfunc != NULL)
			rbuttondownfunc();
	}
	void rbuttonup(bool moved)
	{
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->prerbuttonup(moved))
				return;	// intercept mouse event
		
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened && i->rbuttonup(moved))
				return;	// intercept mouse event

		if(rbuttonupfunc != NULL)
			rbuttonupfunc();
	}

	void mbuttondown()
	{
		if(mbuttondownfunc != NULL)
			mbuttondownfunc();
	}
	void mbuttonup()
	{
		if(mbuttonupfunc != NULL)
			mbuttonupfunc();
	}

	void mousemove()
	{
		g_mouseoveraction = false;

		for(auto i=view.begin(); i!=view.end(); i++)
			if(i->opened)
				i->premousemove();
		
		for(auto i=view.begin(); i!=view.end(); i++)
			if(i->opened)
				if(i->mousemove())
					return;

		if(mousemovefunc != NULL)
			mousemovefunc();
	}

	bool keyup(int k)
	{
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened)
				if(i->keyup(k))
					return true;

		if(anykeyupfunc != NULL)
			anykeyupfunc(k);

		if(keyupfunc[k] != NULL)
			keyupfunc[k]();
			
		return false;
	}

	bool keydown(int k)
	{
		g_keyintercepted = false;
		
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened)
				if(i->keydown(k))
				{
					g_keyintercepted = true;
					return true;
				}

		if(anykeydownfunc != NULL)
			anykeydownfunc(k);

		if(keydownfunc[k] != NULL)
			keydownfunc[k]();
			
		return false;
	}

	bool charin(int k)
	{
		for(auto i=view.rbegin(); i!=view.rend(); i++)
			if(i->opened)
				if(i->charin(k))
					return true;

		return false;
	}

	void resize()
	{
		for(auto i=view.begin(); i!=view.end(); i++)
			i->resize();
	}

	void draw();
};

extern GUI g_GUI;
extern int g_currw;
extern int g_currh;
extern bool g_mouseoveraction;

void AssignKey(int i, void (*down)(), void (*up)());
void AssignAnyKey(void (*down)(int k), void (*up)(int k));
void AssignMouseWheel(void (*wheel)(int delta));
void AssignMouseMove(void (*mouse)());
void AssignLButton(void (*down)(), void (*up)());
void AssignRButton(void (*down)(), void (*up)());
void AssignMButton(void (*down)(), void (*up)());
View* AddView(const char* name, int page=0);
void CloseView(const char* name);
void OpenSoleView(const char* name, int page=0);
bool OpenAnotherView(const char* name, int page=0);
bool ViewOpen(const char* name, int page=0);
void NextPage(const char* name);
bool MousePosition();
void CenterMouse();
void Status(const char* status, bool logthis=false);
void Ortho(int width, int height, float r, float g, float b, float a);

#endif