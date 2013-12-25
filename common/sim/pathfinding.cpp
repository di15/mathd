
#include "unit.h"
#include "pathfinding.h"
#include "main.h"
#include "chat.h"
#include "building.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "physics.h"
#include "map.h"
#include "water.h"
#include "shader.h"
#include "selection.h"

//bool g_open[PATHFIND_SIZE][PATHFIND_SIZE];
int g_pathfindszx;
int g_pathfindszz;
unsigned int* g_open;
vector<CTryStep> g_tryStep;
vector<CTryStep> g_triedStep;
long long g_lastpath = 0;
ColliderCell *g_collidercells = NULL;
unsigned int g_curpath = 0;

CTryStep::CTryStep(int startx, int startz, int endx, int endz, int X, int Z, int prev, float totalD, float stepD)
{
	x = X;
	z = Z;
	float G = totalD + stepD;
	runningD = G;
	//float G = Magnitude2(Vec3f(startx-x,0,startz-z));
	float H = Magnitude3(Vec3f(endx-x,0,endz-z));
	//int G = abs(startx-x) + abs(startz-z);
	//int H = abs(endx-x) + abs(endz-z);
	F = G + H;
	//F = H;
	previous = prev;
	//tried = false;
}

unsigned int& OpenAt(int x, int z)
{
	return g_open[ x + z*g_pathfindszx ];
}

bool CUnit::TryStep(Vec3f vPos, CUnit* ignoreUnit, int ignoreBuilding, bool roadVehicle)
{
	Vec3f vOldPos = camera.Position();
	camera.MoveTo(vPos);
	
	if(Collides(ignoreUnit, ignoreBuilding))
	{
		camera.MoveTo(vOldPos);
		return false;
	}
	
	camera.MoveTo(vOldPos);

	if(roadVehicle && !g_ignored)
	{
		int tilex = vPos.x / TILE_SIZE;
		int tilez = vPos.z / TILE_SIZE;

		CRoad* r = RoadAt(tilex, tilez);
		if(!r->on || !r->finished)
			return false;
	}

	return true;
}

Vec3f CUnit::trysteptoposition(int x, int z)
{
	CUnitType* t = &g_unitType[type];
	float fx = x * t->radius * 2 + t->radius;
	float fz = z * t->radius * 2 + t->radius;
	float fy = 0;	//Bilerp(fx, fz);
	return Vec3f(fx, fy, fz);
}

bool CUnit::canpath()
{
	if(IsMilitary() && nopath)
		return false;
	return true;
}

void CUnit::repath()
{
	//for some reason trucks sometimes have "target"/"supplier" but no "goal"
	if(type == TRUCK)
	{
		if(mode == GOINGTOSUPPLIER)
			goal = g_building[supplier].pos;
		else if(mode == GOINGTODEMROAD)
			goal = RoadPosition(target, target2);
		else if(mode == GOINGTODEMPIPE)
			goal = PipelinePhysPos(target, target2);
		else if(mode == GOINGTODEMPOWL)
			goal = PowerlinePosition(target, target2);
		else if(mode == GOINGTODEMANDERB)
			goal = g_building[target].pos;
	}
}

void FillColliderCells()
{/*
	CUnitType* t = &g_unitType[type];
	const float cellwx = t->radius*2.0f;
	const float cellwz = t->radius*2.0f;*/
	const float cellwx = MIN_RADIUS*2.0f;
	const float cellwz = MIN_RADIUS*2.0f;
	const int numcols = g_hmap.m_widthX*TILE_SIZE / cellwx;
	const int numrows = g_hmap.m_widthZ*TILE_SIZE / cellwz;
	g_collidercells = new ColliderCell[ numcols*numrows ];
	/*
	int ibstartx = -1;
	int ibstartz = -1;
	int ibendx = -1;
	int ibendz = -1;
	*/
#ifdef PATH_DEBUG
	//int uID = UnitID(this);

	//if(uID == 5)
	{
		//g_log<<"roadVeh = "<<roadVehicle<<endl;
		//g_log.flush();
	}
#endif

	/*
	if(ignoreBuilding >= 0)
	{
		CBuilding* b = &g_building[ignoreBuilding];

		CBuildingType* bt = &g_buildingType[b->type];
		Vec3f p = b->pos;
		float hwx = bt->widthX*TILE_SIZE/2.0f;
		float hwz = bt->widthZ*TILE_SIZE/2.0f;
		
		float fstartx = p.x-hwx;
		float fstartz = p.z-hwz;
		float fendx = p.x+hwx;
		float fendz = p.z+hwz;
		
		ibstartx = fstartx/cellwx;
		ibstartz = fstartz/cellwz;
		ibendx = fendx/cellwx;
		ibendz = fendz/cellwz;
		
		float afterendx = fendx - ibendx*cellwx;
		float afterendz = fendz - ibendz*cellwz;
		
		if(afterendx > 0.0f)
			ibendx++;
		if(afterendz > 0.0f)
			ibendz++;
	}*/
	
	for(int x=0; x<numcols; x++)
	{
		float fx = x*cellwx+cellwx/2.0f;
		for(int z=0; z<numrows; z++)
		{
			float fz = z*cellwz+cellwz/2.0f;
			ColliderCell* cell = ColliderCellAt(x, z);

			if(AtWater(fx, fz))
			{
				cell->colliders.push_back(Collider(COLLIDER_TERRAIN, 2));
				continue;
			}

			if(TileUnclimable(fx, fz))
			{
				cell->colliders.push_back(Collider(COLLIDER_TERRAIN, 1));
				continue;
			}

			//if(roadVehicle)
			{
				int tx = fx/TILE_SIZE;
				int tz = fz/TILE_SIZE;

				CRoad* r = RoadAt(tx, tz);

				if(!r->on || !r->finished)
				{
					//if(x >= ibstartx && x < ibendx && z >= ibstartz && z < ibendz)
					{
						//on ignore building
						//Chat("on ignore b");
					}
					//else
					{
						cell->colliders.push_back(Collider(COLLIDER_NOROAD, 0));
						continue;
					}
				}
			}
		}
	}

	for(int i=0; i<UNITS; i++)
	{
		CUnit* u = &g_unit[i];

		if(!u->on)
			continue;
		/*
		if(u == this)
			continue;

		if(u == ignoreUnit)
			continue;

		if(u->type == LABOURER && u->hidden())
			continue;*/
		
		CUnitType* ut = &g_unitType[u->type];
		Vec3f p = u->camera.Position();
		float r = ut->radius;
		
		float fstartx = p.x-r;
		float fstartz = p.z-r;
		float fendx = p.x+r;
		float fendz = p.z+r;
		
		//int extentx = ceil(r/cellwx);
		//int extentz = ceil(r/cellwz);
		
		int startx = max(0, fstartx/cellwx);
		int startz = max(0, fstartz/cellwz);
		int endx = min(ceil(fendx/cellwx), g_hmap.m_widthX*TILE_SIZE/cellwx-1);;
		int endz = min(ceil(fendz/cellwz), g_hmap.m_widthZ*TILE_SIZE/cellwz-1);;
		//int endx = startx+extentx;
		//int endz = startz+extentz;

		for(int x=startx; x<endx; x++)
			for(int z=startz; z<endz; z++)
			{
				ColliderCell* cell = ColliderCellAt(x, z);
				cell->colliders.push_back(Collider(COLLIDER_UNIT, i));
			}
	}

	//if(roadVehicle)
	//	return;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		//if(i == ignoreBuilding)
		//	continue;
		
		CBuildingType* bt = &g_buildingType[b->type];
		Vec3f p = b->pos;
		float hwx = bt->widthX*TILE_SIZE/2.0f;
		float hwz = bt->widthZ*TILE_SIZE/2.0f;
		
		float fstartx = p.x-hwx;
		float fstartz = p.z-hwz;
		float fendx = p.x+hwx;
		float fendz = p.z+hwz;
		
		int startx = fstartx/cellwx;
		int startz = fstartz/cellwz;
		int endx = ceil(fendx/cellwx);
		int endz = ceil(fendz/cellwz);

		for(int x=startx; x<endx; x++)
			for(int z=startz; z<endz; z++)
			{
				ColliderCell* cell = ColliderCellAt(x, z);
				cell->colliders.push_back(Collider(COLLIDER_BUILDING, i));
			}
	}
}

ColliderCell* ColliderCellAt(int x, int z)
{
	const int numcols = g_hmap.m_widthX*TILE_SIZE / (MIN_RADIUS*2.0f);
	return &g_collidercells[x + z*numcols];
}

void FreeColliderCells()
{
	if(g_collidercells)
	{
		delete [] g_collidercells;
		g_collidercells = NULL;
	}
}

/*
ColliderCell* CUnit::collidercellat(int x, int z)
{
	//CUnitType* t = &g_unitType[type];
	//const int numcols = g_hmap.m_widthX*TILE_SIZE / (t->radius*2.0f);
	//return &g_collidercells[x + z*numcols];

	CUnitType* t = &g_unitType[type];
	float ratiox = t->radius / MIN_RADIUS;
	float ratioz = t->radius / MIN_RADIUS;
	return ColliderCellAt(x * ratiox, z * ratioz);
}*/

void CUnit::fillcollidercells()
{
	//int uID = UnitID(this);
	int uID = this - g_unit;

	CUnitType* ut = &g_unitType[type];
	Vec3f p = camera.Position();
	float r = ut->radius;
		
	float fstartx = p.x-r;
	float fstartz = p.z-r;
	float fendx = p.x+r;
	float fendz = p.z+r;
		
	//int extentx = ceil(r/cellwx);
	//int extentz = ceil(r/cellwz);
	const float cellwx = MIN_RADIUS*2.0f;
	const float cellwz = MIN_RADIUS*2.0f;
		
	int startx = max(0, fstartx/cellwx);
	int startz = max(0, fstartz/cellwz);
	int endx = min(ceil(fendx/cellwx), g_hmap.m_widthX*TILE_SIZE/cellwx-1);
	int endz = min(ceil(fendz/cellwz), g_hmap.m_widthZ*TILE_SIZE/cellwz-1);
	//int endx = startx+extentx;
	//int endz = startz+extentz;

	for(int x=startx; x<endx; x++)
		for(int z=startz; z<endz; z++)
		{
			ColliderCell* cell = ColliderCellAt(x, z);
			cell->colliders.push_back(Collider(COLLIDER_UNIT, uID));
		}
}

void CUnit::freecollidercells()
{
	int uID = UnitID(this);

	LastNum("u frecolcel 1");

	CUnitType* ut = &g_unitType[type];
	Vec3f p = camera.Position();
	float r = ut->radius;
		
	float fstartx = p.x-r;
	float fstartz = p.z-r;
	float fendx = p.x+r;
	float fendz = p.z+r;
		
	//int extentx = ceil(r/cellwx);
	//int extentz = ceil(r/cellwz);
	const float cellwx = MIN_RADIUS*2.0f;
	const float cellwz = MIN_RADIUS*2.0f;
		
	int startx = max(0, fstartx/cellwx);
	int startz = max(0, fstartz/cellwz);
	int endx = min(ceil(fendx/cellwx), g_hmap.m_widthX*TILE_SIZE/cellwx-1);
	int endz = min(ceil(fendz/cellwz), g_hmap.m_widthZ*TILE_SIZE/cellwz-1);
	//int endx = startx+extentx;
	//int endz = startz+extentz;

	//LastNum("u frecolcel 2");

	for(int x=startx; x<endx; x++)
		for(int z=startz; z<endz; z++)
		{
			//char msg[128];
			//sprintf(msg, "u frecolcel 3 %d,%d", x, z);
			//LastNum(msg);

			ColliderCell* cell = ColliderCellAt(x, z);
			//cell->colliders.push_back(Collider(COLLIDER_UNIT, uID));
			for(int i=0; i<cell->colliders.size(); i++)
			{
				//LastNum("u frecolcel 4");

				Collider* c = &cell->colliders[i];
				if(c->type == COLLIDER_UNIT && c->ID == uID)
				{
					cell->colliders.erase( cell->colliders.begin() + i );
					//break;
					i --;
				}

				//LastNum("u frecolcel 5");
			}
		}
}

bool CUnit::trystep(CTryStep *bestS, bool roadVeh, int ignoreBuilding, CUnit* ignoreUnit)
{
	//if(collidercellat(bestS->x, bestS->z)->colliders.size() > 0)
	//	return false;
	
	//if(&g_unit[7] == this)
	//	Chat("7 pathing");

	//LastNum("in trystep");
	//g_log<<"in trystep"<<endl;

	//if(IsMilitary() && roadVeh)
	//{
	///	g_log<<"MILITARY ROAD VEHICLE"<<endl;
	//}

	//if(IsMilitary() && ignoreUnit != NULL)
	//{
	//	g_log<<"mil ignore unit = "<<(ignoreUnit-g_unit)<<" "<<g_unitType[ignoreUnit->type].name<<endl;
	//}

	CUnitType* ut = &g_unitType[type];
	//Vec3f p = camera.Position();
	float r = ut->radius;
	Vec3f p = trysteptoposition(bestS->x, bestS->z);
		
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
		return false;
	}

	if(roadVeh)
	{
	//LastNum("in trystep 1");
		for(int x=startx; x<endx; x++)
			for(int z=startz; z<endz; z++)
			{
				bool foundB = false;
				bool noroad = false;
				ColliderCell* cell = ColliderCellAt(x, z);
				//cell->colliders.push_back(Collider(COLLIDER_UNIT, uID));
				for(int i=0; i<cell->colliders.size(); i++)
				{
					Collider* c = &cell->colliders[i];
					if(c->type == COLLIDER_UNIT && &g_unit[c->ID] == this)
						continue;
					if(c->type == COLLIDER_BUILDING && c->ID == ignoreBuilding)
					{
						foundB = true;
						continue;
					}
					if(c->type == COLLIDER_NOROAD)
					{
						noroad = true;
						continue;
					}
					if(c->type == COLLIDER_UNIT)
					{
						if(&g_unit[c->ID] == ignoreUnit)
							continue;
						if(g_unit[c->ID].hidden())
							continue;
					}

					g_colliderType = c->type;
					g_lastCollider = c->ID;

					return false;
				}

				if(!foundB && noroad)
					return false;
			}
	}
	else
	{
	//LastNum("in trystep 2");
	//g_log<<"in trystep 2"<<endl;
		for(int x=startx; x<endx; x++)
			for(int z=startz; z<endz; z++)
			{
				
	//g_log<<"in trystep 2.1 "<<x<<"/"<<(g_hmap.m_widthX*TILE_SIZE/cellwx-1)<<","<<z<<"/"<<(g_hmap.m_widthZ*TILE_SIZE/cellwz-1)<<" "<<endl;

				ColliderCell* cell = ColliderCellAt(x, z);
				//cell->colliders.push_back(Collider(COLLIDER_UNIT, uID));
				for(int i=0; i<cell->colliders.size(); i++)
				{
					Collider* c = &cell->colliders[i];
					//g_log<<"collider "<<i<<" type="<<c->type<<" ID="<<c->ID<<endl;
					//g_log.flush();

					if(c->type == COLLIDER_UNIT && &g_unit[c->ID] == this)
						continue;
					if(c->type == COLLIDER_NOROAD)
						continue;
					if(c->type == COLLIDER_BUILDING && c->ID == ignoreBuilding)
						continue;
					if(c->type == COLLIDER_UNIT)
					{
						if(&g_unit[c->ID] == ignoreUnit)
							continue;
						if(g_unit[c->ID].hidden())
							continue;
					}
					
					g_colliderType = c->type;
					g_lastCollider = c->ID;

					return false;
				}
			}
	}
	
	// LastNum("in trystep 3");
	//g_log<<"in trystep 3"<<endl;

	return true;
}

float CUnit::pathlength()
{
	float len = 0.0f;

	Vec3f orig = camera.Position();

	for(int i=1; i<path.size(); i++)
	{
		len += Magnitude(path[i] - path[i-1]);

		camera.MoveTo(path[i]);

		if(CheckIfArrived())
			break;
	}

	camera.MoveTo(orig);

	return len;
}

int CUnit::stepbeforearrival()
{
	int s = 0;
	Vec3f orig = camera.Position();

	for(int i=0; i<path.size(); i++)
	{
		camera.MoveTo(path[i]);

		if(CheckIfArrived())
			break;

		s = i;
	}

	camera.MoveTo(orig);

	return s;
}
/*
bool CUnit::trydiagstep(CTryStep *bestS, CTryStep* tS)
{
	int dx = tS->x - bestS->x;
	int dz = tS->z - bestS->z;
	
	//if(collidercellat(bestS->x, bestS->z)->colliders.size() > 0)
	//	return false;
	if(collidercellat(bestS->x+dx, bestS->z)->colliders.size() > 0)
		return false;
	if(collidercellat(bestS->x, bestS->z+dz)->colliders.size() > 0)
		return false;
	//if(collidercellat(bestS->x+dx, bestS->z+dz)->colliders.size() > 0)
	//	return false;

	return true;
}
*/

bool CUnit::trydiagstep(CTryStep *nextS, CTryStep* prevS, bool roadVeh, int ignoreBuilding, CUnit* ignoreUnit)
{
	int dx = prevS->x - nextS->x;
	int dz = prevS->z - nextS->z;
	
	//if(collidercellat(nextS->x, nextS->z)->colliders.size() > 0)
	//	return false;
	//if(collidercellat(nextS->x+dx, nextS->z)->colliders.size() > 0)
	//	return false;
	//if(collidercellat(nextS->x, nextS->z+dz)->colliders.size() > 0)
	//	return false;
	//if(collidercellat(nextS->x+dx, nextS->z+dz)->colliders.size() > 0)
	//	return false;

	CTryStep tS;
	tS.x = nextS->x+dx;
	tS.z = nextS->z;

	if(!trystep(&tS, roadVeh, ignoreBuilding, ignoreUnit))
		return false;
	
	tS.x = nextS->x;
	tS.z = nextS->z+dz;
	
	if(!trystep(&tS, roadVeh, ignoreBuilding, ignoreUnit))
		return false;

	return true;
}

bool CUnit::Pathfind(bool nodelay)
{
	
	//char msg[128];
	//sprintf(msg, "Delay left %d (last path = %d, GetTickCount() = %d)", (int)(PATH_DELAY - GetTickCount() + g_lastPath), (int)g_lastPath, (int)GetTickCount());
	//Chat(msg);
	
	/*
	//if(!nodelay)
	if(IsMilitary())
	{
		if(GetTickCount() - g_lastpath < PATH_DELAY)
			return false;
		else
		{
			//sprintf(msg, "Delay left %d @ %d", (int)(PATH_DELAY - GetTickCount() + g_lastPath), (int)GetTickCount());
			//Chat(msg);
			g_lastpath = GetTickCount();
		}
	}*/
	
	g_tryStep.clear();
	g_triedStep.clear();
	path.clear();
	step = 0;
	g_curpath ++;
	/*
	bool dbug = false;

	if(UnitID(this) == 0)
		dbug = true;
		*/
	/*
	CUnitType* ut = &g_unitType[type];
	char msg[128];
	sprintf(msg, "%s #%d path @ %d", ut->name, UnitID(this), (int)GetTickCount());
	Chat(msg);
	*/

	int ignoreBuilding = -1;
	CUnit* ignoreUnit = NULL;
	
	const CUnitType* t = &g_unitType[type];

	bool roadVehicle = t->roaded;

	const float tileSize = t->radius*2.0f;
	const Vec3f pos = camera.Position();
	
	const int maxx = g_hmap.m_widthX*TILE_SIZE/tileSize-1;
	const int maxz = g_hmap.m_widthZ*TILE_SIZE/tileSize-1;

	const int startx = pos.x / tileSize;
	const int startz = pos.z / tileSize;
	int endx = goal.x / tileSize;
	int endz = goal.z / tileSize;

	//endzone
	int ezstartx = -1;
	int ezstartz = -1;
	int ezendx = -1;
	int ezendz = -1;
	
#ifdef PATH_DEBUG
		//int uID = UnitID(this);
	//if(uID == 5)
	if(IsMilitary())
	{
		g_log<<"--------------NEWPATH UNIT"<<UnitID(this)<<" type="<<g_unitType[type].name<<"-------------------"<<endl;
		g_log<<"mode = "<<mode<<endl;
		g_log<<"target/2 = "<<target<<","<<target2<<endl;
		g_log<<"startx/z = "<<startx<<","<<startz<<endl;
		if(mode == GOINGTODEMANDERB)
		{
			g_log<<"demb = "<<g_buildingType[g_building[target].type].name<<endl;
		}
		else if(mode == GOINGTOSUPPLIER)
		{
			g_log<<"supb = "<<g_buildingType[g_building[supplier].type].name<<endl;
			g_log<<"endx/z = "<<endx<<","<<endz<<endl;
		}
		g_log.flush();
	}
#endif

	if(mode == GOINGTONORMJOB || mode == GOINGTOCONJOB || mode == GOINGTOREST || mode == GOINGTOSHOP || mode == GOINGTODEMANDERB)
		ignoreBuilding = target;
	else if(mode == GOINGTOREFUEL)
		ignoreBuilding = fuelStation;
	else if(mode == GOINGTOSUPPLIER)
		ignoreBuilding = supplier;
	else if(mode == GOINGTODEMROAD || mode == GOINGTOROADJOB)
	{
		//do not rely on "goal", it may not point to target
		Vec3f endgoal = RoadPosition(target, target2);
		float fezstartx = endgoal.x - TILE_SIZE;
		float fezstartz = endgoal.z - TILE_SIZE;
		float fezendx = endgoal.x + TILE_SIZE;
		float fezendz = endgoal.z + TILE_SIZE;
		ezstartx = fezstartx / tileSize + 1;
		ezstartz = fezstartz / tileSize + 1;
		ezendx = fezendx / tileSize - 1;
		ezendz = fezendz / tileSize - 1;
		//don't let variables derived from faulty "goal" screw us up
		endx = (ezstartx+ezendx)/2;
		endz = (ezstartz+ezendz)/2;
		
		//g_log<<"road endzone: "<<ezstartx<<","<<ezstartz<<"->"<<ezendx<<","<<ezendz<<" "<<endl;
		//g_log.flush();
	}
	else if(mode == GOINGTODEMPOWL || mode == GOINGTOPOWLJOB)
	{
		//do not rely on "goal", it may not point to target
		Vec3f endgoal = PowerlinePosition(target, target2);
		float fezstartx = endgoal.x - TILE_SIZE/2;
		float fezstartz = endgoal.z - TILE_SIZE/2;
		float fezendx = endgoal.x + TILE_SIZE/2;
		float fezendz = endgoal.z + TILE_SIZE/2;
		ezstartx = fezstartx / tileSize + 1;
		ezstartz = fezstartz / tileSize + 1;
		ezendx = fezendx / tileSize - 1;
		ezendz = fezendz / tileSize - 1;
		//don't let variables derived from faulty "goal" screw us up
		endx = (ezstartx+ezendx)/2;
		endz = (ezstartz+ezendz)/2;

#ifdef PATH_DEBUG
	if(IsMilitary())
		{
			g_log<<"endgoal.x - TILE_SIZE/2 = "<<(endgoal.x - TILE_SIZE/2)<<endl;
			g_log<<"endgoal.z - TILE_SIZE/2 = "<<(endgoal.z - TILE_SIZE/2)<<endl;
			g_log<<"fezstartx / tileSize = "<<(int)(fezstartx / tileSize)<<endl;
			g_log<<"fezstartz / tileSize = "<<(int)(fezstartz / tileSize)<<endl;
			g_log<<"fezendx / tileSize = "<<(int)(fezendx / tileSize)<<endl;
			g_log<<"fezendz / tileSize = "<<(int)(fezendz / tileSize)<<endl;
		}
#endif
	}
	else if(mode == GOINGTODEMPIPE || mode == GOINGTOPIPEJOB)
	{
		//do not rely on "goal", it may not point to target
		Vec3f endgoal = PipelinePhysPos(target, target2);
		float fezstartx = endgoal.x - TILE_SIZE/2;
		float fezstartz = endgoal.z - TILE_SIZE/2;
		float fezendx = endgoal.x + TILE_SIZE/2;
		float fezendz = endgoal.z + TILE_SIZE/2;
		ezstartx = fezstartx / tileSize + 1;
		ezstartz = fezstartz / tileSize + 1;
		ezendx = fezendx / tileSize - 1;
		ezendz = fezendz / tileSize - 1;
		//don't let variables derived from faulty "goal" screw us up
		endx = (ezstartx+ezendx)/2;
		endz = (ezstartz+ezendz)/2;
	}
	else if(mode == GOINGTOTRUCK)
	{
		ignoreUnit = &g_unit[target];
		goal = g_unit[target].camera.Position();
		//char msg[128];
		//sprintf(msg, "ignore u = %s", g_unitType[ignoreUnit->type].name);
		//Chat(msg);
	}
	else if(IsMilitary())
		{
			if(target >= 0)
			{
				if(targetU)
				{
					ignoreUnit = &g_unit[target];
					goal = g_unit[target].camera.Position();
				}
				else
				{
					ignoreBuilding = target;
					goal = g_building[target].pos;
				}
			
				endx = goal.x / tileSize;
				endz = goal.z / tileSize;
			}
		}

	if(type == TRUCK)
	{
		if(driver >= 0)
			ignoreUnit = &g_unit[driver];
		//CBuilding* b = &g_building[ignoreBuilding];
		//char msg[128];
		//sprintf(msg, "mode=%d, ignore b = %s", mode, g_buildingType[b->type].name);
		//Chat(msg);
	}

	//for(int x=0; x<g_pathfindszx; x++)
	//	for(int z=0; z<g_pathfindszz; z++)
	//		OpenAt(x, z) = true;

	
	LastNum("in pathfind 1");

	g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz, -1, 0, 0));

#ifdef PATH_DEBUG
	if(IsMilitary())
	{
		g_log<<"startx,z: "<<startx<<","<<startz<<endl;
	}
#endif
	
#ifdef PATH_DEBUG
	if(IsMilitary())
	{
		if(ignoreBuilding >= 0)
		{
			g_log<<"fillcollidercells ignoreB = "<<g_buildingType[g_building[ignoreBuilding].type].name<<endl;
			g_log.flush();
		}
	}
#endif

	//fillcollidercells(ignoreUnit, ignoreBuilding, roadVehicle);
	/*
	if(startx > 0)
		g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx - 1, startz, -1));
	if(startx < g_pathfindszx-1)
		g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx + 1, startz, -1));
	if(startz > 0)
		g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz - 1, -1));
	if(startz < g_pathfindszz-1)
		g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz + 1, -1));

#ifndef NO_DIAGONAL
	if(startx > 0 && startz > 0)
		g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx - 1, startz - 1, -1));
	if(startx < g_pathfindszx-1 && startz > 0)
		g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx + 1, startz - 1, -1));
	if(startx > 0 && startz < g_pathfindszz-1)
		g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx - 1, startz + 1, -1));
	if(startx < g_pathfindszx-1 && startz < g_pathfindszz-1)
		g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx + 1, startz + 1, -1));
#endif*/

	int i;
	int best;
	CTryStep* tS;
	CTryStep* bestS;
	Vec3f vHalfWay;

	int searchdepth = 0;

	while(true)
	{
		searchdepth ++;

		if(searchdepth > MAX_SEARCH_DEPTH)
		{
			//Chat("path failed1");
			/*
			if(dbug)
			{
				g_log<<"path failed1"<<endl;
				g_log.flush();
			}*/
			goto returnfalse;
		}

		best = -1;
		bestS = NULL;

	//LastNum("in pathfind 1.1");

		for(i=0; i<g_tryStep.size(); i++)
		{
			tS = &g_tryStep[i];
			//if(!OpenAt(tS->x, tS->z))
			if(OpenAt(tS->x, tS->z) == g_curpath)
			{
				//g_tryStep.erase( g_tryStep.begin() + i );
				//i--;
			//if(tS->tried)
				continue;
			}

			if(bestS != NULL && tS->F > bestS->F)
				continue;

			bestS = tS;
			best = i;
		}

		if(best < 0)
		{
			//Chat("path failed2");
			/*
			if(dbug)
			{
				g_log<<"path failed2"<<endl;
				g_log.flush();
			}*/

#ifdef PATH_DEBUG
	if(IsMilitary())
			{
				Chat("mil path fail");
			}
#endif

			goto returnfalse;
		}

		//bestS->tried = true;
	//LastNum("in pathfind 1.2");
		
		//OpenAt(bestS->x, bestS->z) = false;
		OpenAt(bestS->x, bestS->z) = g_curpath;
		g_triedStep.push_back(*bestS);
		g_tryStep.erase( g_tryStep.begin() + best );
		best = g_triedStep.size() - 1;
		bestS = &g_triedStep[ best ];

		//LastNum("in pathfind 1.3");

		//Vec3f vBestS = trysteptoposition(bestS->x, bestS->z);
		
#ifdef PATH_DEBUG
	if(IsMilitary())
		{
			g_log<<"trystep "<<bestS->x<<","<<bestS->z<<" d=("<<(bestS->x-endx)<<","<<(bestS->z-endz)<<") mag="<<Magnitude(trysteptoposition(bestS->x, bestS->z)-trysteptoposition(endx, endz))<<" depth"<<searchdepth<<"/"<<MAX_SEARCH_DEPTH<<" ";
			g_log.flush();
		}
#endif
		//if(bestS->previous >= 0 && !TryStep(vBestS, ignoreUnit, ignoreBuilding, roadVehicle))
		if(bestS->previous >= 0 && !trystep(bestS, roadVehicle, ignoreBuilding, ignoreUnit))
		{
#ifdef PATH_DEBUG
	if(IsMilitary())
			{
				if(ColliderCellAt(bestS->x, bestS->z)->colliders.size() > 0)
				{
					for(int i=0; i<ColliderCellAt(bestS->x, bestS->z)->colliders.size(); i++)
					{
						Collider* c = &ColliderCellAt(bestS->x, bestS->z)->colliders[i];

						g_log<<" [collider type="<<c->type<<" ID="<<c->ID<<" ";

						if(c->type == COLLIDER_UNIT)
						{
							g_log<<"unit="<<g_unitType[g_unit[c->ID].type].name;
						}
						else if(c->type == COLLIDER_BUILDING)
						{
							g_log<<"b="<<g_buildingType[g_building[c->ID].type].name;
						}
						else if(c->type == COLLIDER_TERRAIN)
						{
							if(c->ID == 0 && roadVehicle)
							{
								g_log<<"noroad";
							}
							if(c->ID == 1)
							{
								g_log<<"unclim";
							}
							if(c->ID == 2)
							{
								g_log<<"water";
							}
						}

						g_log<<"] ";
					}
				}

				g_log<<"NOT"<<endl;
				g_log.flush();
			}
#endif
			continue;
		}
		
#ifdef PATH_DEBUG
	if(IsMilitary())
		{
			g_log<<"OK"<<endl;
			g_log.flush();
		}
#endif
		/*
		if(bestS->previous < 0)
		{
#ifndef NO_DIAGONAL

			//vHalfWay = (vBestS + pos) / 2.0f;
			//if(!TryStep(vHalfWay, ignoreUnit, ignoreBuilding, roadVehicle))
			//	continue;
#endif
		}
		else
		{
			tS = &g_tryStep[bestS->previous];
			//Vec3f prevP = trysteptoposition(tS->x, tS->z);
			//if(fabs(prevP.y - vBestS.y) > MAX_CLIMB_INCLINE)
			//	continue;

#ifndef NO_DIAGONAL
			//vHalfWay = (vBestS + prevP) / 2.0f;
			//if(!TryStep(vHalfWay, ignoreUnit, ignoreBuilding, roadVehicle))
			//	continue;
			int dx = tS->x - bestS->x;
			int dz = tS->z - bestS->z;
			if(abs(dx) > 0 && abs(dz) > 0)
			{
				if(!trydiagstep(bestS, tS))
					continue;
			}
#endif
		}*/

		if((bestS->x == endx && bestS->z == endz) || (bestS->x >= ezstartx && bestS->x <= ezendx && bestS->z >= ezstartz && bestS->z <= ezendz))
		{
			
	LastNum("in pathfind 2");
#ifdef PATH_DEBUG
	if(IsMilitary())
			{
				if(bestS->x == endx && bestS->z == endz) 
				{
					g_log<<"reached "<<endx<<","<<endz<<endl;
					g_log.flush();
				}
				if(bestS->x >= ezstartx && bestS->x <= ezendx && bestS->z >= ezstartz && bestS->z <= ezendz)
				{
					g_log<<"reached "<<ezstartx<<","<<ezstartz<<"->"<<ezendx<<","<<ezendz<<endl;
					g_log.flush();
				}
			}
#endif
			/*
			if(dbug)
			{
				g_log<<"path success"<<endl;
				g_log.flush();
			}
			*/
			//step = 0;
			tS = bestS;

			while(tS->previous >= 0)
			{
				path.push_front(trysteptoposition(tS->x, tS->z));
				//g_log<<"path "<<tS->x<<","<<tS->z<<" prev="<<tS->previous<<endl;
				//tS = &g_tryStep[tS->previous];
				tS = &g_triedStep[tS->previous];
			}

			path.push_back(goal);
			
			if(path.size() > 0)
				subgoal = path[0];
			
			//if(path.size() > 3)
			//	subgoal = path[3];

			//Chat("path success");

			/*
			if(dbug)
			{
				g_log<<"pathsize="<<path.size()<<endl;
				g_log.flush();
			}
			*/
			goto returntrue;
		}

		bool cannorth = true;
		bool cansouth = true;
		bool canwest = true;
		bool caneast = true;

		if(bestS->previous < 0 && !trystep(bestS, roadVehicle, ignoreBuilding, ignoreUnit) && g_colliderType == COLLIDER_UNIT)
		{
	LastNum("in pathfind 3");
			//CUnitType* t = &g_unitType[type];
			//const float cellwx = t->radius*2.0f;
			//const float cellwz = t->radius*2.0f;
			const float cellwx = MIN_RADIUS*2.0f;
			const float cellwz = MIN_RADIUS*2.0f;
			const int numcols = g_hmap.m_widthX*TILE_SIZE / cellwx;
			const int numrows = g_hmap.m_widthZ*TILE_SIZE / cellwz;
			float r = t->radius;

			Vec3f p = trysteptoposition(bestS->x, bestS->z);
		
			float fstartx = p.x-r;
			float fstartz = p.z-r;
			float fendx = p.x+r;
			float fendz = p.z+r;
		
			//int extentx = ceil(r/cellwx);
			//int extentz = ceil(r/cellwz);
		
			int istartx = max(0, fstartx/cellwx);
			int istartz = max(0, fstartz/cellwz);
			int iendx = min(ceil(fendx/cellwx), g_hmap.m_widthX*TILE_SIZE/cellwx-1);
			int iendz = min(ceil(fendz/cellwz), g_hmap.m_widthZ*TILE_SIZE/cellwz-1);
			//int endx = startx+extentx;
			//int endz = startz+extentz;

			float xratio = MIN_RADIUS / r;
			float zratio = MIN_RADIUS / r;

			for(int x=istartx; x<iendx; x++)
				for(int z=istartz; z<iendz; z++)
				{
					ColliderCell* cell = ColliderCellAt(x, z);

					for(int j=0; j<cell->colliders.size(); j++)
					{
						Collider* c = &cell->colliders[j];

						if(c->type != COLLIDER_UNIT)
							continue;

						if(&g_unit[c->ID] == this)
							continue;

						CUnit* u2 = &g_unit[c->ID];

						CUnitType* u2t = &g_unitType[u2->type];
						p = u2->camera.Position();
						r = u2t->radius;
		
						fstartx = p.x-r;
						fstartz = p.z-r;
						fendx = p.x+r;
						fendz = p.z+r;
		
						//int extentx = ceil(r/cellwx);
						//int extentz = ceil(r/cellwz);
		
						int u2startx = fstartx/cellwx;
						int u2startz = fstartz/cellwz;
						int u2endx = ceil(fendx/cellwx);
						int u2endz = ceil(fendz/cellwz);
						//int endx = startx+extentx;
						//int endz = startz+extentz;

						for(int x=u2startx; x<u2endx; x++)
							for(int z=u2startz; z<u2endz; z++)
							{
								if(x*xratio == bestS->x && (z*zratio+1 == bestS->z || z*zratio-1 == bestS->z))
								{
									canwest = false;
									caneast = false;
								}
						
								if(z*zratio == bestS->z && (x*xratio+1 == bestS->x || x*xratio-1 == bestS->x))
								{
									cannorth = false;
									cansouth = false;
								}
							}
					}
			}
		}

		float runningD = 0;
		float stepD = 1;

		if(bestS->previous >= 0)
		{
			tS = &g_tryStep[bestS->previous];
			runningD = tS->runningD;
		}

		if(bestS->x > 0 && canwest)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x - 1, bestS->z, best, runningD, stepD));
		//if(bestS->x < g_pathfindszx-1 && caneast)
		if(bestS->x < maxx && caneast)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x + 1, bestS->z, best, runningD, stepD));
		if(bestS->z > 0 && cannorth)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z - 1, best, runningD, stepD));
		//if(bestS->z < g_pathfindszz-1 && cansouth)
		if(bestS->z < maxz && cansouth)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z + 1, best, runningD, stepD));

#ifndef NO_DIAGONAL
		stepD = Magnitude3(Vec3f(1,0,1));

		//if(bestS->x > 0 && bestS->z > 0 && canwest && cannorth)
		if(bestS->x > 0 && bestS->z > 0)
		{
			CTryStep nextS(startx, startz, endx, endz, bestS->x - 1, bestS->z - 1, best, runningD, stepD);
			if(trydiagstep(&nextS, bestS, roadVehicle, ignoreBuilding, ignoreUnit))
				g_tryStep.push_back(nextS);
		}
		//if(bestS->x < g_pathfindszx-1 && bestS->z > 0 && caneast && cannorth)
		//if(bestS->x < g_pathfindszx-1 && bestS->z > 0)
		if(bestS->x < maxx && bestS->z > 0)
		{
			CTryStep nextS(startx, startz, endx, endz, bestS->x + 1, bestS->z - 1, best, runningD, stepD);
			if(trydiagstep(&nextS, bestS, roadVehicle, ignoreBuilding, ignoreUnit))
				g_tryStep.push_back(nextS);
		}
		//if(bestS->x > 0 && bestS->z < g_pathfindszz-1 && canwest && cansouth)
		//if(bestS->x > 0 && bestS->z < g_pathfindszz-1)
		if(bestS->x > 0 && bestS->z < maxz)
		{
			CTryStep nextS(startx, startz, endx, endz, bestS->x - 1, bestS->z + 1, best, runningD, stepD);
			if(trydiagstep(&nextS, bestS, roadVehicle, ignoreBuilding, ignoreUnit))
				g_tryStep.push_back(nextS);
		}
		//if(bestS->x < g_pathfindszx-1 && bestS->z < g_pathfindszz-1 && caneast && cansouth)
		//if(bestS->x < g_pathfindszx-1 && bestS->z < g_pathfindszz-1)
		if(bestS->x < maxx && bestS->z < maxz)
		{
			CTryStep nextS(startx, startz, endx, endz, bestS->x + 1, bestS->z + 1, best, runningD, stepD);
			if(trydiagstep(&nextS, bestS, roadVehicle, ignoreBuilding, ignoreUnit))
				g_tryStep.push_back(nextS);
		}
#endif
	}

	//Chat("path failed3");
	/*
			if(dbug)
			{
				g_log<<"path failed3"<<endl;
				g_log.flush();
			}*/
returnfalse:

	//Chat("path failed");

	return false;

returntrue:
	//char msg[128];
	//sprintf(msg, "path suc pathsize=%d @ %d", (int)path.size(), (int)GetTickCount());
	//Chat(msg);

	return true;
}

void DrawGrid()
{
	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	//glBegin(GL_LINES);
	
	for(int x=0; x<g_pathfindszx; x++)
	{
		vector<Vec3f> vecs;
		vecs.push_back(Vec3f(x*(MIN_RADIUS*2.0f), 0 + 1, 0));
		vecs.push_back(Vec3f(x*(MIN_RADIUS*2.0f), 0 + 1, g_hmap.m_widthZ*TILE_SIZE));
		//glVertex3f(x*(MIN_RADIUS*2.0f), 0 + 1, 0);
		//glVertex3f(x*(MIN_RADIUS*2.0f), 0 + 1, g_hmap.m_widthZ*TILE_SIZE);
		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
		glDrawArrays(GL_LINES, 0, vecs.size());
	}

	for(int z=0; z<g_pathfindszz; z++)
	{
		vector<Vec3f> vecs;
		vecs.push_back(Vec3f(0, 0 + 1, z*(MIN_RADIUS*2.0f)));
		vecs.push_back(Vec3f(g_hmap.m_widthX*TILE_SIZE, 0 + 1, z*(MIN_RADIUS*2.0f)));
		//glVertex3f(0, 0 + 1, z*(MIN_RADIUS*2.0f));
		//glVertex3f(g_hmap.m_widthX*TILE_SIZE, 0 + 1, z*(MIN_RADIUS*2.0f));
		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
		glDrawArrays(GL_LINES, 0, vecs.size());
	}

	
	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR],  0.5f, 0, 0, 1);

	if(g_selectType == SELECT_UNIT && g_selection.size() > 0)
	{
		int i = g_selection[0];
		CUnit* u = &g_unit[i];

		bool roadVeh = false;
		if(u->type == TRUCK)
			roadVeh = true;

		CUnitType* t = &g_unitType[u->type];
		const float cellwx = t->radius*2.0f;
		const float cellwz = t->radius*2.0f;
		const int numcols = g_hmap.m_widthX*TILE_SIZE / cellwx;
		const int numrows = g_hmap.m_widthZ*TILE_SIZE / cellwz;

		float xratio = t->radius / MIN_RADIUS;
		float zratio = t->radius / MIN_RADIUS;

		for(int x=0; x<numcols; x++)
			for(int z=0; z<numrows; z++)
			{
				//ColliderCell* cell = u->collidercellat(x, z);
				ColliderCell* cell = ColliderCellAt(x*xratio, z*zratio);
				if(cell->colliders.size() <= 0)
					continue;

				vector<Vec3f> vecs;
				vecs.push_back(Vec3f(x*cellwx, 1, z*cellwz));
				vecs.push_back(Vec3f((x+1)*cellwx, 1, (z+1)*cellwz));
				vecs.push_back(Vec3f((x+1)*cellwx, 1, z*cellwz));
				vecs.push_back(Vec3f(x*cellwx, 1, (z+1)*cellwz));
				glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
				glDrawArrays(GL_LINES, 0, vecs.size());
			}
	}

	//glEnd();
	glColor4f(1, 1, 1, 1);
}

void DrawUnitSquares()
{
	CUnit* u;
	CUnitType* t;
	float r;
	Vec3f p;

	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 0.5f, 0, 0, 1);
	
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		t = &g_unitType[u->type];
		r = t->radius;
		p = u->camera.Position();

		if(u->Collides())
			glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 1.0f, 0, 0, 1);
		else
			glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 0.2f, 0, 0, 1);
		
		/*
		glVertex3f(p.x - r, 0 + 1, p.z - r);
		glVertex3f(p.x - r, 0 + 1, p.z + r);
		glVertex3f(p.x + r, 0 + 1, p.z + r);
		glVertex3f(p.x + r, 0 + 1, p.z - r);
		glVertex3f(p.x - r, 0 + 1, p.z - r);
		*/

		vector<Vec3f> vecs;
		vecs.push_back(Vec3f(p.x - r, 0 + 1, p.z - r));
		vecs.push_back(Vec3f(p.x - r, 0 + 1, p.z + r));
		vecs.push_back(Vec3f(p.x + r, 0 + 1, p.z + r));
		vecs.push_back(Vec3f(p.x + r, 0 + 1, p.z - r));
		vecs.push_back(Vec3f(p.x - r, 0 + 1, p.z - r));
		
		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
		glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
	}

}

void DrawPaths()
{
	CUnit* u;
	Vec3f p;
	
	//glColor4f(0.2f, 0, 0, 1);

	for(int i=0; i<UNITS; i++)
	{
		glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 0, 1, 0, 1);

		u = &g_unit[i];

		if(!u->on)
			continue;

		//glBegin(GL_LINE_STRIP);

		vector<Vec3f> vecs;
		
		for(int j=0; j<u->path.size(); j++)
		{
			p = u->path[j];
			//glVertex3f(p->x, p->y + 5, p->z);
			p.y += 0.1f;
			vecs.push_back(p);
			//vecs.push_back(p+Vec3f(0,10,0));
		}
		
		if(vecs.size() > 0)
		{
    glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
    glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
		else
		{
			vecs.push_back(u->camera.Position() + Vec3f(0,5,0));
			vecs.push_back(u->goal + Vec3f(0,5,0));

			glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 0.8f, 1, 0.8f, 1);
			
			glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
			glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}

		//glEnd();
	}
	
	//glColor4f(1, 1, 1, 1);
	//glEnable(GL_TEXTURE_2D);
	/*
    float vertices[] =
    {
        //posx, posy    texx, texy
        g_mousestart.x, g_mousestart.y, 0,          0, 0,
        g_mousestart.x, g_mouse.y,0,         1, 0,
        g_mouse.x, g_mouse.y,0,      1, 1,
        
        g_mouse.x, g_mousestart.y,0,      1, 1,
        g_mousestart.x, g_mousestart.y,0,       0, 1
    };*/
}

void DrawVelocities()
{
	CUnit* u;
	Vec3f p;
	
	//glColor4f(0.2f, 0, 0, 1);
	
	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 1, 0, 1, 1);

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		//glBegin(GL_LINE_STRIP);

		vector<Vec3f> vecs;
		
			vecs.push_back(u->camera.Position() + Vec3f(0, 1, 0));
			vecs.push_back(u->camera.Position() + u->camera.Velocity()*100.0f + Vec3f(0, 1, 0));
		
		if(vecs.size() > 0)
		{
    glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
    glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}

		//glEnd();
	}
	
	//glColor4f(1, 1, 1, 1);
	//glEnable(GL_TEXTURE_2D);
	/*
    float vertices[] =
    {
        //posx, posy    texx, texy
        g_mousestart.x, g_mousestart.y, 0,          0, 0,
        g_mousestart.x, g_mouse.y,0,         1, 0,
        g_mouse.x, g_mouse.y,0,      1, 1,
        
        g_mouse.x, g_mousestart.y,0,      1, 1,
        g_mousestart.x, g_mousestart.y,0,       0, 1
    };*/
}