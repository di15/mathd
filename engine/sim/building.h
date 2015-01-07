#ifndef BL_H
#define BL_H

#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "../platform.h"
#include "resources.h"
#include "bltype.h"
#include "../render/vertexarray.h"
#include "utype.h"

class VertexArray;

//#define PROD_DEBUG	//bl's production debug output

//unit manufacturing job
class ManufJob
{
public:
	int utype;
	int owner;
};

class Building
{
public:
	bool on;
	int type;
	int owner;

	Vec2i tilepos;	//position in tiles
	Vec3f drawpos;	//drawing position in centimeters

	bool finished;

	short pownetw;
	short crpipenetw;
	std::list<short> roadnetw;

	int stocked[RESOURCES];
	int inuse[RESOURCES];

	EmitterCounter emitterco[MAX_B_EMITTERS];

	VertexArray drawva;

	int conmat[RESOURCES];
	bool inoperation;

	int price[RESOURCES];	//price of produced goods
	int propprice;	//price of this property

	std::list<int> occupier;
	std::list<int> worker;
	int conwage;
	int opwage;
	int cydelay;	//the frame delay between production cycles, when production target is renewed
	short prodlevel;	//production target level of max RATIO_DENOM
	short cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	unsigned int lastcy;	//last simframe of last production cycle
	std::list<CapSup> capsup;	//capacity suppliers

	int manufprc[UNIT_TYPES];
	std::list<ManufJob> manufjob;
	short transporter[RESOURCES];

	int hp;

	bool excin(int rtype);	//excess input resource right now?
	bool metout();	//met production target for now?
	int netreq(int rtype);	//how much of an input is still required to meet production target
	bool hasworker(int ui);
	bool tryprod();
	bool trymanuf();
	int maxprod();
	void adjcaps();
	void spawn(int utype, int uowner);
	void morecap(int rtype, int amt);
	void lesscap(int rtype, int amt);
	void getether();
	void getether(int rtype, int amt);

	void destroy();
	void fillcollider();
	void freecollider();
	void allocres();
	void remesh();
	bool checkconstruction();
	Building();
	~Building();
};

#define BUILDINGS	256
//#define BUILDINGS	64

extern Building g_building[BUILDINGS];
class Unit;

int NewBl();
void FreeBls();
void DrawBls();
void UpdBls();
void StageCopyVA(VertexArray* to, VertexArray* from, float completion);
void HeightCopyVA(VertexArray* to, VertexArray* from, float completion);
void HugTerrain(VertexArray* va, Vec3f pos);
void Explode(Building* b);
float CompletPct(int* cost, int* current);
void RemWorker(Unit* w);

#endif
