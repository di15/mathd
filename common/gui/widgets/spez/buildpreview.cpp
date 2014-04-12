

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
#include "buildpreview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../../../game/gui/gviewport.h"

BuildPreview::BuildPreview(Widget* parent, const char* n, void (*reframef)(Widget* thisw))
{
	m_parent = parent;
	m_type = WIDGET_BUILDPREVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

#if 0
	Image left_outer_topleftcorner;
	Image left_outer;
	
	Image right_outer_toprightcorner;
	Image right_outer;
	
	Image middle_outer_top;
	Image middle_outer_bottom;
	
	ViewportW viewport;

	Image white_bg;
#endif
	
#if 0
	restext = Text(this, "res ticker", RichText("asdadasdasads"), MAINFONT16, NULL, true, 1, 1, 1, 1);
	leftinnerdiagblur = Image(this, "gui/frames/innerdiagblur32x24halfwht.png", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	rightinnerdiagblur = Image(this, "gui/frames/innerdiagblur32x24halfwht.png", NULL, 1, 1, 1, 1,		1, 0, 0, 1);
	innerbottom = Image(this, "gui/frames/innerbottom3x3.png", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	lefthlineblur = Image(this, "gui/frames/innerhlineblur30x3.png", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	righthlineblur = Image(this, "gui/frames/innerhlineblur30x3.png", NULL, 1, 1, 1, 1,		1, 0, 0, 1);
	whitebg = Image(this, "gui/backg/white.jpg", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
#endif
	
	viewport = ViewportW(this, "viewport", NULL, &DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, VIEWPORT_ENTVIEW);
	
	left_outer_topleftcorner = Image(this, "gui/frames/outertopleft64x64.png", NULL, 1, 1, 1, 1,		1, 0, 0, 1);
	right_outer_toprightcorner = Image(this, "gui/frames/outertopleft64x64.png", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	middle_outer_top = Image(this, "gui/frames/outertop2x64.png", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	middle_outer_bottom = Image(this, "gui/frames/outertop2x64.png", NULL, 1, 1, 1, 1,		0, 1, 1, 0);

	white_bg = Image(this, "gui/backg/white.jpg", NULL, 1, 1, 1, 1,		0, 0, 1, 1);

	reframe();
}

void BuildPreview::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->reframe();



}

void BuildPreview::draw()
{

	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

#if 0
	whitebg.draw();
#endif

	middle_outer_top.draw();

	white_bg.draw();

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->draw();

#if 0
	restext.draw();

	//RichText rt = RichText(")A)JJF)@J)(J)(F$KJ(0jfjfjoi3jfwkjlekf");
	//DrawShadowedTextF(MAINFONT16, m_pos[0]+32, m_pos[1]+4, 0, 0, 50, 50, &rt);
	
	leftinnerdiagblur.draw();
	rightinnerdiagblur.draw();
	lefthlineblur.draw();
	righthlineblur.draw();
	//innerbottom.draw();
#endif
}

void BuildPreview::draw2()
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->draw2();
}

bool BuildPreview::prelbuttonup(bool moved)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->prelbuttonup(moved))
			return true;

	return false;
}

bool BuildPreview::lbuttonup(bool moved)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->lbuttonup(moved))
			return true;

	return false;
}

bool BuildPreview::prelbuttondown()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->prelbuttondown())
			return true;

	return false;
}

bool BuildPreview::lbuttondown()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->lbuttondown())
			return true;

	return false;
}

void BuildPreview::premousemove()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->premousemove();
}

bool BuildPreview::mousemove()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->mousemove())
			return true;

	return false;
}

bool BuildPreview::prerbuttonup(bool moved)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->prerbuttonup(moved))
			return true;

	return false;
}

bool BuildPreview::rbuttonup(bool moved)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->rbuttonup(moved))
			return true;

	return false;
}

bool BuildPreview::prerbuttondown()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->prerbuttondown())
			return true;

	return false;
}

bool BuildPreview::rbuttondown()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->rbuttondown())
			return true;

	return false;
}

bool BuildPreview::mousewheel(int delta) 
{ 
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->mousewheel(delta))
			return true;

	return false; 
}

void BuildPreview::frameupd()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->frameupd();
}

bool BuildPreview::keyup(int k)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->keyup(k))
			return true;

	return false;
}

bool BuildPreview::keydown(int k)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->keydown(k))
			return true;

	return false;
}

bool BuildPreview::charin(int k)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->charin(k))
			return true;

	return false;
}