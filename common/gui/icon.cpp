

#include "icon.h"
#include "../render/texture.h"

Icon g_icon[ICONS];

void DefineIcon(int type, const char* relative, const UString tag)
{
	Icon* i = &g_icon[type];

	i->m_tag = tag;
	//QueueTexture(&i->m_tex, relative, true);
	CreateTexture(i->m_tex, relative, true);
	Texture* t = &g_texture[i->m_tex];
	i->m_width = t->width;
	i->m_height = t->height;
}