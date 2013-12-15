
#include "physics.h"
#include "building.h"
#include "map.h"
#include "road.h"
#include "unit.h"
#include "powerline.h"
#include "pipeline.h"
#include "water.h"
#include "main.h"
#include "chat.h"
#include "pathfinding.h"

int g_lastCollider = -1;
int g_colliderType;
bool g_ignored;

//bool g_debug1 = false;

bool BuildingAdjacent(int i, int j)
{
	CBuilding* bi = &g_building[i];
	CBuilding* bj = &g_building[j];
	
	CBuildingType* ti = &g_buildingType[bi->type];
	CBuildingType* tj = &g_buildingType[bj->type];

	Vec3f pi = bi->pos;
	Vec3f pj = bj->pos;
	
	float hwxi = ti->widthX*TILE_SIZE/2.0f;
	float hwzi = ti->widthZ*TILE_SIZE/2.0f;
	float hwxj = tj->widthX*TILE_SIZE/2.0f;
	float hwzj = tj->widthZ*TILE_SIZE/2.0f;

	float eps = TILE_SIZE/16.0f;

	if(fabs(pi.x-pj.x) <= hwxi+hwxj+eps && fabs(pi.z-pj.z) <= hwzi+hwzj+eps)
		return true;

	return false;
}

bool PowerlineAdjacent(int i, int x, int z)
{
	CBuilding* b = &g_building[i];
	CBuildingType* t = &g_buildingType[b->type];
	
	Vec3f p = b->pos;

	float hwx = t->widthX*TILE_SIZE/2.0f;
	float hwz = t->widthZ*TILE_SIZE/2.0f;

	Vec3f p2 = PowerlinePosition(x, z);

	float hwx2 = TILE_SIZE/2.0f;
	float hwz2 = TILE_SIZE/2.0f;

	if(fabs(p.x-p2.x) <= hwx+hwx2 && fabs(p.z-p2.z) <= hwz+hwz2)
		return true;

	return false;
}

bool PipelineAdjacent(int i, int x, int z)
{
	CBuilding* b = &g_building[i];
	CBuildingType* t = &g_buildingType[b->type];
	
	Vec3f p = b->pos;

	float hwx = t->widthX*TILE_SIZE/2.0f;
	float hwz = t->widthZ*TILE_SIZE/2.0f;

	Vec3f p2 = PipelinePhysPos(x, z);

	float hwx2 = TILE_SIZE/2.0f;
	float hwz2 = TILE_SIZE/2.0f;

	if(fabs(p.x-p2.x) <= hwx+hwx2 && fabs(p.z-p2.z) <= hwz+hwz2)
		return true;

	return false;
}

bool CollidesWithBuildings(float x, float z, float hwx, float hwz, int ignore, float eps)
{
	CBuilding* b;
	CBuildingType* t;
	float hwx2, hwz2;
	Vec3f p;
	g_ignored = false;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		t = &g_buildingType[b->type];
		p = b->pos;
		hwx2 = t->widthX*TILE_SIZE/2.0f;
		hwz2 = t->widthZ*TILE_SIZE/2.0f;

		if(fabs(p.x-x) < hwx+hwx2-eps && fabs(p.z-z) < hwz+hwz2-eps)
		{
			if(i == ignore)
			{
				g_ignored = true;
				continue;
			}
			/*
			if(g_debug1)
			{
				g_log<<"fabs((p.x)"<<p.x<<"-(x)"<<x<<") < (hwx)"<<hwx<<"+(hwx2)"<<hwx2<<" && fabs((p.z)"<<p.z<<"- (z)"<<z<<") < (hwz)"<<hwz<<"+(hwz2)"<<hwz2<<endl;
				g_log<<fabs(p.x-x)<<" < "<<(hwx+hwx2)<<" && "<<fabs(p.z-z)<<" < "<<(hwz+hwz2)<<endl;
			}*/

			g_lastCollider = i;
			g_colliderType = COLLIDER_BUILDING;
			return true;
		}
	}

	return false;
}

bool CollidesWithUnits(float x, float z, float hwx, float hwz, bool isUnit, CUnit* thisU, CUnit* ignore, bool checkPass, float eps)
{
	CUnit* u;
	CUnitType* t;
	float r;
	Vec3f p;

	for(int i=0; i<UNITS; i++)
	{		
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u == ignore)
			continue;

		if(u == thisU)
			continue;

		//if(u->type == LABOURER && u->hidden())
		if(u->hidden())
			continue;

		t = &g_unitType[u->type];

		if(checkPass && isUnit && t->passable)
			continue;

		p = u->camera.Position();
		r = t->radius;

		if(fabs(p.x-x) < hwx+r-eps && fabs(p.z-z) < hwz+r-eps)
		{
			g_lastCollider = i;
			g_colliderType = COLLIDER_UNIT;
			return true;
		}
	}

	return false;
}

bool Offmap(float x, float z, float hwx, float hwz)
{
	if(x - hwx < 0.0f)
		return true;

	if(x + hwx > g_hmap.m_widthX*TILE_SIZE)
		return true;
	
	if(z - hwz < 0.0f)
		return true;

	if(z + hwz > g_hmap.m_widthZ*TILE_SIZE)
		return true;

	return false;
}

bool BuildingCollides(int type, Vec3f pos)
{
	CBuildingType* t = &g_buildingType[type];

	if(Offmap(pos.x, pos.z, t->widthX*TILE_SIZE/2, t->widthZ*TILE_SIZE/2))
		return true;

	if(CollidesWithBuildings(pos.x, pos.z, t->widthX*TILE_SIZE/2, t->widthZ*TILE_SIZE/2))
		return true;

	if(CollidesWithUnits(pos.x, pos.z, t->widthX*TILE_SIZE/2, t->widthZ*TILE_SIZE/2))
		return true;

	int startx = pos.x/TILE_SIZE-t->widthX/2;
	int endx = startx + t->widthX - 1;
	int startz = pos.z/TILE_SIZE-t->widthZ/2;
	int endz = startz + t->widthZ - 1;

	for(int x=startx; x<=endx; x++)
		for(int z=startz; z<=endz; z++)
			if(RoadAt(x, z)->on)
				return true;

	return false;
}

bool BuildingLevel(int type, Vec3f pos)
{
	CBuildingType* t = &g_buildingType[type];
	
	int startx = pos.x/TILE_SIZE-t->widthX/2;
	int endx = startx + t->widthX - 1;
	int startz = pos.z/TILE_SIZE-t->widthZ/2;
	int endz = startz + t->widthZ - 1;

	float compare = g_hmap.getheight(startx, startz);

	if(compare <= WATER_HEIGHT)
		return false;

	for(int x=startx; x<=endx+1; x++)
		for(int z=startz; z<=endz+1; z++)
		{
			if(g_hmap.getheight(x, z) != compare)
				return false;

			if(g_hmap.getheight(x, z) <= WATER_HEIGHT)
				return false;
		}

	return true;
}

bool TileUnclimablei(int tx, int tz)
{
	float h0 = g_hmap.getheight(tx, tz);
	float h1 = g_hmap.getheight(tx+1, tz);
	float h2 = g_hmap.getheight(tx, tz+1);
	float h3 = g_hmap.getheight(tx+1, tz+1);

	float minh = min(h0, min(h1, min(h2, h3)));
	float maxh = max(h0, max(h1, max(h2, h3)));

	if(fabs(maxh - minh) > MAX_CLIMB_INCLINE)
	{
		g_colliderType = COLLIDER_TERRAIN;
		return true;
	}

	return false;
}

bool TileUnclimable(float px, float pz)
{
	int tx = px / TILE_SIZE;
	int tz = pz / TILE_SIZE;
	
	return TileUnclimablei(tx, tz);
}

bool AnyWateri(int tx, int tz)
{
	if(g_hmap.getheight(tx, tz) <= WATER_HEIGHT)
		return true;

	if(g_hmap.getheight(tx+1, tz) <= WATER_HEIGHT)
		return true;

	if(g_hmap.getheight(tx, tz+1) <= WATER_HEIGHT)
		return true;

	if(g_hmap.getheight(tx+1, tz+1) <= WATER_HEIGHT)
		return true;

	return false;
}

bool AnyWater(float px, float pz)
{
	int tx = px / TILE_SIZE;
	int tz = pz / TILE_SIZE;
	
	return AnyWateri(tx, tz);
}

bool AtWater(float px, float pz)
{
	float h = Bilerp(px, pz);

	if(h <= WATER_HEIGHT)
	{
		g_colliderType = COLLIDER_TERRAIN;
		return true;
	}

	return false;
}

// used to check for collisions with passable units
// edit: used for initial spawn of unit
bool CUnit::Collides2(bool checkroad)
{
	CUnitType* t = &g_unitType[type];
	float r = t->radius;
	Vec3f p = camera.Position();

	if(checkroad)
	{
		bool roadVeh = t->roaded;

		int tx = p.x / TILE_SIZE;
		int tz = p.z / TILE_SIZE;

		CRoad* road = RoadAt(tx, tz);

		if(roadVeh && !road->on)
			return true;
		else if(!roadVeh && road->on)
			return true;
	}

	if(AtWater(p.x, p.z))
		return true;
	
	if(TileUnclimable(p.x, p.z))
		return true;

	if(CollidesWithBuildings(p.x, p.z, r, r))
		return true;

	if(CollidesWithUnits(p.x, p.z, r, r, true, this, NULL, false))
		return true;

	if(Offmap(p.x, p.z, r, r))
		return true;

	return false;
}

bool CUnit::confirmcollision(int ctype, int ID, float eps)
{
	CUnitType* ut = &g_unitType[type];
	//Vec3f p = camera.Position();
	float r = ut->radius;
	const Vec3f p = camera.Position();

	if(ctype == COLLIDER_BUILDING)
	{
		CBuilding* b = &g_building[ID];
		CBuildingType* bt = &g_buildingType[b->type];
		Vec3f p2 = b->pos;
		float hwx = bt->widthX*TILE_SIZE/2.0;
		float hwz = bt->widthZ*TILE_SIZE/2.0;

		if(fabs(p2.x-p.x) < r+hwx-eps && fabs(p2.z-p.z) < r+hwz-eps)
			return true;
	}
	else if(ctype == COLLIDER_UNIT)
	{
		CUnit* u2 = &g_unit[ID];
		ut = &g_unitType[u2->type];
		float r2 = ut->radius;
		const Vec3f p2 = u2->camera.Position();
		
		if(fabs(p2.x-p.x) < r+r2-eps && fabs(p2.z-p.z) < r+r2-eps)
			return true;
	}
	else if(ctype == COLLIDER_TERRAIN)
	{
		if(AtWater(p.x, p.z))
			return true;

		if(TileUnclimable(p.x, p.z))
			return true;
	}
	else if(ctype == COLLIDER_NOROAD)
	{
		CRoad* road = RoadAt(p.x/TILE_SIZE, p.z/TILE_SIZE);

		if(!road->on || !road->finished)
			return true;
	}

	return false;
}

bool CUnit::collidesfast(CUnit* ignoreUnit, int ignoreBuilding)
{
	CUnitType* ut = &g_unitType[type];
	//Vec3f p = camera.Position();
	float r = ut->radius;
	const Vec3f p = camera.Position();
		
	const bool roadVeh = ut->roaded;

	float fstartx = p.x-r;
	float fstartz = p.z-r;
	float fendx = p.x+r;
	float fendz = p.z+r;
		
	//int extentx = ceil(r/cellwx);
	//int extentz = ceil(r/cellwz);
	const float cellwx = MIN_RADIUS*2.0f;
	const float cellwz = MIN_RADIUS*2.0f;
		
	//int startx = max(0, fstartx/cellwx);
	//int startz = max(0, fstartz/cellwz);
	//int endx = min(ceil(fendx/cellwx), g_hmap.m_widthX*TILE_SIZE/cellwx-1);
	//int endz = min(ceil(fendz/cellwz), g_hmap.m_widthZ*TILE_SIZE/cellwz-1);
	int startx = fstartx/cellwx;
	int startz = fstartz/cellwz;
	int endx = ceil(fendx/cellwx);
	int endz = ceil(fendz/cellwz);
	//int endx = startx+extentx;
	//int endz = startz+extentz;

	if(startx < 0 || startz < 0 || endx >= g_hmap.m_widthX*TILE_SIZE/cellwx || endz >= g_hmap.m_widthZ*TILE_SIZE/cellwz)
	{
		g_lastCollider = -1;
		g_colliderType = COLLIDER_TERRAIN;
		return true;
	}

	g_ignored = false;

	if(roadVeh)
	{
		for(int x=startx; x<endx; x++)
			for(int z=startz; z<endz; z++)
			{
				ColliderCell* cell = ColliderCellAt(x, z);
				//cell->colliders.push_back(Collider(COLLIDER_UNIT, uID));
				for(int i=0; i<cell->colliders.size(); i++)
				{
					Collider* c = &cell->colliders[i];
					if(c->type == COLLIDER_UNIT && &g_unit[c->ID] == this)
						continue;
					if(c->type == COLLIDER_BUILDING && c->ID == ignoreBuilding)
					{
						if(confirmcollision(c->type, c->ID))
							g_ignored = true;
						continue;
					}
					if(c->type == COLLIDER_UNIT)
					{
						if(&g_unit[c->ID] == ignoreUnit)
						{
							if(confirmcollision(c->type, c->ID))
								g_ignored = true;
							continue;
						}
						if(g_unit[c->ID].hidden())
							continue;
					}

					if(confirmcollision(c->type, c->ID))
					{
						g_colliderType = c->type;
						g_lastCollider = c->ID;

						return true;
					}
				}
			}
	}
	else
	{
		for(int x=startx; x<endx; x++)
			for(int z=startz; z<endz; z++)
			{
				ColliderCell* cell = ColliderCellAt(x, z);
				//cell->colliders.push_back(Collider(COLLIDER_UNIT, uID));
				for(int i=0; i<cell->colliders.size(); i++)
				{
					Collider* c = &cell->colliders[i];
					if(c->type == COLLIDER_UNIT && &g_unit[c->ID] == this)
						continue;
					if(c->type == COLLIDER_NOROAD)
						continue;
					if(c->type == COLLIDER_BUILDING && c->ID == ignoreBuilding)
					{
						if(confirmcollision(c->type, c->ID))
							g_ignored = true;

						continue;
					}
					if(c->type == COLLIDER_UNIT)
					{
						if(&g_unit[c->ID] == ignoreUnit)
						{
							if(confirmcollision(c->type, c->ID))
								g_ignored = true;
							continue;
						}
						if(g_unit[c->ID].hidden())
							continue;
					}
					
					if(confirmcollision(c->type, c->ID))
					{
						g_colliderType = c->type;
						g_lastCollider = c->ID;

						return true;
					}
				}
			}
	}

	return false;
}

bool CUnit::Collides(CUnit* ignoreUnit, int ignoreBuilding)
{
	CUnitType* t = &g_unitType[type];
	float r = t->radius;
	Vec3f p = camera.Position();
	/*
	g_debug1 = false;
	if(UnitID(this) == 0)
		{
			g_debug1=  true;
		}*/

	if(AtWater(p.x, p.z))
		return true;

	if(TileUnclimable(p.x, p.z))
		return true;

	if(CollidesWithBuildings(p.x, p.z, r, r, ignoreBuilding, MIN_RADIUS/2.0f))
	{
		//g_debug1 = false;
		return true;
	}

	//g_debug1 = false;

	if(!t->passable && CollidesWithUnits(p.x, p.z, r, r, true, this, ignoreUnit, MIN_RADIUS/2.0f))
	{
			//if(type == TRUCK)
			//	Chat("truck col");

		return true;
	}

	if(Offmap(p.x, p.z, r, r))
		return true;

	return false;
}