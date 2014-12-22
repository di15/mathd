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
	std::string name;
	float rgba[4];
	std::string depositn;
	int conduit;
};

//conduit
#define CONDUIT_NONE		-1
#define CONDUIT_ROAD		0
#define CONDUIT_POWL		1
#define	CONDUIT_CRPIPE		2

#define RES_NONE			-1
#define RES_DOLLARS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_CHEMICALS		5
#define RES_ELECTRONICS		6
#define RES_IRONORE			7
#define RES_METAL			8
#define RES_STONE			9
#define RES_CEMENT			10
#define RES_COAL			11
#define RES_URANIUM			12
#define RES_PRODUCTION		13
#define RES_CRUDEOIL		14
#define RES_FUEL			15
#define RES_ENERGY			16
#define RESOURCES			17
extern Resource g_resource[RESOURCES];

class Basket
{
public:
	int r[RESOURCES];

	int& operator[](const int i)
	{
		return r[i];
	}
};

class Bundle
{
public:
	unsigned char res;
	int amt;
};

//capacity supply (e.g. electricity, water pressure)
class CapSup
{
public:
	unsigned char rtype;
	int amt;
	int src;
	int dst;
};

void DefR(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a, int conduit);
void Zero(int *b);
bool ResB(int building, int res);
bool TrySub(const int* cost, int* universal, int* stock, int* local, int* netch, int* insufres);

#endif
