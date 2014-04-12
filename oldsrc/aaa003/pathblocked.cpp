

#include "trystep.h"
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
#include "fullpath.h"
#include "pathblocked.h"
#include "pathdebug.h"

bool PathBlocked(int wthread, int utype, int umode, int cmstartx, int cmstartz, int target, int targetx, int targetz, 
				 Unit* thisu, Unit* ignoreu, Building* ignoreb, 
				 int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminz, int cmgoalmaxx, int cmgoalmaxz,
				 int maxsearch)
{
	WorkThread* wt = &g_workthread[wthread];
	wt->trystep.clear();
	wt->pathcnt++;
	wt->heap.resetelems();

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

#if 1
#if 1
		if(CellBlockOpen(cposx, cposz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE
#if 1
					&& 
				CellBlockOpen(cposx, cposz,
					cminx2, cminz2, cmaxx2, cmaxz2,
					cmminx2, cmminz2, cmmaxx2, cmmaxz2,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE
#endif
					)
		{
			wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
			wt->heap.insert(&*wt->trystep.rbegin());
		}
#endif
#endif
	}

	//if(startcenterblocked)
	{
		if( cstartx > 0 && cstartz > 0 && cstartx - 1 >= cstartminx && cstartz - 1 >= cstartminz)
		{
			int cposx = cstartx - 1;
			int cposz = cstartz - 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellBlockOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}

		if( cstartz > 0 && cstartz - 1 >= cstartminz)
		{
			int cposx = cstartx;
			int cposz = cstartz - 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellBlockOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}

#if 1
		if( cstartx < g_pathdim.x - 1 && cstartz > 0 && cstartx + 1 <= cstartmaxx && cstartz - 1 >= cstartminz )
		{
			int cposx = cstartx + 1;
			int cposz = cstartz - 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellBlockOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
#endif
		
		if( cstartx > 0 && cstartx - 1 >= cstartminx )
		{
			int cposx = cstartx - 1;
			int cposz = cstartz;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellBlockOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		
		if( cstartx < g_pathdim.x - 1 && cstartx + 1 <= cstartmaxx )
		{
			int cposx = cstartx + 1;
			int cposz = cstartz;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellBlockOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		
#if 1
		if( cstartx > 0 && cstartz < g_pathdim.y - 1 && cstartx - 1 >= cstartminx && cstartz + 1 <= cstartmaxz )
		{
			int cposx = cstartx - 1;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellBlockOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
#endif

		if( cstartz < g_pathdim.y && cstartz + 1 <= cstartmaxz)
		{
			int cposx = cstartx;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellBlockOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		
#if 1
		if( cstartx < g_pathdim.x - 1 && cstartz < g_pathdim.y && cstartx - 1 >= cstartminx && cstartz + 1 <= cstartmaxz )
		{
			int cposx = cstartx + 1;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHCELL_SIZE + PATHCELL_SIZE/2, cposz * PATHCELL_SIZE + PATHCELL_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHCELL_SIZE;
			int cminz = cmminz/PATHCELL_SIZE;
			int cmaxx = cmmaxx/PATHCELL_SIZE;
			int cmaxz = cmmaxz/PATHCELL_SIZE;
			
			if(CellBlockOpen(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
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

	if(!CellBlockOpen(&goalS,
		cminx, cminz, cmaxx, cmaxz,
		cmminx, cmminz, cmmaxx, cmmaxz,
		ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
		thisu, ignoreu, ignoreb))
		return false;
#endif

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

#if 1
		if(searchdepth > maxsearch)
			return false;
#endif

#if 0
		g_log<<"pathfind 3"<<endl;
		g_log.flush();
#endif
		bestS = NULL;

#if 0
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
		{
			if(heap.hasmore())
			{
				g_log<<"list ran out premat"<<endl;
				g_log.flush();
			}

			return false;
		}

		if(!heap.hasmore())
		{
			g_log<<"bh ran out premat"<<endl;
			g_log.flush();
			return false;
		}

		TryStep* bestS2 = heap.deletemin();

		if(bestS2 != bestS)
		{
			g_log<<"bh (.F="<<bestS2->F<<") doesn't match list (.F="<<bestS->F<<")"<<endl;
			g_log.flush();
		}
#else

		do
		{
			if(!wt->heap.hasmore())
				return true;

			bestS = wt->heap.deletemin();

		}while( wt->triedcell[ PathCellIndex(bestS->cx, bestS->cz) ] == wt->pathcnt );


#endif

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

		if(
#if 1
			bestS->previous && 
#endif
			CellBlockOpen(bestS->cx, bestS->cz,
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
			return false;
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
			CellBlockOpen(bestS->cx, bestS->cz,
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
			runningD = bestS->previous->totalD;

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
			wt->heap.insert(&*wt->trystep.rbegin());
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
			wt->heap.insert(&*wt->trystep.rbegin());
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
			wt->heap.insert(&*wt->trystep.rbegin());
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
			wt->heap.insert(&*wt->trystep.rbegin());
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

			CellBlockOpen(bestS->cx, bestS->cz,
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

			if(CellBlockOpen(bestS->cx - 1, bestS->cz - 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx - 1, bestS->cz - 1, bestS, runningD, cmdiag));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
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

			if(CellBlockOpen(bestS->cx + 1, bestS->cz - 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx + 1, bestS->cz - 1, bestS, runningD, cmdiag));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
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

			if(CellBlockOpen(bestS->cx - 1, bestS->cz + 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx - 1, bestS->cz + 1, bestS, runningD, cmdiag));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
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

			if(CellBlockOpen(bestS->cx + 1, bestS->cz + 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(TryStep(cstartx, cstartz, cgoalx, cgoalz, bestS->cx + 1, bestS->cz + 1, bestS, runningD, cmdiag));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
#endif
#endif
	}

	return false;
}