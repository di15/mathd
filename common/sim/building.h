#ifndef BL_H
#define BL_H

#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "../platform.h"
#include "resources.h"
#include "buildingtype.h"
#include "../render/vertexarray.h"

class VertexArray;

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

	int prodprice[RESOURCES];	//price of produced goods
	int propprice;	//price of this property

	std::list<int> occupier;
	std::list<int> worker;
	int conwage;
	int opwage;
	int cydelay;	//the frame delay between production cycles, when production target is renewed
	int prodlevel;	//production target level of max RATIO_DENOM
	int cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	int lastcy;	//last simframe of last production cycle
	std::list<CapSup> capsup;	//capacity suppliers

	bool excin(int rtype);	//excess input resource right now?
	bool metout();	//met production target for now?
	int stillreq(int rtype);	//how much of an input is still required to meet production target
	bool hasworker(int ui);
	bool tryprod();

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

extern Building g_building[BUILDINGS];
class Unit;

int NewBl();
void FreeBls();
void DrawBl();
void UpdBls();
void StageCopyVA(VertexArray* to, VertexArray* from, float completion);
void HeightCopyVA(VertexArray* to, VertexArray* from, float completion);
void HugTerrain(VertexArray* va, Vec3f pos);
void Explode(Building* b);
float CompletPct(int* cost, int* current);
void RemWorker(Unit* w);

#endif
