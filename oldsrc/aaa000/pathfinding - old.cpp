

#include "pathfinding.h"
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
#if 0
	int cmaxx = ceilf( cmmaxx / PATHCELL_SIZE );
	int cmaxz = ceilf( cmmaxz / PATHCELL_SIZE );
#elif 0
	// faster, this will repeatedly be used in pathfinding
	int cmaxx = cmmaxx / PATHCELL_SIZE + 1;
	int cmaxz = cmmaxz / PATHCELL_SIZE + 1;
#else
	// actually, that isn't even necessary
	int cmaxx = cmmaxx / PATHCELL_SIZE;
	int cmaxz = cmmaxz / PATHCELL_SIZE;
#endif
	
#if 0
	cminx = min(0, cminx-1);
	cminz = min(0, cminz-1);
#endif

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
	int tmaxx = tminx + t->widthx - 1;
	int tmaxz = tminz + t->widthz - 1;

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
	int tmaxx = tminx + t->widthx - 1;
	int tmaxz = tminz + t->widthz - 1;

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

TryStep::TryStep(int startx, int startz, int endx, int endz, int cx, int cz, TryStep* prev, int totalD, int stepD)
{
	this->cx = cx;
	this->cz = cz;
	int G = totalD + stepD;
	runningD = G;
	//float G = Magnitude2(Vec3f(startx-x,0,startz-z));
	float H = Magnitude(Vec2i(endx-cx,endz-cz));
	//int G = abs(startx-x) + abs(startz-z);
	//int H = abs(endx-x) + abs(endz-z);
	F = G + H;
	//F = H;
	previous = prev;
	//tried = false;
}


Unit* g_pathunit = NULL;

inline int CellOpen(const int cx,
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
				return false;

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

bool FindPath(int wthread, int utype, int umode, int cmstartx, int cmstartz, int target, int targetx, int targetz, 
			  list<Vec2i> &path, Vec2i &subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb, 
			  int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminz, int cmgoalmaxx, int cmgoalmaxz)
{
	WorkThread* wt = &g_workthread[wthread];
	wt->trystep.clear();
	wt->pathcnt++;

	UnitT* ut = &g_unitT[utype];

	int cstartx = cmstartx / PATHCELL_SIZE;
	int cstartz = cmstartz / PATHCELL_SIZE;
	int cgoalx = (cmgoalminx + cmgoalmaxx) / 2 / PATHCELL_SIZE;
	int cgoalz = (cmgoalminz + cmgoalmaxz) / 2 / PATHCELL_SIZE;
	
	int cmstartminx = cmstartx - ut->size.x/2;
	int cmstartminz = cmstartz - ut->size.z/2;
	int cmstartmaxx = cmstartminx + ut->size.x;
	int cmstartmaxz = cmstartminz + ut->size.z;

	int cstartminx = cmstartminx / PATHCELL_SIZE;
	int cstartminz = cmstartminz / PATHCELL_SIZE;
	int cstartmaxx = cmstartmaxx / PATHCELL_SIZE;
	int cstartmaxz = cmstartmaxz / PATHCELL_SIZE;

#if 0
	wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cstartx, cstartz, NULL, 0, 0));
#endif
	int runningD = 0;
	const int cmdiag = PATHCELL_DIAG;

	if(thisu == g_pathunit)
	{
		g_log<<"============================================"<<endl;
		g_log<<"unit "<<(int)(thisu-g_unit)<<" pathing frame "<<g_simframe<<endl;
		g_log.flush();

		
		//int cmposx = cgoalx * PATHCELL_SIZE;
		//int cmposz = cgoalz * PATHCELL_SIZE;
		int cmposx = cmstartx;
		int cmposz = cmstartz;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHCELL_SIZE;
		int cminz = cmminz/PATHCELL_SIZE;
		int cmaxx = cmmaxx/PATHCELL_SIZE;
		int cmaxz = cmmaxz/PATHCELL_SIZE;

		int cposx = cmstartx / PATHCELL_SIZE;
		int cposz = cmstartz / PATHCELL_SIZE;
		
		
		g_log<<"cm pos: ("<<cmposx<<","<<cmposz<<")"<<endl;
		g_log<<"cm rect: ("<<cmminx<<","<<cmminz<<")->("<<cmmaxx<<","<<cmmaxz<<")"<<endl;
		g_log<<"pathcell rect: ("<<cminx<<","<<cminz<<")->("<<cmaxx<<","<<cmaxz<<")"<<endl;
		g_log<<"pathcell pos: ("<<cposx<<","<<cposz<<")"<<endl;
		g_log<<"--------------------------------------------"<<endl;
		g_log.flush();
	}

	bool startcenterblocked = true;
	
	// Check if starting centered position is blocked.
	// If not, add it as one of the first steps (with .previous = NULL).
	{
		//Cenetered position
		int cposx = cstartx;
		int cposz = cstartz;

		int cmposx = cstartx * PATHCELL_SIZE + PATHCELL_SIZE/2;
		int cmposz = cstartz * PATHCELL_SIZE + PATHCELL_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHCELL_SIZE;
		int cminz = cmminz/PATHCELL_SIZE;
		int cmaxx = cmmaxx/PATHCELL_SIZE;
		int cmaxz = cmmaxz/PATHCELL_SIZE;

#if 1
		//Midway between centered and starting position
		int cmposx2 = (cmposx + cmstartx) / 2;
		int cmposz2 = (cmposz + cmstartz) / 2;

		int cmminx2 = cmposx2 - ut->size.x/2;
		int cmminz2 = cmposz2 - ut->size.z/2;
		int cmmaxx2 = cmminx2 + ut->size.x;
		int cmmaxz2 = cmminz2 + ut->size.z;

		int cminx2 = cmminx2/PATHCELL_SIZE;
		int cminz2 = cmminz2/PATHCELL_SIZE;
		int cmaxx2 = cmmaxx2/PATHCELL_SIZE;
		int cmaxz2 = cmmaxz2/PATHCELL_SIZE;
#endif

#if 0
#if 1
		if(CellOpen(cposx, cposz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE
#if 1
					&& 
				CellOpen(cposx, cposz,
					cminx2, cminz2, cmaxx2, cmaxz2,
					cmminx2, cmminz2, cmmaxx2, cmmaxz2,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE
#endif
					)
		{
			startcenterblocked = false;
			wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
		}
#endif
#endif
	}

	if(startcenterblocked)
	{
		if( cstartx > 0 && cstartz > 0 && cstartx - 1 >= cstartminx && cstartz - 1 >= cstartminz)
		{
#if 0
			int cposx = cstartx - 1;
			int cposz = cstartz - 1;

			int cmposx = (cposx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
			int cmposz = (cposz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#elif 0
	
			int cposx = cstartx - 1;
			int cposz = cstartz - 1;

			int cmposx = cposx * PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmposz = cposz * PATHCELL_SIZE + PATHCELL_SIZE/2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(Trace(utype, umode, Vec2i(cmstartx, cmstartz), Vec2i(cmposx, cmposz), thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#else
			int cposx = cstartx - 1;
			int cposz = cstartz - 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2f fdir = Vec2f(dir.x, dir.y);
			Vec2f fdirs = Normalize(fdir) * ut->cmspeed;
			Vec2i stepto = from + Vec2i(fdirs.x, fdirs.y);
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#endif
		}

		if( cstartz > 0 && cstartz - 1 >= cstartminz)
		{
#if 0
			int cposx = cstartx;
			int cposz = cstartz - 1;

			int cmposx = (cposx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
			int cmposz = (cposz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#elif 0
	
			int cposx = cstartx;
			int cposz = cstartz - 1;

			int cmposx = cposx * PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmposz = cposz * PATHCELL_SIZE + PATHCELL_SIZE/2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(Trace(utype, umode, Vec2i(cmstartx, cmstartz), Vec2i(cmposx, cmposz), thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#else
			int cposx = cstartx;
			int cposz = cstartz - 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2f fdir = Vec2f(dir.x, dir.y);
			Vec2f fdirs = Normalize(fdir) * ut->cmspeed;
			Vec2i stepto = from + Vec2i(fdirs.x, fdirs.y);
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#endif
		}

#if 1
		if( cstartx < g_pathdim.x - 1 && cstartz > 0 && cstartx + 1 <= cstartmaxx && cstartz - 1 >= cstartminz )
		{
#if 0
			int cposx = cstartx + 1;
			int cposz = cstartz - 1;

			int cmposx = (cposx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
			int cmposz = (cposz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#elif 0
	
			int cposx = cstartx + 1;
			int cposz = cstartz - 1;

			int cmposx = cposx * PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmposz = cposz * PATHCELL_SIZE + PATHCELL_SIZE/2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(Trace(utype, umode, Vec2i(cmstartx, cmstartz), Vec2i(cmposx, cmposz), thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#else
			int cposx = cstartx + 1;
			int cposz = cstartz - 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2f fdir = Vec2f(dir.x, dir.y);
			Vec2f fdirs = Normalize(fdir) * ut->cmspeed;
			Vec2i stepto = from + Vec2i(fdirs.x, fdirs.y);
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#endif
		}
#endif
		
		if( cstartx > 0 && cstartx - 1 >= cstartminx )
		{
#if 0
			int cposx = cstartx - 1;
			int cposz = cstartz;

			int cmposx = (cposx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
			int cmposz = (cposz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#elif 0
			
			int cposx = cstartx - 1;
			int cposz = cstartz;

			int cmposx = cposx * PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmposz = cposz * PATHCELL_SIZE + PATHCELL_SIZE/2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(Trace(utype, umode, Vec2i(cmstartx, cmstartz), Vec2i(cmposx, cmposz), thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#else
			int cposx = cstartx - 1;
			int cposz = cstartz;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2f fdir = Vec2f(dir.x, dir.y);
			Vec2f fdirs = Normalize(fdir) * ut->cmspeed;
			Vec2i stepto = from + Vec2i(fdirs.x, fdirs.y);
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#endif
		}
		
		if( cstartx < g_pathdim.x - 1 && cstartx + 1 <= cstartmaxx )
		{
#if 0
			int cposx = cstartx + 1;
			int cposz = cstartz;

			int cmposx = (cposx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
			int cmposz = (cposz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#elif 0
			
			int cposx = cstartx + 1;
			int cposz = cstartz;

			int cmposx = cposx * PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmposz = cposz * PATHCELL_SIZE + PATHCELL_SIZE/2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(Trace(utype, umode, Vec2i(cmstartx, cmstartz), Vec2i(cmposx, cmposz), thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#else
			int cposx = cstartx + 1;
			int cposz = cstartz;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2f fdir = Vec2f(dir.x, dir.y);
			Vec2f fdirs = Normalize(fdir) * ut->cmspeed;
			Vec2i stepto = from + Vec2i(fdirs.x, fdirs.y);
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#endif
		}
		
#if 1
		if( cstartx > 0 && cstartz < g_pathdim.y - 1 && cstartx - 1 >= cstartminx && cstartz + 1 <= cstartmaxz )
		{
#if 0
			int cposx = cstartx - 1;
			int cposz = cstartz + 1;

			int cmposx = (cposx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
			int cmposz = (cposz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#elif 0

			int cposx = cstartx - 1;
			int cposz = cstartz + 1;

			int cmposx = cposx * PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmposz = cposz * PATHCELL_SIZE + PATHCELL_SIZE/2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(Trace(utype, umode, Vec2i(cmstartx, cmstartz), Vec2i(cmposx, cmposz), thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#else
			int cposx = cstartx - 1;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2f fdir = Vec2f(dir.x, dir.y);
			Vec2f fdirs = Normalize(fdir) * ut->cmspeed;
			Vec2i stepto = from + Vec2i(fdirs.x, fdirs.y);
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#endif
		}
#endif

		if( cstartz < g_pathdim.y && cstartz + 1 <= cstartmaxz)
		{
#if 0
			int cposx = cstartx;
			int cposz = cstartz + 1;

			int cmposx = (cposx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
			int cmposz = (cposz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#elif 0
	
			int cposx = cstartx;
			int cposz = cstartz + 1;

			int cmposx = cposx * PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmposz = cposz * PATHCELL_SIZE + PATHCELL_SIZE/2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(Trace(utype, umode, Vec2i(cmstartx, cmstartz), Vec2i(cmposx, cmposz), thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#else
			int cposx = cstartx;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2f fdir = Vec2f(dir.x, dir.y);
			Vec2f fdirs = Normalize(fdir) * ut->cmspeed;
			Vec2i stepto = from + Vec2i(fdirs.x, fdirs.y);
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#endif
		}
		
#if 1
		if( cstartx < g_pathdim.x - 1 && cstartz < g_pathdim.y && cstartx - 1 >= cstartminx && cstartz + 1 <= cstartmaxz )
		{
#if 0
			int cposx = cstartx + 1;
			int cposz = cstartz + 1;

			int cmposx = (cposx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
			int cmposz = (cposz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#elif 0
	
			int cposx = cstartx + 1;
			int cposz = cstartz + 1;

			int cmposx = cposx * PATHCELL_SIZE + PATHCELL_SIZE/2;
			int cmposz = cposz * PATHCELL_SIZE + PATHCELL_SIZE/2;

			int cmminx = cmposx - ut->size.x/2;
			int cmminz = cmposz - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;

			if(Trace(utype, umode, Vec2i(cmstartx, cmstartz), Vec2i(cmposx, cmposz), thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#else
			int cposx = cstartx + 1;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2f fdir = Vec2f(dir.x, dir.y);
			Vec2f fdirs = Normalize(fdir) * ut->cmspeed;
			Vec2i stepto = from + Vec2i(fdirs.x, fdirs.y);
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
#endif
		}
#endif
	}

#if 0
	/*
	 End goal might be area, like the area
	 around the corner of a tile,
	 which might have a building,
	 so goal area might still be reachable if
	 exact goal cell isn't.
	*/

	int cmposx = cgoalx * PATHCELL_SIZE;
	int cmposz = cgoalz * PATHCELL_SIZE;

	int cmminx = cmposx - ut->size.x/2;
	int cmminz = cmposz - ut->size.z/2;
	int cmmaxx = cmminx + ut->size.x;
	int cmmaxz = cmminz + ut->size.z;

	int cminx = cmminx/PATHCELL_SIZE;
	int cminz = cmminz/PATHCELL_SIZE;
	int cmaxx = cmmaxx/PATHCELL_SIZE;
	int cmaxz = cmmaxz/PATHCELL_SIZE;

	TryStep goalS;
	goalS.cx = cgoalx;
	goalS.cz = cgoalz;

	if(!CellOpen(&goalS,
		cminx, cminz, cmaxx, cmaxz,
		cmminx, cmminz, cmmaxx, cmmaxz,
		ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
		thisu, ignoreu, ignoreb))
		return false;
#endif

	TryStep* tS;
	TryStep* bestS;
	Vec2i halfway;
#if 0
		g_log<<"pathfind 2"<<endl;
		g_log.flush();
#endif
	int searchdepth = 0;
	bool firstcenterblocked = false;

	while(true)
	{
		searchdepth ++;

		if(searchdepth > (g_pathdim.x + g_pathdim.y)*3)
			return false;
#if 0
		g_log<<"pathfind 3"<<endl;
		g_log.flush();
#endif
		bestS = NULL;

		for(auto stepiter = wt->trystep.begin(); stepiter != wt->trystep.end(); stepiter++)
		{
			tS = &*stepiter;
			if(wt->triedcell[ PathCellIndex(tS->cx, tS->cz) ] == wt->pathcnt)
				continue;

			if(bestS != NULL && tS->F > bestS->F)
				continue;

			bestS = tS;
		}

		if(!bestS)
			return false;
#if 0
		g_log<<"pathfind 4 "<<bestS->cx<<","<<bestS->cz<<endl;
		g_log.flush();
#endif
		wt->triedcell[ PathCellIndex(bestS->cx, bestS->cz) ] = wt->pathcnt;

		int cmposx = bestS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2;
		int cmposz = bestS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHCELL_SIZE;
		int cminz = cmminz/PATHCELL_SIZE;
		int cmaxx = cmmaxx/PATHCELL_SIZE;
		int cmaxz = cmmaxz/PATHCELL_SIZE;

		if(thisu == g_pathunit)
		{
			g_log<<"try cell "<<endl;
			g_log.flush();

			int cposx = cmstartx / PATHCELL_SIZE;
			int cposz = cmstartz / PATHCELL_SIZE;
		
			g_log<<"cm pos: ("<<cmposx<<","<<cmposz<<")"<<endl;
			g_log<<"cm rect: ("<<cmminx<<","<<cmminz<<")->("<<cmmaxx<<","<<cmmaxz<<")"<<endl;
			g_log<<"pathcell rect: ("<<cminx<<","<<cminz<<")->("<<cmaxx<<","<<cmaxz<<")"<<endl;
			g_log<<"pathcell pos: ("<<bestS->cx<<","<<bestS->cz<<")"<<endl;

			if(bestS->previous)
			{
				g_log<<"\tfrom pathcell pos: ("<<bestS->previous->cx<<","<<bestS->previous->cz<<")"<<endl;
			}
		}

		//if(bestS->previous >= 0 && !TryStep(vBestS, ignoreUnit, ignoreBuilding, roadVehicle))
		//if(bestS->previous && !trystep(bestS, roadVehicle, ignoreBuilding, ignoreUnit))
		if(
#if 1
			bestS->previous && 
#endif
			CellOpen(bestS->cx, bestS->cz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) != COLLIDER_NONE)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"cell closed"<<endl;
				g_log<<"--------------------------------------------"<<endl;
				g_log.flush();
			}

			continue;
		}

		// Arrived at goal area?
		if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"arrived at goal!"<<endl;
				g_log<<"--------------------------------------------"<<endl;
				g_log.flush();
			}

			// Reconstruct the path, following the path steps
			// Keep adding steps as long as it is not the first one (one without a .previous)
			//for(tS = bestS; tS->previous; tS = tS->previous)
			for(tS = bestS; tS; tS = tS->previous)
				path.push_front(Vec2i(tS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2, tS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2));

#if 0
			// If the first centered step isn't block, and the unit isn't exactly at that center,
			// it can still clip on the corner of another unit between it and the center.
			// So check if a half-way point collides.
			if(!firstcenterblocked)
			{
				cmposx = (tS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartx) / 2;
				cmposz = (tS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2 + cmstartz) / 2;

				cmminx = cmposx - ut->size.x/2;
				cmminz = cmposz - ut->size.z/2;
				cmmaxx = cmminx + ut->size.x;
				cmmaxz = cmminz + ut->size.z;

				cminx = cmminx/PATHCELL_SIZE;
				cminz = cmminz/PATHCELL_SIZE;
				cmaxx = cmmaxx/PATHCELL_SIZE;
				cmaxz = cmmaxz/PATHCELL_SIZE;

				if(CellOpen(tS->cx, tS->cz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) != COLLIDER_NONE)
						firstcenterblocked = true;
			}

			// Only add the first centered step if it isn't blocked
			if(!firstcenterblocked)
				path.push_front(Vec2i(tS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2, tS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2));
#endif

			/*
			An infinitely recurring repath can still occur
			if the first centered step is blocked by a corner
			between it and the starting position and thus
			the first step isn't included in the path,
			but may still be required to turn around a corner
			at the second step.

			TO DO: fix above
			*/

#if 1
			path.push_back(Vec2i(cmgoalx, cmgoalz));
#endif

			if(path.size() > 0)
				subgoal = *path.begin();

			return true;
		}

		if(thisu == g_pathunit)
		{
			g_log<<"cell open"<<endl;
			g_log<<"--------------------------------------------"<<endl;
			g_log.flush();
		}

		bool cannorth = true;
		bool cansouth = true;
		bool canwest = true;
		bool caneast = true;
#if 1

#if 0
		cmposx = bestS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2;
		cmposz = bestS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2;

		cmminx = cmposx - ut->size.x/2;
		cmminz = cmposz - ut->size.z/2;
		cmmaxx = cmminx + ut->size.x;
		cmmaxz = cmminz + ut->size.z;

		cminx = cmminx/PATHCELL_SIZE;
		cminz = cmminz/PATHCELL_SIZE;
		cmaxx = cmmaxx/PATHCELL_SIZE;
		cmaxz = cmmaxz/PATHCELL_SIZE;
#endif
		
		/*
		Movement from the starting cell assumes
		the unit is first centered in the starting cell,
		so that it doesn't clip any of the surrounding cells
		with any of its edges.
		If the centered position collides with something,
		we need to check which directions we can move out of the cell.
		*/
		//if(bestS->previous < 0 && !trystep(bestS, roadVehicle, ignoreBuilding, ignoreUnit) && g_colliderType == COLLIDER_UNIT)
		if(!bestS->previous && 
			CellOpen(bestS->cx, bestS->cz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) != COLLIDER_NONE)
		{
			firstcenterblocked = true;

			float xratio = PATHCELL_SIZE / 2.0f / (float)ut->size.x;
			float zratio = PATHCELL_SIZE / 2.0f / (float)ut->size.z;

			for(int x=cminx; x<=cmaxx; x++)
				for(int z=cminz; z<=cmaxz; z++)
				{
					ColliderCell* cell = ColliderCellAt(x, z);

					for(auto uiter = cell->units.begin(); uiter != cell->units.end(); uiter++)
					{
						Unit* u2 = &g_unit[*uiter];

						if(u2 == thisu)
							continue;

						if(u2 == ignoreu)
							continue;

						UnitT* u2t = &g_unitT[u2->type];

						int cmminx2 = u2->cmpos.x - u2t->size.x/2;
						int cmminz2 = u2->cmpos.y - u2t->size.z/2;
						int cmmaxx2 = cmminx2 + u2t->size.x;
						int cmmaxz2 = cmminz2 + u2t->size.z;

						//int extentx = ceil(r/cellwx);
						//int extentz = ceil(r/cellwz);

						int cminx2 = cmminx2 / PATHCELL_SIZE;
						int cminz2 = cmminz2 / PATHCELL_SIZE;
						int cmaxx2 = cmmaxx2 / PATHCELL_SIZE;
						int cmaxz2 = cmmaxz2 / PATHCELL_SIZE;
						//int endx = startx+extentx;
						//int endz = startz+extentz;

#if 0
						for(int x=cminx2; x<=cmaxx2; x++)
							for(int z=cminz2; z<=cmaxz2; z++)
							{
								if(x*xratio == bestS->cx && (z*zratio+1 == bestS->cz || z*zratio-1 == bestS->cz))
								{
									canwest = false;
									caneast = false;
								}

								if(z*zratio == bestS->cz && (x*xratio+1 == bestS->cx || x*xratio-1 == bestS->cx))
								{
									cannorth = false;
									cansouth = false;
								}
							}
#elif 0
						// Intersecting on x-axis
						if(cmmaxx2 > cmminx && cmminx2 < cmmaxx)
						{
							// Other unit on lesser z side?
							if(cmmaxz2 < cmminz)
							{
								cannorth = false;
							}
							// Other unit on greater z side?
							if(cmminz2 > cmmaxz)
							{
								cansouth = false;
							}
						}
						// Intersecting on z-axis
						if(cmmaxz2 > cmminz && cmminz2 < cmmaxz)
						{
							// Other unit on lesser x side?
							if(cmmaxx2 < cmminx)
							{
								canwest = false;
							}
							// Other unit on greater x side?
							if(cmminx2 > cmmaxx)
							{
								caneast = false;
							}
						}
#elif 1
						// Intersecting on x-axis
						if(cmmaxx2 > cmminx && cmminx2 < cmmaxx)
						{
							cannorth = false;
							cansouth = false;

							// Other unit on lesser x side?
							if(cmmaxx2 < cmmaxx)
							{
								canwest = false;
							}
							// Other unit on greater x side?
							if(cmminx2 > cmminx)
							{
								caneast = false;
							}
						}
						// Intersecting on z-axis
						if(cmmaxz2 > cmminz && cmminz2 < cmmaxz)
						{
							canwest = false;
							caneast = false;

							// Other unit on lesser z side?
							if(cmmaxz2 < cmmaxz)
							{
								cannorth = false;
							}
							// Other unit on greater z side?
							if(cmminz2 > cmminz)
							{
								cansouth = false;
							}
						}
#endif
					}
				}
		}
#endif

		if(bestS->previous)
			runningD = bestS->previous->runningD;

		if(bestS->cx > 0 && 
			canwest && 
			wt->triedcell[ PathCellIndex(bestS->cx - 1, bestS->cz) ] != wt->pathcnt)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"\t can go west from "<<bestS->cx<<","<<bestS->cz<<" to "<<(bestS->cx - 1)<<","<<bestS->cz<<endl;
				g_log.flush();
			}

			wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx - 1, bestS->cz, bestS, runningD, PATHCELL_SIZE));
		}
		if(bestS->cx < g_pathdim.x-1 && 
			caneast && 
			wt->triedcell[ PathCellIndex(bestS->cx + 1, bestS->cz) ] != wt->pathcnt)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"\t can go east from "<<bestS->cx<<","<<bestS->cz<<" to "<<(bestS->cx + 1)<<","<<bestS->cz<<endl;
				g_log.flush();
			}

			wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx + 1, bestS->cz, bestS, runningD, PATHCELL_SIZE));
		}
		if(bestS->cz > 0 && 
			cannorth && 
			wt->triedcell[ PathCellIndex(bestS->cx, bestS->cz - 1) ] != wt->pathcnt)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"\t can go north from "<<bestS->cx<<","<<bestS->cz<<" to "<<(bestS->cx)<<","<<(bestS->cz - 1)<<endl;
				g_log.flush();
			}

			wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx, bestS->cz - 1, bestS, runningD, PATHCELL_SIZE));
		}
		if(bestS->cz < g_pathdim.y-1 && 
			cansouth && 
			wt->triedcell[ PathCellIndex(bestS->cx, bestS->cz + 1) ] != wt->pathcnt)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"\t can go north from "<<bestS->cx<<","<<bestS->cz<<" to "<<(bestS->cx)<<","<<(bestS->cz + 1)<<endl;
				g_log.flush();
			}
			wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx, bestS->cz + 1, bestS, runningD, PATHCELL_SIZE));
		}

#if 1
#ifndef NO_DIAGONAL
		//int stepD = cmdiag;

#if 0
		

		int cmposx = bestS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2;
		int cmposz = bestS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHCELL_SIZE;
		int cminz = cmminz/PATHCELL_SIZE;
		int cmaxx = cmmaxx/PATHCELL_SIZE;
		int cmaxz = cmmaxz/PATHCELL_SIZE;

		if(thisu == g_pathunit)
		{
			g_log<<"try cell "<<endl;
			g_log.flush();

			int cposx = cmstartx / PATHCELL_SIZE;
			int cposz = cmstartz / PATHCELL_SIZE;
		
			g_log<<"cm pos: ("<<cmposx<<","<<cmposz<<")"<<endl;
			g_log<<"cm rect: ("<<cmminx<<","<<cmminz<<")->("<<cmmaxx<<","<<cmmaxz<<")"<<endl;
			g_log<<"pathcell rect: ("<<cminx<<","<<cminz<<")->("<<cmaxx<<","<<cmaxz<<")"<<endl;
			g_log<<"pathcell pos: ("<<bestS->cx<<","<<bestS->cz<<")"<<endl;

			if(bestS->previous)
			{
				g_log<<"\tfrom pathcell pos: ("<<bestS->previous->cx<<","<<bestS->previous->cz<<")"<<endl;
			}
		}

			CellOpen(bestS->cx, bestS->cz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) != COLLIDER_NONE)
#endif

		//if(bestS->x > 0 && bestS->z > 0 && canwest && cannorth)
		if(bestS->cx > 0 && 
			bestS->cz > 0 && 
			wt->triedcell[ PathCellIndex(bestS->cx - 1, bestS->cz - 1) ] != wt->pathcnt)
		{
			cmposx = bestS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2 - PATHCELL_SIZE/2;
			cmposz = bestS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2 - PATHCELL_SIZE/2;

			cmminx = cmposx - ut->size.x/2;
			cmminz = cmposz - ut->size.z/2;
			cmmaxx = cmminx + ut->size.x;
			cmmaxz = cmminz + ut->size.z;

			cminx = cmminx/PATHCELL_SIZE;
			cminz = cmminz/PATHCELL_SIZE;
			cmaxx = cmmaxx/PATHCELL_SIZE;
			cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(bestS->cx - 1, bestS->cz - 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx - 1, bestS->cz - 1, bestS, runningD, cmdiag));
		}
		//if(bestS->x < g_pathfindszx-1 && bestS->z > 0 && caneast && cannorth)
		//if(bestS->x < g_pathfindszx-1 && bestS->z > 0)
		if(bestS->cx < g_pathdim.x-1 &&
			bestS->cz > 0 && 
			wt->triedcell[ PathCellIndex(bestS->cx + 1, bestS->cz - 1) ] != wt->pathcnt)
		{
			cmposx = bestS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2 + PATHCELL_SIZE/2;
			cmposz = bestS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2 - PATHCELL_SIZE/2;

			cmminx = cmposx - ut->size.x/2;
			cmminz = cmposz - ut->size.z/2;
			cmmaxx = cmminx + ut->size.x;
			cmmaxz = cmminz + ut->size.z;

			cminx = cmminx/PATHCELL_SIZE;
			cminz = cmminz/PATHCELL_SIZE;
			cmaxx = cmmaxx/PATHCELL_SIZE;
			cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(bestS->cx + 1, bestS->cz - 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx + 1, bestS->cz - 1, bestS, runningD, cmdiag));
		}
		//if(bestS->x > 0 && bestS->z < g_pathfindszz-1 && canwest && cansouth)
		//if(bestS->x > 0 && bestS->z < g_pathfindszz-1)
		if(bestS->cx > 0 && 
			bestS->cz < g_pathdim.y-1 && 
			wt->triedcell[ PathCellIndex(bestS->cx - 1, bestS->cz + 1) ] != wt->pathcnt)
		{
			cmposx = bestS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2 - PATHCELL_SIZE/2;
			cmposz = bestS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2 + PATHCELL_SIZE/2;

			cmminx = cmposx - ut->size.x/2;
			cmminz = cmposz - ut->size.z/2;
			cmmaxx = cmminx + ut->size.x;
			cmmaxz = cmminz + ut->size.z;

			cminx = cmminx/PATHCELL_SIZE;
			cminz = cmminz/PATHCELL_SIZE;
			cmaxx = cmmaxx/PATHCELL_SIZE;
			cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(bestS->cx - 1, bestS->cz + 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx - 1, bestS->cz + 1, bestS, runningD, cmdiag));
		}
		//if(bestS->x < g_pathfindszx-1 && bestS->z < g_pathfindszz-1 && caneast && cansouth)
		//if(bestS->x < g_pathfindszx-1 && bestS->z < g_pathfindszz-1)
		if(bestS->cx < g_pathdim.x-1 && 
			bestS->cz < g_pathdim.y-1 && 
			wt->triedcell[ PathCellIndex(bestS->cx + 1, bestS->cz + 1) ] != wt->pathcnt)
		{
			cmposx = bestS->cx * PATHCELL_SIZE + PATHCELL_SIZE/2 + PATHCELL_SIZE/2;
			cmposz = bestS->cz * PATHCELL_SIZE + PATHCELL_SIZE/2 + PATHCELL_SIZE/2;

			cmminx = cmposx - ut->size.x/2;
			cmminz = cmposz - ut->size.z/2;
			cmmaxx = cmminx + ut->size.x;
			cmmaxz = cmminz + ut->size.z;

			cminx = cmminx/PATHCELL_SIZE;
			cminz = cmminz/PATHCELL_SIZE;
			cmaxx = cmmaxx/PATHCELL_SIZE;
			cmaxz = cmmaxz/PATHCELL_SIZE;

			if(CellOpen(bestS->cx + 1, bestS->cz + 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx + 1, bestS->cz + 1, bestS, runningD, cmdiag));
		}
#endif
#endif
	}

	return false;
}

static vector<Vec3f> gridvecs;

void DrawGrid()
{
	glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glBegin(GL_LINES);
	
	glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &gridvecs[0]);
	glDrawArrays(GL_LINES, 0, gridvecs.size());

#if 1
	glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR],  0.5f, 0, 0, 1);
    
	if(g_selection.units.size() > 0)
	{
		int i = *g_selection.units.begin();
		Unit* u = &g_unit[i];
        
		bool roadVeh = false;
#if 0
		if(u->type == UNIT_TRUCK)
			roadVeh = true;
#endif
        
		UnitT* t = &g_unitT[u->type];
		
		for(int x=0; x<g_pathdim.x; x++)
			for(int z=0; z<g_pathdim.y; z++)
			{
				ColliderCell* cell = ColliderCellAt(x, z);
				if(cell->units.size() <= 0)
					continue;

				bool foundother = false;

				for(auto uiter = cell->units.begin(); uiter != cell->units.end(); uiter++)
				{
					if(&g_unit[*uiter] != u)
					{
						foundother = true;
						break;
					}
				}

				if(!foundother)
					continue;
                
				vector<Vec3f> vecs;
				vecs.push_back(Vec3f(x*PATHCELL_SIZE, 1, z*PATHCELL_SIZE));
				vecs.push_back(Vec3f((x+1)*PATHCELL_SIZE, 1, (z+1)*PATHCELL_SIZE));
				vecs.push_back(Vec3f((x+1)*PATHCELL_SIZE, 1, z*PATHCELL_SIZE));
				vecs.push_back(Vec3f(x*PATHCELL_SIZE, 1, (z+1)*PATHCELL_SIZE));
				
				vecs[0].y = g_hmap.accheight(vecs[0].x, vecs[0].z) + TILE_SIZE/100;
				vecs[1].y = g_hmap.accheight(vecs[1].x, vecs[1].z) + TILE_SIZE/100;
				vecs[2].y = g_hmap.accheight(vecs[2].x, vecs[2].z) + TILE_SIZE/100;
				vecs[3].y = g_hmap.accheight(vecs[3].x, vecs[3].z) + TILE_SIZE/100;

				glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
				glDrawArrays(GL_LINES, 0, vecs.size());
			}
	}
#endif

	if(gridvecs.size() > 0)
		return;
    
	for(int x=0; x<g_pathdim.x; x++)
	{
		for(int z=0; z<g_pathdim.y; z++)
		{
			int i = gridvecs.size();

			gridvecs.push_back(Vec3f(x*PATHCELL_SIZE, 0 + 1, z*PATHCELL_SIZE));
			gridvecs.push_back(Vec3f(x*PATHCELL_SIZE, 0 + 1, (z+1)*PATHCELL_SIZE));
			gridvecs[i+0].y = g_hmap.accheight(gridvecs[i+0].x, gridvecs[i+0].z) + TILE_SIZE/100;
			gridvecs[i+1].y = g_hmap.accheight(gridvecs[i+1].x, gridvecs[i+1].z) + TILE_SIZE/100;
			//glVertex3f(x*(MIN_RADIUS*2.0f), 0 + 1, 0);
			//glVertex3f(x*(MIN_RADIUS*2.0f), 0 + 1, g_map.m_widthZ*TILE_SIZE);
		}
	}
    
	for(int z=0; z<g_pathdim.y; z++)
	{
		for(int x=0; x<g_pathdim.x; x++)
		{
			int i = gridvecs.size();
			gridvecs.push_back(Vec3f(x*PATHCELL_SIZE, 0 + 1, z*PATHCELL_SIZE));
			gridvecs.push_back(Vec3f((x+1)*PATHCELL_SIZE, 0 + 1, z*PATHCELL_SIZE));
			gridvecs[i+0].y = g_hmap.accheight(gridvecs[i+0].x, gridvecs[i+0].z) + TILE_SIZE/100;
			gridvecs[i+1].y = g_hmap.accheight(gridvecs[i+1].x, gridvecs[i+1].z) + TILE_SIZE/100;
			//glVertex3f(0, 0 + 1, z*(MIN_RADIUS*2.0f));
			//glVertex3f(g_map.m_widthX*TILE_SIZE, 0 + 1, z*(MIN_RADIUS*2.0f));
		}
	}
  
	//glEnd();
	//glColor4f(1, 1, 1, 1);
}

void DrawUnitSquares()
{
	Unit* u;
	UnitT* t;
	Vec3f p;
    
	glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0.5f, 0, 0, 1);
    
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];
        
		if(!u->on)
			continue;
        
		t = &g_unitT[u->type];
		p = Vec3f(u->cmpos.x, u->drawpos.y + TILE_SIZE/100, u->cmpos.y);
        
#if 1
		if(u->collided)
			glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 1.0f, 0, 0, 1);
		else
			glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0.2f, 0, 0, 1);
#endif

		/*
         glVertex3f(p.x - r, 0 + 1, p.z - r);
         glVertex3f(p.x - r, 0 + 1, p.z + r);
         glVertex3f(p.x + r, 0 + 1, p.z + r);
         glVertex3f(p.x + r, 0 + 1, p.z - r);
         glVertex3f(p.x - r, 0 + 1, p.z - r);
         */
        
		Vec3i vmin(u->cmpos.x - t->size.x/2, 0, u->cmpos.y - t->size.z/2);

		vector<Vec3f> vecs;
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z));
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z + t->size.z));
		vecs.push_back(Vec3f(vmin.x + t->size.x, 0 + 1, vmin.z + t->size.z));
		vecs.push_back(Vec3f(vmin.x + t->size.x, 0 + 1, vmin.z));
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z));
		
		vecs[0].y = g_hmap.accheight(vecs[0].x, vecs[0].z) + TILE_SIZE/100;
		vecs[1].y = g_hmap.accheight(vecs[1].x, vecs[1].z) + TILE_SIZE/100;
		vecs[2].y = g_hmap.accheight(vecs[2].x, vecs[2].z) + TILE_SIZE/100;
		vecs[3].y = g_hmap.accheight(vecs[3].x, vecs[3].z) + TILE_SIZE/100;
		vecs[4].y = g_hmap.accheight(vecs[4].x, vecs[4].z) + TILE_SIZE/100;
        
		glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
		glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
	}
    
}

void DrawPaths()
{   
#if 1
	int i = 0;

	if(g_selection.units.size() <= 0)
		return;

	i = *g_selection.units.begin();
#else
	for(int i=0; i<UNITS; i++)
#endif
	{
		glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0, 1, 0, 1);
        
		Unit* u = &g_unit[i];

#if 0
		if(!u->on)
			continue;
#endif

		vector<Vec3f> vecs;

		Vec3f p;
		p.x = u->cmpos.x;
		p.z = u->cmpos.y;
		p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/100;
        
		for(auto piter = u->path.begin(); piter != u->path.end(); piter++)
		{
			p.x = piter->x;
			p.z = piter->y;
			p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/100;
			//glVertex3f(p->x, p->y + 5, p->z);
			vecs.push_back(p);
			//vecs.push_back(p+Vec3f(0,10,0));
		}
		
#if 0
		p.x = u->goal.x;
		p.z = u->goal.y;
		p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/100;
#endif
        
		if(vecs.size() > 1)
		{
            glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
            glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
		else
		{
			//vecs.push_back(u->camera.Position() + Vec3f(0,5,0));
			//vecs.push_back(u->goal + Vec3f(0,5,0));
            
			glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0.8f, 1, 0.8f, 1);
            
			glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
			glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
	}
}

#if 0
void DrawVelocities()
{
	CUnit* u;
	Vec3f p;
    
	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 1, 0, 1, 1);
    
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];
        
		if(!u->on)
			continue;
        
		vector<Vec3f> vecs;
        
        vecs.push_back(u->camera.Position() + Vec3f(0, 1, 0));
        vecs.push_back(u->camera.Position() + u->camera.Velocity()*100.0f + Vec3f(0, 1, 0));
        
		if(vecs.size() > 0)
		{
            glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
            glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
	}
}

#endif


void LogPathDebug()
{
	if(g_pathunit)
	{
		g_pathunit = NULL;
		return;
	}

	if(g_selection.units.size() <= 0)
		return;

	int i = *g_selection.units.begin();

	g_pathunit = &g_unit[i];
}