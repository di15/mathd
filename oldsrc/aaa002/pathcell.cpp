

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
#include "trystep.h"
#include "../math/vec2i.h"
#include "pathdebug.h"

ColliderCell *g_collidercell = NULL;
Vec2i g_pathdim(0,0);

ColliderCell::ColliderCell()
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

inline Vec2i PathCellPos(int cmposx, int cmposz)
{
	return Vec2i(cmposx/PATHCELL_SIZE, cmposz/PATHCELL_SIZE);
}

inline unsigned int PathCellIndex(int cx, int cz)
{
	return cz * g_pathdim.x + cx;
}

void FreePathCells()
{
	if(g_collidercell)
	{
		delete [] g_collidercell;
		g_collidercell = NULL;
	}

	g_pathdim = Vec2i(0,0);

	for(int i=0; i<WORKTHREADS; i++)
		g_workthread[i].destroy();
}

void AllocPathCells(int cmwx, int cmwz)
{
	FreePathCells();
	g_pathdim.x = cmwx / PATHCELL_SIZE;
	g_pathdim.y = cmwz / PATHCELL_SIZE;
	g_collidercell = new ColliderCell [ g_pathdim.x * g_pathdim.y ];

	for(int i=0; i<WORKTHREADS; i++)
	{
		g_workthread[i].alloc(g_pathdim.x, g_pathdim.y);
	}
}

ColliderCell* ColliderCellAt(int cx, int cz)
{
	return &g_collidercell[ PathCellIndex(cx, cz) ];
}

void FillPathCells()
{
	const int cwx = g_pathdim.x;
	const int cwz = g_pathdim.y;

	for(int x=0; x<cwx; x++)
		for(int z=0; z<cwz; z++)
		{
			int cmx = x*PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmz = z*PATHCELL_SIZE + PATHCELL_SIZE/2;
			ColliderCell* cell = ColliderCellAt(x, z);
            
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
	int cminx = cmminx / PATHCELL_SIZE;
	int cminz = cmminz / PATHCELL_SIZE;
	int cmaxx = cmmaxx / PATHCELL_SIZE;
	int cmaxz = cmmaxz / PATHCELL_SIZE;

	for(int cx = cminx; cx <= cmaxx; cx++)
		for(int cz = cminz; cz <= cmaxz; cz++)
		{
			ColliderCell* c = ColliderCellAt(cx, cz);
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
	int cminx = cmminx / PATHCELL_SIZE;
	int cminz = cmminz / PATHCELL_SIZE;
	int cmaxx = cmmaxx / PATHCELL_SIZE;
	int cmaxz = cmmaxz / PATHCELL_SIZE;

	for(int cx = cminx; cx <= cmaxx; cx++)
		for(int cz = cminz; cz <= cmaxz; cz++)
		{
			ColliderCell* c = ColliderCellAt(cx, cz);
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
	int cminx = cmminx / PATHCELL_SIZE;
	int cminz = cmminz / PATHCELL_SIZE;
	int cmaxx = cmmaxx / PATHCELL_SIZE;
	int cmaxz = cmmaxz / PATHCELL_SIZE;

	for(int cx = cminx; cx <= cmaxx; cx++)
		for(int cz = cminz; cz <= cmaxz; cz++)
		{
			ColliderCell* c = ColliderCellAt(cx, cz);

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
	int cminx = cmminx / PATHCELL_SIZE;
	int cminz = cmminz / PATHCELL_SIZE;
	int cmaxx = cmmaxx / PATHCELL_SIZE;
	int cmaxz = cmmaxz / PATHCELL_SIZE;

	for(int cx = cminx; cx <= cmaxx; cx++)
		for(int cz = cminz; cz <= cmaxz; cz++)
		{
			ColliderCell* c = ColliderCellAt(cx, cz);

			if(c->building == bi)
				c->building = -1;
		}
}

bool CellForcedNeighbour(const int cx,
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
		const int offcmminx = cmminx - PATHCELL_SIZE;
		const int offcmminz = cmminz;
		const int offcmmaxx = cmmaxx - PATHCELL_SIZE;
		const int offcmmaxz = cmmaxz;

		for(int z=cminz; z<=cmaxz; z++)
		{
			if(CellOpen(cx-1, z,
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
		const int offcmminx = cmminx + PATHCELL_SIZE;
		const int offcmminz = cmminz;
		const int offcmmaxx = cmmaxx + PATHCELL_SIZE;
		const int offcmmaxz = cmmaxz;

		for(int z=cminz; z<=cmaxz; z++)
		{
			if(CellOpen(cx+1, z,
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
		const int offcmminz = cmminz - PATHCELL_SIZE;
		const int offcmmaxx = cmmaxx;
		const int offcmmaxz = cmmaxz - PATHCELL_SIZE;

		for(int x=cminx; x<=cmaxx; x++)
		{
			if(CellOpen(x, cz-1,
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
		const int offcmminz = cmminz + PATHCELL_SIZE;
		const int offcmmaxx = cmmaxx;
		const int offcmmaxz = cmmaxz + PATHCELL_SIZE;
		
		for(int x=cminx; x<=cmaxx; x++)
		{
			if(CellOpen(x, cz+1,
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
		const int offcmminx = cmminx - PATHCELL_SIZE;
		const int offcmminz = cmminz - PATHCELL_SIZE;
		const int offcmmaxx = cmmaxx - PATHCELL_SIZE;
		const int offcmmaxz = cmmaxz - PATHCELL_SIZE;

		if(CellOpen(cx-1, cz-1,
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
		const int offcmminx = cmminx + PATHCELL_SIZE;
		const int offcmminz = cmminz - PATHCELL_SIZE;
		const int offcmmaxx = cmmaxx + PATHCELL_SIZE;
		const int offcmmaxz = cmmaxz - PATHCELL_SIZE;

		if(CellOpen(cx+1, cz-1,
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
		const int offcmminx = cmminx - PATHCELL_SIZE;
		const int offcmminz = cmminz + PATHCELL_SIZE;
		const int offcmmaxx = cmmaxx - PATHCELL_SIZE;
		const int offcmmaxz = cmmaxz + PATHCELL_SIZE;

		if(CellOpen(cx-1, cz+1,
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
		const int offcmminx = cmminx + PATHCELL_SIZE;
		const int offcmminz = cmminz + PATHCELL_SIZE;
		const int offcmmaxx = cmmaxx + PATHCELL_SIZE;
		const int offcmmaxz = cmmaxz + PATHCELL_SIZE;

		if(CellOpen(cx+1, cz+1,
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

	if(northopen && ( !nwcorneropen || !necorneropen ) )
		return true;

	if(southopen && ( !swcorneropen || !secorneropen ) )
		return true;

	if(westopen && ( !nwcorneropen || !swcorneropen ) )
		return true;

	if(eastopen && ( !necorneropen || !secorneropen ) )
		return true;

	return false;
}

int CellOpen(const int cx,
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
	ColliderCell* cell = ColliderCellAt(cx, cz);

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

			if(thisu == g_pathunit)
			{
				g_log<<"\t\t============="<<endl;
				g_log<<"\t\tcollide with? "<<*uiter<<" at cell "<<cx<<","<<cz<<endl;
				g_log.flush();

				//int cmposx = (cmminx2+cmmaxx2)/2;
				//int cmposz = (cmminz2+cmmaxz2)/2;
				int cmposx = u->cmpos.x;
				int cmposz = u->cmpos.y;

				g_log<<"\t\tcm pos?: ("<<cmposx<<","<<cmposz<<")"<<endl;
				g_log<<"\t\tcm rect: ("<<cmminx2<<","<<cmminz2<<")->("<<cmmaxx2<<","<<cmmaxz2<<")"<<endl;
				//g_log<<"\t\tpathcell rect: ("<<cminx<<","<<cminz<<")->("<<cmaxx<<","<<cmaxz<<")"<<endl;
				g_log<<"\t\tintersection pathcell pos: ("<<cx<<","<<cz<<")"<<endl;
				g_log.flush();
			}

			if(cmmaxx > cmminx2 && cmmaxz > cmminz2 && cmminx < cmmaxx2 && cmminz < cmmaxz2)
			{
#if 0
				g_log<<"collides with u"<<endl;
#endif


				if(thisu == g_pathunit)
				{
					g_log<<"\t\tyes collision"<<endl;
					g_log<<"\t\t============="<<endl;
					g_log.flush();
				}

				return COLLIDER_UNIT;
			}
#else
			return COLLIDER_UNIT;
#endif

			if(thisu == g_pathunit)
			{
				g_log<<"\t\tno collision"<<endl;
				g_log<<"\t\t============="<<endl;
				g_log.flush();
			}
		}
	}

	return COLLIDER_NONE;
}

int CellBlockOpen(const int cx,
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
	ColliderCell* cell = ColliderCellAt( cx, cz );

	if(thisu == g_pathunit)
	{
		g_log<<"\t============================================"<<endl;
		g_log<<"\tcell open? "<<cx<<","<<cz<<endl;
		g_log.flush();
		
		int cmposx = (cmminx+cmmaxx)/2;
		int cmposz = (cmminz+cmmaxz)/2;

		g_log<<"\tcm pos?: ("<<cmposx<<","<<cmposz<<")"<<endl;
		g_log<<"\tcm rect: ("<<cmminx<<","<<cmminz<<")->("<<cmmaxx<<","<<cmmaxz<<")"<<endl;
		g_log<<"\tpathcell rect: ("<<cminx<<","<<cminz<<")->("<<cmaxx<<","<<cmaxz<<")"<<endl;
		g_log<<"\tpathcell pos: ("<<cx<<","<<cz<<")"<<endl;
		g_log.flush();
	}

#if 0
		g_log<<"cellopen? "<<bestS->cx<<","<<bestS->cz<<endl;
		g_log.flush();
#endif

	if(roaded && !cell->hasroad)
	{
#if 0
		g_log<<"no road"<<endl;
#endif
		return COLLIDER_OTHER;
	}

	if(landborne && !cell->hasland)
	{
#if 0
		g_log<<"no land"<<endl;
#endif
		return COLLIDER_OTHER;
	}

	if(seaborne && !cell->haswater)
	{
#if 0
		g_log<<"no water"<<endl;
#endif
		return COLLIDER_OTHER;
	}

	if(cminx < 0 || cminz < 0 || cmaxx >= g_pathdim.x || cmaxz >= g_pathdim.y)
	{
#if 0
		g_log<<"off map "<<cminx<<","<<cminz<<"->"<<cmaxx<<","<<cmaxz<<" ("<<g_pathdim.x<<","<<g_pathdim.y<<")"<<endl;
#endif
		return COLLIDER_OTHER;
	}

	for(int x=cminx; x<=cmaxx; x++)
		for(int z=cminz; z<=cmaxz; z++)
		{
			cell = ColliderCellAt(x, z);

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

					if(thisu == g_pathunit)
					{
						g_log<<"\t\t============="<<endl;
						g_log<<"\t\tcollide with? "<<*uiter<<" at cell "<<x<<","<<z<<endl;
						g_log.flush();
		
						//int cmposx = (cmminx2+cmmaxx2)/2;
						//int cmposz = (cmminz2+cmmaxz2)/2;
						int cmposx = u->cmpos.x;
						int cmposz = u->cmpos.y;

						g_log<<"\t\tcm pos?: ("<<cmposx<<","<<cmposz<<")"<<endl;
						g_log<<"\t\tcm rect: ("<<cmminx2<<","<<cmminz2<<")->("<<cmmaxx2<<","<<cmmaxz2<<")"<<endl;
						//g_log<<"\t\tpathcell rect: ("<<cminx<<","<<cminz<<")->("<<cmaxx<<","<<cmaxz<<")"<<endl;
						g_log<<"\t\tintersection pathcell pos: ("<<x<<","<<z<<")"<<endl;
						g_log<<"\t\tdminmax: ("<<(cmmaxx-cmminx2)<<","<<(cmmaxz-cmminz2)<<","<<(cmmaxx2-cmminx)<<","<<(cmmaxz2-cmminz)<<") all positive=collision"<<endl;
						g_log.flush();
					}

					if(cmmaxx > cmminx2 && cmmaxz > cmminz2 && cmminx < cmmaxx2 && cmminz < cmmaxz2)
					{
#if 0
						g_log<<"collides with u"<<endl;
#endif
						

						if(thisu == g_pathunit)
						{
							g_log<<"\t\tyes collision"<<endl;
							g_log<<"\t\t============="<<endl;
							g_log.flush();
						}

						return COLLIDER_UNIT;
					}
#else
					return COLLIDER_UNIT;
#endif

					if(thisu == g_pathunit)
					{
						g_log<<"\t\tno collision"<<endl;
						g_log<<"\t\t============="<<endl;
						g_log.flush();
					}
				}
			}
		}

	return COLLIDER_NONE;
}