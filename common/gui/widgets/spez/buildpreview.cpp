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

//viewport
void Resize_BP_VP(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 10;
#if 0
	thisw->m_pos[2] = (parw->m_pos[0]+parw->m_pos[2])/2;
	thisw->m_pos[3] = (parw->m_pos[1]+parw->m_pos[3])/2;
#else
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[3] - 10;
#endif
}

//title
void Resize_BP_Tl(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 10;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[3] - 10;
}

//params text block: construction material
void Resize_BP_PB_C(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 32 + 20;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[1] + 32 + 10 + 100;
}

//inputs
void Resize_BP_PB_I(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 32 + 10 + 100;
	thisw->m_pos[2] = (parw->m_pos[0]+parw->m_pos[2])/2;
	thisw->m_pos[3] = parw->m_pos[1] + 32 + 10 + 200;
}

//outputs
void Resize_BP_PB_O(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = (parw->m_pos[0]+parw->m_pos[2])/2;
	thisw->m_pos[1] = parw->m_pos[1] + 32 + 10 + 100;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[1] + 32 + 10 + 200;
}

//description block
void Resize_BP_Ds(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 32 + 10 + 200;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[3] - 10;
}

BuildPreview::BuildPreview(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : WindowW(parent, n, reframef)
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

#if 0
	viewport = ViewportW(this, "viewport", NULL, &DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, VIEWPORT_ENTVIEW);

	left_outer_topleftcorner = Image(this, "gui/frames/outertopleft64x64.png", true, NULL, 1, 1, 1, 1,		1, 0, 0, 1);
	right_outer_toprightcorner = Image(this, "gui/frames/outertopleft64x64.png", true, NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	middle_outer_top = Image(this, "gui/frames/outertop2x64.png", true, NULL, 1, 1, 1, 1,		0, 0, 1, 1);
	middle_outer_bottom = Image(this, "gui/frames/outertop2x64.png", true, NULL, 1, 1, 1, 1,		0, 1, 1, 0);

	white_bg = Image(this, "gui/backg/white.jpg", true, NULL, 1, 1, 1, 1,		0, 0, 1, 1);

	title_text = Text(this, "title", RichText("Buildings"), MAINFONT32, NULL, true, 1, 1, 1, 1);
	params_block = TextBlock(this, "params block", RichText("params block"), MAINFONT16, NULL, 1, 1, 1, 1);
	desc_block = TextBlock(this, "desc block", RichText("desc block"), MAINFONT16, NULL, 1, 1, 1, 1);
#endif

	m_subwidg.push_back(new  ViewportW(this, "viewport", Resize_BP_VP, &DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, VIEWPORT_ENTVIEW));
	m_subwidg.push_back(new Text(this, "title", RichText("Buildings"), MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));
	m_subwidg.push_back(new TextBlock(this, "conmat block", RichText("conmat block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_C, 1, 0, 0, 1));
	m_subwidg.push_back(new TextBlock(this, "input block", RichText("input block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_I, 0, 1, 0, 1));
	m_subwidg.push_back(new TextBlock(this, "output block", RichText("output block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_O, 1, 0, 1, 1));
	m_subwidg.push_back(new TextBlock(this, "desc block", RichText("description block \n1 \n2 \n3"), MAINFONT16, Resize_BP_Ds, 0, 1, 1, 1));
	
	if(reframefunc)
		reframefunc(this);

	reframe();
}

#if 0
void BuildPreview::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

	viewport.m_pos[0] = m_pos[0] + 10;
	viewport.m_pos[1] = m_pos[1] + 10;
	viewport.m_pos[2] = (m_pos[0]+m_pos[2])/2;
	viewport.m_pos[3] = (m_pos[1]+m_pos[3])/2;

	white_bg.m_pos[0] = m_pos[0];
	white_bg.m_pos[1] = m_pos[1];
	white_bg.m_pos[2] = m_pos[2];
	white_bg.m_pos[3] = m_pos[3];

	title_text.m_pos[0] = m_pos[0] + 10;
	title_text.m_pos[1] = m_pos[1] + 10;
	title_text.m_pos[2] = m_pos[2] - 10;
	title_text.m_pos[3] = m_pos[1] + 64;

	params_block.m_pos[0] = (m_pos[0]+m_pos[2])/2;
	params_block.m_pos[1] = m_pos[1] + 10;
	params_block.m_pos[2] = m_pos[2] - 10;
	params_block.m_pos[3] = (m_pos[1]+m_pos[3])/2;

	desc_block.m_pos[0] = m_pos[0] + 10;
	desc_block.m_pos[1] = (m_pos[1]+m_pos[3])/2;
	desc_block.m_pos[2] = m_pos[2] - 10;
	desc_block.m_pos[3] = m_pos[3] - 10;

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

	viewport.draw();

	title_text.draw();
	params_block.draw();
	desc_block.draw();

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

void BuildPreview::drawover()
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->drawover();
}

void BuildPreview::frameupd()
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->frameupd();
}

void BuildPreview::inev(InEv* ev)
{
	for(auto i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->inev(ev);
}
#endif
