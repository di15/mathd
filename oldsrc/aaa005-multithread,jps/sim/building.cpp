

#include "building.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "buildingtype.h"
#include "../render/model.h"
#include "../render/foliage.h"
#include "../phys/collision.h"
#include "../render/water.h"

Building g_building[BUILDINGS];

void Building::destroy()
{
	on = false;
}

Building::Building()
{
	on = false;
}

Building::~Building()
{
	destroy();
}

void FreeBuildings()
{
	for(int i=0; i<BUILDINGS; i++)
	{
		g_building[i].destroy();
		g_building[i].on = false;
	}
}

int NewBuilding()
{
	for(int i=0; i<BUILDINGS; i++)
		if(!g_building[i].on)
			return i;

	return -1;
}


bool BuildingLevel(int type, Vec2i tpos)
{
#if 1
	BuildingT* t = &g_buildingT[type];
    
	Vec2i tmin;
	Vec2i tmax;

	tmin.x = tpos.x - t->widthx/2;
	tmin.y = tpos.y - t->widthz/2;
	tmax.x = tmin.x + t->widthx;
	tmax.y = tmin.y + t->widthz;
    
	float miny = g_hmap.getheight(tmin.x, tmin.y);
	float maxy = g_hmap.getheight(tmin.x, tmin.y);
	
	bool haswater = false;
	bool hasland = false;
    
	for(int x=tmin.x; x<=tmax.x; x++)
		for(int z=tmin.y; z<=tmax.y; z++)
		{
			float thisy = g_hmap.getheight(x, z);

			if(thisy < miny)
				miny = thisy;

			if(thisy > maxy)
				maxy = thisy;

			// Must have two adject water tiles to be water-vessel-accessible
			if(thisy < WATER_LEVEL)
			{
				// If z is along building edge and x and x+1 are water tiles
				if((z==tmin.y || z==tmax.y) && x+1 <= g_hmap.m_widthx && x+1 <= tmax.x && g_hmap.getheight(x+1, z) < WATER_LEVEL)
						haswater = true;
				// If x is along building edge and z and z+1 are water tiles
				if((x==tmin.x || x==tmax.x) && z+1 <= g_hmap.m_widthz && z+1 <= tmax.y && g_hmap.getheight(x, z+1) < WATER_LEVEL)
						haswater = true;
			}
			// Must have two adjacent land tiles to be road-accessible
			else if(thisy > WATER_LEVEL)
			{
				// If z is along building edge and x and x+1 are land tiles
				if((z==tmin.y || z==tmax.y) && x+1 <= g_hmap.m_widthx && x+1 <= tmax.x && g_hmap.getheight(x+1, z) > WATER_LEVEL)
						hasland = true;
				// If x is along building edge and z and z+1 are land tiles
				if((x==tmin.x || x==tmax.x) && z+1 <= g_hmap.m_widthz && z+1 <= tmax.y && g_hmap.getheight(x, z+1) > WATER_LEVEL)
						hasland = true;
			}
		}

	if(miny < WATER_LEVEL)
	{
#if 0
		haswater = true;
#endif
		miny = WATER_LEVEL;
	}

#if 0
	if(maxy > WATER_LEVEL)
		hasland = true;
#endif

	if(maxy - miny > MAX_CLIMB_INCLINE)
		return false;

	if(t->foundation == FOUNDATION_LAND)
	{
		if(haswater)
			return false;
		if(!hasland)
			return false;
	}
	else if(t->foundation == FOUNDATION_SEA)
	{
		if(!haswater)
			return false;
		if(hasland)
			return false;
	}
	else if(t->foundation == FOUNDATION_COASTAL)
	{
		if(!haswater || !hasland)
			return false;
	}

#if 0
	for(int x=tmin.x; x<=tmax.x; x++)
		for(int z=tmin.y; z<=tmax.y; z++)
		{
			if(g_hmap.getheight(x, z) != compare)
				return false;
            
			if(g_hmap.getheight(x, z) <= WATER_LEVEL)
				return false;
		}
#endif
#endif
    
	return true;
}

bool CheckCanPlace(int type, Vec2i pos)
{
	BuildingT* t = &g_buildingT[type];

	if(!BuildingLevel(type, pos))
		return false;

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = pos.x - t->widthx/2;
	tmin.y = pos.y - t->widthz/2;
	tmax.x = tmin.x + t->widthx - 1;
	tmax.y = tmin.y + t->widthz - 1;

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;
		
		BuildingT* t2 = &g_buildingT[b->type];

		Vec2i tmin2;
		Vec2i tmax2;

		tmin2.x = b->tilepos.x - t2->widthx/2;
		tmin2.y = b->tilepos.y - t2->widthz/2;
		tmax2.x = tmin2.x + t2->widthx - 1;
		tmax2.y = tmin2.y + t2->widthz - 1;

		if(tmin.x <= tmax2.x && tmax.x >= tmin2.x && tmin.y <= tmax2.y && tmax.y >= tmin2.y)
			return false;
	}

	return true;
}

bool PlaceBuilding(int type, Vec2i pos, bool finished, int stateowner, int corpowner, int unitowner)
{
	int i = NewBuilding();

	if(i < 0)
		return false;

	Building* b = &g_building[i];
	b->on = true;
	b->type = type;
	b->tilepos = pos;

	BuildingT* t = &g_buildingT[type];

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = pos.x - t->widthx/2;
	tmin.y = pos.y - t->widthz/2;
	tmax.x = tmin.x + t->widthx;
	tmax.y = tmin.y + t->widthz;

	b->drawpos = Vec3f(pos.x*TILE_SIZE, Highest(tmin.x, tmin.y, tmax.x, tmax.y), pos.y*TILE_SIZE);

	if(t->foundation == FOUNDATION_SEA)
		b->drawpos.y = WATER_LEVEL;

#if 1
	if(t->widthx % 2 == 1)
		b->drawpos.x += TILE_SIZE/2;
	
	if(t->widthz % 2 == 1)
		b->drawpos.z += TILE_SIZE/2;

	b->stateowner = stateowner;
	b->corpowner = corpowner;
	b->unitowner = unitowner;

	b->finished = finished;

	int cmminx = tmin.x*TILE_SIZE;
	int cmminz = tmin.y*TILE_SIZE;
	int cmmaxx = cmminx + t->widthx*TILE_SIZE;
	int cmmaxz = cmminz + t->widthz*TILE_SIZE;

	ClearFoliage(cmminx, cmminz, cmmaxx, cmmaxz);

#endif

#if 0
	char msg[128];

	sprintf(msg, "place at %f,%f,%f", u->fpos.x, u->fpos.y, u->fpos.z);

	MessageBox(g_hWnd, msg, "asd", NULL);
#endif

	return true;
}

void DrawBuildings()
{
	//return;
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];
		
		if(!b->on)
			continue;

		const BuildingT* t = &g_buildingT[b->type];
		Model* m = &g_model[ t->model ];

		if(!b->finished)
			m = &g_model[ t->cmodel ];

		m->draw(0, b->drawpos, 0);
	}
}