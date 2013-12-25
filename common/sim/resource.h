
#ifndef _RESOURCE_H
#define _RESOURCE_H

#include <string>

using namespace std;

class CResource
{
public:
	char icon;
	bool physical;
	bool capacity;
	bool global;
	char name[32];
	float rgba[4];
};

enum RESOURCE{CURRENC, LABOUR, HOUSING, CONSUMERGOODS, CHEMICALS, ELECTRONICS, RESEARCH, PRODUCE, PRODUCTION, ORE, METAL, CRUDE, ZETROL, STONE, CEMENT, ELECTRICITY, URONIUM, COHL, RESOURCES};
extern CResource g_resource[RESOURCES];
extern int g_insufficient;
extern int g_resEd;

#define TIME_CHAR	((char)0x15)

void Zero(float *r);
bool ResB(int building, int res);
void InitResources();
void ResourceTicker();
bool TrySubtract(float* cost, float* global, float* stock=NULL, float* local=NULL, float* netch=NULL);
void PlusWage();
void MinusWage();
void NumPadWage();
void PlusTruckWage();
void MinusTruckWage();
void NumPadPrice(int r);
void PlusPrice(int r);
void MinusPrice(int r);
void RDelta(int player);

#endif