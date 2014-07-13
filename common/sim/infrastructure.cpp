

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
	//if(!get)
	//	return;

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

	//if(!get)
	//	return;

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

// Merge two networks that have been found to be touching.
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

	//if(!get)
	//	return;

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

bool ConduitType::comparetiles(ConduitTile* ct, int x, int z)
{
	ConduitTile* ct2 = get(x, z, false);

	if(!ct2->on)
		return false;

	if(!ct2->finished)
		return false;

	if(ct2->netw < 0 && ct->netw >= 0)
	{
		ct2->netw = ct->netw;
		return true;
	}
	else if(ct->netw < 0 && ct2->netw >= 0)
	{
		ct->netw = ct2->netw;
		return true;
	}
	else if(ct->netw >= 0 && ct2->netw >= 0 && ct->netw != ct2->netw)
	{
		mergenetw(ct->netw, ct2->netw);
		return true;
	}

	return false;
}

// Building adjacent?
bool ConduitType::badj(int i, int x, int z)
{
	Building* b = &g_building[i];
	BuildingT* t = &g_bltype[b->type];

	Vec2i tp = b->tilepos;

	int minx = (tp.x - t->widthx/2)*TILE_SIZE;
	int minz = (tp.y - t->widthz/2)*TILE_SIZE;
	int maxx = minx + t->widthx*TILE_SIZE;
	int maxz = minz + t->widthz*TILE_SIZE;

	//Vec3i p2 = RoadPhysPos(x, z);
	Vec2i p2 = Vec2i(x, z) + physoff;

	const int hwx2 = TILE_SIZE/2;
	const int hwz2 = TILE_SIZE/2;
	int minx2 = p2.x - hwx2;
	int minz2 = p2.y - hwz2;
	int maxx2 = minx2 + TILE_SIZE;
	int maxz2 = minz2 + TILE_SIZE;

	if(maxx >= minx2 && maxz >= minz2 && minx <= maxx2 && minz <= maxz2)
		return true;

	return false;
}

bool ConduitType::compareb(Building* b, ConduitTile* ct)
{
	if(!blconduct)
	{
		std::list<short>& bnetw = *(std::list<short>*)(((char*)b)+netwoff);

		if(bnetw.size() <= 0 && ct->netw >= 0)
		{
			bnetw.push_back(ct->netw);
			return true;
		}/*
		 else if(r->netw < 0 && b->roadnetw >= 0)
		 {
		 pow->netw = b->pownetw;
		 return true;
		 }
		 else if(pow->netw >= 0 && b->pownetw >= 0 && pow->netw != b->pownetw)
		 {
		 MergePow(pow->netw, b->pownetw);
		 return true;
		 }*/
		else if(bnetw.size() > 0 && ct->netw >= 0)
		{
			bool found = false;
			for(auto netwiter = bnetw.begin(); netwiter != bnetw.end(); netwiter++)
			{
				if(*netwiter == ct->netw)
				{
					found = true;
					break;
				}
			}
			if(!found)
				bnetw.push_back(ct->netw);
		}

		return false;
	}
	else
	{
		short& bnetw = *(short*)(((char*)b)+netwoff);

		if(bnetw < 0 && ct->netw >= 0)
		{
			bnetw = ct->netw;
			return true;
		}
		else if(ct->netw < 0 && bnetw >= 0)
		{
			ct->netw = bnetw;
			return true;
		}
		else if(ct->netw >= 0 && bnetw >= 0 && ct->netw != bnetw)
		{
			mergenetw(ct->netw, bnetw);
			return true;
		}

		return false;
	}

	return false;
}

bool ConduitType::renetwtiles()
{
	bool change = false;

	//if(!get)
	//	return;

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

			if(x > 0 && comparetiles(ct, x-1, z))
				change = true;
			if(x < g_hmap.m_widthx-1 && comparetiles(ct, x+1, z))
				change = true;
			if(z > 0 && comparetiles(ct, x, z-1))
				change = true;
			if(z < g_hmap.m_widthz-1 && comparetiles(ct, x, z+1))
				change = true;

			for(int i=0; i<BUILDINGS; i++)
			{
				b = &g_building[i];

				if(!b->on)
					continue;

				if(!badj(i, x, z))
					continue;

				if(compareb(b, ct))
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

		if(renetwtiles())
			change = true;
	}
	while(change);

#if 0
	CheckRoadAccess();
#endif
}

bool ConduitType::tilelevel(float iterx, float iterz, float testx, float testz, float dx, float dz, int i, float d, ConduitTile* (*planfunc)(int x, int z))
{
	//return true;

	bool n = false, e = false, s = false, w = false;
	int ix, iz;

	if(i > 0)
	{
		float x = iterx - dx;
		float z = iterz - dz;

		ix = x;
		iz = z;

		if(ix == testx)
		{
			if(iz == testz+1)	n = true;
			else if(iz == testz-1)	s = true;
		}
		else if(iz == testz)
		{
			if(ix == testx+1)	w = true;
			else if(ix == testx-1)	e = true;
		}

		float prevx = x - dx;
		float prevz = z - dz;

		if((int)x != prevx && (int)z != prevz)
		{
			ix = prevx;

			if(ix == testx)
			{
				if(iz == testz+1)	n = true;
				else if(iz == testz-1)	s = true;
			}
			else if(iz == testz)
			{
				if(ix == testx+1)	w = true;
				else if(ix == testx-1)	e = true;
			}
		}
	}

	if(i < d)
	{
		float x = iterx + dx;
		float z = iterz + dz;

		ix = x;
		iz = z;

		if(ix == testx)
		{
			if(iz == testz+1)	n = true;
			else if(iz == testz-1)	s = true;
		}
		else if(iz == testz)
		{
			if(ix == testx+1)	w = true;
			else if(ix == testx-1)	e = true;
		}

		if(i > 0)
		{
			float prevx = x - dx;
			float prevz = z - dz;

			if((int)x != prevx && (int)z != prevz)
			{
				ix = prevx;

				if(ix == testx)
				{
					if(iz == testz+1)	n = true;
					else if(iz == testz-1)	s = true;
				}
				else if(iz == testz)
				{
					if(ix == testx+1)	w = true;
					else if(ix == testx-1)	e = true;
				}
			}
		}
	}

	ix = testx;
	iz = testz;

	if(g_hmap.getheight(ix, iz) <= WATER_LEVEL)		return false;
	if(g_hmap.getheight(ix+1, iz) <= WATER_LEVEL)	return false;
	if(g_hmap.getheight(ix, iz+1) <= WATER_LEVEL)	return false;
	if(g_hmap.getheight(ix+1, iz+1) <= WATER_LEVEL)	return false;

	if(ix > 0)
	{
		if(RoadAt(ix-1, iz)->on)	w = true;
		//if(RoadPlanAt(ix-1, iz)->on)	w = true;
		if(planfunc != NULL)
			if(planfunc(ix-1, iz)->on) w = true;
	}

	if(ix < g_hmap.m_widthx-1)
	{
		if(RoadAt(ix+1, iz)->on)	e = true;
		//if(RoadPlanAt(ix+1, iz)->on)	e = true;
		if(planfunc != NULL)
			if(planfunc(ix+1, iz)->on) w = true;
	}

	if(iz > 0)
	{
		if(RoadAt(ix, iz-1)->on)	s = true;
		//if(RoadPlanAt(ix, iz-1)->on)	s = true;
		if(planfunc != NULL)
			if(planfunc(ix, iz-1)->on) w = true;
	}

	if(iz < g_hmap.m_widthz-1)
	{
		if(RoadAt(ix, iz+1)->on)	n = true;
		//if(RoadPlanAt(ix, iz+1)->on)	n = true;
		if(planfunc != NULL)
			if(planfunc(ix, iz+1)->on) w = true;
	}
#if 0
	g_log<<"level? ix"<<ix<<","<<iz<<endl;
	g_log.flush();
#endif
	if((n && e && s && w) || (n && e && s && !w) || (n && e && !s && w) || (n && e && !s && !w) || (n && !e && s && w)
			|| (n && !e && !s && w) || (!n && e && s && !w) || (!n && !e && s && w) || (!n && !e && !s && !w) || (!n && e && s && w))
	{
		float compare = g_hmap.getheight(ix, iz);
		if(fabs(g_hmap.getheight(ix+1, iz) - compare) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - compare) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz+1) - compare) > ROAD_MAX_SIDEW_INCLINE)	return false;
	}
	else if((n && !e && s && !w) || (n && !e && !s && !w) || (!n && !e && s && !w))
	{
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix+1, iz)) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - g_hmap.getheight(ix+1, iz+1)) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix, iz+1)) > ROAD_MAX_FOREW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz) - g_hmap.getheight(ix+1, iz+1)) > ROAD_MAX_FOREW_INCLINE)	return false;
	}
	else if((!n && e && !s && w) || (!n && e && !s && !w) || (!n && !e && !s && w))
	{
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix+1, iz)) > ROAD_MAX_FOREW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - g_hmap.getheight(ix+1, iz+1)) > ROAD_MAX_FOREW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix, iz+1)) > ROAD_MAX_SIDEW_INCLINE)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz) - g_hmap.getheight(ix+1, iz+1)) > ROAD_MAX_SIDEW_INCLINE)	return false;
	}

#if 0
	g_log<<"level yes! ix"<<ix<<","<<iz<<endl;
	g_log.flush();
#endif

	return true;
}


void ConduitType::updplans(char owner, Vec3f start, Vec3f end)
{
	clearplans();

	int x1 = Clipi(start.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z1 = Clipi(start.z/TILE_SIZE, 0, g_hmap.m_widthz-1);

	int x2 = Clipi(end.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z2 = Clipi(end.z/TILE_SIZE, 0, g_hmap.m_widthz-1);

#if 0
	g_log<<"road plan "<<x1<<","<<z1<<"->"<<x2<<","<<z2<<endl;
	g_log.flush();
#endif
	/*if(!py->mousekeys[MOUSE_LEFT])
	{
	x1 = x2;
	z1 = z2;
	}*/

	float dx = x2-x1;
	float dz = z2-z1;

	float d = sqrtf(dx*dx + dz*dz);

	dx /= d;
	dz /= d;

	int prevx = x1;
	int prevz = z1;

	int i = 0;

	//PlaceRoad(x1, z1, g_localPlayer, true);
	//if(RoadLevel(x2, z2, dx, dz, i, d))
	//	PlaceRoad(x2, z2, g_localPlayer, true);

	for(float x=x1, z=z1; i<=d; x+=dx, z+=dz, i++)
	{
		if(RoadLevel(x, z, x, z, dx, dz, i, d))
		{
#if 0
			g_log<<"place road urp "<<x<<","<<z<<endl;
			g_log.flush();
#endif
			PlaceRoad(x, z, stateowner, true);
		}

		if((int)x != prevx && (int)z != prevz)
		{
			if(RoadLevel(x, z, prevx, z, dx, dz, i, d))
				PlaceRoad(prevx, z, stateowner, true);
		}

		prevx = x;
		prevz = z;

#if 0
		g_log<<"place road "<<x<<","<<z<<endl;
		g_log.flush();
#endif
	}

	if((int)x2 != prevx && (int)z2 != prevz)
	{
		if(RoadLevel(x2, z1, prevx, z2, dx, dz, i, d))
			PlaceRoad(prevx, z2, stateowner, true);
	}

	for(x1=0; x1<g_hmap.m_widthx; x1++)
		for(z1=0; z1<g_hmap.m_widthz; z1++)
			MeshRoad(x1, z1, true);


		ClearPowlPlans();

	int x1 = Clipi(start.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z1 = Clipi(start.z/TILE_SIZE, 0, g_hmap.m_widthz-1);

	int x2 = Clipi(end.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z2 = Clipi(end.z/TILE_SIZE, 0, g_hmap.m_widthz-1);

	/*if(!py->mousekeys[MOUSE_LEFT])
	{
		x1 = x2;
		z1 = z2;
	}*/

	PlacePowl(x1, z1, stateowner, true);
	PlacePowl(x2, z2, stateowner, true);

	float dx = x2-x1;
	float dz = z2-z1;

	float d = sqrtf(dx*dx + dz*dz);

	dx /= d;
	dz /= d;

	int prevx = x1;
	int prevz = z1;

	int i = 0;

	for(float x=x1, z=z1; i<=d; x+=dx, z+=dz, i++)
	{
		PlacePowl(x, z, stateowner, true);

		if((int)x != prevx && (int)z != prevz)
			PlacePowl(prevx, z, stateowner, true);

		prevx = x;
		prevz = z;
	}

	if((int)x2 != prevx && (int)z2 != prevz)
		PlacePowl(prevx, z2, stateowner, true);

	for(x1=0; x1<g_hmap.m_widthx; x1++)
		for(z1=0; z1<g_hmap.m_widthz; z1++)
			MeshPowl(x1, z1, true);
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

ConduitTile* ConduitType::get(int tx, int tz, bool plan)
{
	ConduitTile* tilesarr = *cotiles[(int)plan];
	return &tilesarr[ tx + tz*g_hmap.m_widthx ];
}

void DefCo(char ctype, unsigned short netwoff, unsigned short maxforwincl, unsigned short maxsideincl, bool blconduct, 
		   Vec2i physoff, Vec3f drawoff, ConduitTile** cotiles, ConduitTile** coplans,
	void (*checkplace)(int tx, int tz),
	//Vec3f (*drawpos)(int tx, int tz),
	//Vec2i (*physpos)(int tx, int tz),
	//void (*draw)(int tx, int ty, bool plan),
	//void (*updplans)(char owner, Vec3f start, Vec3f end),
	//void (*clearplans)(),
	//void (*renetw)(),
	void (*place)(int tx, int tz, char owner, bool plan)
	//ConduitTile* (*get)(int tx, int tz, bool plan)
	)
{
	ConduitType* ct = &g_cotype[ctype];
	ct->netwoff = netwoff;
	ct->maxforwincl = maxforwincl;
	ct->maxsideincl = maxsideincl;
	ct->physoff = physoff;
	ct->drawoff = drawoff;
	ct->cotiles[0] = cotiles;
	ct->cotiles[1] = coplans;
	ct->checkplace = checkplace;
	//ct->drawpos = drawpos;
	//ct->physpos = physpos;
	//ct->draw = draw;
	//ct->updplans = updplans;
	//ct->clearplans = clearplans;
	//ct->renetw = renetw;
	ct->place = place;
	//ct->get = get;

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