



#include "main.h"
#include "shader.h"
#include "gui.h"
#include "image.h"
#include "font.h"
#include "3dmath.h"
#include "map.h"
#include "building.h"
#include "menu.h"
#include "selection.h"
#include "chat.h"
#include "billboard.h"
#include "tileset.h"
#include "editor.h"
#include "shadow.h"
#include "particle.h"
#include "billboard.h"
#include "numpad.h"
#include "player.h"
#include "powerline.h"
#include "pipeline.h"
#include "road.h"
#include "minimap.h"
#include "script.h"
#include "inspect.h"
#include "scenery.h"

CGUI g_GUI;
bool g_keyintercepted = false;

void OrthoMode(int left, int top, int right, int bottom)
{
	glMatrixMode(GL_PROJECTION);						
	glPushMatrix();									
	glLoadIdentity();								
	glOrtho(left, right, bottom, top, 0, 1);	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();		
	glDisable(GL_DEPTH_TEST);
}

void PerspectiveMode()
{
	glEnable(GL_DEPTH_TEST);
	glMatrixMode( GL_PROJECTION );							
	glPopMatrix();											
	glMatrixMode( GL_MODELVIEW );						
}

/*
inline void DrawImage(unsigned int tex, float left, float top, float right, float bottom)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);		glVertex2f(left, top);
	glTexCoord2f(0, 1);		glVertex2f(left, bottom);
	glTexCoord2f(1, 1);		glVertex2f(right, bottom);
	glTexCoord2f(1, 0);		glVertex2f(right, top);
	glEnd();
}*/

void DrawImage(unsigned int tex, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom)
{
    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1iARB(g_shader[SHADER::ORTHO].m_slot[SLOT::TEXTURE0], 0);
    
    float vertices[] =
    {
        //posx, posy    texx, texy
        left, top,0,          texleft, textop,
        right, top,0,         texright, textop,
        right, bottom,0,      texright, texbottom,
        
        right, bottom,0,      texright, texbottom,
        left, bottom,0,       texleft, texbottom,
        left, top,0,          texleft, textop
    };
    
    glVertexAttribPointer(g_shader[SHADER::ORTHO].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
    glVertexAttribPointer(g_shader[SHADER::ORTHO].m_slot[SLOT::TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[3]);
		
	//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
	//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

inline void DrawSquare(float r, float g, float b, float a, float left, float top, float right, float bottom)
{
	//return;
	/*
	glDisable(GL_TEXTURE_2D);
	glColor4f(r, g, b, a);

	glBegin(GL_QUADS);
	glVertex2f(left, top);
	glVertex2f(left, bottom);
	glVertex2f(right, bottom);
	glVertex2f(right, top);
	glEnd();

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);*/

    glUniform4fARB(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], r, g, b, a);
    
    float vertices[] =
    {
        //posx, posy    texx, texy
        left, top,0,          0, 0,
        right, top,0,         1, 0,
        right, bottom,0,      1, 1,
        
        right, bottom,0,      1, 1,
        left, bottom,0,       0, 1,
        left, top,0,          0, 0
    };
    
    glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
		
	//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
	//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void CWidget::Image(const char* nm, const char* filepath, float left, float top, float right, float bottom, float r, float g, float b, float a, float texleft, float textop, float texright, float texbottom)
{
	type = IMAGE;
	name = nm;
	//CreateTexture(tex, filepath, true);
	CreateTexture(tex, filepath, false);
	//CreateTexture(tex, filepath, clamp);
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	texc[0] = texleft;
	texc[1] = textop;
	texc[2] = texright;
	texc[3] = texbottom;
	ldown = false;
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
}

void CWidget::Image(const char* filepath, float left, float top, float right, float bottom, float r, float g, float b, float a, float texleft, float textop, float texright, float texbottom)
{
	type = IMAGE;
	//CreateTexture(tex, filepath, true);
	CreateTexture(tex, filepath, false);
	//CreateTexture(tex, filepath, clamp);
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	texc[0] = texleft;
	texc[1] = textop;
	texc[2] = texright;
	texc[3] = texbottom;
	ldown = false;
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
}

void CWidget::Image(unsigned int t, float left, float top, float right, float bottom, float r, float g, float b, float a, float texleft, float textop, float texright, float texbottom)
{
	type = IMAGE;
	tex = t;
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	texc[0] = texleft;
	texc[1] = textop;
	texc[2] = texright;
	texc[3] = texbottom;
	ldown = false;
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
}

void CWidget::InsDraw(void (*inst)())
{
	type = INSDRAW;
	clickfunc = inst;
	ldown = false;
}

void CWidget::Button(const char* n, const char* filepath, const char* t, int f, float left, float top, float right, float bottom, void (*click)(), void (*overf)(), void (*out)())
{
	type = BUTTON;
	name = n;
    text = t;
    font = f;
    float length = 0;
    for(int i=0; i<strlen(t); i++)
        length += g_font[f].glyph[t[i]].w;
    tpos[0] = (left+right)/2.0f - length/2.0f;
    tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	over = false;
	ldown = false;
	CreateTexture(tex, filepath, true);
	CreateTexture(bgtex, "gui\\buttonbg.png", true);
	CreateTexture(bgovertex, "gui\\buttonbgover.png", true);
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	clickfunc = click;
	overfunc = overf;
	clickfunc2 = NULL;
	overfunc2 = NULL;
	outfunc = out;
}

void CWidget::Button(const char* filepath, const char* t, int f, float left, float top, float right, float bottom, void (*click)(), void (*overf)(), void (*out)())
{
	type = BUTTON;
    text = t;
    font = f;
    float length = 0;
    for(int i=0; i<strlen(t); i++)
        length += g_font[f].glyph[t[i]].w;
    tpos[0] = (left+right)/2.0f - length/2.0f;
    tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	over = false;
	ldown = false;
	CreateTexture(tex, filepath, true);
	CreateTexture(bgtex, "gui\\buttonbg.png", true);
	CreateTexture(bgovertex, "gui\\buttonbgover.png", true);
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	clickfunc = click;
	overfunc = overf;
	clickfunc2 = NULL;
	overfunc2 = NULL;
	outfunc = out;
}

void CWidget::Button(const char* filepath, const char* t, int f, float left, float top, float right, float bottom, void (*click2)(int p), int parm)
{
	type = BUTTON;
    text = t;
    font = f;
    float length = 0;
    for(int i=0; i<strlen(t); i++)
        length += g_font[f].glyph[t[i]].w;
    tpos[0] = (left+right)/2.0f - length/2.0f;
    tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	over = false;
	ldown = false;
	CreateTexture(tex, filepath, true);
	CreateTexture(bgtex, "gui\\buttonbg.png", true);
	CreateTexture(bgovertex, "gui\\buttonbgover.png", true);
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = click2;
	overfunc2 = NULL;
	outfunc = NULL;
	param = parm;
}

void CWidget::Button(const char* filepath, const char* t, int f, float left, float top, float right, float bottom, void (*click2)(int p), void (*overf2)(int p), void (*out)(), int parm)
{
	type = BUTTON;
    text = t;
    font = f;
    float length = 0;
    for(int i=0; i<strlen(t); i++)
        length += g_font[f].glyph[t[i]].w;
    tpos[0] = (left+right)/2.0f - length/2.0f;
    tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	over = false;
	ldown = false;
	CreateTexture(tex, filepath, true);
	CreateTexture(bgtex, "gui\\buttonbg.png", true);
	CreateTexture(bgovertex, "gui\\buttonbgover.png", true);
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = click2;
	overfunc2 = overf2;
	outfunc = out;
	param = parm;
}

void CWidget::TouchListener(float left, float top, float right, float bottom, void (*click2)(int p), void (*overf2)(int p), void (*out)(), int parm)
{
	type = TOUCHLISTENER;
	over = false;
	ldown = false;
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = click2;
	overfunc2 = overf2;
	outfunc = out;
	param = parm;
}

void CWidget::HScroller(float top, float bottom, void (*click2)(int p), void (*out)())
{
	type = HSCROLLER;
	over = false;
	ldown = false;
	vel[0] = 0;
	vel[1] = 0;
	pos[0] = HSCROLLER_SIDESP;
	pos[1] = top;
	pos[2] = HSCROLLER_SIDESP;
	pos[3] = bottom;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = click2;
	outfunc = out;
}

void CWidget::CheckBox(const char* n, const char* t, int f, float left, float top, float right, float bottom, int sel, float r, float g, float b, float a, void (*change)())
{
	type = CHECKBOX;
	name = n;
	text = t;
	font = f;
	pos[0] = left;
	pos[1] = top;
    pos[2] = right;
    pos[3] = bottom;
	ldown = false;
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
	selected = sel;
	changefunc = change;
	CreateTexture(frametex, "gui\\frame.jpg", true);
	CreateTexture(filledtex, "gui\\accept.png", true);
}

void CWidget::DropDown(const char* n, int f, float left, float top, float width, void (*change)())
{
	type = DROPDOWN;
	name = n;
	font = f;
	opened = false;
	selected = 0;
	scroll = 0;
	mousescroll = false;
	ldown = false;
	changefunc = change;
	pos[0] = left;
	pos[1] = top;
	pos[2] = left+width;
	pos[3] = top+g_font[f].gheight;
	CreateTexture(frametex, "gui\\frame.jpg", true);
	CreateTexture(filledtex, "gui\\filled.jpg", true);
	CreateTexture(uptex, "gui\\up.jpg", true);
	CreateTexture(downtex, "gui\\down.jpg", true);
}

void CWidget::ListBox(const char* n, int f, float left, float top, float right, float bottom, void (*change)())
{
	type = LISTBOX;
	name = n;
	font = f;
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	opened = false;
	selected = -1;
	scroll = 0;
	mousescroll = false;
	ldown = false;
	changefunc = change;
	CreateTexture(frametex, "gui\\frame.jpg", true);
	CreateTexture(filledtex, "gui\\filled.jpg", true);
	CreateTexture(uptex, "gui\\up.jpg", true);
	CreateTexture(downtex, "gui\\down.jpg", true);
}

void CWidget::TextArea(const char* n, const char* t, int f, float left, float top, float right, float bottom, float r, float g, float b, float a, void (*change)())
{
	type = TEXTAREA;
	name = n;
	value = t;
	font = f;
	pos[0] = left;
	pos[1] = top;
    pos[2] = right;
    pos[3] = bottom;
	ldown = false;
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
	changefunc = change;
	scroll = 0;
	caret = strlen(t);
	lines = CountLines(t, f, left, top, right-left-square(), bottom-top);
	CreateTexture(frametex, "gui\\frame.jpg", true);
	CreateTexture(filledtex, "gui\\filled.jpg", true);
	CreateTexture(uptex, "gui\\up.jpg", true);
	CreateTexture(downtex, "gui\\down.jpg", true);
}

void CWidget::BarButton(unsigned int sprite, float bar, float left, float top, float right, float bottom, void (*click)(), void (*overf)(), void (*out)())
{
	type = BARBUTTON;
	over = false;
	ldown = false;
	tex = sprite;
	CreateTexture(bgtex, "gui\\buttonbg.png", true);
	CreateTexture(bgovertex, "gui\\buttonbgover.png", true);
	pos[0] = left;
	pos[1] = top;
	pos[2] = right;
	pos[3] = bottom;
	healthbar = bar;
	clickfunc = click;
	overfunc = overf;
	outfunc = out;
}

void CWidget::EditBox(const char* n, int f, const char* t, float left, float top, float width, bool pw, int maxl, void (*change2)(int p), int parm)
{
	type = EDITBOX;
	name = n;
	font = f;
	value = t;
	caret = strlen(t);
	opened = false;
	passw = pw;
	maxlen = maxl;
	pos[0] = left;
	pos[1] = top;
	pos[2] = left+width;
	pos[3] = top+g_font[f].gheight;
	CreateTexture(frametex, "gui\\frame.jpg", true);
	param = parm;
	changefunc2 = change2;
}

void CWidget::Text_draw()
{
	//glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
	//glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 0.8f, 0.8f, 0.8f, 1.0f);
	//float color[] = {0.8f, 0.8f, 0.8f, 1.0f};
	//DrawShadowedText(font, pos[0], pos[1], text.c_str(), color);
	if(shadow)
		DrawShadowedText(font, pos[0], pos[1], text.c_str(), rgba);
	else
	{
		glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], rgba[0], rgba[1], rgba[2], rgba[3]);
		DrawLine(font, pos[0], pos[1], text.c_str());
	}
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
}

void CWidget::TextBox_draw()
{
    float width = pos[2] - pos[0];
    float height = pos[3] - pos[1];
    
    DrawBoxShadText(font, pos[0], pos[1], width, height, text.c_str(), rgba, 0, -1);
	
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
}

void CWidget::TextArea_draw()
{
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	DrawImage(frametex, pos[0], pos[1], pos[2], pos[3]);
	
	DrawImage(frametex, pos[2]-square(), pos[1], pos[2], pos[3]);
	DrawImage(uptex, pos[2]-square(), pos[1], pos[2], pos[1]+square());
	DrawImage(downtex, pos[2]-square(), pos[3]-square(), pos[2], pos[3]);
	DrawImage(filledtex, pos[2]-square(), pos[1]+square()+scrollspace()*topratio(), pos[2], pos[1]+square()+scrollspace()*bottomratio());

    float width = pos[2] - pos[0] - square();
    float height = pos[3] - pos[1];
    
    DrawBoxShadText(font, pos[0], pos[1], width, height, value.c_str(), rgba, scroll, opened ? caret : -1);
}

void CWidget::Link_draw()
{
	glDisable(GL_TEXTURE_2D);

	float color[] = {1, 1, 1, 1};

	if(over)
	{
		//glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		//glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
		//glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 0.8f, 0.8f, 0.8f, 1.0f);
		color[0] = 0.8f;
		color[1] = 0.8f;
		color[2] = 0.8f;
	}

	DrawShadowedText(font, pos[0], pos[1], text.c_str(), color);

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	//glEnable(GL_TEXTURE_2D);
}

void CWidget::BarButton_draw()
{
	if(over)
		DrawImage(bgovertex, pos[0], pos[1], pos[2], pos[3]);
	else
		DrawImage(bgtex, pos[0], pos[1], pos[2], pos[3]);

	DrawImage(tex, pos[0], pos[1], pos[2], pos[3]);

	Use(COLOR2D);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::HEIGHT], (float)g_height);
	DrawSquare(1, 0, 0, 1, pos[0], pos[3]-5, pos[2], pos[3]);
	float bar = (pos[2] - pos[0]) * healthbar;
	DrawSquare(0, 1, 0, 1, pos[0], pos[3]-5, pos[0]+bar, pos[3]);
	Use(ORTHO);
}

void CWidget::HScroller_draw()
{

	for(int i=0; i<subwidg.size(); i++)
	{
		subwidg[i].draw();
	}
}

bool CWidget::HScroller_checkover()
{
	over = false;

	if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[3])
	{
		over = true;
		return true;
	}

	return false;
}

void CWidget::HScroller_mousemove()
{
	HScroller_checkover();

	if(ldown)
	{
		vel[0] = g_mouse.x - mousedown[0];
		//vel[1] = g_mouse.y - mousedown[1];
		mousedown[0] = g_mouse.x;
		//mousedown[1] = g_mouse.y;
		movesub();
	}
	else
	{
		for(int i=subwidg.size()-1; i>=0; i--)
			subwidg[i].mousemove();
	}
}

void CWidget::Button_mousemove()
{
	if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[3])
	{
		if(overfunc != NULL)
			overfunc();
		if(overfunc2 != NULL)
			overfunc2(param);
			
		over = true;
	}
	else
	{
		if(over && outfunc != NULL)
			outfunc();

		over = false;
	}
}

void CWidget::BarButton_mousemove()
{
	if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[3])
	{
		if(overfunc != NULL)
			overfunc();
			
		over = true;
	}
	else
	{
		if(over && outfunc != NULL)
			outfunc();

		over = false;
	}
}

void CWidget::Link_mousemove()
{
	if(g_mouse.x >= pos[0] && g_mouse.y >= pos[1] && 
		g_mouse.x <= pos[0]+strlen(text.c_str())*g_font[font].gheight/2 && 
		g_mouse.y <= pos[1]+g_font[font].gheight)
		over = true;
	else
		over = false;
}

void CWidget::CheckBox_mousemove()
{
	if(g_mouse.x >= pos[0] && g_mouse.y >= pos[1] && 
		g_mouse.x <= pos[2] && 
		g_mouse.y <= pos[3])
		over = true;
	else
		over = false;
}

void CWidget::select(int which)
{
	selected = which;

	if(selected >= options.size())
		selected = options.size()-1;

	if(selected < 0)
		selected = 0;
}

void CWidget::clear()
{
	selected = -1;
	scroll = 0;
	options.clear();
	text = "";
}

void CWidget::erase(int which)
{
	options.erase( options.begin() + which );
	if(selected == which)
		selected = -1;

	if(scroll + rowsshown() > options.size())
		scroll = options.size() - rowsshown();

	if(scroll < 0)
		scroll = 0;
}

void CWidget::movesub()
{
	float changex = 0;
	if(vel[0] < 0.0f)
	{
		changex = max(vel[0], g_width-HSCROLLER_SIDESP-pos[2]);
		if(changex > vel[0])
			vel[0] = 0;
	}
	else if(vel[0] > 0.0f)
	{
		changex = min(vel[0], HSCROLLER_SIDESP-pos[0]);
		if(changex < vel[0])
			vel[0] = 0;
	}
	else
		return;

	vel[0] *= 0.9f;

	pos[0] += changex;
	pos[2] += changex;

	CWidget* sw;
	for(int i=0; i<subwidg.size(); i++)
	{
		sw = &subwidg[i];
		sw->pos[0] += changex;
		sw->pos[2] += changex;
		sw->tpos[0] += changex;
	}
}

int CWidget::rowsshown()
{
	int rows;
		
	if(type == DROPDOWN)
	{
		rows = MAX_OPTIONS_SHOWN;
	}
	else if(type == LISTBOX || type == TEXTAREA)
	{
		rows = (pos[3]-pos[1])/g_font[font].gheight;
	}
		
	if(rows > options.size() && type != TEXTAREA)
		rows = options.size();

	return rows;
}

int CWidget::square()
{
	return g_font[font].gheight;
}

float CWidget::scrollspace()
{
	if(type == DROPDOWN)
		return g_font[font].gheight*(rowsshown())-square();
	else if(type == LISTBOX)
		return (pos[3]-pos[1]-square()*2);
	else
		return 0;
}

void CWidget::changetext(const char* newt)
{
	text = newt;

	if(type == BUTTON)
	{
		float length = 0;
		for(int i=0; i<strlen(newt); i++)
			length += g_font[font].glyph[newt[i]].w;
		tpos[0] = (pos[0]+pos[2])/2.0f - length/2.0f;
		tpos[1] = (pos[1]+pos[3])/2.0f - g_font[font].gheight/2.0f;
	}
}

void CWidget::HScroller_frame()
{
	if(ldown)
	{
	}
	else
	{
		movesub();
	}
}

void CWidget::Image_draw()
{
	//glColor4fv(rgba);
	glUniform4fv(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, rgba);
	DrawImage(tex, pos[0], pos[1], pos[2], pos[3], texc[0], texc[1], texc[2], texc[3]);
	//glColor4f(1,1,1,1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
}

void CWidget::InsDraw_draw()
{
	if(clickfunc != NULL)
		clickfunc();
}

void CWidget::Button_draw()
{
	if(over)
		DrawImage(bgovertex, pos[0], pos[1], pos[2], pos[3]);
	else
		DrawImage(bgtex, pos[0], pos[1], pos[2], pos[3]);

	DrawImage(tex, pos[0], pos[1], pos[2], pos[3]);
    
	DrawShadowedText(font, tpos[0], tpos[1], text.c_str());
}

void CWidget::CheckBox_draw()
{
	DrawImage(frametex, pos[0], pos[1], pos[0]+square(), pos[1]+square());

	if(selected > 0)
		DrawImage(filledtex, pos[0], pos[1], pos[0]+square(), pos[1]+square());

	DrawShadowedText(font, pos[0]+square()+5, pos[1], text.c_str());
}

void CWidget::DropDown_draw()
{
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	DrawImage(frametex, pos[0], pos[1], pos[2], pos[3]);

	if(!opened)
		DrawImage(downtex, pos[2]-square(), pos[1], pos[2], pos[1]+square());

	if(options.size() <= 0)
		return;

	if(selected < 0)
		return;

	if(selected >= options.size())
		return;

	DrawShadowedText(font, pos[0]+3, pos[1], options[selected].c_str());
}

void CWidget::DropDown_draw2()
{
	if(!opened)
		return;

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	DrawImage(frametex, pos[0], pos[1]+g_font[font].gheight, pos[2], pos[3]+g_font[font].gheight*rowsshown());
	DrawImage(frametex, pos[2]-square(), pos[1], pos[2], pos[3]+g_font[font].gheight*rowsshown());
	DrawImage(uptex, pos[2]-square(), pos[1], pos[2], pos[1]+square());
	DrawImage(downtex, pos[2]-square(), pos[3]+g_font[font].gheight*rowsshown()-square(), pos[2], pos[3]+g_font[font].gheight*rowsshown());
	DrawImage(filledtex, pos[2]-square(), pos[3]+scrollspace()*topratio(), pos[2], pos[3]+scrollspace()*bottomratio());

	for(int i=(int)scroll; i<(int)scroll+rowsshown(); i++)
		DrawShadowedText(font, pos[0]+3, pos[3]+g_font[font].gheight*(i-(int)scroll), options[i].c_str());
}

void CWidget::ListBox_draw()
{
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	CFont* f = &g_font[font];
	int rows = rowsshown();

	DrawImage(frametex, pos[0], pos[1], pos[2], pos[3]);

	DrawImage(frametex, pos[2]-square(), pos[1], pos[2], pos[3]);
	DrawImage(uptex, pos[2]-square(), pos[1], pos[2], pos[1]+square());
	DrawImage(downtex, pos[2]-square(), pos[3]-square(), pos[2], pos[3]);
	DrawImage(filledtex, pos[2]-square(), pos[1]+square()+scrollspace()*topratio(), pos[2], pos[1]+square()+scrollspace()*bottomratio());

	if(selected >= 0 && selected >= (int)scroll && selected < (int)scroll+rowsshown())
	{
		glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 0.5f);
		DrawImage(filledtex, pos[0], pos[1]+(selected-(int)scroll)*f->gheight, pos[2]-square(), pos[1]+(selected-(int)scroll+1)*f->gheight);
		glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	}
	
	for(int i=(int)scroll; i<(int)scroll+rowsshown(); i++)
		DrawShadowedText(font, pos[0]+3, pos[1]+g_font[font].gheight*(i-(int)scroll), options[i].c_str());
}

void CWidget::EditBox_draw()
{
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);

	DrawImage(frametex, pos[0], pos[1], pos[2], pos[3]);

	if(over)
		//glColor4f(1, 1, 1, 1);
		glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	else
		//glColor4f(0.8f, 0.8f, 0.8f, 1);
		glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 0.8f, 0.8f, 0.8f, 1);

	string val = value;

	if(passw)
	{
		val = "";
		for(int i=0; i<value.length(); i++)
			val.append("*");
	}

	if(opened)
	{
		int len = val.length();

		if(caret > len)
			caret = len;

		string out = val.substr(0, caret);
		out.append("|");
		out.append(val.substr(caret, len-caret));

		DrawShadowedText(font, pos[0]+10, pos[1], out.c_str());
	}
	else
		DrawShadowedText(font, pos[0]+10, pos[1], val.c_str());

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
}

void CWidget::DropDown_mousemove()
{
	if(!mousescroll)
		return;

	int dy = g_mouse.y - mousedown[1];/*
	int topspace = topratio() * scrollspace();
	int bottomspace = scrollspace() - bottomratio() * scrollspace();

	if(dy < 0 && abs(dy) > topspace)
	{
		scroll = 0;
		return;
	}
	else if(dy > 0 && dy > bottomspace)
	{
		scroll = options.size()-rowsshown();
		if(scroll < 0)
			scroll = 0;

		//char msg[128];
		//sprintf(msg, "bottomspace=%d", bottomspace);
		//MessageBox(NULL, msg, " ", NULL);

		return;
	}
	*/
	float topy = pos[3]+square()+scrollspace()*topratio();
	float newtopy = topy + dy;

	//topratio = (float)scroll / (float)(options.size());
	//topy = pos[3]+square+scrollspace*topratio
	//topy = pos[3]+square+scrollspace*((float)scroll / (float)(options.size()))
	//topy - pos[3] - square = scrollspace*(float)scroll / (float)(options.size())
	//(topy - pos[3] - square)*(float)(options.size())/scrollspace = scroll

	scroll = (newtopy - pos[3] - square())*(float)(options.size())/scrollspace();

	if(scroll < 0)
	{
		scroll = 0;
		return;
	}
	else if(scroll + rowsshown() > options.size())
	{
		scroll = options.size() - rowsshown();
		return;
	}

	mousedown[1] = g_mouse.y;
}

void CWidget::ListBox_mousemove()
{
	if(!mousescroll)
		return;

	int dy = g_mouse.y - mousedown[1];

	float topy = pos[3]+square()+scrollspace()*topratio();
	float newtopy = topy + dy;

	//topratio = (float)scroll / (float)(options.size());
	//topy = pos[3]+square+scrollspace*topratio
	//topy = pos[3]+square+scrollspace*((float)scroll / (float)(options.size()))
	//topy - pos[3] - square = scrollspace*(float)scroll / (float)(options.size())
	//(topy - pos[3] - square)*(float)(options.size())/scrollspace = scroll

	scroll = (newtopy - pos[3] - square())*(float)(options.size())/scrollspace();

	if(scroll < 0)
	{
		scroll = 0;
		return;
	}
	else if(scroll + rowsshown() > options.size())
	{
		scroll = options.size() - rowsshown();
		return;
	}

	mousedown[1] = g_mouse.y;
}

void CWidget::EditBox_mousemove()
{
	if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[3])
		over = true;
	else
		over = false;
}

void CWidget::TextArea_mousemove()
{
	if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[3])
		over = true;
	else
		over = false;
}

bool CWidget::HScroller_lbuttondown()
{
	if(over)
	{
		/*
		for(int i=subwidg.size()-1; i>=0; i--)
		{
			if(subwidg[i].lbuttondown())
				return true;
		}*/
			
		ldown = true;
		mousedown[0] = g_mouse.x;
		mousedown[1] = g_mouse.y;
		
		return true;	// intercept mouse event
	}
	else
	{
		ldown = false;
	}
	
	return false;
}

bool CWidget::CheckBox_lbuttondown()
{
	if(over)
	{
		ldown = true;
		return true;	// intercept mouse event
	}

	return false;
}

bool CWidget::DropDown_prelbuttondown()
{
	if(!opened)
		return false;

	for(int i=(int)scroll; i<(int)scroll+rowsshown(); i++)
	{
		// list item?
		if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2]-square() && g_mouse.y >= pos[3]+g_font[font].gheight*(i-(int)scroll)
			&& g_mouse.y <= pos[3]+g_font[font].gheight*(i-(int)scroll+1))
		{
			ldown = true;
			return true;	// intercept mouse event
		}
	}

	// scroll bar?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[3]+scrollspace()*topratio() && g_mouse.x <= pos[2] && 
			g_mouse.y <= pos[3]+scrollspace()*bottomratio())
	{
		ldown = true;
		mousescroll = true;
		mousedown[1] = g_mouse.y;
		return true;	// intercept mouse event
	}

	// up button?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
	{
		ldown = true;
		return true;
	}

	// down button?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[3]+scrollspace() && g_mouse.x <= pos[2] && g_mouse.y <= pos[3]+scrollspace()+g_font[font].gheight)
	{
		ldown = true;
		return true;
	}

	return true;
}


bool CWidget::DropDown_lbuttondown()
{
	if(g_mouse.x >= pos[0] && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
	{
		ldown = true;
		return true;
	}

	return false;
}

bool CWidget::ListBox_lbuttondown()
{
	CFont* f = &g_font[font];

	for(int i=(int)scroll; i<(int)scroll+rowsshown(); i++)
	{
		int row = i-(int)scroll;
		// list item?
		if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2]-square() && g_mouse.y >= pos[1]+f->gheight*row
			&& g_mouse.y <= pos[1]+f->gheight*(row+1))
		{
			ldown = true;
			return true;	// intercept mouse event
		}
	}

	// scroll bar?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1]+square()+scrollspace()*topratio() && g_mouse.x <= pos[2] && 
			g_mouse.y <= pos[1]+square()+scrollspace()*bottomratio())
	{
		ldown = true;
		mousescroll = true;
		mousedown[1] = g_mouse.y;
		return true;	// intercept mouse event
	}

	// up button?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[1]+square())
	{
		ldown = true;
		return true;
	}

	// down button?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[3]-square() && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
	{
		ldown = true;
		return true;
	}

	return false;
}

bool CWidget::EditBox_lbuttondown()
{
	if(over)
	{
		ldown = true;
		return true;
	}

	return false;
}

bool CWidget::TextArea_lbuttondown()
{
	if(over)
	{
		ldown = true;
		return true;
	}

	return false;
}

bool CWidget::HScroller_lbuttonup(bool moved)
{
	if(ldown)
	{
		ldown = false;
		
		if(!moved)
		{
			for(int i=subwidg.size()-1; i>=0; i--)
			{
				if(subwidg[i].lbuttondown())
					break;
			}

			for(int i=subwidg.size()-1; i>=0; i--)
			{
				if(subwidg[i].lbuttonup(moved))
				{
					vel[0] = 0;
					vel[1] = 0;
					return true;
				}
			}
			
			goto outf;
		}

		return true;
	}

outf:
	if(outfunc != NULL)
	{
		vel[0] = 0;
		vel[1] = 0;
		outfunc();
		return true;	// intercept mouse event
	}

	return false;
}

bool CWidget::CheckBox_lbuttonup()
{
	if(over && ldown)
	{
		if(selected <= 0)
			selected = 1;
		else
			selected  = 0;

		if(changefunc != NULL)
			changefunc();

		ldown = false;

		return true;
	}
	
	ldown = false;

	return false;
}

bool CWidget::DropDown_prelbuttonup()
{
	if(!opened)
		return false;

	if(!ldown)
	{
		opened = false;
		return false;
	}
	
	ldown = false;

	if(mousescroll)
	{
		mousescroll = false;
		return true;	// intercept mouse event
	}

	for(int i=(int)scroll; i<(int)scroll+rowsshown(); i++)
	{
		// list item?
		if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2]-square() && g_mouse.y >= pos[3]+g_font[font].gheight*(i-(int)scroll)
			&& g_mouse.y <= pos[3]+g_font[font].gheight*(i-(int)scroll+1))
		{
			selected = i;
			opened = false;
			if(changefunc != NULL)
				changefunc();
			return true;	// intercept mouse event
		}
	}

	// up button?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
	{
		scroll--;
		if(scroll < 0)
			scroll = 0;

		return true;
	}

	// down button?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[3]+scrollspace() && g_mouse.x <= pos[2] && g_mouse.y <= pos[3]+scrollspace()+g_font[font].gheight)
	{
		scroll++;
		if(scroll+rowsshown() > options.size())
			scroll = options.size() - rowsshown();

		return true;
	}
	
	opened = false;

	return true;	// intercept mouse event
}

bool CWidget::DropDown_lbuttonup()
{
	if(!ldown)
		return false;

	ldown = false;

	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[1]+square())
	{
		opened = true;
		return true;
	}

	return false;
}

bool CWidget::EditBox_lbuttonup()
{
	if(over && ldown)
	{
		opened = true;
		ldown = false;
		return true;
	}

	ldown = false;

	if(opened)
	{
		opened = false;
		return true;
	}

	return false;
}

bool CWidget::TextArea_lbuttonup()
{
	if(over && ldown)
	{
		opened = true;
		ldown = false;
		return true;
	}

	ldown = false;

	if(opened)
	{
		opened = false;
		return true;
	}

	return false;
}

bool CWidget::ListBox_lbuttonup()
{
	if(!ldown)
		return false;
	
	ldown = false;

	if(mousescroll)
	{
		mousescroll = false;
		return true;	// intercept mouse event
	}
	
	CFont* f = &g_font[font];

	for(int i=(int)scroll; i<(int)scroll+rowsshown(); i++)
	{
		int row = i-(int)scroll;

		// list item?
		if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2]-square() && g_mouse.y >= pos[1]+f->gheight*row
			&& g_mouse.y <= pos[1]+f->gheight*(row+1))
		{
			selected = i;
			if(changefunc != NULL)
				changefunc();

			return true;	// intercept mouse event
		}
	}

	// up button?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[1]+square())
	{
		if(rowsshown() < (int)((pos[3]-pos[1])/f->gheight))
		{
			return true;
		}

		scroll--;
		if(scroll < 0)
			scroll = 0;

		return true;
	}

	// down button?
	if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[3]-square() && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
	{
		scroll++;
		if(scroll+rowsshown() > options.size())
			scroll = options.size() - rowsshown();

		return true;
	}

	return true;	// intercept mouse event
}

bool CWidget::EditBox_keydown(int k)
{
	if(!opened)
		return false;
	
	int len = value.length();

	if(caret > len)
		caret = len;
/*
	if(k == VK_BACK)
	{
		int len = value.length();
		
		if(caret <= 0 || len <= 0)
			return true;

		string before = value.substr(0, caret-1);
		string after = value.substr(caret, len-caret);
		value = before;
		value.append(after);

		caret--;
	}
	else if(k == VK_DELETE)
	{
		int len = value.length();
		
		if(caret >= len || len <= 0)
			return true;

		string before = value.substr(0, caret);
		string after = value.substr(caret+1, len-caret);
		value = before;
		value.append(after);
	}
	else*/ if(k == VK_LEFT)
	{
		if(caret <= 0)
			return true;

		caret --;
	}
	else if(k == VK_RIGHT)
	{
		int len = value.length();
		
		if(caret >= len)
			return true;

		caret ++;
	}
	else if(k == 190 && !g_keys[VK_SHIFT])
		placechar('.');
	/*
	else if(k == VK_SHIFT)
		return true;
	else if(k == VK_CAPITAL)
		return true;
	else if(k == VK_SPACE)
	{
		int len = value.length();

		if(len >= maxlen)
			return true;

		char addchar = ' ';
		string before = value.substr(0, caret);
		string after = value.substr(caret, len-caret);
		value = before + addchar + after;
		caret ++;
	}/*
	else if(k >= 'A' && k <= 'Z')
	{
		int len = value.length();

		if(len >= maxlen)
			return true;

		char addchar = k;

		if(!g_keys[VK_SHIFT] && !g_keys[VK_CAPITAL])
			addchar += 32;

		string before = value.substr(0, caret);
		string after = value.substr(caret, len-caret);
		value = before + addchar + after;
		caret ++;
	}*//*
	else if(k == 190 || k == 188)
	{
		int len = value.length();

		if(len >= maxlen)
			return true;

		char addchar = k;

		if(g_keys[VK_SHIFT])
		{
			if(k == 190)
				addchar = '>';
			else if(k == 188)
				addchar = '<';
		}
		else
		{
			if(k == 190)
				addchar = '.';
			else if(k == 188)
				addchar = ',';
		}
		
		string before = value.substr(0, caret);
		string after = value.substr(caret, len-caret);
		value = before + addchar + after;
		caret ++;
	}
	else if((k >= '!' && k <= '@') || (k >= '[' && k <= '`') || (k >= '{' && k <= '~'))
	{
		int len = value.length();

		if(len >= maxlen)
			return true;

		char addchar = k;

		if(g_keys[VK_SHIFT])
		{
			if(k == '0')
				addchar = ')';
			else if(k == '1')
				addchar = '!';
			else if(k == '2')
				addchar = '@';
			else if(k == '3')
				addchar = '#';
			else if(k == '4')
				addchar = '$';
			else if(k == '5')
				addchar = '%';
			else if(k == '6')
				addchar = '^';
			else if(k == '7')
				addchar = '&';
			else if(k == '8')
				addchar = '*';
			else if(k == '9')
				addchar = '(';
			else if(k == '`')
				addchar = '~';
		}

		string before = value.substr(0, caret);
		string after = value.substr(caret, len-caret);
		value = before + addchar + after;
		caret ++;
	}*/
	else if(k == VK_TAB)
	{
		for(int i=0; i<g_GUI.view.size(); i++)
		{
			CView* v = &g_GUI.view[i];
			for(int j=0; j<v->widget.size(); j++)
			{
				CWidget* w = &v->widget[j];
				if(w == this)
				{
					opened = false;

					for(int l=j+1; l<v->widget.size();l++)
					{
						w = &v->widget[l];
						if(w->type == EDITBOX)
						{
							w->opened = true;
							return true;
						}
					}
					for(int l=0; l<j; l++)
					{
						w = &v->widget[l];
						if(w->type == EDITBOX)
						{
							w->opened = true;
							return true;
						}
					}

					opened = true;
					return true;
				}
			}
		}
	}
	
	if(changefunc2 != NULL)
		changefunc2(param);

	return true;
}

bool CWidget::TextArea_keydown(int k)
{
	if(!opened)
		return false;
	
	int len = value.length();

	if(caret > len)
		caret = len;

	if(k == VK_LEFT)
	{
		if(caret <= 0)
			return true;

		caret --;
	}
	else if(k == VK_RIGHT)
	{
		int len = value.length();
		
		if(caret >= len)
			return true;

		caret ++;
	}
	else if(k == 190 && !g_keys[VK_SHIFT])
		placechar('.');
	else if(k == VK_TAB)
	{
		for(int i=0; i<g_GUI.view.size(); i++)
		{
			CView* v = &g_GUI.view[i];
			for(int j=0; j<v->widget.size(); j++)
			{
				CWidget* w = &v->widget[j];
				if(w == this)
				{
					opened = false;

					for(int l=j+1; l<v->widget.size();l++)
					{
						w = &v->widget[l];
						if(w->type == EDITBOX)
						{
							w->opened = true;
							return true;
						}
					}
					for(int l=0; l<j; l++)
					{
						w = &v->widget[l];
						if(w->type == EDITBOX)
						{
							w->opened = true;
							return true;
						}
					}

					opened = true;
					return true;
				}
			}
		}
	}
	
	if(changefunc != NULL)
		changefunc();

	if(changefunc2 != NULL)
		changefunc2(param);

	return true;
}

bool CWidget::EditBox_keyup(int k)
{
	if(!opened)
		return false;

	return true;
}

void CWidget::changevalue(const char* newv)
{
	value = newv;
	if(caret > strlen(newv))
		caret = strlen(newv);
	if(type == TEXTBOX || type == TEXTAREA)
	{
		lines = CountLines(value.c_str(), MAINFONT8, pos[0], pos[1], pos[2]-pos[0]-square(), pos[3]-pos[1]);
	}
}

void CWidget::placechar(int k)
{
	int len = value.length();

	if(type == EDITBOX && len >= maxlen)
		return;

	char addchar = k;

	string before = value.substr(0, caret);
	string after = value.substr(caret, len-caret);
	value = before + addchar + after;
	caret ++;
}

bool CWidget::EditBox_charin(int k)
{
	if(!opened)
		return false;
	
	int len = value.length();

	if(caret > len)
		caret = len;

	if(k == VK_BACK)
	{
		int len = value.length();
		
		if(caret <= 0 || len <= 0)
			return true;

		string before = value.substr(0, caret-1);
		string after = value.substr(caret, len-caret);
		value = before;
		value.append(after);

		caret--;
	}
	else if(k == VK_DELETE)
	{
		int len = value.length();
		
		if(caret >= len || len <= 0)
			return true;

		string before = value.substr(0, caret);
		string after = value.substr(caret+1, len-caret);
		value = before;
		value.append(after);
	}
	else if(k == VK_SHIFT)
		return true;
	else if(k == VK_CAPITAL)
		return true;
	else if(k == VK_RETURN)
		return true;
	else if(k == VK_SPACE)
	{
		placechar(' ');
	}
	else 
	{
		placechar(k);
	}
	
	if(changefunc != NULL)
		changefunc();

	if(changefunc2 != NULL)
		changefunc2(param);

	return true;
}

bool CWidget::TextArea_keyup(int k)
{
	if(!opened)
		return false;

	return true;
}

bool CWidget::TextArea_charin(int k)
{
	if(!opened)
		return false;
	
	int len = value.length();

	if(caret > len)
		caret = len;

	if(k == VK_BACK)
	{
		int len = value.length();
		
		if(caret <= 0 || len <= 0)
			return true;

		string before = value.substr(0, caret-1);
		string after = value.substr(caret, len-caret);
		value = before;
		value.append(after);

		caret--;
	}
	else if(k == VK_DELETE)
	{
		int len = value.length();
		
		if(caret >= len || len <= 0)
			return true;

		string before = value.substr(0, caret);
		string after = value.substr(caret+1, len-caret);
		value = before;
		value.append(after);
	}
	else if(k == VK_SHIFT)
		return true;
	else if(k == VK_CAPITAL)
		return true;
	else if(k == VK_SPACE)
	{
		placechar(' ');
	}
	else if(k == VK_RETURN)
	{
		placechar('\n');
	}
	//else if(k >= 'A' && k <= 'Z' || k >= 'a' && k <= 'z')
	//else if(k == 190 || k == 188)
	//else if((k >= '!' && k <= '@') || (k >= '[' && k <= '`') || (k >= '{' && k <= '~') || (k >= '0' || k <= '9'))
	else
	{
		placechar(k);
	}
	
	if(changefunc != NULL)
		changefunc();

	if(changefunc2 != NULL)
		changefunc2(param);

	return true;
}

bool CWidget::lbuttonup(bool moved)
{
	switch(type)
	{
	case TOUCHLISTENER: 
	//MessageBox(g_hWnd, "tl up", "", NULL);
	case BUTTON: return Button_lbuttonup();
	case LINK: return Link_lbuttonup();
	case DROPDOWN: return DropDown_lbuttonup();
	case LISTBOX: return ListBox_lbuttonup();
	case BARBUTTON: return BarButton_lbuttonup();
	case HSCROLLER: return HScroller_lbuttonup(moved);
	case CHECKBOX: return CheckBox_lbuttonup();
	case EDITBOX: return EditBox_lbuttonup();
	case TEXTAREA: return TextArea_lbuttonup();
	default: return false;
	}
}

bool CWidget::prelbuttonup(bool moved)
{
	switch(type)
	{
	case DROPDOWN: return DropDown_prelbuttonup();
	default: return false;
	}
}

bool CWidget::lbuttondown()
{
	switch(type)
	{
	case TOUCHLISTENER:
	//MessageBox(g_hWnd, "tl down", "", NULL);
	case BUTTON: return Button_lbuttondown();
	case LINK: return Link_lbuttondown();
	case DROPDOWN: return DropDown_lbuttondown();
	case LISTBOX: return ListBox_lbuttondown();
	case EDITBOX: return EditBox_lbuttondown();
	case BARBUTTON: return BarButton_lbuttondown();
	case HSCROLLER: return HScroller_lbuttondown();
	case CHECKBOX: return CheckBox_lbuttondown();
	case TEXTAREA: return TextArea_lbuttondown();
	default: return false;
	}
}

bool CWidget::prelbuttondown()
{
	switch(type)
	{
	case DROPDOWN: return DropDown_prelbuttondown();
	default: return false;
	}
}

void CWidget::mousemove()
{
	switch(type)
	{
	case TOUCHLISTENER:
	//MessageBox(g_hWnd, "tl move", "", NULL);
	case BUTTON: Button_mousemove(); break;
	case LINK: Link_mousemove(); break;
	case DROPDOWN: DropDown_mousemove(); break;
	case LISTBOX: ListBox_mousemove(); break;
	case EDITBOX: EditBox_mousemove(); break;
	case BARBUTTON: BarButton_mousemove(); break;
	case HSCROLLER: HScroller_mousemove(); break;
	case CHECKBOX: CheckBox_mousemove(); break;
	case TEXTAREA: TextArea_mousemove(); break;
	default: break;
	}
}

void CView::draw()
{
	for(int i=0; i<widget.size(); i++)
	{
		char msg[128];
		sprintf(msg, "%s w#%d t=%d", name.c_str(), i, widget[i].type);
		LastNum(msg);
		widget[i].draw();
	}
}

void CView::draw2()
{
	for(int i=widget.size()-1; i>=0; i--)
		widget[i].draw2();
}

void DrawSelector()
{
	if(g_mode != PLAY)
		return;

	if(!g_mousekeys[0])
		return;

	if(!g_canselect)
		return;

	if(OverMinimap())
		return;

	CView* v;
	CWidget* w;
	for(int i=0; i<g_GUI.view.size(); i++)
	{
		v = &g_GUI.view[i];
		if(!v->opened)
			continue;

		for(int j=0; j<v->widget.size(); j++)
		{
			w = &v->widget[j];

			if(w->ldown)
				return;
		}
	}
	if(g_build != NOTHING)
		return;

	/*
	glDisable(GL_TEXTURE_2D);
	//glColor4f(0, 1, 0, 1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 0, 1, 0, 1);
	glBegin(GL_LINE_STRIP);
	
	glVertex2i(g_mousestart.x, g_mousestart.y);
	glVertex2i(g_mousestart.x, g_mouse.y);
	glVertex2i(g_mouse.x, g_mouse.y);
	glVertex2i(g_mouse.x, g_mousestart.y);
	glVertex2i(g_mousestart.x, g_mousestart.y);

	glEnd();
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);*/

	glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 0, 1, 0, 1);
    
    float vertices[] =
    {
        //posx, posy    texx, texy
        g_mousestart.x, g_mousestart.y, 0,          0, 0,
        g_mousestart.x, g_mouse.y,0,         1, 0,
        g_mouse.x, g_mouse.y,0,      1, 1,
        
        g_mouse.x, g_mousestart.y,0,      1, 1,
        g_mousestart.x, g_mousestart.y,0,       0, 1
    };
    
    glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
    
    glDrawArrays(GL_LINE_STRIP, 0, 5);
}

void CGUI::draw()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	Use(ORTHO);
	//OrthoMode(0, 0, g_width, g_height);
	
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::HEIGHT], (float)g_height);
    glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::POSITION]);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::TEXCOORD0]);

	/*
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NULL);
    
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NULL);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

	glColor4f(1,1,1,1);
	*/

	for(int i=0; i<view.size(); i++)
		if(view[i].opened)
		{
			//LastNum(view[i].name.c_str());
			view[i].draw();
		}
	
	for(int i=0; i<view.size(); i++)
		if(view[i].opened)
			view[i].draw2();
	
	Use(COLOR2D);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::HEIGHT], (float)g_height);
	DrawSelector();
	Use(ORTHO);

	//DrawImage(g_depth, 0, 0, 128, 128);

	//PerspectiveMode();
	TurnOffShader();
}

void AssignKey(int i, void (*down)(), void (*up)())
{
	g_GUI.assignKey(i, down, up);
}

void AssignMouseWheel(void (*wheel)(int delta))
{
	g_GUI.assignMouseWheel(wheel);
}

void AssignMouseMove(void (*mouse)())
{
	g_GUI.assignMouseMove(mouse);
}

void AssignLButton(void (*down)(), void (*up)())
{
	g_GUI.assignLButton(down, up);
}

void AssignRButton(void (*down)(), void (*up)())
{
	g_GUI.assignRButton(down, up);
}

void AssignMButton(void (*down)(), void (*up)())
{
	g_GUI.assignMButton(down, up);
}

void AddView(const char* name, int page)
{
	CView view(name, page);
	g_GUI.view.push_back(view);
}

void AddImage(const char* name, const char* filepath, float left, float top, float right, float bottom, float r, float g, float b, float a, float tleft, float ttop, float tright, float tbottom)
{
	int last = g_GUI.view.size()-1;
	CWidget image;
	image.Image(name, filepath, left, top, right, bottom, r, g, b, a, tleft, ttop, tright, tbottom);
	g_GUI.view[last].widget.push_back(image);
}

void AddImage(const char* filepath, float left, float top, float right, float bottom, float r, float g, float b, float a, float tleft, float ttop, float tright, float tbottom)
{
	int last = g_GUI.view.size()-1;
	CWidget image;
	image.Image(filepath, left, top, right, bottom, r, g, b, a, tleft, ttop, tright, tbottom);
	g_GUI.view[last].widget.push_back(image);
}

void AddImage(CView* v, unsigned int tex, float left, float top, float right, float bottom, float r, float g, float b, float a, float tleft, float ttop, float tright, float tbottom)
{
	CWidget image;
	image.Image(tex, left, top, right, bottom, r, g, b, a, tleft, ttop, tright, tbottom);
	v->widget.push_back(image);
}

void AddLink(const char* t, int f, float left, float top, void (*click)())
{
	int last = g_GUI.view.size()-1;
	CWidget link;
	link.Link(t, f, left, top, click);
	g_GUI.view[last].widget.push_back(link);
}

void AddInsDraw(void (*inst)())
{
	int last = g_GUI.view.size()-1;
	CWidget insdraw;
	insdraw.InsDraw(inst);
	g_GUI.view[last].widget.push_back(insdraw);
}

void AddButton(const char* filepath, const char* t, int f, float left, float top, float right, float bottom, void (*click)(), void (*overf)(), void (*out)())
{
	int last = g_GUI.view.size()-1;
	CWidget button;
	button.Button(filepath, t, f, left, top, right, bottom, click, overf, out);
	g_GUI.view[last].widget.push_back(button);
}

void AddButton(const char* filepath, const char* t, int f, float left, float top, float right, float bottom, void (*click2)(int p), int parm)
{
	int last = g_GUI.view.size()-1;
	CWidget button;
	button.Button(filepath, t, f, left, top, right, bottom, click2, parm);
	g_GUI.view[last].widget.push_back(button);
}

void AddButton(const char* filepath, const char* t, int f, float left, float top, float right, float bottom, void (*click2)(int p), void (*over2)(int p), void (*out)(), int parm)
{
	int last = g_GUI.view.size()-1;
	CWidget button;
	button.Button(filepath, t, f, left, top, right, bottom, click2, over2, out, parm);
	g_GUI.view[last].widget.push_back(button);
}

void AddTouchListener(float left, float top, float right, float bottom, void (*click2)(int p), void (*over2)(int p), void (*out)(), int parm)
{
	int last = g_GUI.view.size()-1;
	CWidget tl;
	tl.TouchListener(left, top, right, bottom, click2, over2, out, parm);
	g_GUI.view[last].widget.push_back(tl);
}

void AddButton(CView* v, char* filepath, const char* t, int f, float left, float top, float right, float bottom, void (*click)(), void (*overf)(), void (*out)())
{
	CWidget button;
	button.Button(filepath, t, f, left, top, right, bottom, click, overf, out);
	v->widget.push_back(button);
}

void AddHScroller(float top, float bottom, void (*click2)(int p), void (*out)())
{
	int last = g_GUI.view.size()-1;
	CWidget buildsel;
	buildsel.HScroller(top, bottom, click2, out);
	g_GUI.view[last].widget.push_back(buildsel);
}

void AddText(const char* n, const char* t, int f, float left, float top, float r, float g, float b, float a, bool shdw)
{
	int last = g_GUI.view.size()-1;
	CWidget text;
	text.Text(n, t, f, left, top, shdw, r, g, b, a);
	g_GUI.view[last].widget.push_back(text);
}

void AddText(CView* v, char* n, char* t, int f, float left, float top)
{
	CWidget text;
	text.Text(n, t, f, left, top);
	v->widget.push_back(text);
}

void AddText(const char* t, int f, float left, float top)
{
	int last = g_GUI.view.size()-1;
	CWidget text;
	text.Text(t, f, left, top);
	g_GUI.view[last].widget.push_back(text);
}

void AddDropDown(const char* n, int f, float left, float top, float width, void (*change)())
{
	int last = g_GUI.view.size()-1;
	CWidget dropdown;
	dropdown.DropDown(n, f, left, top, width, change);
	g_GUI.view[last].widget.push_back(dropdown);
}

void AddOption(const char* option)
{
	int last = g_GUI.view.size()-1;
	int lastdropdown = g_GUI.view[last].widget.size()-1;
	g_GUI.view[last].widget[lastdropdown].options.push_back(option);
}

void SelectOption()
{
	int last = g_GUI.view.size()-1;
	int lastdropdown = g_GUI.view[last].widget.size()-1;
	int lastoption = g_GUI.view[last].widget[lastdropdown].options.size()-1;
	CWidget* dropdown = &g_GUI.view[last].widget[lastdropdown];
	dropdown->selected = lastoption;
	dropdown->scroll = lastoption;
	if(dropdown->scroll+dropdown->rowsshown() > dropdown->options.size())
		dropdown->scroll = dropdown->options.size() - dropdown->rowsshown();
	if(dropdown->scroll < 0)
		dropdown->scroll = 0;
}

void AddBarButton(CView* v, unsigned int sprite, float bar, float left, float top, float right, float bottom, void (*click)(), void (*overf)(), void (*out)())
{
	CWidget BarButton;
	BarButton.BarButton(sprite, bar, left, top, right, bottom, click,  overf, out);
	v->widget.push_back(BarButton);
}

void AddEditBox(const char* n, int f, const char* t, float left, float top, float width, bool pw, int maxl, void (*change2)(int p), int parm)
{
	int last = g_GUI.view.size()-1;
	CWidget editbox;
	editbox.EditBox(n, f, t, left, top, width, pw, maxl, change2, parm);
	g_GUI.view[last].widget.push_back(editbox);
}

void AddTextBox(const char* n, const char* t, int f, float left, float top, float right, float bottom, float r, float g, float b, float a)
{
	int last = g_GUI.view.size()-1;
	CWidget textbox;
	textbox.TextBox(n, t, f, left, top, right, bottom, r, g, b, a);
	g_GUI.view[last].widget.push_back(textbox);
}

void AddTextArea(const char* n, const char* t, int f, float left, float top, float right, float bottom, float r, float g, float b, float a, void (*change)())
{
	int last = g_GUI.view.size()-1;
	CWidget textarea;
	textarea.TextArea(n, t, f, left, top, right, bottom, r, g, b, a, change);
	g_GUI.view[last].widget.push_back(textarea);
}

void AddListBox(const char* n, int f, float left, float top, float right, float bottom, void (*change)())
{
	int last = g_GUI.view.size()-1;
	CWidget listbox;
	listbox.ListBox(n, f, left, top, right, bottom, change);
	g_GUI.view[last].widget.push_back(listbox);
}

void AddCheckBox(const char* n, const char* t, int f, float left, float top, float right, float bottom, int sel, float r, float g, float b, float a, void (*change)())
{
	int last = g_GUI.view.size()-1;
	CWidget checkbox;
	checkbox.CheckBox(n, t, f, left, top, right, bottom, sel, r, g, b, a, change);
	g_GUI.view[last].widget.push_back(checkbox);
}

void CloseView(const char* name)
{
	for(int i=0; i<g_GUI.view.size(); i++)
		if(stricmp(g_GUI.view[i].name.c_str(), name) == 0)
			g_GUI.view[i].opened = false;
}

void OpenSoleView(const char* name, int page)
{
	for(int i=0; i<g_GUI.view.size(); i++)
	{
		if(stricmp(g_GUI.view[i].name.c_str(), name) == 0 && g_GUI.view[i].page == page)
			g_GUI.view[i].opened = true;
		else
			g_GUI.view[i].opened = false;
	}
}

bool OpenAnotherView(const char* name, int page)
{
	for(int i=0; i<g_GUI.view.size(); i++)
	{
		if(stricmp(g_GUI.view[i].name.c_str(), name) == 0 && g_GUI.view[i].page == page)
		{
			g_GUI.view[i].opened = true;
			return true;
		}
	}

	return false;
}

void NextPage(const char* name)
{
	int page = 0;

	for(int i=0; i<g_GUI.view.size(); i++)
	{
		if(stricmp(g_GUI.view[i].name.c_str(), name) == 0 && g_GUI.view[i].opened)
		{
			page = g_GUI.view[i].page;
			g_GUI.view[i].opened = false;
			break;
		}
	}

	if(!OpenAnotherView(name, page+1))
		OpenAnotherView(name, 0);
}

void BuildSel(const char* name, const char* graphic, const char* desc, float* cost, float* inp, float* outp, void (*click2)(int p), int param)
{
	CView* v = g_GUI.getview("build selector");
	CWidget* mw = &v->widget[1];

	if(mw->subwidg.size() > 0)
		mw->pos[2] += HSCROLLER_SPACING;

	CWidget optbg;
	optbg.Image("gui\\buildbg.png", mw->pos[2], mw->pos[1], mw->pos[2]+HSCROLLER_OPTIONW, mw->pos[3], 1, 1, 1, 1.0f);
	mw->subwidg.push_back(optbg);

	CWidget touchl;
	touchl.TouchListener(mw->pos[2], mw->pos[1], mw->pos[2]+HSCROLLER_OPTIONW, mw->pos[3], click2, NULL, NULL, param);
	mw->subwidg.push_back(touchl);

	CWidget bsprite;
	bsprite.Image(graphic, mw->pos[2]+20, mw->pos[1]+20, mw->pos[2]+20+HSCROLLER_OPTIONW-20-20, mw->pos[1]+20+HSCROLLER_OPTIONW-20-20, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(bsprite);

	CWidget namebox;
	//title.Text(name, name, MAINFONT8, mw->pos[0]+5, mw->pos[1], true);
	namebox.TextBox(name, name, MAINFONT8, mw->pos[2]+10, mw->pos[1]+10, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(namebox);

	CWidget descbox;
	char descall[1024];
	sprintf(descall, "%s", desc);

	if(stricmp(desc, "") != 0)
		strcat(descall, "\n\n");
	
	strcat(descall, "Cost: ");
	for(int i=0; i<RESOURCES; i++)
	{
		if(cost[i] <= 0.0f)
			continue;

		char addt[64];
		sprintf(addt, "%c%d ", g_resource[i].icon, (int)cost[i]);
		strcat(descall, addt);
	}

	strcat(descall, "\n");

	bool hasinout = false;

	if(inp != NULL && outp != NULL)
	{
		for(int i=0; i<RESOURCES; i++)
		{
			if(inp[i] > 0.0f || outp[i] > 0.0f)
			{
				hasinout = true;
				break;
			}
		}
	}
	
	if(hasinout)
	{
		strcat(descall, "Input: ");
		for(int i=0; i<RESOURCES; i++)
		{
			if(inp[i] <= 0.0f)
				continue;

			char addt[64];
			sprintf(addt, "%c%d ", g_resource[i].icon, (int)inp[i]);
			strcat(descall, addt);
		}
		strcat(descall, "\n");

		strcat(descall, "Output: ");
		for(int i=0; i<RESOURCES; i++)
		{
			if(outp[i] <= 0.0f)
				continue;

			char addt[64];
			sprintf(addt, "%c%d ", g_resource[i].icon, (int)outp[i]);
			strcat(descall, addt);
		}
	}
	
	//descbox.TextBox("desc", descall, MAINFONT8, mw->pos[2]+10, mw->pos[1]+20+HSCROLLER_OPTIONW-30-20, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3]-10, 1, 1, 1, 1.0f);
	descbox.TextBox("desc", descall, MAINFONT8, mw->pos[2]+10, mw->pos[1]+20+75, mw->pos[2]+HSCROLLER_OPTIONW-10, mw->pos[3]-10, 1, 1, 1, 1.0f);
	mw->subwidg.push_back(descbox);

	mw->pos[2] += HSCROLLER_OPTIONW;
}

void AddFrame(int left, int top, int right, int bottom, int leftoff, int rightoff, int titlewidth, int undertextw)
{
	int midheight = bottom - top - 60 - 52;

	AddImage("gui\\frames\\topleft.png",			left - leftoff,						top,						left + 29 - leftoff + 1-1,			top + 52 + 1-1,					1,1,1,1);
	AddImage("gui\\frames\\topleftraising.png",		left + 29 - leftoff,				top,						left + 29 + 31 - leftoff,			top + 52 + 1-1,					1,1,1,1);
	AddImage("gui\\frames\\topraised.png",			left + 29 + 31 - leftoff,			top,						right - 34 - 35 + rightoff,			top + 52 + 1-1,					1,1,1,1);
	AddImage("gui\\frames\\toprightlowering.png",	right - 34 - 35 + rightoff,			top,						right - 34 + rightoff,				top + 52 + 1-1,					1,1,1,1);
	AddImage("gui\\frames\\topright.png",			right - 34 + rightoff,				top,						right + rightoff,					top + 52 + 1-1,					1,1,1,1);
	
	AddImage("gui\\frames\\leftleftborder.png",		left - leftoff,						top + 52,					left + 14 - leftoff,				bottom - 60,					1,1,1,1,		0, 0, 1, ((bottom-60)-(top+52))/170.0f);
	AddImage("gui\\frames\\lefttopborder.png",		left + 14 - leftoff,				top + 52,					left + 14 + 16 - leftoff,			bottom - 60 - midheight/2,		1,1,1,1);
	AddImage("gui\\frames\\leftbottomborder.png",	left + 14 - leftoff,				bottom - 60 - midheight/2,	left + 14 + 16 - leftoff,			bottom - 60,					1,1,1,1);
	
	AddImage("gui\\frames\\rightrightborder.png",	right - 20 + rightoff,				top + 52,					right + rightoff,					bottom - 60,					1,1,1,1,		0, 0, 1, ((bottom-60)-(top+52))/170.0f);
	AddImage("gui\\frames\\righttopborder.png",		right - 20 - 17 + rightoff,			top + 52,					right - 20 + rightoff,				top + 52 + midheight/2,			1,1,1,1);
	AddImage("gui\\frames\\rightbottomborder.png",	right - 20 - 17 + rightoff,			top + 52 + midheight/2,		right - 20 + rightoff,				bottom - 60,					1,1,1,1);

	AddImage("gui\\frames\\bottomleftcorner.png",	left - leftoff,						bottom - 60 - 1+1,			left + 35 - leftoff,				bottom,							1,1,1,1);
	AddImage("gui\\frames\\bottomleftlowering.png", left + 35 - leftoff,				bottom - 60 - 1+1,			left + 35 + 32 - leftoff,			bottom,							1,1,1,1);
	AddImage("gui\\frames\\bottomlowered.png",		left + 35 + 32 - leftoff,			bottom - 60 - 1+1,			right - 35 - 30 + rightoff,			bottom,							1,1,1,1);
	AddImage("gui\\frames\\bottomrightraising.png",	right - 35 - 30 + rightoff,			bottom - 60 - 1+1,			right - 35 + rightoff,				bottom,							1,1,1,1);
	AddImage("gui\\frames\\bottomrightcorner.png",	right - 35 + rightoff,				bottom - 60 - 1+1,			right + rightoff,					bottom,							1,1,1,1);
}

void AddMiniFrame(int left, int top, int right, int bottom, int titlewidth, int undertextw, bool fillbg)
{
	int midheight = bottom - top - 32 - 23;

	if(fillbg)
	{
		AddImage("gui\\backg\\white.jpg",					left + 7 + 10,					top + 23,					right - 18 - 10,			bottom - 32,					1,1,1,1);
	}

	AddImage("gui\\miniframes\\minitopleft.png",			left,							top,						left + 10 + 1-1,			top + 23 + 1-1,					1,1,1,1);
	AddImage("gui\\miniframes\\minitopleftraising.png",		left + 10,						top,						left + 10 + 14,				top + 23 + 1-1,					1,1,1,1);
	AddImage("gui\\miniframes\\minitopraised.png",			left + 10 + 14,					top,						right - 21 - 15,			top + 23 + 1-1,					1,1,1,1);
	AddImage("gui\\miniframes\\minitoprightlowering.png",	right - 21 - 15,				top,						right - 21,					top + 23 + 1-1,					1,1,1,1);
	AddImage("gui\\miniframes\\minitopright.png",			right - 21,						top,						right,						top + 23 + 1-1,					1,1,1,1);
	
	AddImage("gui\\miniframes\\minileftleft.png",			left,							top + 23,					left + 7,					bottom - 32,					1,1,1,1);
	AddImage("gui\\miniframes\\minilefttop.png",			left + 7,						top + 23,					left + 7 + 10,				bottom - 32 - midheight/2,		1,1,1,1);
	AddImage("gui\\miniframes\\minileftbottom.png",			left + 7,						bottom - 32 - midheight/2,	left + 7 + 10,				bottom - 32,					1,1,1,1);
	
	AddImage("gui\\miniframes\\minirightright.png",			right - 18,						top + 23,					right,						bottom - 32,					1,1,1,1);
	AddImage("gui\\miniframes\\minirighttop.png",			right - 18 - 10,				top + 23,					right - 18,					top + 23 + midheight/2,			1,1,1,1);
	AddImage("gui\\miniframes\\minirightbottom.png",		right - 18 - 10,				top + 23 + midheight/2,		right - 18,					bottom - 32,					1,1,1,1);

	AddImage("gui\\miniframes\\minibottomleft.png",			left,							bottom - 32 - 1+1,			left + 10,					bottom,							1,1,1,1);
	AddImage("gui\\miniframes\\minibottomleftlowering.png", left + 10,						bottom - 32 - 1+1,			left + 10 + 15,				bottom,							1,1,1,1);
	AddImage("gui\\miniframes\\minibottomlowered.png",		left + 10 + 15,					bottom - 32 - 1+1,			right - 21 - 15,			bottom,							1,1,1,1);
	AddImage("gui\\miniframes\\minibottomrightraising.png",	right - 21 - 15,				bottom - 32 - 1+1,			right - 21,					bottom,							1,1,1,1);
	AddImage("gui\\miniframes\\minibottomright.png",		right - 21,						bottom - 32 - 1+1,			right,						bottom,							1,1,1,1);
}

void AddGrayBGFr(int left, int top, int right, int bottom)
{
	int midheight = bottom - top - 32 - 23;

	AddImage("gui\\grayframes\\grayfbg.png",				left + 23,						top + 17,					right - 21,					bottom - 18,					1,1,1,1,		0, 0, ((right-21)-(left+23))/46.0f, ((bottom-18)-(top+17))/46.0f);

	AddImage("gui\\grayframes\\grayftopleft.png",			left,							top,						left + 21 + 1-1,			top + 17 + 1-1,					1,1,1,1);
	AddImage("gui\\grayframes\\grayftop.png",				left + 21,						top,						right - 20,					top + 17 + 1-1,					1,1,1,1);
	AddImage("gui\\grayframes\\grayftopright.png",			right - 20,						top,						right,						top + 17 + 1-1,					1,1,1,1);
	
	AddImage("gui\\grayframes\\grayfleft.png",				left,							top + 17,					left + 23,					bottom - 18,					1,1,1,1,		0, 0, 1, ((bottom-18)-(top+17))/98.0f);
	
	AddImage("gui\\grayframes\\grayfright.png",				right - 21,						top + 17,					right,						bottom - 18,					1,1,1,1,		0, 0, 1, ((bottom-18)-(top+17))/80.0f);

	AddImage("gui\\grayframes\\grayfbottomleft.png",		left,							bottom - 18 - 1+1,			left + 22,					bottom,							1,1,1,1);
	AddImage("gui\\grayframes\\grayfbottom.png",			left + 22,						bottom - 18 - 1+1,			right - 26,					bottom,							1,1,1,1);
	AddImage("gui\\grayframes\\grayfbottomright.png",		right - 26,						bottom - 18 - 1+1,			right,						bottom,							1,1,1,1);
}

void FillHScroller()
{
	AddView("build selector");
	
	//AddImage("gui\\backg\\white.jpg", 0, g_height - 242, g_width, g_height - 60, 1,1,1,0.5f);
	AddImage("gui\\backg\\white.jpg",				14 + 16 - 7,						g_height - 242,					g_width - 20 + 11,				g_height - 60,				1,1,1,1.0f);

	AddHScroller(g_height - 242, g_height - 60, &Click_BuildNum, &Click_OutBuild);

	//enum BUILDINGTYPE{PIPELINE=-4, POWERLINE=-3, ROAD=-2, NOTHING=-1, APARTMENT=0, SHOPCPLX, FACTORY, FARM, MINE, SMELTER, DERRICK, REFINERY, REACTOR, COMBUSTOR, QUARRY, CEMPLANT, CHEMPLANT, ELECPLANT, BUILDING_TYPES};
	
	BuildSel("Pipeline", "gui\\centerp\\pipelineicon.png", "Pumps crude.", g_pipelineCost, NULL, NULL, Click_BuildNum, PIPELINE);
	BuildSel("Powerline", "gui\\centerp\\powerlineicon.png", "Conducts electricity.", g_powerlineCost, NULL, NULL, Click_BuildNum, POWERLINE);
	BuildSel("Road", "gui\\centerp\\roadicon.png", "Allows transportation.", g_roadCost, NULL, NULL, Click_BuildNum, ROAD);
	BuildSel("Apartment", "gui\\centerp\\apartmenticon.png", "", g_buildingType[APARTMENT].cost, g_buildingType[APARTMENT].input, g_buildingType[APARTMENT].output, Click_BuildNum, APARTMENT);
	BuildSel("Shopping Complex", "gui\\centerp\\shopcplxicon.png", "", g_buildingType[SHOPCPLX].cost, g_buildingType[SHOPCPLX].input, g_buildingType[SHOPCPLX].output, Click_BuildNum, SHOPCPLX);
	BuildSel("Factory", "gui\\centerp\\factoryicon.png", "", g_buildingType[FACTORY].cost, g_buildingType[FACTORY].input, g_buildingType[FACTORY].output, Click_BuildNum, FACTORY);
	BuildSel("Farm", "gui\\centerp\\farmicon.png", "", g_buildingType[FARM].cost, g_buildingType[FARM].input, g_buildingType[FARM].output, Click_BuildNum, FARM);
	BuildSel("Mine", "gui\\centerp\\mineicon.png", "", g_buildingType[MINE].cost, g_buildingType[MINE].input, g_buildingType[MINE].output, Click_BuildNum, MINE);
	BuildSel("Smelter", "gui\\centerp\\smeltericon.png", "", g_buildingType[SMELTER].cost, g_buildingType[SMELTER].input, g_buildingType[SMELTER].output, Click_BuildNum, SMELTER);
	BuildSel("Derrick", "gui\\centerp\\derrickicon.png", "", g_buildingType[DERRICK].cost, g_buildingType[DERRICK].input, g_buildingType[DERRICK].output, Click_BuildNum, DERRICK);
	BuildSel("Refinery", "gui\\centerp\\refineryicon.png", "", g_buildingType[REFINERY].cost, g_buildingType[REFINERY].input, g_buildingType[REFINERY].output, Click_BuildNum, REFINERY);
	BuildSel("Reactor", "gui\\centerp\\reactoricon.png", "", g_buildingType[REACTOR].cost, g_buildingType[REACTOR].input, g_buildingType[REACTOR].output, Click_BuildNum, REACTOR);
	BuildSel("Combustor", "gui\\centerp\\combustoricon.png", "", g_buildingType[COMBUSTOR].cost, g_buildingType[COMBUSTOR].input, g_buildingType[COMBUSTOR].output, Click_BuildNum, COMBUSTOR);
	BuildSel("Quarry", "gui\\centerp\\quarryicon.png", "", g_buildingType[QUARRY].cost, g_buildingType[QUARRY].input, g_buildingType[QUARRY].output, Click_BuildNum, QUARRY);
	BuildSel("Cement Plant", "gui\\centerp\\cemplanticon.png", "", g_buildingType[CEMPLANT].cost, g_buildingType[CEMPLANT].input, g_buildingType[CEMPLANT].output, Click_BuildNum, CEMPLANT);
	BuildSel("Chemical Plant", "gui\\centerp\\chemplanticon.png", "", g_buildingType[CHEMPLANT].cost, g_buildingType[CHEMPLANT].input, g_buildingType[CHEMPLANT].output, Click_BuildNum, CHEMPLANT);
	BuildSel("Electronics Plant", "gui\\centerp\\elecplanticon.png", "", g_buildingType[ELECPLANT].cost, g_buildingType[ELECPLANT].input, g_buildingType[ELECPLANT].output, Click_BuildNum, ELECPLANT);
	//BuildSel("R&D Facility", "gui\\centerp\\rndfacicon.png", "", g_buildingType[RNDFACILITY].cost, g_buildingType[RNDFACILITY].input, g_buildingType[RNDFACILITY].output, Click_BuildNum, RNDFACILITY);

	AddFrame(0, g_height - 242 - 52, g_width, g_height, 7, 11, 200, 200);

	/*
	{
		CView* v = g_GUI.getview("build selector");
		CWidget* mw = &v->widget[1];

		if(mw->subwidg.size() > 0)
			mw->pos[2] += HSCROLLER_SPACING;

		CWidget optbg;
		optbg.Image("gui\\buildbg.png", mw->pos[2], 0, mw->pos[2]+HSCROLLER_OPTIONW*1.7f, g_height-90, 1, 1, 1, 1.0f);
		mw->subwidg.push_back(optbg);
		optbg.Image("gui\\buildbg.png", mw->pos[2]+HSCROLLER_OPTIONW*1.7f, 0, mw->pos[2]+HSCROLLER_OPTIONW*3.4f, g_height-90, 1, 1, 1, 1.0f);
		mw->subwidg.push_back(optbg);

		CWidget textbox;
		//title.Text(name, name, MAINFONT8, mw->pos[0]+5, mw->pos[1], true);
		char text[1024];
		text[0] = '\0';
		for(int i=0; i<RESOURCES/2; i++)
		{
			char addt[64];
			sprintf(addt, "%c %s\n\n", g_resource[i].icon, g_resource[i].name);
			strcat(text, addt);
		}

		textbox.TextBox("resources", text, MAINFONT8, mw->pos[2]+20, 0+10, mw->pos[2]+HSCROLLER_OPTIONW*1.7f-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
		mw->subwidg.push_back(textbox);

		text[0] = '\0';
		for(int i=RESOURCES/2; i<RESOURCES; i++)
		{
			char addt[64];
			sprintf(addt, "%c %s\n\n", g_resource[i].icon, g_resource[i].name);
			strcat(text, addt);
		}
		
		textbox.TextBox("resources2", text, MAINFONT8, mw->pos[2]+HSCROLLER_OPTIONW*1.7f+20, 0+10, mw->pos[2]+HSCROLLER_OPTIONW*5.4f-10, mw->pos[1]+50, 1, 1, 1, 1.0f);
		mw->subwidg.push_back(textbox);

		mw->pos[2] += HSCROLLER_OPTIONW*2;
	}*/
}

void AddPCSBG()
{
	int pcsheight = g_height;
	AddImage("gui\\centerp\\pcsgray.jpg", g_width/2 - pcsheight/2, g_height/2 - pcsheight/2, g_width/2 + pcsheight/2, g_height/2 + pcsheight/2);
	AddFrame(0, 0, g_width, g_height, 0, 0, 200, 200);
}

void ReGUIEd()
{
	AddView("editor");
	AddImage("gui\\lbar.jpg", 0, MMBAR_HEIGHT, LBAR_WIDTH, g_height);
	AddImage("gui\\minimap.png", 0, 0, MMBAR_WIDTH, MMBAR_HEIGHT);
	AddInsDraw(&DrawMinimap);
	AddButton("gui\\edsave.png", "", MAINFONT8, g_width-44*2, TBAR_HEIGHT, g_width-44, TBAR_HEIGHT + 44, &Click_EdSave, NULL, NULL);
	AddButton("gui\\edload.png", "", MAINFONT8, g_width-44, TBAR_HEIGHT, g_width, TBAR_HEIGHT + 44, &Click_EdLoad, NULL, NULL);
	AddDropDown("edcat", MAINFONT8, 10, MMBAR_HEIGHT, LBAR_WIDTH-20, &Change_EdCat);
	AddOption("Tiles");
	AddOption("Buildings");
	AddOption("Units");
	AddOption("Inspector");
	AddOption("Players");
	AddOption("Script");
	AddOption("Scenery");

	AddView("ed script");
	AddListBox("triggers", MAINFONT8, LBAR_WIDTH/2-44, MMBAR_HEIGHT+20, LBAR_WIDTH/2+44, MMBAR_HEIGHT+20+12*6, &Change_Triggers);
	AddEditBox("newtrigger", MAINFONT8, "new trigger", LBAR_WIDTH/2-44, MMBAR_HEIGHT+20+12*6+5, 88, false, 15, &Change_NewTrigger, 0);
	AddButton("gui\\buttonbg.png", "Add", MAINFONT8, LBAR_WIDTH/2 - 44, MMBAR_HEIGHT+20+12*6+5+15, LBAR_WIDTH/2 + 44, MMBAR_HEIGHT+20+12*6+5+15*2, &Click_AddTrigger, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Rename", MAINFONT8, LBAR_WIDTH/2 - 44, MMBAR_HEIGHT+20+12*6+5*2+15*2, LBAR_WIDTH/2 + 44, MMBAR_HEIGHT+20+12*6+5*2+15*3, &Click_RenameTrigger, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Edit", MAINFONT8, LBAR_WIDTH/2 - 44, MMBAR_HEIGHT+20+12*6+5*3+15*3, LBAR_WIDTH/2 + 44, MMBAR_HEIGHT+20+12*6+5*3+15*4, &Click_EditTrigger, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Delete", MAINFONT8, LBAR_WIDTH/2 - 44, MMBAR_HEIGHT+20+12*6+5*4+15*4, LBAR_WIDTH/2 + 44, MMBAR_HEIGHT+20+12*6+5*4+15*5, &Click_DelTrigger, NULL, NULL);
	ReGUIScript();

	AddView("ed players");
	AddText("Activity:", MAINFONT8, 5, MMBAR_WIDTH+40);
	AddDropDown("activity", MAINFONT8, 10, MMBAR_HEIGHT+60, LBAR_WIDTH-20, &Change_Activity);
	for(int i=0; i<AI_ACTIVITIES; i++)
	{
		AddOption(activitystr[i]);
		if(g_player[g_selP].activity == i)
			SelectOption();
	}
	AddImage("logo", "gui\\factions\\player.png", LBAR_WIDTH/2-44, MMBAR_HEIGHT+80, LBAR_WIDTH/2+44, MMBAR_HEIGHT+80+88);
	AddButton("gui\\buttonbg.png", "More Info", MAINFONT8, LBAR_WIDTH/2-44, MMBAR_HEIGHT+80+88+10, LBAR_WIDTH/2+44, MMBAR_HEIGHT+80+88+10+15, &Click_EdPMore, NULL, NULL);

	AddView("ed inspector");

	AddView("ed units");
	AddDropDown("unit type", MAINFONT8, 10, MMBAR_HEIGHT+40, LBAR_WIDTH-20, &Change_EdUnitType);
	for(int i=0; i<UNIT_TYPES; i++)
	{
		AddOption(g_unitType[i].name);
	}
	AddButton("gui\\cancel.png", "", MAINFONT8, LBAR_WIDTH/2, MMBAR_HEIGHT + 60, LBAR_WIDTH/2+44, MMBAR_HEIGHT + 60 + 44, &Click_Destroy, NULL, NULL);

	AddView("ed build selector");
	AddButton("gui\\build.png", "", MAINFONT8, LBAR_WIDTH/2-44, MMBAR_HEIGHT + 44, LBAR_WIDTH/2, MMBAR_HEIGHT + 44*2, &Click_OpenBuild, NULL, NULL);
	AddButton("gui\\cancel.png", "", MAINFONT8, LBAR_WIDTH/2, MMBAR_HEIGHT + 44, LBAR_WIDTH/2+44, MMBAR_HEIGHT + 44*2, &Click_Destroy, NULL, NULL);

	AddView("tiles");
	AddButton("gui\\raise.png", "", MAINFONT8, LBAR_WIDTH/2-44, MMBAR_HEIGHT + 44, LBAR_WIDTH/2, MMBAR_HEIGHT + 44*2, &Click_Raise, NULL, NULL);
	AddButton("gui\\lower.png", "", MAINFONT8, LBAR_WIDTH/2, MMBAR_HEIGHT + 44, LBAR_WIDTH/2+44, MMBAR_HEIGHT + 44*2, &Click_Lower, NULL, NULL);
	AddButton("gui\\spread.png", "", MAINFONT8, LBAR_WIDTH/2-44, MMBAR_HEIGHT + 44*2, LBAR_WIDTH/2, MMBAR_HEIGHT + 44*3, &Click_Spread, NULL, NULL);
	float btop = MMBAR_HEIGHT + 44*3;
	float bleft = LBAR_WIDTH/2-44;
	CTileType* tt;
	int num = 0;
	for(int i=0; i<TILETYPES; i++)
	{
		tt = &g_tiletype[i];
		if(!tt->on)
			continue;
		if(!stricmp(tt->m_name, "none"))
			continue;
		char full[128];
		sprintf(full, "tiles\\%s\\1111", tt->m_name);
		FindTextureExtension(full);
		AddButton(full, "", MAINFONT8, bleft + (num%4)*22, btop + (num/4)*22, bleft + (num%4+1)*22, btop + (num/4+1)*22, &Click_TileType, NULL, NULL, i);
		num ++;
	}

	AddView("ed scenery");
	AddDropDown("scenery type", MAINFONT8, 10, MMBAR_HEIGHT+20, LBAR_WIDTH-20, &Change_ScT);
	for(int i=0; i<SCENERY_TYPES; i++)
	{
		//char opt[32];
		//sprintf(opt, "Player %d", i);
		//AddOption(opt);
		AddOption(g_sceneryType[i].name);
		if(g_scT == i)
			SelectOption();
	}
	AddButton("gui\\cancel.png", "", MAINFONT8, LBAR_WIDTH/2, MMBAR_HEIGHT + 60, LBAR_WIDTH/2+44, MMBAR_HEIGHT + 60 + 44, &Click_Destroy, NULL, NULL);

	AddView("editor selp");
	AddDropDown("player", MAINFONT8, 10, MMBAR_HEIGHT+20, LBAR_WIDTH-20, &Change_SelP);
	for(int i=0; i<PLAYERS; i++)
	{
		//char opt[32];
		//sprintf(opt, "Player %d", i);
		//AddOption(opt);
		AddOption(facnamestr[i]);
		if(g_selP == i)
			SelectOption();
	}
	
	AddView("ed pmore");
	AddMiniFrame(g_width/2 - 150, g_height/2 - 100, g_width/2 + 150, g_height/2 + 150, 0, 0, true);
	AddText("Starting Resources", MAINFONT8, g_width/2 - 100, g_height/2 - 90);
	CResource* r;
	num = 0;
	for(int i=0; i<RESOURCES; i++)
	{
		r = &g_resource[i];
		if(r->capacity)
			continue;
		char value[32];
		if(i == CURRENC)
			sprintf(value, "%.2f", (float)g_player[g_selP].global[i]);
		else
			sprintf(value, "%d", (int)g_player[g_selP].global[i]);
		AddEditBox(r->name, MAINFONT8, value, g_width/2 - 100 + 100*(num%2) + 15*(num%2), g_height/2 - 75 + 20*(num/2), 90, false, 9, &Change_EdPRes, i);
		char icon[8];
		sprintf(icon, "%c", r->icon);
		AddText(icon, MAINFONT8, g_width/2 - 100 + 100*(num%2) + 15*(num%2) - 10, g_height/2 - 75  + 20*(num/2));
		num ++;
	}
	AddButton("gui\\buttonbg.png", "Done", MAINFONT8, g_width/2 - 44*2, g_height/2 + 150 - 44, g_width/2 + 44*0, g_height/2 + 150 - 44 + 15, &Click_ClosePMore, NULL, NULL);
	AddButton("gui\\buttonbg.png", "More", MAINFONT8, g_width/2 - 44*0, g_height/2 + 150 - 44, g_width/2 + 44*2, g_height/2 + 150 - 44 + 15, &Click_EdPMore2, NULL, NULL);
	
	AddView("ed pmore2");
	AddMiniFrame(g_width/2 - 150, g_height/2 - 100, g_width/2 + 150, g_height/2 + 150, 0, 0, true);
	AddText("Wages for Jobs", MAINFONT8, g_width/2 - 100, g_height/2 - 90);
	num = 0;
	for(int i=0; i<BUILDING_TYPES; i++)
	{
		if(g_buildingType[i].input[LABOUR] <= 0.0f)
			continue;
		char value[32];
		sprintf(value, "%.2f", (float)g_player[g_selP].wage[i]);
		AddEditBox(g_buildingType[i].name, MAINFONT8, value, g_width/2 - 100 + 100*(num%2) + 15*(num%2) + 50, g_height/2 - 75 + 20*(num/2), 50, false, 9, &Change_EdPWage, i);
		char bldg[32];
		sprintf(bldg, "%s", g_buildingType[i].name);
		bldg[7] = '\0';
		strcat(bldg, ":");
		AddText(bldg, MAINFONT8, g_width/2 - 100 + 100*(num%2) + 15*(num%2) - 10, g_height/2 - 75  + 20*(num/2));
		num ++;
	}
	char value[32];
	sprintf(value, "%.2f", (float)g_player[g_selP].truckwage);
	AddEditBox("truck wage", MAINFONT8, value, g_width/2 - 100 + 100*(num%2) + 15*(num%2) + 50, g_height/2 - 75 + 20*(num/2), 50, false, 9, &Change_EdPWage, -1);
	AddText("Truck wg", MAINFONT8, g_width/2 - 100 + 100*(num%2) + 15*(num%2) - 10, g_height/2 - 75  + 20*(num/2));
	num++;
	sprintf(value, "%.2f", (float)g_player[g_selP].conwage);
	AddEditBox("construction wage", MAINFONT8, value, g_width/2 - 100 + 100*(num%2) + 15*(num%2) + 50, g_height/2 - 75 + 20*(num/2), 50, false, 9, &Change_EdPWage, -3);
	AddText("Constructn", MAINFONT8, g_width/2 - 100 + 100*(num%2) + 15*(num%2) - 10, g_height/2 - 75  + 20*(num/2));
	num++;
	AddButton("gui\\buttonbg.png", "Done", MAINFONT8, g_width/2 - 44*2, g_height/2 + 150 - 44, g_width/2 + 44*0, g_height/2 + 150 - 44 + 15, &Click_ClosePMore, NULL, NULL);
	AddButton("gui\\buttonbg.png", "More", MAINFONT8, g_width/2 - 44*0, g_height/2 + 150 - 44, g_width/2 + 44*2, g_height/2 + 150 - 44 + 15, &Click_EdPMore3, NULL, NULL);
	
	AddView("ed pmore3");
	AddMiniFrame(g_width/2 - 150, g_height/2 - 100, g_width/2 + 150, g_height/2 + 150, 0, 0, true);
	AddText("Selling Prices", MAINFONT8, g_width/2 - 100, g_height/2 - 90);
	num = 0;
	for(int i=0; i<RESOURCES; i++)
	{
		if(i == CURRENC)
			continue;
		if(i == LABOUR)
			continue;

		char value[32];
		sprintf(value, "%.2f", (float)g_player[g_selP].price[i]);
		AddEditBox(g_resource[i].name, MAINFONT8, value, g_width/2 - 100 + 100*(num%2) + 15*(num%2) + 20, g_height/2 - 75 + 15*(num/2), 80, false, 9, &Change_EdPPrice, i);
		char icon[16];
		sprintf(icon, "%c", g_resource[i].icon);
		AddText(icon, MAINFONT8, g_width/2 - 100 + 100*(num%2) + 15*(num%2) - 10, g_height/2 - 75  + 15*(num/2));
		num ++;
	}
	sprintf(value, "%.2f", (float)g_player[g_selP].transportprice);
	AddEditBox("transport price", MAINFONT8, value, g_width/2 - 100 + 100*(num%2) + 15*(num%2) + 50, g_height/2 - 75 + 15*(num/2), 50, false, 9, &Change_EdPPrice, -2);
	AddText("Transport", MAINFONT8, g_width/2 - 100 + 100*(num%2) + 15*(num%2) - 10, g_height/2 - 75  + 15*(num/2));
	num++;
	AddButton("gui\\buttonbg.png", "Done", MAINFONT8, g_width/2 - 44*2, g_height/2 + 150 - 44, g_width/2 + 44*0, g_height/2 + 150 - 44 + 15, &Click_ClosePMore, NULL, NULL);
	AddButton("gui\\buttonbg.png", "More", MAINFONT8, g_width/2 - 44*0, g_height/2 + 150 - 44, g_width/2 + 44*2, g_height/2 + 150 - 44 + 15, &Click_EdPMore, NULL, NULL);

	AddView("ed trigger");
	AddMiniFrame(g_width/2-150, g_height/2-100, g_width/2+150, g_height/2+150, 0, 0, true);
	AddCheckBox("enabled", "enabled", MAINFONT8, g_width/2-44, g_height/2-100+13, g_width/2+44, g_height/2-100+13+15, 1, 1.0f, 1.0f, 1.0f, 1.0f, &Change_TrigEnabled);
	AddListBox("conditions", MAINFONT8, g_width/2-100-10, g_height/2-100+30, g_width/2-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6, NULL);
	AddListBox("effects", MAINFONT8, g_width/2+10, g_height/2-100+30, g_width/2+100+10, g_height/2-100+30+g_font[MAINFONT8].gheight*6, NULL);
	AddEditBox("newcondition", MAINFONT8, "new condition", g_width/2-100-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5, 100, false, 15, NULL, 0);
	AddEditBox("neweffect", MAINFONT8, "new effect", g_width/2+10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5, 100, false, 15, NULL, 0);
	AddButton("gui\\buttonbg.png", "Add Condition", MAINFONT8, g_width/2-100-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*2+15, g_width/2-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*2+15*2, &Click_AddCondition, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Add Effect", MAINFONT8, g_width/2+10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*2+15, g_width/2+100+10,  g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*2+15*2, &Click_AddAction, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Rename", MAINFONT8, g_width/2-100-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*3+15*2, g_width/2-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*3+15*3, &Click_RenCondition, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Rename", MAINFONT8, g_width/2+10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*3+15*2, g_width/2+100+10,  g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*3+15*3, &Click_RenEffect, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Edit", MAINFONT8, g_width/2-100-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*4+15*3, g_width/2-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*4+15*4, &Click_EdCondition, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Edit", MAINFONT8, g_width/2+10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*4+15*3, g_width/2+100+10,  g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*4+15*4, &Click_EdEffect, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Delete", MAINFONT8, g_width/2-100-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*5+15*4, g_width/2-10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*5+15*5, &Click_DelCondition, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Delete", MAINFONT8, g_width/2+10, g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*5+15*4, g_width/2+100+10,  g_height/2-100+30+g_font[MAINFONT8].gheight*6+5*5+15*5, &Click_DelEffect, NULL, NULL);
	AddButton("gui\\buttonbg.png", "Done", MAINFONT8, g_width/2 - 44, g_height/2 + 150 - 44, g_width/2 + 44, g_height/2 + 150 - 44 + 15, &Click_CloseTrigger, NULL, NULL);

	AddView("ed condition");
	AddMiniFrame(g_width/2-150, g_height/2-100, g_width/2+150, g_height/2+150, 0, 0, true);	
	AddText("Condition", MAINFONT8, g_width/2-44, g_height/2-100+13);
	AddText("Type:", MAINFONT8, g_width/2-100, g_height/2-100+30);
	AddDropDown("type", MAINFONT8, g_width/2-100+44, g_height/2-100+30, 150, &Change_CondType);
	AddOption("none");	// CONDITION_NONE = 0
	AddOption("on load map");	// CONDITION_ONLOADMAP = 1
	AddOption("on place building");	// CONDITION_ONPLACEB = 2
	AddOption("on build unit");	// CONDITION_ONBUILDU = 3
	AddOption("on place other bldg");	// CONDITION_ONPLACEOTHERB = 4
	AddOption("on build other unit");	// CONDITION_ONBUILDOTHERU = 5
	AddOption("set to specific price");	// CONDITION_PRICESET = 6
	AddOption("unit count reached"); // CONDITION_UNITCOUNT = 7
	AddOption("captured building"); // CONDITION_CAPTUREB = 8
	AddOption("message closed");	// CONDITION_MESSAGECLOSED = 9
	AddOption("road inaccessability");	// CONDITION_ROADINACCESSABILITY = 10
	AddOption("all road accessible");	// CONDITION_ALLROADACCESSIBLE = 11
	AddOption("on finished building");	// CONDITION_ONFINISHEDB = 12
	AddOption("all construction fini");	// CONDITION_ALLCONSTRUCTIONFINI = 13
	AddButton("gui\\buttonbg.png", "Done", MAINFONT8, g_width/2 - 44, g_height/2 + 150 - 44, g_width/2 + 44, g_height/2 + 150 - 44 + 15, &Click_CloseCondition, NULL, NULL);
	
	AddView("ed con ucount");
	AddText("Owner:", MAINFONT8, g_width/2-100, g_height/2-100+30+15+10);
	AddDropDown("owner", MAINFONT8, g_width/2-100+50, g_height/2-100+30+15+10, 125, &Change_ConUCntOwner);
	for(int i=0; i<PLAYERS; i++)
		AddOption(facnamestr[i]);
	AddText("Count:", MAINFONT8, g_width/2-100, g_height/2-100+30+15*2+10*2);
	AddEditBox("count", MAINFONT8, "", g_width/2-100+50, g_height/2-100+30+15*2+10*2, 100, false, 15, &Change_ConUCntCount, 0);
	AddText("Unit:", MAINFONT8, g_width/2-100, g_height/2-100+30+15*3+10*3);
	AddDropDown("unitt", MAINFONT8, g_width/2-100+50, g_height/2-100+30+15*3+10*3, 125, &Change_ConUCntType);
	for(int i=0; i<UNIT_TYPES; i++)
		AddOption(g_unitType[i].name);

	AddView("ed con selb");
	AddText("Bldg:", MAINFONT8, g_width/2-100, g_height/2-100+30+15+10);
	AddDropDown("build", MAINFONT8, g_width/2-100+44, g_height/2-100+30+15+10, 150, &Change_ConBuild);
	for(int i=0; i<BUILDING_TYPES; i++)
		AddOption(g_buildingType[i].name);
	AddOption("Pipeline");	// These ID's must match building.h
	AddOption("Powerline");
	AddOption("Road");

	AddView("ed con selu");
	AddText("Unit:", MAINFONT8, g_width/2-100, g_height/2-100+30+15+10);
	AddDropDown("unitt", MAINFONT8, g_width/2-100+44, g_height/2-100+30+15+10, 150, &Change_ConUnit);
	for(int i=0; i<UNIT_TYPES; i++)
		AddOption(g_unitType[i].name);
	
	AddView("ed con selr");
	AddText("Res:", MAINFONT8, g_width/2-100, g_height/2-100+30+15+10);
	AddDropDown("res", MAINFONT8, g_width/2-100+44, g_height/2-100+30+15+10, 150, &Change_ConRes);
	for(int i=0; i<RESOURCES; i++)
	{
		char otext[32];
		sprintf(otext, "%c %s", g_resource[i].icon, g_resource[i].name);
		AddOption(otext);
	}

	AddView("ed con price");
	AddText("Price:", MAINFONT8, g_width/2-100, g_height/2-100+30+15*2+10*2);
	AddEditBox("price", MAINFONT8, "", g_width/2-100+44, g_height/2-100+30+15*2+10*2, 150, false, 15, &Change_ConPrice, 0);
	
	AddView("ed con mapb");
	AddText("Bldg:", MAINFONT8, g_width/2-100-15, g_height/2-100+50);
	AddText("building", "", MAINFONT8, g_width/2-100+44, g_height/2-100+50);
	AddText("Owner:", MAINFONT8, g_width/2-100-15, g_height/2-100+50+15+10);
	AddText("owner", "", MAINFONT8, g_width/2-100+44, g_height/2-100+50+15+10);
	AddButton("gui\\buttonbg.png", "select", MAINFONT8, g_width/2-100+30, g_height/2-100+50+15*2+10*2, g_width/2-100+30+100, g_height/2-100+50+15*3+10*2, &Click_ConSelB, NULL, NULL);

	AddView("ed effect");
	AddMiniFrame(g_width/2-150, g_height/2-100, g_width/2+150, g_height/2+150, 0, 0, true);	
	AddText("Effect", MAINFONT8, g_width/2-44, g_height/2-100+13);
	AddText("Type:", MAINFONT8, g_width/2-100, g_height/2-100+30);
	AddDropDown("type", MAINFONT8, g_width/2-100+44, g_height/2-100+30, 150, &Change_EffType);
	AddOption("none");	// EFFECT_NONE = 0
	AddOption("enable trigger");	// EFFECT_ENABTRIGGER = 1
	AddOption("disable trigger");	// EFFECT_DISABTRIGGER = 2
	AddOption("message");	// EFFECT_MESSAGE = 3
	AddOption("victory");	// EFFECT_VICTORY = 4
	AddOption("loss");	// EFFECT_LOSS = 5
	AddOption("message image");	// EFFECT_MESSAGEIMG = 6
	AddOption("reset conditions");	// EFFECT_RESETCONDITIONS = 7
	AddOption("execute trigger");	// EFFECT_EXECUTETRIGGER = 8
	AddButton("gui\\buttonbg.png", "Done", MAINFONT8, g_width/2 - 44, g_height/2 + 150 - 44, g_width/2 + 44, g_height/2 + 150 - 44 + 15, &Click_CloseEffect, NULL, NULL);
	
	AddView("ed eff selt");
	AddText("Trigger:", MAINFONT8, g_width/2-100-15, g_height/2-100+30+15+10);
	AddDropDown("trigger", MAINFONT8, g_width/2-100+44, g_height/2-100+30+15+10, 150, &Change_EffTrigger);

	AddView("ed eff message");
	int x = g_width/2-100;
	int y = g_height/2-100+50;
	for(int i=0; i<RESOURCES; i++)
	{
		char btext[4];
		sprintf(btext, "%c", g_resource[i].icon);
		AddButton("gui\\buttonbg.png", btext, MAINFONT8, x, y, x+15, y+15, &Click_MessageR, NULL, NULL, i);
		x += 15;
		if(x > g_width/2+100)
		{
			x = g_width/2-100;
			y += 15;
		}
	}
	char btext[4];
	sprintf(btext, "%c", (char)TIME_CHAR);
	AddButton("gui\\buttonbg.png", btext, MAINFONT8, x, y, x+15, y+15, &Click_MessageR, NULL, NULL, RESOURCES);
	AddTextArea("message", "", MAINFONT8, g_width/2-100, g_height/2-100+50+30, g_width/2+100, g_height/2+100, 1, 1, 1, 1, &Change_Message);

	AddView("ed eff selg");
	AddImage("graphic", "", g_width/2-50, g_height/2-50+50, g_width/2+50, g_height/2+50+50, 1, 1, 1, 1);
	AddText("Path:", MAINFONT8, g_width/2-100-15, g_height/2-100+50);
	AddEditBox("path", MAINFONT8, "", g_width/2-100+44, g_height/2-100+50, 150, false, 1024, &Change_EffGPath, 0);
	AddButton("gui\\buttonbg.png", "Choose", MAINFONT8, g_width/2-100+30, g_height/2-100+50+20, g_width/2-100+30+100, g_height/2-100+50+20+15, &Click_EffPickGPath, NULL, NULL);
	AddText("W:", MAINFONT8, g_width/2-100-15, g_height/2-100+50+20+15+10);
	AddEditBox("width", MAINFONT8, "", g_width/2-100+15, g_height/2-100+50+20+15+10, 60, false, 16, &Change_EffGSize, 0);
	AddText("H:", MAINFONT8, g_width/2, g_height/2-100+50+20+15+10);
	AddEditBox("height", MAINFONT8, "", g_width/2+15, g_height/2-100+50+20+15+10, 60, false, 16, &Change_EffGSize, 0);
	
	AddView("ed eff selm");
	AddText("Path:", MAINFONT8, g_width/2-100-15, g_height/2-100+50);
	AddEditBox("path", MAINFONT8, "", g_width/2-100+44, g_height/2-100+50, 150, false, 1024, &Change_EffMPath, 0);
	AddButton("gui\\buttonbg.png", "Choose", MAINFONT8, g_width/2-100+30, g_height/2-100+50+20, g_width/2-100+30+100, g_height/2-100+50+20+15, &Click_EffPickMPath, NULL, NULL);

	AddView("name unique");
	AddGrayBGFr(g_width/2-100, g_height/2-50, g_width/2+100, g_height/2+50);
	AddText("Name must be unique.", MAINFONT8, g_width/2-80, g_height/2-40);
	AddButton("gui\\buttonbg.png", "OK", MAINFONT8, g_width/2 - 44, g_height/2+50 - 30, g_width/2 + 44, g_height/2+50 - 30 + 15, &Click_CloseUnique, NULL, NULL);
}

void OpenPlay()
{
	OpenSoleView("game");
	OpenAnotherView("chat");
	if(g_gameover)
		OpenAnotherView("game over");
}

void OpenEditor()
{
	OpenSoleView("editor");
	OpenAnotherView("chat");
}

void OpenMenu()
{
	OpenSoleView("main");
}

void RedoGUI()
{
	g_GUI.view.clear();

	AddView("logo");
	AddImage("gui\\centerp\\dmd.jpg", g_width/2 - g_height/2, 0, g_width/2 + g_height/2, g_height, 1,1,1,0);

	AddView("loading");
	AddPCSBG();
	AddText("title", "LOADING...", UNSTEADYOVERSTEER20, g_width/2 - 50, 28, 0.6f, 0.6f, 0.6f, 1.0f, true);
	//AddText("status", "", MAINFONT8, g_width/2 - g_height/2 + g_height*0.712f, g_height*0.349f);
	AddText("status", "", UNSTEADYOVERSTEER8, g_width/2-120, g_height - 50, 0.6f, 0.6f, 0.6f, 1.0f, false);
	/*
	g_GUI.getview("loading")->widget[1].rgba[0] = 0;
	g_GUI.getview("loading")->widget[1].rgba[1] = 0;
	g_GUI.getview("loading")->widget[1].rgba[2] = 0;
	g_GUI.getview("loading")->widget[1].shadow = false;*/

	AddView("main");
	AddPCSBG();
	AddText("title", "CORPORATION-STATES", UNSTEADYOVERSTEER20, g_width/2 - 140, 28, 0.6f, 0.6f, 0.6f, 1.0f, true);
	//AddImage("fonts\\gulim32.png", g_width/2 - g_height/2, 0, g_width/2 + g_height/2, g_height);
	//AddLink(CLink("Join", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*3, &Click_OpenJoin));
	//AddLink(CLink("Host", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*2, &Click_OpenHost));
	AddLink("Play", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*2, &Click_Play);
	AddLink("Editor", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*1, &Click_Editor);
	AddLink("Settings", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*0, &Click_Settings);
	AddLink("Credits", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*1, &Click_Credits);
	AddLink("Quit", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*2, &Click_Quit);
	
	AddView("play");
	AddPCSBG();
	AddText("title", "PLAY GAME", UNSTEADYOVERSTEER20, g_width/2 - 50, 28, 0.6f, 0.6f, 0.6f, 1.0f, true);
	AddLink("New Game", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*1, &Click_NewGame);
	AddLink("Load Game", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*0, &Click_OpenLoad);
	AddLink("Back", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*1, &Click_BackToMain);
	
	AddView("ingame");
	AddPCSBG();
	AddText("title", "IN-GAME", UNSTEADYOVERSTEER20, g_width/2 - 50, 28, 0.6f, 0.6f, 0.6f, 1.0f, true);
	AddLink("Save Game", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*2, &Click_OpenSave);
	AddLink("New Game", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*1, &Click_NewGame);
	AddLink("Load Game", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*0, &Click_OpenLoad);
	AddLink("Editor", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*1, &Click_Editor);
	AddLink("Back", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*2, &Click_BackToGame);
	
	AddView("save");
	AddPCSBG();
	AddText("title", "SAVE GAME", UNSTEADYOVERSTEER20, g_width/2 - 50, 28, 0.6f, 0.6f, 0.6f, 1.0f, true);
	AddLink("          Slot 1", MAINFONT32, g_width/2 - 150, g_height/2 - g_font[MAINFONT32].gheight*2, &Click_SaveSlot1);
	AddLink("          Slot 2", MAINFONT32, g_width/2 - 150, g_height/2 - g_font[MAINFONT32].gheight*1, &Click_SaveSlot2);
	AddLink("          Slot 3", MAINFONT32, g_width/2 - 150, g_height/2 - g_font[MAINFONT32].gheight*0, &Click_SaveSlot3);
	AddLink("Back", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*1, &Click_BackToInGame);
	
	AddView("load");
	AddPCSBG();
	AddText("title", "LOAD GAME", UNSTEADYOVERSTEER20, g_width/2 - 50, 28, 0.6f, 0.6f, 0.6f, 1.0f, true);
	AddLink("          Slot 1", MAINFONT32, g_width/2 - 150, g_height/2 - g_font[MAINFONT32].gheight*2, &Click_LoadSlot1);
	AddLink("          Slot 2", MAINFONT32, g_width/2 - 150, g_height/2 - g_font[MAINFONT32].gheight*1, &Click_LoadSlot2);
	AddLink("          Slot 3", MAINFONT32, g_width/2 - 150, g_height/2 - g_font[MAINFONT32].gheight*0, &Click_LoadSlot3);
	AddLink("Back", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*1, &Click_BackToInGame);
	
	AddView("pause");
	AddPCSBG();
	AddLink("Back", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*2, &Click_BackToGame);
	AddLink("Settings", MAINFONT32, g_width/2 - 50, g_height/2 - g_font[MAINFONT32].gheight*1, &Click_Settings);
	AddLink("Credits", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*0, &Click_Credits);
	AddLink("Quit", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*1, &Click_Quit);

	AddView("settings");
	AddPCSBG();
	AddText("title", "SETTINGS", UNSTEADYOVERSTEER20, g_width/2 - 50, 28, 0.6f, 0.6f, 0.6f, 1.0f, true);
	AddText("Fullscreen:", MAINFONT32, g_width/2 - 225, g_height/2 - g_font[MAINFONT32].gheight*2);
	AddDropDown("fullscreen", MAINFONT32, g_width/2 - 25, g_height/2 - g_font[MAINFONT32].gheight*2, 250, &Change_Fullscreen);
	AddOption("No");
	if(!g_fullscreen)
		SelectOption();
	AddOption("Yes");
	if(g_fullscreen)
		SelectOption();
	AddText("Resolution:", MAINFONT32, g_width/2 - 225, g_height/2 - g_font[MAINFONT32].gheight*1);
	AddDropDown("resolution", MAINFONT32, g_width/2 - 25, g_height/2 - g_font[MAINFONT32].gheight*1, 250, &Change_Resolution);
	for(int i=0; i<g_resolution.size(); i++)
	{
		char res[64];
		sprintf(res, "%d x %d", g_resolution[i].width, g_resolution[i].height);
		AddOption(res);
		if(g_resolution[i].width == g_selectedRes.width && g_resolution[i].height == g_selectedRes.height)
			SelectOption();
	}
	AddText("Bit depth:", MAINFONT32, g_width/2 - 225, g_height/2 - g_font[MAINFONT32].gheight*0);
	AddDropDown("bpp", MAINFONT32, g_width/2 - 25, g_height/2 - g_font[MAINFONT32].gheight*0, 250, &Change_BPP);
	for(int i=0; i<g_bpps.size(); i++)
	{
		char bpp[64];
		sprintf(bpp, "%d", g_bpps[i]);
		AddOption(bpp);
		if(g_bpps[i] == g_bpp)
			SelectOption();
	}
	AddLink("Back", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*2, &Click_BackToMain);

	AddView("credits");
	AddPCSBG();
	AddText("title", "CREDITS", UNSTEADYOVERSTEER20, g_width/2 - 50, 28, 0.6f, 0.6f, 0.6f, 1.0f, true);
	AddText("PROGRAMMING", MAINFONT16, g_width/2 - 150, g_height/2 - g_font[MAINFONT16].gheight*4);
	AddText("Denis Ivanov", MAINFONT16, g_width/2 - 150, g_height/2 - g_font[MAINFONT16].gheight*3);
	//AddText("Volodar", MAINFONT16, g_width/2 - 50, g_height/2 - g_font[MAINFONT16].gheight*2);
	AddText("MODELS", MAINFONT16, g_width/2 - 150, g_height/2 + g_font[MAINFONT16].gheight*0);
	AddText("Denis Ivanov", MAINFONT16, g_width/2 - 150, g_height/2 + g_font[MAINFONT16].gheight*1);
	//AddText("Sudden", MAINFONT16, g_width/2 - 50, g_height/2 + g_font[MAINFONT16].gheight*2);
	AddText("ART", MAINFONT16, g_width/2 + 50, g_height/2 - g_font[MAINFONT16].gheight*4);
	AddText("Denis Ivanov", MAINFONT16, g_width/2 + 50, g_height/2 - g_font[MAINFONT16].gheight*3);
	AddText("VOICE", MAINFONT16, g_width/2 + 50, g_height/2 + g_font[MAINFONT16].gheight*0);
	AddText("\"Inkling\"", MAINFONT16, g_width/2 + 50, g_height/2 + g_font[MAINFONT16].gheight*1);
	AddLink("Back", MAINFONT32, g_width/2 - 50, g_height/2 + g_font[MAINFONT32].gheight*4, &Click_BackToMain);
	
	AddView("game");
	AddImage("gui\\lbar.jpg", 0, MMBAR_HEIGHT, LBAR_WIDTH, g_height);
	//AddMiniFrame(0, MMBAR_HEIGHT, LBAR_WIDTH, g_height, 200, 200, true);
	AddImage("gui\\minimap.png", 0, 0, MMBAR_WIDTH, MMBAR_HEIGHT);
	AddInsDraw(&DrawMinimap);
	AddImage("gui\\tbar.jpg", MMBAR_WIDTH, 0, g_width, TBAR_HEIGHT);
	AddText("resource", "", MAINFONT8, MMBAR_WIDTH, 0);
	//AddButton("gui\\build.png", LBAR_WIDTH/2 - 25, MMBAR_HEIGHT + 44*0.4f, LBAR_WIDTH/2 + 19, MMBAR_HEIGHT + 44*1.4f, &Click_OpenBuild, NULL, NULL);
	AddButton("gui\\build.png", "", MAINFONT8, LBAR_WIDTH/2 - 44/2, MMBAR_HEIGHT, LBAR_WIDTH/2 + 44/2, MMBAR_HEIGHT + 44, &Click_OpenBuild, NULL, NULL);
	AddButton("gui\\question.png", "", MAINFONT8, LBAR_WIDTH/2 - 44/2, MMBAR_HEIGHT + 44+10, LBAR_WIDTH/2 + 44/2, MMBAR_HEIGHT + 44*2+10, &Click_LastHint, NULL, NULL);
	AddButton("gui\\exitsign.png", "", MAINFONT8, LBAR_WIDTH/2 - 44/2, MMBAR_HEIGHT + 44*2+10*2, LBAR_WIDTH/2 + 44/2, MMBAR_HEIGHT + 44*3+10*2, &Click_Exit, NULL, NULL);
	
	AddView("chat");
	AddText("chat0", "", MAINFONT8, MMBAR_WIDTH, TBAR_HEIGHT+0*g_font[MAINFONT8].gheight);
	AddText("chat1", "", MAINFONT8, MMBAR_WIDTH, TBAR_HEIGHT+1*g_font[MAINFONT8].gheight);
	AddText("chat2", "", MAINFONT8, MMBAR_WIDTH, TBAR_HEIGHT+2*g_font[MAINFONT8].gheight);
	AddText("chat3", "", MAINFONT8, MMBAR_WIDTH, TBAR_HEIGHT+3*g_font[MAINFONT8].gheight);
	AddText("chat4", "", MAINFONT8, MMBAR_WIDTH, TBAR_HEIGHT+4*g_font[MAINFONT8].gheight);
	AddText("chat5", "", MAINFONT8, MMBAR_WIDTH, TBAR_HEIGHT+5*g_font[MAINFONT8].gheight);
	AddText("fps", "", MAINFONT8, MMBAR_WIDTH, TBAR_HEIGHT+7*g_font[MAINFONT8].gheight);

	/*
	AddText("", "Time", MAINFONT16, g_width-148, TBAR_HEIGHT);
	AddText("time", "0:00", MAINFONT32, g_width-148, TBAR_HEIGHT+16);
	
	AddText("", "Next Wave In", MAINFONT16, g_width-148, TBAR_HEIGHT+80);
	AddText("wave", "0:00", MAINFONT32, g_width-148, TBAR_HEIGHT+96);
	*/
	
	ReGUIEd();

	//AddView("quit");	//Ask the player if they want to quit the multiplayer game
	// TO DO

	AddView("game message");
	//AddMiniFrame(g_width/2-150, g_height/2-100, g_width/2+150, g_height/2+150, 0, 0, true);	
	AddGrayBGFr(g_width/2-150, g_height/2-100, g_width/2+150, g_height/2+150);	
	AddImage("graphic", "", g_width/2-50, g_height/2-50+50, g_width/2+50, g_height/2+50+50, 1, 1, 1, 1);
	AddTextBox("message", "", MAINFONT8, g_width/2-100, g_height/2-100+13, g_width/2+100, g_height/2-100+200, 1, 1, 1, 1);
	AddButton("gui\\buttonbg.png", "Continue", MAINFONT8, g_width/2 - 44, g_height/2 + 150 -44-44+15, g_width/2 + 44, g_height/2 + 150 -44+15, &Click_CloseMessage, NULL, NULL);
	
	FillHScroller();
	MakeBInspect();
	
	AddView("game over");
	AddText("", "Game Over", MAINFONT32, g_width/2-150, g_height/2-32, 1.0f, 0.5f, 0.5f, 0.75f, true);
	AddText("reason", "", MAINFONT8, g_width/2-150, g_height/2+32, 1.0f, 0.5f, 0.5f, 0.75f, true);
	
	AddView("victory");
	AddText("", "You Win!", MAINFONT32, g_width/2-150, g_height/2-32);

	if(g_mode == LOGO)
		OpenSoleView("logo");
	else if(g_mode == LOADING || g_mode == RELOADING)
		OpenSoleView("loading");
	else if(g_mode == MENU)
		OpenMenu();
	else if(g_mode == PLAY)
	{
		OpenPlay();
	}
	else if(g_mode == EDITOR)
	{
		OpenEditor();
	}
}

void Status(const char* status, bool logthis)
{
	if(logthis)
	{
		g_log<<status<<endl;
		g_log.flush();
	}

	char upper[1024];
	int i;
	for(i=0; i<strlen(status); i++)
	{
		upper[i] = toupper(status[i]);
	}
	upper[i] = '\0';
	
	//g_GUI.getview("loading")->gettext("status")->text = status;
	g_GUI.getview("loading")->getwidget("status", TEXT)->text = upper;
}
