

#include "infrastructure.h"
#include "../render/heightmap.h"
#include "building.h"
#include "../phys/collision.h"

ConduitType g_cotype[CONDUITS];


void ConduitType::updplans(char owner, Vec3f start, Vec3f end)
{
}

void ConduitType::clearplans()
{
	if(!get)
		return;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
			get(x, z, true)->on = false;
}

void ConduitType::resetnetw()
{
	int lastnetw = 0;
	Building* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(blconduct)
		{
			short* netw = (short*)(((char*)b)+netwoff);
			*netw = -1;
		}
		else
		{
			std::list<short>* netw = (std::list<short>*)(((char*)b)+netwoff);
			netw->clear();
		}
	}

	if(!get)
		return;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			ConduitTile* ct = get(x, z, false);

			if(!ct->on)
				continue;

			if(!ct->finished)
				continue;

			ct->netw = lastnetw++;
		}
}

// If buildings conduct the resource,
// merge the networks of touching buildings.
bool ConduitType::renetwb()
{
	if(!blconduct)
		return false;

	bool change = false;
	
	Building* b;
	Building* b2;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
		short& netw = *(short*)(((char*)b)+netwoff);

		if(!b->on)
			continue;

		for(int j=i+1; j<BUILDINGS; j++)
		{
			b2 = &g_building[j];

			if(!b2->on)
				continue;

			if(!BlAdj(i, j))
				continue;
			
			short& netw2 = *(short*)(((char*)b2)+netwoff);

			if(netw < 0 && netw2 >= 0)
			{
				netw = netw2;
				change = true;
			}
			else if(netw2 < 0 && netw >= 0)
			{
				netw2 = netw;
				change = true;
			}
			else if(netw >= 0 && netw2 >= 0 && netw != netw2)
			{
				mergenetw(netw, netw2);
				change = true;
			}
		}
	}

	return change;
}

void ConduitType::mergenetw(int A, int B)
{
	int mini = imin(A, B);
	int maxi = imax(A, B);

	Building* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;
		
		short& netw = *(short*)(((char*)b)+netwoff);

		if(netw == maxi)
			netw = mini;
	}

	ConduitTile* ct;

	if(!get)
		return;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			ct = get(x, z, false);

			if(!ct->on)
				continue;

			if(!ct->finished)
				continue;

			if(ct->netw == maxi)
				ct->netw = mini;
		}
}

bool ConduitType::renetwtiles()
{
	bool change = false;

	if(!get)
		return;

	ConduitTile* ct;
	Building* b;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			ct = get(x, z, false);

			if(!ct->on)
				continue;

			if(!ct->finished)
				continue;

			if(x > 0 && CompareRoad(r, x-1, z))
				change = true;
			if(x < g_hmap.m_widthx-1 && CompareRoad(r, x+1, z))
				change = true;
			if(z > 0 && CompareRoad(r, x, z-1))
				change = true;
			if(z < g_hmap.m_widthz-1 && CompareRoad(r, x, z+1))
				change = true;

			for(int i=0; i<BUILDINGS; i++)
			{
				b = &g_building[i];

				if(!b->on)
					continue;

				if(!RoadAdjacent(i, x, z))
					continue;

				if(CompareBRoad(b, r))
					change = true;
			}
		}

	return change;
}

void ConduitType::renetw()
{
	resetnetw();

	bool change;

	do
	{
		change = false;

		if(renetwb())
			change = true;

		if(ReRoadRoad())
			change = true;
	}
	while(change);

#if 0
	CheckRoadAccess();
#endif
}

ConduitTile::ConduitTile()
{
}

ConduitTile::~ConduitTile()
{
}

char ConduitTile::condtype()
{
	return CON_NONE;
}

int ConduitTile::netreq(int res)
{
	return 0;
}

void ConduitTile::destroy()
{
}

void ConduitTile::allocate()
{

}

bool ConduitTile::checkconstruction()
{

	return false;
}

void ConduitTile::fillcollider()
{
}

void ConduitTile::freecollider()
{
}

void DefCo(char ctype, unsigned short netwoff, unsigned short maxforwincl, unsigned short maxsideincl, bool blconduct, 
	void (*checkplace)(int tx, int tz),
	Vec3f (*drawpos)(int tx, int tz),
	Vec2i (*physpos)(int tx, int tz),
	void (*draw)(int tx, int ty, bool plan),
	//void (*updplans)(char owner, Vec3f start, Vec3f end),
	//void (*clearplans)(),
	//void (*renetw)(),
	void (*place)(int tx, int tz, char owner, bool plan),
	ConduitTile* (*get)(int tx, int tz, bool plan))
{
	ConduitType* ct = &g_cotype[ctype];
	ct->netwoff = netwoff;
	ct->maxforwincl = maxforwincl;
	ct->maxsideincl = maxsideincl;
	ct->checkplace = checkplace;
	ct->drawpos = drawpos;
	ct->physpos = physpos;
	ct->draw = draw;
	//ct->updplans = updplans;
	//ct->clearplans = clearplans;
	//ct->renetw = renetw;
	ct->place = place;
	ct->get = get;

	class RoadTile : public ConduitTile
	{
	public:
		int extra;
	};

	ConduitTile* conduits = new RoadTile[100];

	delete [] conduits;
}

void CoConMat(char ctype, char rtype, short ramt)
{
	ConduitType* ct = &g_cotype[ctype];
	ct->conmat[rtype] = ramt;
}