

#include "pathcell.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sys/workthread.h"
#include "../sim/unit.h"
#include "../sim/unittype.h"
#include "../sim/building.h"
#include "../sim/buildingtype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../sim/road.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/sim.h"
#include "../phys/trace.h"
#include "binheap.h"
#include "pathnode.h"
#include "../math/vec2i.h"
#include "pathdebug.h"

ColliderTile *g_collidertile = NULL;
Vec2i g_pathdim(0,0);

ColliderTile::ColliderTile()
{
#if 0
	bool hasroad;
	bool hasland;
	bool haswater;
	bool abrupt;	//abrupt incline?
	list<int> units;
	list<int> foliage;
	int building;
#endif

	hasroad = false;
	hasland = false;
	haswater = false;
	abrupt = false;
	building = -1;
}

inline Vec2i PathNodePos(int cmposx, int cmposz)
{
	return Vec2i(cmposx/PATHNODE_SIZE, cmposz/PATHNODE_SIZE);
}

inline unsigned int PathNodeIndex(int cx, int cz)
{
	return cz * g_pathdim.x + cx;
}

void FreePathGrid()
{
	if(g_collidertile)
	{
		delete [] g_collidertile;
		g_collidertile = NULL;
	}

	if(g_pathnode)
	{
		delete [] g_pathnode;
		g_pathnode = NULL;
	}

	g_pathdim = Vec2i(0,0);

	for(int i=0; i<WORKTHREADS; i++)
		g_workthread[i].destroy();
}

void AllocPathGrid(int cmwx, int cmwz)
{
	FreePathGrid();
	g_pathdim.x = cmwx / PATHNODE_SIZE;
	g_pathdim.y = cmwz / PATHNODE_SIZE;
	g_collidertile = new ColliderTile [ g_pathdim.x * g_pathdim.y ];
	g_pathnode = new PathNode [ g_pathdim.x * g_pathdim.y ];

	for(int i=0; i<WORKTHREADS; i++)
	{
		g_workthread[i].alloc(g_pathdim.x, g_pathdim.y);
	}
}

ColliderTile* ColliderTileAt(int cx, int cz)
{
	return &g_collidertile[ PathNodeIndex(cx, cz) ];
}

void FillColliderGrid()
{
	const int cwx = g_pathdim.x;
	const int cwz = g_pathdim.y;

	for(int x=0; x<cwx; x++)
		for(int z=0; z<cwz; z++)
		{
			int cmx = x*PATHNODE_SIZE + PATHNODE_SIZE/2;
			int cmz = z*PATHNODE_SIZE + PATHNODE_SIZE/2;
			ColliderTile* cell = ColliderTileAt(x, z);
            
			//g_log<<"cell "<<x<<","<<z<<" cmpos="<<cmx<<","<<cmz<<" y="<<g_hmap.accheight(cmx, cmz)<<endl;

			if(AtLand(cmx, cmz))
			{
				cell->hasland = true;
				//g_log<<"land"<<endl;
			}
			else
				cell->hasland = false;

			if(AtWater(cmx, cmz))
				cell->haswater = true;
			else
				cell->haswater = false;
            
			if(TileUnclimable(cmx, cmz))
				cell->abrupt = true;
			else
				cell->abrupt = false;
            
			int tx = cmx/TILE_SIZE;
			int tz = cmz/TILE_SIZE;

			RoadTile* r = RoadAt(tx, tz);

			if(r->on && r->finished)
				cell->hasroad = true;
			else
				cell->hasroad = false;
		}

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];
	
		if(!u->on)
			continue;

		u->fillcollider();
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		b->fillcollider();
	}

	for(int i=0; i<WORKTHREADS; i++)
		g_workthread[i].resetcells();
}

void Unit::fillcollider()
{
	UnitT* t = &g_unitT[type];
	int ui = this - g_unit;

	//cm = centimeter position
	int cmminx = cmpos.x - t->size.x/2;
	int cmminz = cmpos.y - t->size.z/2;
	int cmmaxx = cmminx + t->size.x;
	int cmmaxz = cmminz + t->size.z;

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminz = cmminz / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxz = cmmaxz / PATHNODE_SIZE;

	for(int cx = cminx; cx <= cmaxx; cx++)
		for(int cz = cminz; cz <= cmaxz; cz++)
		{
			ColliderTile* c = ColliderTileAt(cx, cz);
			c->units.push_back(ui);
		}
}

void Building::fillcollider()
{
	BuildingT* t = &g_buildingT[type];
	int bi = this - g_building;

	//t = tile position
	int tminx = tilepos.x - t->widthx/2;
	int tminz = tilepos.y - t->widthz/2;
	int tmaxx = tminx + t->widthx;
	int tmaxz = tminz + t->widthz;

	//cm = centimeter position
	int cmminx = tminx*TILE_SIZE;
	int cmminz = tminz*TILE_SIZE;
	int cmmaxx = tmaxx*TILE_SIZE;
	int cmmaxz = tmaxz*TILE_SIZE;

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminz = cmminz / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxz = cmmaxz / PATHNODE_SIZE;

	for(int cx = cminx; cx <= cmaxx; cx++)
		for(int cz = cminz; cz <= cmaxz; cz++)
		{
			ColliderTile* c = ColliderTileAt(cx, cz);
			c->building = bi;
		}
}

void Unit::freecollider()
{
	UnitT* t = &g_unitT[type];
	int ui = this - g_unit;

	//cm = centimeter position
	int cmminx = cmpos.x - t->size.x/2;
	int cmminz = cmpos.y - t->size.z/2;
	int cmmaxx = cmminx + t->size.x;
	int cmmaxz = cmminz + t->size.z;

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminz = cmminz / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxz = cmmaxz / PATHNODE_SIZE;

	for(int cx = cminx; cx <= cmaxx; cx++)
		for(int cz = cminz; cz <= cmaxz; cz++)
		{
			ColliderTile* c = ColliderTileAt(cx, cz);

			auto cuiter = c->units.begin();
			while(cuiter != c->units.end())
			{
				if(*cuiter == ui)
				{
					cuiter = c->units.erase(cuiter);
					continue;
				}

				cuiter ++;
			}
		}
}

void Building::freecollider()
{
	BuildingT* t = &g_buildingT[type];
	int bi = this - g_building;

	//t = tile position
	int tminx = tilepos.x - t->widthx/2;
	int tminz = tilepos.y - t->widthz/2;
	int tmaxx = tminx + t->widthx;
	int tmaxz = tminz + t->widthz;

	//cm = centimeter position
	int cmminx = tminx*TILE_SIZE;
	int cmminz = tminz*TILE_SIZE;
	int cmmaxx = tmaxx*TILE_SIZE;
	int cmmaxz = tmaxz*TILE_SIZE;

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminz = cmminz / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxz = cmmaxz / PATHNODE_SIZE;

	for(int cx = cminx; cx <= cmaxx; cx++)
		for(int cz = cminz; cz <= cmaxz; cz++)
		{
			ColliderTile* c = ColliderTileAt(cx, cz);

			if(c->building == bi)
				c->building = -1;
		}
}

bool NodeForcedNeighbour(const int cx,
					const int cz,
					const int cminx,
					const int cminz,
					const int cmaxx,
					const int cmaxz,
					const int cmminx,
					const int cmminz,
					const int cmmaxx,
					const int cmmaxz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb)
{
	//side lengths
	bool westopen = true;
	bool eastopen = true;
	bool southopen = true;
	bool northopen = true;

	//corner cells
	bool nwcorneropen = true;
	bool necorneropen = true;
	bool secorneropen = true;
	bool swcorneropen = true;

	if(cminx > 0)
	{
		const int offcmminx = cmminx - PATHNODE_SIZE;
		const int offcmminz = cmminz;
		const int offcmmaxx = cmmaxx - PATHNODE_SIZE;
		const int offcmmaxz = cmmaxz;

		for(int z=cminz; z<=cmaxz; z++)
		{
			if(NodeWalkable(cx-1, z,
				cminx-1, cminz, cmaxx-1, cmaxz,
				offcmminx, offcmminz, offcmmaxx, offcmmaxz,
				roaded, landborne, seaborne, airborne, 
				thisu, ignoreu, ignoreb) != COLLIDER_NONE)
			{
				westopen = false;
				break;
			}
		}
	}
	else
		westopen = false;
	
	if(cminx < g_pathdim.x-1)
	{
		const int offcmminx = cmminx + PATHNODE_SIZE;
		const int offcmminz = cmminz;
		const int offcmmaxx = cmmaxx + PATHNODE_SIZE;
		const int offcmmaxz = cmmaxz;

		for(int z=cminz; z<=cmaxz; z++)
		{
			if(NodeWalkable(cx+1, z,
				cminx-1, cminz, cmaxx+1, cmaxz,
				offcmminx, offcmminz, offcmmaxx, offcmmaxz,
				roaded, landborne, seaborne, airborne, 
				thisu, ignoreu, ignoreb) != COLLIDER_NONE)
			{
				eastopen = false;
				break;
			}
		}
	}
	else
		eastopen = false;

	if(cminz > 0)
	{
		const int offcmminx = cmminx;
		const int offcmminz = cmminz - PATHNODE_SIZE;
		const int offcmmaxx = cmmaxx;
		const int offcmmaxz = cmmaxz - PATHNODE_SIZE;

		for(int x=cminx; x<=cmaxx; x++)
		{
			if(NodeWalkable(x, cz-1,
				cminx, cminz-1, cmaxx, cmaxz-1,
				offcmminx, offcmminz, offcmmaxx, offcmmaxz,
				roaded, landborne, seaborne, airborne, 
				thisu, ignoreu, ignoreb) != COLLIDER_NONE)
			{
				northopen = false;
				break;
			}
		}
	}
	else
		northopen = false;
	
	if(cminz < g_pathdim.y-1)
	{
		const int offcmminx = cmminx;
		const int offcmminz = cmminz + PATHNODE_SIZE;
		const int offcmmaxx = cmmaxx;
		const int offcmmaxz = cmmaxz + PATHNODE_SIZE;
		
		for(int x=cminx; x<=cmaxx; x++)
		{
			if(NodeWalkable(x, cz+1,
				cminx, cminz+1, cmaxx, cmaxz+1,
				offcmminx, offcmminz, offcmmaxx, offcmmaxz,
				roaded, landborne, seaborne, airborne, 
				thisu, ignoreu, ignoreb) != COLLIDER_NONE)
			{
				southopen = false;
				break;
			}
		}
	}
	else
		southopen = false;

	if(cminx > 0 && cminz > 0)
	{
		const int offcmminx = cmminx - PATHNODE_SIZE;
		const int offcmminz = cmminz - PATHNODE_SIZE;
		const int offcmmaxx = cmmaxx - PATHNODE_SIZE;
		const int offcmmaxz = cmmaxz - PATHNODE_SIZE;

		if(NodeWalkable(cx-1, cz-1,
			cminx-1, cminz-1, cmaxx-1, cmaxz-1,
			offcmminx, offcmminz, offcmmaxx, offcmmaxz,
			roaded, landborne, seaborne, airborne, 
			thisu, ignoreu, ignoreb) != COLLIDER_NONE)
		{
			nwcorneropen = false;
		}
	}
	else
		nwcorneropen = false;

	if(cminx < g_pathdim.x-1 && cminz > 0)
	{
		const int offcmminx = cmminx + PATHNODE_SIZE;
		const int offcmminz = cmminz - PATHNODE_SIZE;
		const int offcmmaxx = cmmaxx + PATHNODE_SIZE;
		const int offcmmaxz = cmmaxz - PATHNODE_SIZE;

		if(NodeWalkable(cx+1, cz-1,
			cminx+1, cminz-1, cmaxx+1, cmaxz-1,
			offcmminx, offcmminz, offcmmaxx, offcmmaxz,
			roaded, landborne, seaborne, airborne, 
			thisu, ignoreu, ignoreb) != COLLIDER_NONE)
		{
			necorneropen = false;
		}
	}
	else
		necorneropen = false;

	if(cminx > 0 && cminz < g_pathdim.y-1)
	{
		const int offcmminx = cmminx - PATHNODE_SIZE;
		const int offcmminz = cmminz + PATHNODE_SIZE;
		const int offcmmaxx = cmmaxx - PATHNODE_SIZE;
		const int offcmmaxz = cmmaxz + PATHNODE_SIZE;

		if(NodeWalkable(cx-1, cz+1,
			cminx-1, cminz+1, cmaxx-1, cmaxz+1,
			offcmminx, offcmminz, offcmmaxx, offcmmaxz,
			roaded, landborne, seaborne, airborne, 
			thisu, ignoreu, ignoreb) != COLLIDER_NONE)
		{
			swcorneropen = false;
		}
	}
	else
		swcorneropen = false;
	

	if(cminx < g_pathdim.x-1 && cminz < g_pathdim.y-1)
	{
		const int offcmminx = cmminx + PATHNODE_SIZE;
		const int offcmminz = cmminz + PATHNODE_SIZE;
		const int offcmmaxx = cmmaxx + PATHNODE_SIZE;
		const int offcmmaxz = cmmaxz + PATHNODE_SIZE;

		if(NodeWalkable(cx+1, cz+1,
			cminx+1, cminz+1, cmaxx+1, cmaxz+1,
			offcmminx, offcmminz, offcmmaxx, offcmmaxz,
			roaded, landborne, seaborne, airborne, 
			thisu, ignoreu, ignoreb) != COLLIDER_NONE)
		{
			secorneropen = false;
		}
	}
	else
		secorneropen = false;

	bool atwestedge = false;
	bool ateastedge = false;
	bool atsouthedge = false;
	bool atnorthedge = false;
	
	if(cminx-1 < 0) atwestedge = true;
	if(cmaxx+1 >= g_pathdim.x) ateastedge = true;
	if(cminz-1 < 0) atnorthedge = true;
	if(cmaxz+1 >= g_pathdim.y) atsouthedge = true;

	if(northopen && ( (!nwcorneropen && !atwestedge) || (!necorneropen && !ateastedge) ) )
	{
		g_log<<"forced neib "<<cx<<","<<cz<<" by north"<<endl;
		g_log.flush();
		return true;
	}

	if(southopen && ( (!swcorneropen && !atwestedge) || (!secorneropen && !ateastedge) ) )
	{
		g_log<<"forced neib "<<cx<<","<<cz<<" by south"<<endl;
		g_log.flush();
		return true;
	}

	if(westopen && ( (!nwcorneropen && !atnorthedge) || (!swcorneropen && !atsouthedge) ) )
	{
		g_log<<"forced neib "<<cx<<","<<cz<<" by west"<<endl;
		g_log.flush();
		return true;
	}

	if(eastopen && ( (!necorneropen && !atnorthedge) || (!secorneropen && !atsouthedge) ) )
	{
		g_log<<"forced neib "<<cx<<","<<cz<<" by east"<<endl;
		g_log.flush();
		return true;
	}

	return false;
}

int NodeWalkable(const int cx,
					const int cz,
					const int cminx,
					const int cminz,
					const int cmaxx,
					const int cmaxz,
					const int cmminx,
					const int cmminz,
					const int cmmaxx,
					const int cmmaxz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb)
{
	ColliderTile* cell = ColliderTileAt(cx, cz);

	if(cell->building >= 0 && &g_building[ cell->building ] != ignoreb)
	{
		Building* b = &g_building[cell->building];
		BuildingT* t2 = &g_buildingT[b->type];

		int tminx = b->tilepos.x - t2->widthx/2;
		int tminz = b->tilepos.y - t2->widthz/2;
		int tmaxx = tminx + t2->widthx;
		int tmaxz = tminz + t2->widthz;

		int minx2 = tminx*TILE_SIZE;
		int minz2 = tminz*TILE_SIZE;
		int maxx2 = tmaxx*TILE_SIZE;
		int maxz2 = tmaxz*TILE_SIZE;

		if(cmminx < maxx2 && cmminz < maxz2 && cmmaxx > minx2 && cmmaxz > minz2)
			return COLLIDER_BUILDING;
	}

	for(auto uiter = cell->units.begin(); uiter != cell->units.end(); uiter++)
	{
		Unit* u = &g_unit[*uiter];

		if(u != thisu && u != ignoreu && !u->hidden())
		{
#if 1
			UnitT* t = &g_unitT[u->type];

			int cmminx2 = u->cmpos.x - t->size.x/2;
			int cmminz2 = u->cmpos.y - t->size.z/2;
			int cmmaxx2 = cmminx2 + t->size.x;
			int cmmaxz2 = cmminz2 + t->size.z;

			if(cmmaxx > cmminx2 && cmmaxz > cmminz2 && cmminx < cmmaxx2 && cmminz < cmmaxz2)
			{

				return COLLIDER_UNIT;
			}
#else
			return COLLIDER_UNIT;
#endif
		}
	}

	return COLLIDER_NONE;
}

int NodeBlockWalkable(const int cx,
					const int cz,
					const int cminx,
					const int cminz,
					const int cmaxx,
					const int cmaxz,
					const int cmminx,
					const int cmminz,
					const int cmmaxx,
					const int cmmaxz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb)
{
	ColliderTile* cell = ColliderTileAt( cx, cz );

	if(roaded && !cell->hasroad)
	{
		return COLLIDER_OTHER;
	}

	if(landborne && !cell->hasland)
	{
		return COLLIDER_OTHER;
	}

	if(seaborne && !cell->haswater)
	{
		return COLLIDER_OTHER;
	}

	if(cminx < 0 || cminz < 0 || cmaxx >= g_pathdim.x || cmaxz >= g_pathdim.y)
	{
		return COLLIDER_OTHER;
	}

	for(int x=cminx; x<=cmaxx; x++)
		for(int z=cminz; z<=cmaxz; z++)
		{
			cell = ColliderTileAt(x, z);

			if(cell->building >= 0 && &g_building[ cell->building ] != ignoreb)
			{
				Building* b = &g_building[cell->building];
				BuildingT* t2 = &g_buildingT[b->type];

				int tminx = b->tilepos.x - t2->widthx/2;
				int tminz = b->tilepos.y - t2->widthz/2;
				int tmaxx = tminx + t2->widthx;
				int tmaxz = tminz + t2->widthz;
				
				int minx2 = tminx*TILE_SIZE;
				int minz2 = tminz*TILE_SIZE;
				int maxx2 = tmaxx*TILE_SIZE;
				int maxz2 = tmaxz*TILE_SIZE;
				
				if(cmminx < maxx2 && cmminz < maxz2 && cmmaxx > minx2 && cmmaxz > minz2)
					return COLLIDER_BUILDING;
			}

			for(auto uiter = cell->units.begin(); uiter != cell->units.end(); uiter++)
			{
				Unit* u = &g_unit[*uiter];

				if(u != thisu && u != ignoreu && !u->hidden())
				{
#if 1
					UnitT* t = &g_unitT[u->type];
					
					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminz2 = u->cmpos.y - t->size.z/2;
					int cmmaxx2 = cmminx2 + t->size.x;
					int cmmaxz2 = cmminz2 + t->size.z;

					if(cmmaxx > cmminx2 && cmmaxz > cmminz2 && cmminx < cmmaxx2 && cmminz < cmmaxz2)
					{

						return COLLIDER_UNIT;
					}
#else
					return COLLIDER_UNIT;
#endif
				}
			}
		}

	return COLLIDER_NONE;
}