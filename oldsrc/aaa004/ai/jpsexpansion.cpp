

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
#include "jpspath.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "jpsexpansion.h"


/*
Straight expansions
*/

bool Expansion_W(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx = cx - 1;
	int currcz = cz;
	int runningD2 = PATHNODE_SIZE;

	while(currcx >= 0)
	{
		int cmposx = currcx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx, currcz) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx, currcz,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				g_log<<"reached goal exp w"<<endl;

				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_W));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();

				return true;
			}

			if(NodeForcedNeighbour(currcx, currcz,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_W));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
			else
			{
				wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			}
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			break;
		}

		currcx--;
		runningD2 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_E(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx = cx + 1;
	int currcz = cz;
	int runningD2 = PATHNODE_SIZE;

	while(currcx < g_pathdim.x)
	{
		int cmposx = currcx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx, currcz) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx, currcz,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_E));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp e"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx, currcz,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_E));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
			else
			{
				wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			}
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			break;
		}

		currcx++;
		runningD2 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_N(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx = cx;
	int currcz = cz - 1;
	int runningD2 = PATHNODE_SIZE;

	while(currcz >= 0)
	{
		int cmposx = currcx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx, currcz) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx, currcz,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_N));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp n"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx, currcz,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_N));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
			else
			{
				wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			}
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			break;
		}

		currcz--;
		runningD2 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_S(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx = cx;
	int currcz = cz + 1;
	int runningD2 = PATHNODE_SIZE;

	while(currcz < g_pathdim.y)
	{
		int cmposx = currcx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx, currcz) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx, currcz,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_S));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp s"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx, currcz,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_S));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
			else
			{
				wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			}
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			break;
		}

		currcz++;
		runningD2 += PATHNODE_SIZE;
	}

	return false;
}

/*
Diagonal expansions
*/

bool Expansion_NW(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD, 
					const int cmdiag,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx = cx - 1;
	int currcz = cz - 1;
	int runningD2 = cmdiag;

	while(currcx >= 0 && currcz >= 0)
	{
		int cmposx = currcx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx, currcz) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx, currcz,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE
			&&
			NodeWalkable(cmaxx+1, cmaxz,
			cminx+1, cminz, cmaxx+1, cmaxz,
			cmminx+PATHNODE_SIZE, cmminz, cmmaxx+PATHNODE_SIZE, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE
			&&
			NodeWalkable(cmaxx, cmaxz+1,
			cminx, cminz+1, cmaxx, cmaxz+1,
			cmminx, cmminz+PATHNODE_SIZE, cmmaxx, cmmaxz+PATHNODE_SIZE,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_NW));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp nw"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx, currcz,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_NW));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
			else
			{
				wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;

				if(Expansion_NW_W(currcx, currcz, 
					roaded, landborne, seaborne, airborne,
					thisu, ignoreu, ignoreb,
					ut, wt,
					cstartx, cstartz, cgoalx, cgoalz, 
					bestS, runningD, runningD2, 
					cmgoalminx, cmgoalminz, cmgoalmaxx, cmgoalmaxz,
					reachedS))
					return true;

				if(Expansion_NW_N(currcx, currcz, 
					roaded, landborne, seaborne, airborne,
					thisu, ignoreu, ignoreb,
					ut, wt,
					cstartx, cstartz, cgoalx, cgoalz, 
					bestS, runningD, runningD2, 
					cmgoalminx, cmgoalminz, cmgoalmaxx, cmgoalmaxz,
					reachedS))
					return true;
			}
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			break;
		}
		currcx--;
		currcz--;
		runningD2 += cmdiag;
	}

	return false;
}

bool Expansion_NE(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD, 
					const int cmdiag,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{

	int currcx = cx + 1;
	int currcz = cz - 1;
	int runningD2 = cmdiag;

	while(currcx < g_pathdim.x && currcz >= 0)
	{
		int cmposx = currcx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx, currcz) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx, currcz,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE
			&&
			NodeWalkable(cminx-1, cmaxz,
			cminx-1, cminz, cmaxx+1, cmaxz,
			cmminx-PATHNODE_SIZE, cmminz, cmmaxx-PATHNODE_SIZE, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE
			&&
			NodeWalkable(cminx, cmaxz+1,
			cminx, cminz+1, cmaxx, cmaxz+1,
			cmminx, cmminz+PATHNODE_SIZE, cmmaxx, cmmaxz+PATHNODE_SIZE,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_NE));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp ne"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx, currcz,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_NE));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
			else
			{
				wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;

				if(Expansion_NE_E(currcx, currcz, 
					roaded, landborne, seaborne, airborne,
					thisu, ignoreu, ignoreb,
					ut, wt,
					cstartx, cstartz, cgoalx, cgoalz, 
					bestS, runningD, runningD2, 
					cmgoalminx, cmgoalminz, cmgoalmaxx, cmgoalmaxz,
					reachedS))
					return true;

				if(Expansion_NE_N(currcx, currcz, 
					roaded, landborne, seaborne, airborne,
					thisu, ignoreu, ignoreb,
					ut, wt,
					cstartx, cstartz, cgoalx, cgoalz, 
					bestS, runningD, runningD2, 
					cmgoalminx, cmgoalminz, cmgoalmaxx, cmgoalmaxz,
					reachedS))
					return true;
			}
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			break;
		}

		currcx++;
		currcz--;
		runningD2 += cmdiag;
	}

	return false;
}

bool Expansion_SW(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD, 
					const int cmdiag,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx = cx - 1;
	int currcz = cz + 1;
	int runningD2 = cmdiag;

	while(currcx >= 0 && currcz < g_pathdim.y)
	{
		int cmposx = currcx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx, currcz) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx, currcz,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE
			&&
			NodeWalkable(cmaxx+1, cminz,
			cminx+1, cminz, cmaxx+1, cmaxz,
			cmminx+PATHNODE_SIZE, cmminz, cmmaxx+PATHNODE_SIZE, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE
			&&
			NodeWalkable(cmaxx, cminz-1,
			cminx, cminz-1, cmaxx, cmaxz-1,
			cmminx, cmminz-PATHNODE_SIZE, cmmaxx, cmmaxz-PATHNODE_SIZE,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_SW));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp sw"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx, currcz,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_SW));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
			else
			{
				wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;

				if(Expansion_SW_W(currcx, currcz, 
					roaded, landborne, seaborne, airborne,
					thisu, ignoreu, ignoreb,
					ut, wt,
					cstartx, cstartz, cgoalx, cgoalz, 
					bestS, runningD, runningD2, 
					cmgoalminx, cmgoalminz, cmgoalmaxx, cmgoalmaxz,
					reachedS))
					return true;

				if(Expansion_SW_S(currcx, currcz, 
					roaded, landborne, seaborne, airborne,
					thisu, ignoreu, ignoreb,
					ut, wt,
					cstartx, cstartz, cgoalx, cgoalz, 
					bestS, runningD, runningD2, 
					cmgoalminx, cmgoalminz, cmgoalmaxx, cmgoalmaxz,
					reachedS))
					return true;
			}
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			break;
		}
		currcx--;
		currcz++;
		runningD2 += cmdiag;
	}

	return false;
}

bool Expansion_SE(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD, 
					const int cmdiag,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx = cx + 1;
	int currcz = cz + 1;
	int runningD2 = cmdiag;

	while(currcx < g_pathdim.x && currcz < g_pathdim.y)
	{
		int cmposx = currcx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx, currcz) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx, currcz,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE
			&&
			NodeWalkable(cminx-1, cminz,
			cminx-1, cminz, cmaxx-1, cmaxz,
			cmminx-PATHNODE_SIZE, cmminz, cmmaxx-PATHNODE_SIZE, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE
			&&
			NodeWalkable(cminx, cminz-1,
			cminx, cminz-1, cmaxx, cmaxz-1,
			cmminx, cmminz-PATHNODE_SIZE, cmmaxx, cmmaxz-PATHNODE_SIZE,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_SE));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp se"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx, currcz,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx, currcz, bestS, runningD, runningD2, EXP_SE));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
			else
			{
				wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;

				if(Expansion_SE_E(currcx, currcz, 
					roaded, landborne, seaborne, airborne,
					thisu, ignoreu, ignoreb,
					ut, wt,
					cstartx, cstartz, cgoalx, cgoalz, 
					bestS, runningD, runningD2, 
					cmgoalminx, cmgoalminz, cmgoalmaxx, cmgoalmaxz,
					reachedS))
					return true;

				if(Expansion_SE_S(currcx, currcz, 
					roaded, landborne, seaborne, airborne,
					thisu, ignoreu, ignoreb,
					ut, wt,
					cstartx, cstartz, cgoalx, cgoalz, 
					bestS, runningD, runningD2, 
					cmgoalminx, cmgoalminz, cmgoalmaxx, cmgoalmaxz,
					reachedS))
					return true;
			}
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx, currcz) ] = wt->pathcnt;
			break;
		}
		currcx++;
		currcz++;
		runningD2 += cmdiag;
	}

	return false;
}

/*
Straight expansions after diagonal
*/

bool Expansion_NW_W(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					int runningD2,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx2 = cx - 1;
	int currcz2 = cz;
	int runningD3 = runningD2 + PATHNODE_SIZE;

	while(currcx2 >= 0)
	{
		int cmposx = currcx2 * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz2 * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx2, currcz2,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_NW_W));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp nw w"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx2, currcz2,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_NW_W));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
#if 1
			else
				wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
#endif
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
			break;
		}

		currcx2--;
		runningD3 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_NW_N(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					int runningD2,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx2 = cx;
	int currcz2 = cz - 1;
	int runningD3 = runningD2 + PATHNODE_SIZE;

	while(currcz2 >= 0)
	{
		int cmposx = currcx2 * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz2 * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx2, currcz2,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_NW_N));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp nw n"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx2, currcz2,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_NW_N));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
#if 1
			else
				wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
#endif
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
			break;
		}

		currcz2--;
		runningD3 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_NE_E(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					int runningD2,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx2 = cx + 1;
	int currcz2 = cz;
	int runningD3 = runningD2 + PATHNODE_SIZE;

	while(currcx2 < g_pathdim.x)
	{
		int cmposx = currcx2 * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz2 * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx2, currcz2,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_NE_E));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();

				g_log<<"reached goal exp ne e"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx2, currcz2,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				char comment[128];
				sprintf(comment, "exp ne e D:%d bS->D:%d(%d)", runningD+runningD3, bestS->totalD, bestS->totalD/PATHNODE_SIZE);
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_NE_E, comment));
				wt->heap.insert(&*wt->trystep.rbegin());
				g_log<<"push exp ne e totalD:"<<runningD<<"+"<<runningD3<<" = "<<(runningD+runningD3)<<endl;
				break;
			}
#if 1
			else
				wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
#endif
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
			break;
		}

		currcx2++;
		runningD3 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_NE_N(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					int runningD2,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx2 = cx;
	int currcz2 = cz - 1;
	int runningD3 = runningD2 + PATHNODE_SIZE;

	while(currcz2 >= 0)
	{
		int cmposx = currcx2 * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz2 * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx2, currcz2,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_NE_N));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				
				g_log<<"reached goal exp ne n"<<endl;
				return true;
			}

			if(NodeForcedNeighbour(currcx2, currcz2,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_NE_N));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
#if 1
			else
				wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
#endif
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
			break;
		}

		currcz2--;
		runningD3 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_SW_W(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					int runningD2,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx2 = cx - 1;
	int currcz2 = cz;
	int runningD3 = runningD2 + PATHNODE_SIZE;

	while(currcx2 >= 0)
	{
		int cmposx = currcx2 * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz2 * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx2, currcz2,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_SW_W));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp sw w"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx2, currcz2,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_SW_W));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
#if 1
			else
				wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
#endif
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
			break;
		}

		currcx2--;
		runningD3 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_SW_S(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					int runningD2,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx2 = cx;
	int currcz2 = cz + 1;
	int runningD3 = runningD2 + PATHNODE_SIZE;

	while(currcz2 < g_pathdim.y)
	{
		int cmposx = currcx2 * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz2 * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx2, currcz2,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_SW_S));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp sw s"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx2, currcz2,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_SW_S));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
#if 1
			else
				wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
#endif
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
			break;
		}

		currcz2++;
		runningD3 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_SE_E(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					int runningD2,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	int currcx2 = cx + 1;
	int currcz2 = cz;
	int runningD3 = runningD2 + PATHNODE_SIZE;

	while(currcx2 < g_pathdim.x)
	{
		int cmposx = currcx2 * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz2 * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;

		if(wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] == wt->pathcnt)
			break;

		if(NodeBlockWalkable(currcx2, currcz2,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_SE_E));
				wt->heap.insert(&*wt->trystep.rbegin());
				*reachedS = &*wt->trystep.rbegin();
				g_log<<"reached goal exp se e"<<endl;

				return true;
			}

			if(NodeForcedNeighbour(currcx2, currcz2,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_SE_E));
				wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
#if 1
			else
				wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
#endif
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
			break;
		}

		currcx2++;
		runningD3 += PATHNODE_SIZE;
	}

	return false;
}

bool Expansion_SE_S(const int cx,
					const int cz,
					const bool roaded, 
					const bool landborne, 
					const bool seaborne, 
					const bool airborne, 
					const Unit* thisu, 
					const Unit* ignoreu, 
					const Building* ignoreb,
					const UnitT* ut,
					WorkThread* wt,
					const int cstartx,
					const int cstartz,
					const int cgoalx, 
					const int cgoalz,
					PathNode* bestS,
					int runningD,
					int runningD2,
					const int cmgoalminx,
					const int cmgoalminz,
					const int cmgoalmaxx,
					const int cmgoalmaxz,
					PathNode** reachedS)
{
	PathNode* prevN = &g_pathnode[ PathNodeIndex(cx,cz) ];

	int currcx2 = cx;
	int currcz2 = cz + 1;
	int runningD3 = runningD2 + PATHNODE_SIZE;

	while(currcz2 < g_pathdim.y)
	{
		int cmposx = currcx2 * PATHNODE_SIZE + PATHNODE_SIZE/2;
		int cmposz = currcz2 * PATHNODE_SIZE + PATHNODE_SIZE/2;

		int cmminx = cmposx - ut->size.x/2;
		int cmminz = cmposz - ut->size.z/2;
		int cmmaxx = cmminx + ut->size.x;
		int cmmaxz = cmminz + ut->size.z;

		int cminx = cmminx/PATHNODE_SIZE;
		int cminz = cmminz/PATHNODE_SIZE;
		int cmaxx = cmmaxx/PATHNODE_SIZE;
		int cmaxz = cmmaxz/PATHNODE_SIZE;
		
		PathNode* n = &g_pathnode[ PathNodeIndex(currcx2,currcz2) ];

		if(wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] == wt->pathcnt)
		{
			if(n->totalD <= runningD + runningD3)
				break;
		}

		*n = PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, prevN, runningD, runningD3);

		if(NodeBlockWalkable(currcx2, currcz2,
			cminx, cminz, cmaxx, cmaxz,
			cmminx, cmminz, cmmaxx, cmmaxz,
			ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
			thisu, ignoreu, ignoreb) == COLLIDER_NONE)
		{
			// Arrived at goal area?
			if(cmmaxx > cmgoalminx && cmmaxz > cmgoalminz && cmminx < cmgoalmaxx && cmminz < cmgoalmaxz)
			{
				//wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_SE_S));
				//wt->heap.insert(&*wt->trystep.rbegin());
				//*reachedS = &*wt->trystep.rbegin();
				//g_log<<"reached goal exp se s"<<endl;
				*reachedS = n;

				return true;
			}

			if(NodeForcedNeighbour(currcx2, currcz2,
				cminx, cminz, cmaxx, cmaxz,
				cmminx, cmminz, cmmaxx, cmmaxz,
				ut->roaded, ut->landborne, ut->seaborne, ut->airborne,
				thisu, ignoreu, ignoreb))
			{
				//wt->trystep.push_back(PathNode(cstartx, cstartz, cgoalx, cgoalz, currcx2, currcz2, bestS, runningD, runningD3, EXP_SE_S));
				//wt->heap.insert(&*wt->trystep.rbegin());
				break;
			}
#if 1
			else
				wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
#endif
		}
		else
		{
			wt->triedcell[ PathNodeIndex(currcx2, currcz2) ] = wt->pathcnt;
			break;
		}

		prevN = &g_pathnode[ PathNodeIndex(currcx2,currcz2) ];
		currcz2++;
		runningD3 += PATHNODE_SIZE;
	}

	return false;
}