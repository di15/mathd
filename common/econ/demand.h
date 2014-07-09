
#ifndef DEMAND_H
#define DEMAND_H

#include "../math/vec2i.h"
#include "../platform.h"
#include "../sim/resources.h"
#include "../sim/unittype.h"
#include "../sim/buildingtype.h"
#include "../sim/build.h"

#define RATIO_DENOM		100000

#define DEM_NODE		0	//	unknown node
#define DEM_RNODE		1	//	resource demand
#define DEM_BNODE		2	//	demand at building, for building
#define DEM_UNODE		3	//	demand for unit: worker, transport, or military

class DemNode
{
public:
	int demtype;
	DemNode* parent;

	DemNode();
};

class RDemNode : public DemNode
{
public:
	int rtype;
	int ramt;
	int btype;
	int bi;
	int utype;
	int ui;
	DemsAtB* supbp;
	DemsAtU* supup;

	RDemNode() : DemNode()
	{
		demtype = DEM_RNODE;
		rtype = -1;
		ramt = 0;
		btype = -1;
		bi = -1;
		utype = -1;
		ui = -1;
		supbp = NULL;
		supup = NULL;
	}
};

class DemsAtB : public DemNode
{
public:
	int bi;
	int btype;
	std::list<DemNode*> condems;	//construction material
	std::list<DemNode*> proddems;	//production input raw materials
	std::list<DemNode*> manufdems;	//manufacturing input raw materials
	int prodratio;
	int condem[RESOURCES];
	int supplying[RESOURCES];

	DemsAtB() : DemNode()
	{
		demtype = DEM_BNODE;
		prodratio = 0;
		Zero(supplying);
		Zero(condem);
	}

	~DemsAtB()
	{
		auto riter = condems.begin();
		while(riter != condems.end())
		{
			delete *riter;
			riter = condems.erase(riter);
		}

		riter = proddems.begin();
		while(riter != proddems.end())
		{
			delete *riter;
			riter = proddems.erase(riter);
		}

		riter = manufdems.begin();
		while(riter != manufdems.end())
		{
			delete *riter;
			riter = manufdems.erase(riter);
		}
	}
};

class DemsAtU : public DemNode
{
public:
	int ui;
	int utype;
	std::list<DemNode*> manufdems;
	std::list<DemNode*> consumdems;
	DemsAtU* opup;	//operator/driver
	int prodratio;
	int timeused;
	int totaldem[RESOURCES];
	
	DemsAtU() : DemNode()
	{
		demtype = DEM_UNODE;
		prodratio = 0;
		Zero(totaldem);
		opup = NULL;
		timeused = 0;
	}

	~DemsAtU()
	{
		auto riter = manufdems.begin();
		while(riter != manufdems.end())
		{
			delete *riter;
			riter = manufdems.erase(riter);
		}
		
		riter = consumdems.begin();
		while(riter != consumdems.end())
		{
			delete *riter;
			riter = consumdems.erase(riter);
		}
	}
};

class DemTree
{
public:
	std::list<DemNode*> nodes;
	std::list<DemsAtB*> supbpcopy;	//master copy, this one will be freed
	std::list<DemsAtU*> supupcopy;	//master copy, this one will be freed

	void free()
	{
		auto biter = supbpcopy.begin();
		while(biter != supbpcopy.end())
		{
			delete *biter;
			biter = supbpcopy.erase(biter);
		}
		
		auto uiter = supupcopy.begin();
		while(uiter != supupcopy.end())
		{
			delete *uiter;
			uiter = supupcopy.erase(uiter);
		}
		
		auto riter = nodes.begin();
		while(riter != nodes.end())
		{
			delete *riter;
			riter = nodes.erase(riter);
		}
	}

	~DemTree()
	{
		free();
	}
};

void CalcDem();

#endif
