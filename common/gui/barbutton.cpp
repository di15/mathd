

#include "widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "dropdowns.h"
#include "hscroller.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"
#include "gui.h"

BarButton::BarButton(Widget* parent, unsigned int sprite, float bar, void (*reframef)(Widget* thisw), void (*click)(), void (*overf)(), void (*out)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_BARBUTTON;
	m_over = false;
	m_ldown = false;
	m_tex = sprite;
	CreateTexture(m_bgtex, "gui\\buttonbg.png", true);
	CreateTexture(m_bgovertex, "gui\\buttonbgover.png", true);
	reframefunc = reframef;
	m_healthbar = bar;
	clickfunc = click;
	overfunc = overf;
	outfunc = out;
	reframe();
}

void BarButton::draw()
{
	if(m_over)
		DrawImage(g_texture[m_bgovertex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);
	else
		DrawImage(g_texture[m_bgtex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

	DrawImage(g_texture[m_tex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

	UseS(SHADER_COLOR2D);
    glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_currw);
    glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_currh);
	DrawSquare(1, 0, 0, 1, m_pos[0], m_pos[3]-5, m_pos[2], m_pos[3]);
	float bar = (m_pos[2] - m_pos[0]) * m_healthbar;
	DrawSquare(0, 1, 0, 1, m_pos[0], m_pos[3]-5, m_pos[0]+bar, m_pos[3]);
	
	Ortho(g_currw, g_currh, 1, 1, 1, 1);
}

bool BarButton::mousemove()
{
	if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
	{
		if(overfunc != NULL)
			overfunc();
			
		m_over = true;

		return true;
	}
	else
	{
		if(m_over && outfunc != NULL)
			outfunc();

		m_over = false;

		return false;
	}

	return false;
}

