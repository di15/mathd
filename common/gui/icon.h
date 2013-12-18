

#ifndef ICON_H
#define ICON_H

#include "../platform.h"
#include "../ustring.h"

class Icon
{
public:
	unsigned int m_tex;
	int m_width;
	int m_height;
	UString m_tag;
};

#define ICON_CENTS			0
#define ICON_LABOUR			1
#define ICON_HOUSING		2
#define ICON_FOODWS			3
#define ICON_FOOD			4
#define ICON_CHEMICALS		5
#define ICON_ELECTRONICS	6
#define ICON_RESEARCH		7
#define ICON_PRODUCTION		8
#define ICON_MENDOLITE		9
#define ICON_URONITE		10
#define ICON_MENDOLIUM		11
#define ICON_CRUDE			12
#define ICON_ZETROL			13
#define ICON_STONE			14
#define ICON_CEMENT			15
#define ICON_ENERGY			16
#define ICON_URONIUM		17
#define ICON_COHL			18
#define ICON_TIME			19
#define ICON_ZETROLRET		20
#define ICONS				21

extern Icon g_icon[ICONS];

#ifndef _SERVER
void DefineIcon(int type, const char* relative, const UString tag);
#endif

#endif