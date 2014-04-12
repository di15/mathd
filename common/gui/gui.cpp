


#include "../render/shader.h"
#include "gui.h"
#include "../texture.h"
#include "font.h"
#include "../math/3dmath.h"
#include "../platform.h"
#include "../window.h"
#include "draw2d.h"
#include "../render/shadow.h"
#include "../render/heightmap.h"
#include "../../game/gmain.h"

GUI g_GUI;
int g_currw;
int g_currh;

/*
Determines if the cursor is over an actionable widget, like a drop-down selector.
If it is, we don't want to scroll if the mouse is at the edge of the screen because
the user is trying to do something.
*/
bool g_mouseoveraction = false;

void GUI::draw()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	Ortho(g_width, g_height, 1, 1, 1, 1);
	
#if 0
	DrawImage(g_texture[0].texname, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);
#endif

	for(auto i=view.begin(); i!=view.end(); i++)
		if(i->opened)
			i->draw();
	
	for(auto i=view.begin(); i!=view.end(); i++)
		if(i->opened)
			i->draw2();

#if 0
	DrawImage(g_depth, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);
#endif

#if 0
	//if(g_depth != -1)
	DrawImage(g_depth, 0, 0, 150, 150, 0, 1, 1, 0);
#endif

#if 0
	if(g_mode == PLAY)
		DrawImage(g_tiletexs[TILE_PRERENDER], 0, 0, 150, 150, 0, 1, 1, 0);
#endif

	UseS(SHADER_COLOR2D);
    glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_height);
	glEnable(GL_DEPTH_TEST);
	//DrawSelector();
}

void AssignKey(int i, void (*down)(), void (*up)())
{
	g_GUI.assignKey(i, down, up);
}

void AssignAnyKey(void (*down)(int k), void (*up)(int k))
{
	g_GUI.assignAnyKey(down, up);
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

View* AddView(const char* name, int page)
{
	View view(name, page);
	g_GUI.view.push_back(view);
	return &*g_GUI.view.rbegin();
}

void CloseView(const char* name)
{
	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
		if(_stricmp(i->name.c_str(), name) == 0)
			i->opened = false;
}

void OpenSoleView(const char* name, int page)
{
	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
	{
		if(_stricmp(i->name.c_str(), name) == 0 && i->page == page)
			i->opened = true;
		else
			i->opened = false;
	}
}

bool OpenAnotherView(const char* name, int page)
{
	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
	{
		if(_stricmp(i->name.c_str(), name) == 0 && i->page == page)
		{
#if 0
			g_log<<"open an "<<name<<endl;
			g_log.flush();
#endif
			i->opened = true;
			return true;
		}
	}

	return false;
}

bool ViewOpen(const char* name, int page)
{
	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
		if(_stricmp(i->name.c_str(), name) == 0 && i->page == page)
			return i->opened;

	return false;
}

void NextPage(const char* name)
{
	int page = 0;

	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
	{
		if(_stricmp(i->name.c_str(), name) == 0 && i->opened)
		{
			page = i->page;
			i->opened = false;
			break;
		}
	}

	if(!OpenAnotherView(name, page+1))
		OpenAnotherView(name, 0);
}

void Status(const char* status, bool logthis)
{
	if(logthis)
	{
		g_log<<status<<endl;
		g_log.flush();
	}

#if 1
	g_log<<status<<endl;
	g_log.flush();
#endif
	/*
	char upper[1024];
	int i;
	for(i=0; i<strlen(status); i++)
	{
		upper[i] = toupper(status[i]);
	}
	upper[i] = '\0';*/
	
	//g_GUI.getview("loading")->getwidget("status", WIDGET_TEXT)->m_text = upper;
	View* loadingview = g_GUI.getview("loading");
	
	if(!loadingview)
		return;
	
	Widget* statustext = loadingview->getwidget("status", WIDGET_TEXT);
	
	if(!statustext)
		return;
	
	statustext->m_text = RichText(UString(status));
}

bool MousePosition()
{
	Vec2i old = g_mouse;
	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(g_hWnd, &mouse);

	if(mouse.x == old.x && mouse.y == old.y)
		return false;

	g_mouse.x = mouse.x;
	g_mouse.y = mouse.y;

	return true;
}

void CenterMouse()
{
	g_mouse.x = g_width/2;
	g_mouse.y = g_height/2;
	POINT temp;
	temp.x = g_mouse.x;
	temp.y = g_mouse.y;
	ClientToScreen(g_hWnd, &temp);
	SetCursorPos(temp.x, temp.y);
}

void Ortho(int width, int height, float r, float g, float b, float a)
{
	UseS(SHADER_ORTHO);
	glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_WIDTH], (float)width);
	glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_HEIGHT], (float)height);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], r, g, b, a);
	glEnableVertexAttribArray(g_shader[SHADER_ORTHO].m_slot[SSLOT_POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER_ORTHO].m_slot[SSLOT_TEXCOORD0]);
	g_currw = width;
	g_currh = height;
}