


#ifndef RESOURCES_H
#define RESOURCES_H

#include "../platform.h"

class Resource
{
public:
	char icon;
	bool physical;
	bool capacity;
	bool global;
	string name;
	float rgba[4];
	string depositn;
};

#define RES_NONE			-1
#define RES_DOLLARS			0
#define RES_PESOS			1
#define RES_EUROS			2
#define RES_POUNDS			3
#define RES_FRANCS			4
#define RES_YENS			5
#define RES_RUPEES			6
#define RES_ROUBLES			7
#define RES_LABOUR			8
#define RES_HOUSING			9
#define RES_FARMPRODUCTS	10
#define RES_FOOD			11
#define RES_CHEMICALS		12
#define RES_ELECTRONICS		13
#define RES_RESEARCH		14
#define RES_PRODUCTION		15
#define RES_IRONORE			16
#define RES_URANIUMORE		17
#define RES_STEEL			18
#define RES_CRUDEOIL		19
#define RES_WSFUEL			20
#define RES_RETFUEL			21
#define RES_STONE			22
#define RES_CEMENT			23
#define RES_ENERGY			24
#define RES_ENRICHEDURAN	25
#define RES_COAL			26
#define RES_LOGS			27
#define RES_LUMBER			28
#define RES_WATER			29
#define RESOURCES			30
extern Resource g_resource[RESOURCES];

void DefineRes(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a);
void Zero(int *b);
bool ResB(int building, int res);

#endif