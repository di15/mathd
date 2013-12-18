
#ifndef WIDGET_H
#define WIDGET_H

#include "../utils.h"
#include "../texture.h"
#include "font.h"
#include "../render/shader.h"
#include "../window.h"
#include "draw2d.h"
#include "richtext.h"

#define MAX_OPTIONS_SHOWN	5
#define LBAR_WIDTH		100
#define MMBAR_WIDTH		(LBAR_WIDTH*128.0f/110.0f)
#define MMBAR_HEIGHT	(MMBAR_WIDTH*1.0f/1.0f)
#define MINIMAP_X1		(MMBAR_WIDTH*6.0f/128.0f)
#define MINIMAP_X2		(MMBAR_WIDTH*121.0f/128.0f)
#define MINIMAP_Y1		(MMBAR_HEIGHT*6.0f/128.0f)
#define MINIMAP_Y2		(MMBAR_HEIGHT*114.0f/128.0f)
#define TBAR_HEIGHT		15

#define HSCROLLER_SPACING	16
#define HSCROLLER_OPTIONW	173
#define HSCROLLER_SIDESP	(((14+17-7)+(20+18-11))/2)

#define STATUS_ALPHA	(0.75f)

#define WIDGET_IMAGE				1
#define WIDGET_BUTTON				2
#define WIDGET_TEXT					3
#define WIDGET_LINK					4
#define WIDGET_DROPDOWNSELECTOR		5 
#define WIDGET_EDITBOX				6
#define WIDGET_BARBUTTON			7
#define WIDGET_HSCROLLER			8
#define WIDGET_TOUCHLISTENER		9
#define WIDGET_TEXTBLOCK			10
#define WIDGET_CHECKBOX				11
#define WIDGET_INSDRAW				12
#define WIDGET_LISTBOX				13
#define WIDGET_TEXTAREA				14
#define WIDGET_VIEWPORT				15
#define WIDGET_FRAME				16

class Widget
{
public:
	int m_type;
	Widget* m_parent;
	float m_pos[4];
	float m_texc[4];	//texture coordinates
    float m_tpos[4];	//text pos
	unsigned int m_tex;
	unsigned int m_bgtex;
	unsigned int m_bgovertex;
	bool m_over;
	bool m_ldown;	//was the left mouse button pressed while over this (i.e. drag)?
	string m_name;
	RichText m_text;
	int m_font;
	unsigned int m_frametex, m_filledtex, m_uptex, m_downtex;
	bool m_opened;
	vector<RichText> m_options;
	int m_selected;
	float m_scroll[2];
	bool m_mousescroll;
	int m_mousedown[2];
	float m_vel[2];
	int m_param;
	float m_rgba[4];
	RichText m_value;
	int m_caret;
	bool m_passw;
	int m_maxlen;
	bool m_shadow;
	list<Widget*> m_subwidg;
	int m_lines;
	int m_alignment;
	RichText m_label;
	
	void (*clickfunc)();
	void (*clickfunc2)(int p);
	void (*overfunc)();
	void (*overfunc2)(int p);
	void (*outfunc)();
	void (*changefunc)();
	void (*changefunc2)(int p);
	void (*reframefunc)(Widget* thisw);

	Widget() 
	{
		clickfunc = NULL;
		clickfunc2 = NULL;
		overfunc = NULL;
		overfunc2 = NULL;
		outfunc = NULL;
		changefunc = NULL;
		changefunc2 = NULL;
		m_caret = 0;
		m_parent = NULL;
		m_opened = false;
		m_ldown = false;
		reframefunc = NULL;
	}

	virtual ~Widget()
	{
		for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
			delete *i;

		m_subwidg.clear();
	}
	
	virtual void draw()	{}
	virtual void draw2()	{}
	//virtual void newover()	{}
	virtual bool prelbuttonup(bool moved)	{ return false; }
	virtual bool lbuttonup(bool moved)	{ return false; }
	virtual bool prelbuttondown() { return false; }
	virtual bool lbuttondown() { return false; }
	virtual bool prerbuttonup(bool moved)	{ return false; }
	virtual bool rbuttonup(bool moved)	{ return false; }
	virtual bool prerbuttondown() { return false; }
	virtual bool rbuttondown() { return false; }
	virtual void premousemove()	{}
	virtual void frameupd()	{}
	virtual bool mousemove() { return false; }
	virtual bool keyup(int k) { return false; }
	virtual bool keydown(int k) { return false; }
	virtual bool charin(int k) { return false; }
	virtual bool mousewheel(int delta) { return false; }
	virtual void changevalue(const char* newv);
	virtual void placechar(unsigned int k);
	virtual void select(int which);
	virtual void clear();
	virtual void erase(int which)	{}
	virtual void reframe();	//resized or moved
	virtual void changetext(const char* newt);
	virtual int scrollframe(int i) const	{ return m_pos[i];	}
	virtual int frame(int i)	const { return m_pos[i];	}
	virtual Widget* getsubwidg(const char* name, int type)
	{
		for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
			if((*i)->m_type == type && _stricmp((*i)->m_name.c_str(), name) == 0)
				return *i;

		return NULL;
	}
	virtual bool delprev();
	virtual bool delnext();
	virtual void copyval() {}
	virtual void pasteval() {}
	virtual void selectall() {}
};

#endif