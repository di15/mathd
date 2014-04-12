

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

#define ICON_DOLLARS		0
#define ICON_PESOS			1
#define ICON_EUROS			2
#define ICON_POUNDS			3
#define ICON_FRANCS			4
#define ICON_YENS			5
#define ICON_RUPEES			6
#define ICON_ROUBLES		7
#define ICON_LABOUR			8
#define ICON_HOUSING		9
#define ICON_FARMPRODUCT	10
#define ICON_FOOD			11
#define ICON_CHEMICALS		12
#define ICON_ELECTRONICS	13
#define ICON_RESEARCH		14
#define ICON_PRODUCTION		15
#define ICON_IRONORE		16
#define ICON_URANIUMORE		17
#define ICON_STEEL			18
#define ICON_CRUDEOIL		19
#define ICON_WSFUEL			20
#define ICON_STONE			21
#define ICON_CEMENT			22
#define ICON_ENERGY			23
#define ICON_ENRICHEDURAN	24
#define ICON_COAL			25
#define ICON_TIME			26
#define ICON_RETFUEL		27
#define ICON_LOGS			28
#define ICON_LUMBER			29
#define ICON_WATER			30
#define ICONS				31

extern Icon g_icon[ICONS];

#ifndef _SERVER
void DefineIcon(int type, const char* relative, const UString tag);
#endif

#endif