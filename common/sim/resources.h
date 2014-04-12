


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
#define RES_WSFOOD			11
#define RES_RETFOOD			12
#define RES_CHEMICALS		13
#define RES_ELECTRONICS		14
#define RES_RESEARCH		15
#define RES_PRODUCTION		16
#define RES_IRONORE			17
#define RES_URANIUMORE		18
#define RES_STEEL			19
#define RES_CRUDEOIL		20
#define RES_WSFUEL			21
#define RES_RETFUEL			22
#define RES_STONE			23
#define RES_CEMENT			24
#define RES_ENERGY			25
#define RES_ENRICHEDURAN	26
#define RES_COAL			27
#define RES_LOGS			28
#define RES_LUMBER			29
#define RES_WATER			30
#define RESOURCES			31
extern Resource g_resource[RESOURCES];

class Basket
{
public:
	int r[RESOURCES];
};

class Bundle
{
public:
	unsigned char res;
	int amt;
};

void DefineRes(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a);
void Zero(int *b);
bool ResB(int building, int res);

#endif