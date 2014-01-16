


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

#define RES_FUNDS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_FOOD			4
#define RES_CHEMICALS		5
#define RES_ELECTRONICS		6
#define RES_RESEARCH		7
#define RES_PRODUCTION		8
#define RES_IRONORE			9
#define RES_URANIUMORE		10
#define RES_STEEL			11
#define RES_CRUDEOIL		12
#define RES_WSFUEL			13
#define RES_RETFUEL			14
#define RES_STONE			15
#define RES_CEMENT			16
#define RES_ENERGY			17
#define RES_ENRICHEDURAN	18
#define RES_COAL			19
#define RESOURCES			20
extern Resource g_resource[RESOURCES];

void DefineRes(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a);
void Zero(int *b);

#endif