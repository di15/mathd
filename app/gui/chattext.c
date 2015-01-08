
#include "chattext.h"
#include "../../engine/gui/gui.h"
#include "../../engine/sim/player.h"
#include "../../engine/sys/window.h"

void Resize_ChatLine(Widget* thisw)
{
	Font* f;
	int i;
	float topy;

	f = &g_font[thisw->m_font];
	i = 0;

	sscanf(thisw->m_name.c_str(), "%d", &i);

	topy = g_height - 200 - CHAT_LINES * f->gheight;

	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = topy + f->gheight * i;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = topy + f->gheight * (i+1);
}

void Resize_ChatPrompt(Widget* thisw)
{
	Font* f;
	int i;

	f = &g_font[thisw->m_font];
	i = CHAT_LINES;
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 30 + f->gheight * i;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = 30 + f->gheight * (i+1);
}

void AddChat(ViewLayer* playview)
{
	char name[32];
	int i;
	Text* textw;
	RichText rt;

	RichText_Init(&rt);

	for(i=0; i<CHAT_LINES; i++)
	{
		sprintf(name, "%d", i);
		textw = (Text*)malloc(sizeof(Text));
		Widget_Text_Init(textw, playview, name, &rt, MAINFONT16, Resize_ChatLine, true, 1.0f, 1.0f, 1.0f, 1.0f);
		Widget_Add((Widget*)playview, textw);
	}

	RichText_Free(&rt);
}

void AddChat(RichText* newl)
{
	GUI* gui;
	ViewLayer* playview;
	int i;
	char name[32];
	char name2[32];
	Text* textw;
	Text* textw2;

	gui = &g_gui;
	playview = (ViewLayer*)gui->get("play");

	for(i=0; i<CHAT_LINES-1; i++)
	{
		sprintf(name, "%d", i);
		sprintf(name2, "%d", i+1);
		
		textw = (Text*)Widget_Get(playview, name);
		textw2 = (Text*)Widget_Get(playview, name2);

		RichText_Free(&textw->m_text);
		RichText_Copy(&textw->m_text, &textw2->m_text);
	}

	sprintf(name, "%d", CHAT_LINES-1);
	textw = (Text*)Widget_Get(playview, name);
	RichText_Free(&textw->m_text);
	RichText_Copy(&textw->m_text, newl);
}