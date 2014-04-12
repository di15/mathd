

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
#include "resticker.h"
#include "../../../platform.h"

ResTicker::ResTicker(Widget* parent, const char* n, void (*reframef)(Widget* thisw))
{
	m_parent = parent;
	m_type = WIDGET_RESTICKER;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

#if 0
	Text restext;
	Image leftinnerdiagblur;
	Image rightinnerdiagblur;
	Image innerbottom;
	Image lefthlineblur;
	Image righthlineblur;
	Image whitebg;
#endif
	
	restext = Text(this, "res ticker", RichText("asdadasdasads"), MAINFONT16, NULL, true, 1, 1, 1, 1);
	leftinnerdiagblur = Image(this, "gui/frames/innerdiagblur32x24halfwht.png", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	rightinnerdiagblur = Image(this, "gui/frames/innerdiagblur32x24halfwht.png", NULL, 1, 1, 1, 1,		1, 0, 0, 1);
	innerbottom = Image(this, "gui/frames/innerbottom3x3.png", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	lefthlineblur = Image(this, "gui/frames/innerhlineblur30x3.png", NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	righthlineblur = Image(this, "gui/frames/innerhlineblur30x3.png", NULL, 1, 1, 1, 1,		1, 0, 0, 1);
	whitebg = Image(this, "gui/backg/white.jpg", NULL, 1, 1, 1, 1,		0, 0, 1, 1);

	reframe();
}

void ResTicker::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->reframe();

	leftinnerdiagblur.m_pos[0] = m_pos[0];
	leftinnerdiagblur.m_pos[1] = m_pos[1];
	leftinnerdiagblur.m_pos[2] = m_pos[0]+32;
	leftinnerdiagblur.m_pos[3] = m_pos[1]+24;
	
	rightinnerdiagblur.m_pos[0] = m_pos[2]-32;
	rightinnerdiagblur.m_pos[1] = m_pos[1];
	rightinnerdiagblur.m_pos[2] = m_pos[2];
	rightinnerdiagblur.m_pos[3] = m_pos[1]+24;

	restext.m_pos[0] = m_pos[0]+32;
	restext.m_pos[1] = m_pos[1];
	restext.m_pos[2] = m_pos[2]-32;
	restext.m_pos[3] = m_pos[1]+24-3;

#if 1
	whitebg.m_pos[0] = m_pos[0]+32;
	whitebg.m_pos[1] = m_pos[1];
	whitebg.m_pos[2] = m_pos[2]-32;
	whitebg.m_pos[3] = m_pos[1]+24;
#else
	whitebg.m_pos[0] = m_pos[0];
	whitebg.m_pos[1] = m_pos[1];
	whitebg.m_pos[2] = m_pos[2];
	whitebg.m_pos[3] = m_pos[1]+24;
#endif

	innerbottom.m_pos[0] = m_pos[0]+32;
	innerbottom.m_pos[1] = m_pos[1]+24-3;
	innerbottom.m_pos[2] = m_pos[2]-32;
	innerbottom.m_pos[3] = m_pos[1]+24;

	float centerw = (m_pos[0]+m_pos[2])/2;

	lefthlineblur.m_pos[0] = m_pos[0]+32;
	lefthlineblur.m_pos[1] = m_pos[1]+24-3;
	lefthlineblur.m_pos[2] = centerw;
	lefthlineblur.m_pos[3] = m_pos[1]+24;
	
	righthlineblur.m_pos[0] = centerw;
	righthlineblur.m_pos[1] = m_pos[1]+24-3;
	righthlineblur.m_pos[2] = m_pos[2]-32;
	righthlineblur.m_pos[3] = m_pos[1]+24;
}

void ResTicker::draw()
{
#if 0
	Text restext;
	Image leftinnerdiagblur;
	Image rightinnerdiagblur;
	Image innerbottom;
	Image lefthlineblur;
	Image righthlineblur;
	Image whitebg;
#endif

	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	whitebg.draw();

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->draw();

	restext.draw();

	//RichText rt = RichText(")A)JJF)@J)(J)(F$KJ(0jfjfjoi3jfwkjlekf");
	//DrawShadowedTextF(MAINFONT16, m_pos[0]+32, m_pos[1]+4, 0, 0, 50, 50, &rt);
	
	leftinnerdiagblur.draw();
	rightinnerdiagblur.draw();
	lefthlineblur.draw();
	righthlineblur.draw();
	//innerbottom.draw();
}

void ResTicker::draw2()
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->draw2();
}

bool ResTicker::prelbuttonup(bool moved)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->prelbuttonup(moved))
			return true;

	return false;
}

bool ResTicker::lbuttonup(bool moved)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->lbuttonup(moved))
			return true;

	return false;
}

bool ResTicker::prelbuttondown()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->prelbuttondown())
			return true;

	return false;
}

bool ResTicker::lbuttondown()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->lbuttondown())
			return true;

	return false;
}

void ResTicker::premousemove()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->premousemove();
}

bool ResTicker::mousemove()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->mousemove())
			return true;

	return false;
}

bool ResTicker::prerbuttonup(bool moved)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->prerbuttonup(moved))
			return true;

	return false;
}

bool ResTicker::rbuttonup(bool moved)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->rbuttonup(moved))
			return true;

	return false;
}

bool ResTicker::prerbuttondown()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->prerbuttondown())
			return true;

	return false;
}

bool ResTicker::rbuttondown()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->rbuttondown())
			return true;

	return false;
}

bool ResTicker::mousewheel(int delta) 
{ 
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->mousewheel(delta))
			return true;

	return false; 
}

void ResTicker::frameupd()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->frameupd();
}

bool ResTicker::keyup(int k)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->keyup(k))
			return true;

	return false;
}

bool ResTicker::keydown(int k)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->keydown(k))
			return true;

	return false;
}

bool ResTicker::charin(int k)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		if((*i)->charin(k))
			return true;

	return false;
}