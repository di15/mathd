

#include "pathnode.h"
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
#include "partialpath.h"
#include "reconstructpath.h"
#include "pathdebug.h"

bool PartialPath(int wthread, int utype, int umode, int cmstartx, int cmstartz, int target, int targetx, int targetz, 
			  list<Vec2i> &path, Vec2i &subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb, 
			  int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminz, int cmgoalmaxx, int cmgoalmaxz,
			  int maxsearch)
{
	WorkThread* wt = &g_workthread[wthread];
	wt->trystep.clear();
	wt->pathcnt++;
	wt->heap.resetelems();

	UnitT* ut = &g_unitT[utype];

	int cstartx = cmstartx / PATHNODE_SIZE;
	int cstartz = cmstartz / PATHNODE_SIZE;
	int cgoalx = (cmgoalminx + cmgoalmaxx) / 2 / PATHNODE_SIZE;
	int cgoalz = (cmgoalminz + cmgoalmaxz) / 2 / PATHNODE_SIZE;
	
	int cmstartminx = cmstartx - ut->size.x/2;
	int cmstartminz = cmstartz - ut->size.z/2;
	int cmstartmaxx = cmstartminx + ut->size.x;
	int cmstartmaxz = cmstartminz + ut->size.z;

	int cstartminx = cmstartminx / PATHNODE_SIZE;
	int cstartminz = cmstartminz / PATHNODE_SIZE;
	int cstartmaxx = cmstartmaxx / PATHNODE_SIZE;
	int cstartmaxz = cmstartmaxz / PATHNODE_SIZE;

#if 0
	wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cstartx, cstartz, NULL, 0, 0));
#endif
	int runningD = 0;
	const int cmdiag = PATHNODE_DIAG;

	if(thisu == g_pathunit)
	{
		g_log<<"============================================"<<endl;
		g_log<<"unit "<<(int)(thisu-g_unit)<<" pathing frame "<<g_simframe<<endl;
		g_log.flush();

		
		//int cmposx = cgoalx * PATHNODE_SIZE;
		//int cmposz = cgoalz * PATHNODE_SIZE;
		int cmposx = cmstartx;
		int cmposz = cmstartz;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		int cposx = cmstartx / PATHNODE_SIZE;
		int cposz = cmstartz / PATHNODE_SIZE;
		
		
		g_log<<"cm pos: ("<<cmposx<<","<<cmposz<<")"<<endl;
		g_log<<"cm rect: ("<<cmminx<<","<<cmminz<<")->("<<cmmaxx<<","<<cmmaxz<<")"<<endl;
		g_log<<"pathcell rect: ("<<cminx<<","<<cminz<<")->("<<cmaxx<<","<<cmaxz<<")"<<endl;
		g_log<<"pathcell pos: ("<<cposx<<","<<cposz<<")"<<endl;
		g_log<<"--------------------------------------------"<<endl;
		g_log.flush();
	}
	
	// Check if starting centered position is blocked.
	// If not, add it as one of the first steps (with .previous = NULL).
	{
		//Cenetered position
		int cposx = cstartx;
		int cposz = cstartz;

		int cmposx = cstartx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = cstartz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

#if 1
		//Midway between centered and starting position
		int cmposx2 = (cmposx + cmstartx) / 2;
		int cmposz2 = (cmposz + cmstartz) / 2;

		int cmminx2 = cmposx2 - ut->size.x/2;
		int cmminz2 = cmposz2 - ut->size.z/2;
		int cmmaxx2 = cmminx2 + ut->size.x;
		int cmmaxz2 = cmminz2 + ut->size.z;

		int cminx2 = cmminx2/PATHNODE_SIZE;
		int cminz2 = cmminz2/PATHNODE_SIZE;
		int cmaxx2 = cmmaxx2/PATHNODE_SIZE;
		int cmaxz2 = cmmaxz2/PATHNODE_SIZE;
#endif

#if 1
#if 1
		if(NodeBlockWalkable(cposx, cposz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE
#if 1
					&& 
				NodeBlockWalkable(cposx, cposz,
					cminx2, cminz2, cmaxx2, cmaxz2,
					cmminx2, cmminz2, cmmaxx2, cmmaxz2,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE
#endif
					)
		{
			wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
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
			Vec2i to(cposx * PATHNODE_SIZE + PATHNODE_SIZE/2, cposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHNODE_SIZE;
			int cminz = cmminz/PATHNODE_SIZE;
			int cmaxx = cmmaxx/PATHNODE_SIZE;
			int cmaxz = cmmaxz/PATHNODE_SIZE;
			
			if(NodeBlockWalkable(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}

		if( cstartz > 0 && cstartz - 1 >= cstartminz)
		{
			int cposx = cstartx;
			int cposz = cstartz - 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHNODE_SIZE + PATHNODE_SIZE/2, cposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHNODE_SIZE;
			int cminz = cmminz/PATHNODE_SIZE;
			int cmaxx = cmmaxx/PATHNODE_SIZE;
			int cmaxz = cmmaxz/PATHNODE_SIZE;
			
			if(NodeBlockWalkable(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}

#if 1
		if( cstartx < g_pathdim.x - 1 && cstartz > 0 && cstartx + 1 <= cstartmaxx && cstartz - 1 >= cstartminz )
		{
			int cposx = cstartx + 1;
			int cposz = cstartz - 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHNODE_SIZE + PATHNODE_SIZE/2, cposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHNODE_SIZE;
			int cminz = cmminz/PATHNODE_SIZE;
			int cmaxx = cmmaxx/PATHNODE_SIZE;
			int cmaxz = cmmaxz/PATHNODE_SIZE;
			
			if(NodeBlockWalkable(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
#endif
		
		if( cstartx > 0 && cstartx - 1 >= cstartminx )
		{
			int cposx = cstartx - 1;
			int cposz = cstartz;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHNODE_SIZE + PATHNODE_SIZE/2, cposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHNODE_SIZE;
			int cminz = cmminz/PATHNODE_SIZE;
			int cmaxx = cmmaxx/PATHNODE_SIZE;
			int cmaxz = cmmaxz/PATHNODE_SIZE;
			
			if(NodeBlockWalkable(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		
		if( cstartx < g_pathdim.x - 1 && cstartx + 1 <= cstartmaxx )
		{
			int cposx = cstartx + 1;
			int cposz = cstartz;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHNODE_SIZE + PATHNODE_SIZE/2, cposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHNODE_SIZE;
			int cminz = cmminz/PATHNODE_SIZE;
			int cmaxx = cmmaxx/PATHNODE_SIZE;
			int cmaxz = cmmaxz/PATHNODE_SIZE;
			
			if(NodeBlockWalkable(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		
#if 1
		if( cstartx > 0 && cstartz < g_pathdim.y - 1 && cstartx - 1 >= cstartminx && cstartz + 1 <= cstartmaxz )
		{
			int cposx = cstartx - 1;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHNODE_SIZE + PATHNODE_SIZE/2, cposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHNODE_SIZE;
			int cminz = cmminz/PATHNODE_SIZE;
			int cmaxx = cmmaxx/PATHNODE_SIZE;
			int cmaxz = cmmaxz/PATHNODE_SIZE;
			
			if(NodeBlockWalkable(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
#endif

		if( cstartz < g_pathdim.y && cstartz + 1 <= cstartmaxz)
		{
			int cposx = cstartx;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHNODE_SIZE + PATHNODE_SIZE/2, cposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHNODE_SIZE;
			int cminz = cmminz/PATHNODE_SIZE;
			int cmaxx = cmmaxx/PATHNODE_SIZE;
			int cmaxz = cmmaxz/PATHNODE_SIZE;
			
			if(NodeBlockWalkable(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		
#if 1
		if( cstartx < g_pathdim.x - 1 && cstartz < g_pathdim.y && cstartx - 1 >= cstartminx && cstartz + 1 <= cstartmaxz )
		{
			int cposx = cstartx + 1;
			int cposz = cstartz + 1;
			Vec2i from(cmstartx, cmstartz);
			Vec2i to(cposx * PATHNODE_SIZE + PATHNODE_SIZE/2, cposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i stepto = from + scaleddir;
			
			int cmminx = stepto.x - ut->size.x/2;
			int cmminz = stepto.y - ut->size.z/2;
			int cmmaxx = cmminx + ut->size.x;
			int cmmaxz = cmminz + ut->size.z;

			int cminx = cmminx/PATHNODE_SIZE;
			int cminz = cmminz/PATHNODE_SIZE;
			int cmaxx = cmmaxx/PATHNODE_SIZE;
			int cmaxz = cmmaxz/PATHNODE_SIZE;
			
			if(NodeBlockWalkable(cposx, cposz,
						cminx, cminz, cmaxx, cmaxz,
						cmminx, cmminz, cmmaxx, cmmaxz,
						ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
						thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, cposx, cposz, NULL, 0, 0));
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

	int cmposx = cgoalx * PATHNODE_SIZE;
	int cmposz = cgoalz * PATHNODE_SIZE;

	int cmminx = cmposx - ut->size.x/2;
	int cmminz = cmposz - ut->size.z/2;
	int cmmaxx = cmminx + ut->size.x;
	int cmmaxz = cmminz + ut->size.z;

	int cminx = cmminx/PATHNODE_SIZE;
	int cminz = cmminz/PATHNODE_SIZE;
	int cmaxx = cmmaxx/PATHNODE_SIZE;
	int cmaxz = cmmaxz/PATHNODE_SIZE;

	PathNode goalS;
	goalS.cx = cgoalx;
	goalS.cz = cgoalz;

	if(!NodeBlockWalkable(&goalS,
		cminx, cminz, cmaxx, cmaxz,
		cmminx, cmminz, cmmaxx, cmmaxz,
		ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
		thisu, ignoreu, ignoreb))
		return false;
#endif

	PathNode* bestS;
	Vec2i halfway;
	PathNode* closestS = NULL;
	int closest = 0;

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
		{
			if(closestS)
			{
				ReconstructPath(path, closestS, subgoal, cmgoalx, cmgoalz);

				return true;
			}

			return false;
		}
#endif

		bestS = NULL;

		do
		{
			if(!wt->heap.hasmore())
			{
				if(closestS)
				{
					ReconstructPath(path, closestS, subgoal, cmgoalx, cmgoalz);
					return true;
				}

				return false;
			}

			bestS = wt->heap.deletemin();

		}while( wt->triedcell[ PathNodeIndex(bestS->cx, bestS->cz) ] == wt->pathcnt );

		wt->triedcell[ PathNodeIndex(bestS->cx, bestS->cz) ] = wt->pathcnt;

		int thisdistance = Magnitude(Vec2i(bestS->cx - cgoalx, bestS->cz - cgoalz));

		if( !closestS || thisdistance < closest )
		{
			closestS = bestS;
			closest = thisdistance;
		}

		int cmposx = bestS->cx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = bestS->cz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(
#if 1
			bestS->previous && 
#endif
			NodeBlockWalkable(bestS->cx, bestS->cz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) != COLLIDER_NONE)
		{
			continue;
		}

		// Arrived at goal area?
		if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
		{
			ReconstructPath(path, bestS, subgoal, cmgoalx, cmgoalz);

			return true;
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
			NodeBlockWalkable(bestS->cx, bestS->cz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) != COLLIDER_NONE)
		{
			firstcenterblocked = true;

			float xratio = PATHNODE_SIZE / 2.0f / (float)ut->size.x;
			float zratio = PATHNODE_SIZE / 2.0f / (float)ut->size.z;

			for(int x=cminx; x<=cmaxx; x++)
				for(int z=cminz; z<=cmaxz; z++)
				{
					ColliderTile* cell = ColliderTileAt(x, z);

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

						int cminx2 = cmminx2 / PATHNODE_SIZE;
						int cminz2 = cmminz2 / PATHNODE_SIZE;
						int cmaxx2 = cmmaxx2 / PATHNODE_SIZE;
						int cmaxz2 = cmmaxz2 / PATHNODE_SIZE;
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
			wt->triedcell[ PathNodeIndex(bestS->cx - 1, bestS->cz) ] != wt->pathcnt)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"\t can go west from "<<bestS->cx<<","<<bestS->cz<<" to "<<(bestS->cx - 1)<<","<<bestS->cz<<endl;
				g_log.flush();
			}

			wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, bestS->cx - 1, bestS->cz, bestS, runningD, PATHNODE_SIZE));
			wt->heap.insert(&*wt->trystep.rbegin());
		}
		if(bestS->cx < g_pathdim.x-1 && 
			caneast && 
			wt->triedcell[ PathNodeIndex(bestS->cx + 1, bestS->cz) ] != wt->pathcnt)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"\t can go east from "<<bestS->cx<<","<<bestS->cz<<" to "<<(bestS->cx + 1)<<","<<bestS->cz<<endl;
				g_log.flush();
			}

			wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, bestS->cx + 1, bestS->cz, bestS, runningD, PATHNODE_SIZE));
			wt->heap.insert(&*wt->trystep.rbegin());
		}
		if(bestS->cz > 0 && 
			cannorth && 
			wt->triedcell[ PathNodeIndex(bestS->cx, bestS->cz - 1) ] != wt->pathcnt)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"\t can go north from "<<bestS->cx<<","<<bestS->cz<<" to "<<(bestS->cx)<<","<<(bestS->cz - 1)<<endl;
				g_log.flush();
			}

			wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, bestS->cx, bestS->cz - 1, bestS, runningD, PATHNODE_SIZE));
			wt->heap.insert(&*wt->trystep.rbegin());
		}
		if(bestS->cz < g_pathdim.y-1 && 
			cansouth && 
			wt->triedcell[ PathNodeIndex(bestS->cx, bestS->cz + 1) ] != wt->pathcnt)
		{
			if(thisu == g_pathunit)
			{
				g_log<<"\t can go north from "<<bestS->cx<<","<<bestS->cz<<" to "<<(bestS->cx)<<","<<(bestS->cz + 1)<<endl;
				g_log.flush();
			}
			wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, bestS->cx, bestS->cz + 1, bestS, runningD, PATHNODE_SIZE));
			wt->heap.insert(&*wt->trystep.rbegin());
		}

#if 1
#ifndef NO_DIAGONAL
		//int stepD = cmdiag;

#if 0
		

		int cmposx = bestS->cx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = bestS->cz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(thisu == g_pathunit)
		{
			g_log<<"try cell "<<endl;
			g_log.flush();

			int cposx = cmstartx / PATHNODE_SIZE;
			int cposz = cmstartz / PATHNODE_SIZE;
		
			g_log<<"cm pos: ("<<cmposx<<","<<cmposz<<")"<<endl;
			g_log<<"cm rect: ("<<cmminx<<","<<cmminz<<")->("<<cmmaxx<<","<<cmmaxz<<")"<<endl;
			g_log<<"pathcell rect: ("<<cminx<<","<<cminz<<")->("<<cmaxx<<","<<cmaxz<<")"<<endl;
			g_log<<"pathcell pos: ("<<bestS->cx<<","<<bestS->cz<<")"<<endl;

			if(bestS->previous)
			{
				g_log<<"\tfrom pathcell pos: ("<<bestS->previous->cx<<","<<bestS->previous->cz<<")"<<endl;
			}
		}

			NodeBlockWalkable(bestS->cx, bestS->cz,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) != COLLIDER_NONE)
#endif

		//if(bestS->x > 0 && bestS->z > 0 && canwest && cannorth)
		if(bestS->cx > 0 && 
			bestS->cz > 0 && 
			wt->triedcell[ PathNodeIndex(bestS->cx - 1, bestS->cz - 1) ] != wt->pathcnt)
		{
			cmposx = bestS->cx * PATHNODE_SIZE + PATHNODE_SIZE/2 - PATHNODE_SIZE/2;
			cmposz = bestS->cz * PATHNODE_SIZE + PATHNODE_SIZE/2 - PATHNODE_SIZE/2;

			cmminx = cmposx - ut->size.x/2;
			cmminz = cmposz - ut->size.z/2;
			cmmaxx = cmminx + ut->size.x;
			cmmaxz = cmminz + ut->size.z;

			cminx = cmminx/PATHNODE_SIZE;
			cminz = cmminz/PATHNODE_SIZE;
			cmaxx = cmmaxx/PATHNODE_SIZE;
			cmaxz = cmmaxz/PATHNODE_SIZE;

			if(NodeBlockWalkable(bestS->cx - 1, bestS->cz - 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, bestS->cx - 1, bestS->cz - 1, bestS, runningD, cmdiag));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		//if(bestS->x < g_pathfindszx-1 && bestS->z > 0 && caneast && cannorth)
		//if(bestS->x < g_pathfindszx-1 && bestS->z > 0)
		if(bestS->cx < g_pathdim.x-1 &&
			bestS->cz > 0 && 
			wt->triedcell[ PathNodeIndex(bestS->cx + 1, bestS->cz - 1) ] != wt->pathcnt)
		{
			cmposx = bestS->cx * PATHNODE_SIZE + PATHNODE_SIZE/2 + PATHNODE_SIZE/2;
			cmposz = bestS->cz * PATHNODE_SIZE + PATHNODE_SIZE/2 - PATHNODE_SIZE/2;

			cmminx = cmposx - ut->size.x/2;
			cmminz = cmposz - ut->size.z/2;
			cmmaxx = cmminx + ut->size.x;
			cmmaxz = cmminz + ut->size.z;

			cminx = cmminx/PATHNODE_SIZE;
			cminz = cmminz/PATHNODE_SIZE;
			cmaxx = cmmaxx/PATHNODE_SIZE;
			cmaxz = cmmaxz/PATHNODE_SIZE;

			if(NodeBlockWalkable(bestS->cx + 1, bestS->cz - 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, bestS->cx + 1, bestS->cz - 1, bestS, runningD, cmdiag));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		//if(bestS->x > 0 && bestS->z < g_pathfindszz-1 && canwest && cansouth)
		//if(bestS->x > 0 && bestS->z < g_pathfindszz-1)
		if(bestS->cx > 0 && 
			bestS->cz < g_pathdim.y-1 && 
			wt->triedcell[ PathNodeIndex(bestS->cx - 1, bestS->cz + 1) ] != wt->pathcnt)
		{
			cmposx = bestS->cx * PATHNODE_SIZE + PATHNODE_SIZE/2 - PATHNODE_SIZE/2;
			cmposz = bestS->cz * PATHNODE_SIZE + PATHNODE_SIZE/2 + PATHNODE_SIZE/2;

			cmminx = cmposx - ut->size.x/2;
			cmminz = cmposz - ut->size.z/2;
			cmmaxx = cmminx + ut->size.x;
			cmmaxz = cmminz + ut->size.z;

			cminx = cmminx/PATHNODE_SIZE;
			cminz = cmminz/PATHNODE_SIZE;
			cmaxx = cmmaxx/PATHNODE_SIZE;
			cmaxz = cmmaxz/PATHNODE_SIZE;

			if(NodeBlockWalkable(bestS->cx - 1, bestS->cz + 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, bestS->cx - 1, bestS->cz + 1, bestS, runningD, cmdiag));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
		//if(bestS->x < g_pathfindszx-1 && bestS->z < g_pathfindszz-1 && caneast && cansouth)
		//if(bestS->x < g_pathfindszx-1 && bestS->z < g_pathfindszz-1)
		if(bestS->cx < g_pathdim.x-1 && 
			bestS->cz < g_pathdim.y-1 && 
			wt->triedcell[ PathNodeIndex(bestS->cx + 1, bestS->cz + 1) ] != wt->pathcnt)
		{
			cmposx = bestS->cx * PATHNODE_SIZE + PATHNODE_SIZE/2 + PATHNODE_SIZE/2;
			cmposz = bestS->cz * PATHNODE_SIZE + PATHNODE_SIZE/2 + PATHNODE_SIZE/2;

			cmminx = cmposx - ut->size.x/2;
			cmminz = cmposz - ut->size.z/2;
			cmmaxx = cmminx + ut->size.x;
			cmmaxz = cmminz + ut->size.z;

			cminx = cmminx/PATHNODE_SIZE;
			cminz = cmminz/PATHNODE_SIZE;
			cmaxx = cmmaxx/PATHNODE_SIZE;
			cmaxz = cmmaxz/PATHNODE_SIZE;

			if(NodeBlockWalkable(bestS->cx + 1, bestS->cz + 1,
					cminx, cminz, cmaxx, cmaxz,
					cmminx, cmminz, cmmaxx, cmmaxz,
					ut->roaded, ut->landborne, ut->seaborne, ut->airborne, 
					thisu, ignoreu, ignoreb) == COLLIDER_NONE)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, bestS->cx + 1, bestS->cz + 1, bestS, runningD, cmdiag));
				wt->heap.insert(&*wt->trystep.rbegin());
			}
		}
#endif
#endif
	}

	return false;
}