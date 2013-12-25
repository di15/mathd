
#ifndef _BUILDING_H
#define _BUILDING_H

#include <vector>
#include "particle.h"
#include "resource.h"

using namespace std;

class Shader;
class CUnit;

class ManufJob
{
public:
	int utype;
	int owner;
};

class CBuildingType
{
public:
	void draw(int frame, bool finished, Vec3f pos);
	//void Define(char* n, char* modelfile, char* conmodelfile, int wX, int wZ, float hp);
	//void Sprite(char* file);
	//void Cost(float currency, float labour, float consumergoods, float produce, float production, float ore, float metal, float crude, float zetrol, float stone, float cement, float electricity, float uronium, float cohl, float housing, float chemicals, float electronics);
	//void Input(float currency, float labour, float consumergoods, float produce, float production, float ore, float metal, float crude, float zetrol, float stone, float cement, float electricity, float uronium, float cohl, float housing, float chemicals, float electronics);
	//void Output(float currency, float labour, float consumergoods, float produce, float production, float ore, float metal, float crude, float zetrol, float stone, float cement, float electricity, float uronium, float cohl, float housing, float chemicals, float electronics);
	//void Emitter(int type, Vec3f offset) { emitter.push_back(EmitterPlace(type, offset)); }

	char name[32];
	unsigned int sprite;
	int widthX, widthZ;
	int model;
	int conmodel;
	vector<EmitterPlace> emitter;
	float cost[RESOURCES];
	float input[RESOURCES];
	float output[RESOURCES];
	float hitpoints;
	float maxprodquota;
	vector<int> buildable;
};

//enum BUILDINGTYPE{PIPELINE=-4, POWERLINE=-3, ROAD=-2, NOTHING=-1, APARTMENT=0, SHOPCPLX, FACTORY, FARM, MINE, SMELTER, DERRICK, REFINERY, REACTOR, COMBUSTOR, QUARRY, CEMPLANT, CHEMPLANT, ELECPLANT, RNDFACILITY, BUILDING_TYPES};
#define NOTHING			-1
#define APARTMENT		0
#define SHOPCPLX		1
#define FACTORY			2
#define FARM			3
#define MINE			4
#define SMELTER			5
#define DERRICK			6
#define REFINERY		7
#define REACTOR			8
#define COMBUSTOR		9
#define QUARRY			10
#define CEMPLANT		11
#define CHEMPLANT		12
#define ELECPLANT		13
/*
#define BUILDING_TYPES	14
#define PIPELINE		14
#define POWERLINE		15
#define ROAD			16
#define BUILDABLES		17*/

#define RNDFACILITY		14
#define BUILDING_TYPES	15
#define PIPELINE		15
#define POWERLINE		16
#define ROAD			17
#define BUILDABLES		18
extern CBuildingType g_buildingType[BUILDING_TYPES];

extern int g_build;
extern bool g_canPlace;

class CSupply
{
public:
	int supplierb;
	float getting;
};

class CCycleH
{
public:
	float produced[RESOURCES];
	float consumed[RESOURCES];

	void reset()
	{
		Zero(produced);
		Zero(consumed);
	}
};

class CBuilding
{
public:
	bool on;
	int owner;
	bool finished;
	int frame;
	int type;
	float hp;
	Vec3f pos;
	float stock[RESOURCES];
	float conmat[RESOURCES];	// construction material
	int pownetw;	//power network
	int pipenetw;	//pipe network
	vector<int> occupier;
	vector<ManufJob> produ;	//units in production
	vector<EmitterCounter> emitter;
	float prodquota;
	float quotafilled[RESOURCES];
	unsigned int framesago;
	vector<CCycleH> cycleh;
	CCycleH recenth;
	vector<int> roadnetw;
	int transporter[RESOURCES];
	bool forsale;
	float saleprice;

	CBuilding() 
	{ 
		on = false; 
		finished = false; 
		owner = -1;
		frame = 0;
		Zero(stock);
		Zero(conmat);
		pownetw = -1;
		prodquota = 1;
		Zero(quotafilled);
	}
	void convert(int newowner);
	void fillemitters();
	void draw();
	bool trybuy(int buyer);
	void set(int t, int o, Vec3f p);
	void EmitParticles();
	void spawn(int ut, int ownr);
	void UpdateQueueU();
	void process();
	bool BuyE();
	bool BuyCrude();
	bool excessoutput();
	bool excessinput(int res);
	void update();
	bool CheckConstruction();
	void Allocate();
	void Emit(int t);
	void EmitRes(int r);
	void Explode();
	bool QueueU(int ut, int buyerplayer);
	bool DequeueU(int ut, int ownr);
	void addoccupier(CUnit* u);
	void RemoveWorker(CUnit* u);
	bool Working(CUnit* u);
	void destroy();
	void cycle();
	float netreq(int res);
	void fillcollidercells();
	void freecollidercells();
};

#define BUILDINGS	128
extern CBuilding g_building[BUILDINGS];

class Shader;

#define HOVER_SIZE	64
enum HOVERTEX{HOVER_NOELEC, HOVER_NOROAD, HOVER_NOCRUDE, HOVERTEXS};

extern unsigned int g_hovertex[HOVERTEXS];

void DrawBStats(Matrix projmodlview);
void DrawBStatus(Matrix projmodlview);
void LoadHoverTex();
int BuildingID(CBuilding* b);
void InitBuildings();
void BSprites();
void DrawSelB();
bool CheckCanPlace(int btype, Vec3f vTile);
int PlaceB(int player, int btype, Vec3f vTile);
void DrawBuildings();
void UpdateBuildings();

#endif