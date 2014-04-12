

#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
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
#include "astarpath.h"
#include "reconstructpath.h"
#include "pathdebug.h"

void AStarPath(int utype, int umode, int cmstartx, int cmstartz, int target, int target2, int targtype, 
			  list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb, 
			  int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminz, int cmgoalmaxx, int cmgoalmaxz,
			  int maxsearch)
{
	UnitT* ut = &g_unitT[utype];

	PathJob* pj = new PathJob;
	pj->utype = utype;
	pj->umode = umode;
	pj->cmstartx = cmstartx;
	pj->cmstartz = cmstartz;
	pj->target = target;
	pj->target2 = target2;
	pj->targtype = targtype;
	pj->path = path;
	pj->subgoal = subgoal;

	short thisuindex = -1;

	if(thisu)
		thisuindex = thisu - g_unit;

	pj->thisu = thisuindex;

	short ignoreuindex = -1;

	if(ignoreu)
		ignoreuindex = ignoreu - g_unit;

	pj->ignoreu = ignoreuindex;

	short ignorebindex = -1;

	if(ignoreb)
		ignorebindex = ignoreb - g_building;

	pj->ignoreb = ignorebindex;
	pj->cmgoalx = (cmgoalminx+cmgoalmaxx)/2;
	pj->cmgoalz = (cmgoalminz+cmgoalmaxz)/2;
	pj->ngoalx = pj->cmgoalx / PATHNODE_SIZE;
	pj->ngoalz = pj->cmgoalz / PATHNODE_SIZE;
	pj->cmgoalminx = cmgoalminx;
	pj->cmgoalminz = cmgoalminz;
	pj->cmgoalmaxx = cmgoalmaxx;
	pj->cmgoalmaxz = cmgoalmaxz;
	pj->roaded = ut->roaded;
	pj->landborne = ut->landborne;
	pj->seaborne = ut->seaborne;
	pj->airborne = ut->airborne;
	pj->callback = Callback_UnitPath;
	pj->pjtype = PATHJOB_ASTAR;
	pj->maxsearch = maxsearch;

	// Returns the path from location `<startX, startY>` to location `<endX, endY>`.
	//return function(finder, startNode, endNode, clearance, toClear)
	
	pj->process();
	delete pj;
}

void IdentifySuccessors_A(PathJob* pj, PathNode* node)
{
	Vec2i npos = PathNodePos(node);

	int thisdistance = Magnitude(Vec2i(npos.x - pj->ngoalx, npos.y - pj->ngoalz));

	if( !pj->closestnode || thisdistance < pj->closest )
	{
		pj->closestnode = node;
		pj->closest = thisdistance;
	}

	int runningD = 0;

	if(node->previous)
		runningD = node->previous->totalD;
	
	bool walkable_nw = Walkable(pj, npos.x - 1, npos.y - 1);
	bool walkable_n = Walkable(pj, npos.x, npos.y - 1);
	bool walkable_ne = Walkable(pj, npos.x + 1, npos.y - 1);
	bool walkable_e = Walkable(pj, npos.x + 1, npos.y);
	bool walkable_se = Walkable(pj, npos.x + 1, npos.y + 1);
	bool walkable_s = Walkable(pj, npos.x, npos.y + 1);
	bool walkable_sw = Walkable(pj, npos.x - 1, npos.y + 1);
	bool walkable_w = Walkable(pj, npos.x - 1, npos.y);
	
	int newD = runningD + PATHNODE_SIZE;

	if(walkable_w)
	{
		Vec2i nextnpos(npos.x - 1, npos.y);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);
		
		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = Manhattan( nextnpos - Vec2i(pj->ngoalx, pj->ngoalz) );
			nextn->F = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened ) 
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}
	if(walkable_e)
	{
		Vec2i nextnpos(npos.x + 1, npos.y);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);
		
		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = Manhattan( nextnpos - Vec2i(pj->ngoalx, pj->ngoalz) );
			nextn->F = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened ) 
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}
	if(walkable_n)
	{
		Vec2i nextnpos(npos.x, npos.y - 1);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);
		
		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = Manhattan( nextnpos - Vec2i(pj->ngoalx, pj->ngoalz) );
			nextn->F = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened ) 
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}
	if(walkable_s)
	{
		Vec2i nextnpos(npos.x, npos.y + 1);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);
		
		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = Manhattan( nextnpos - Vec2i(pj->ngoalx, pj->ngoalz) );
			nextn->F = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened ) 
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}

	newD = runningD + PATHNODE_DIAG;

	if(walkable_nw && walkable_n && walkable_w)
	{
		Vec2i nextnpos(npos.x - 1, npos.y - 1);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);
		
		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = Manhattan( nextnpos - Vec2i(pj->ngoalx, pj->ngoalz) );
			nextn->F = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened ) 
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}

	if(walkable_ne && walkable_n && walkable_e)
	{
		Vec2i nextnpos(npos.x + 1, npos.y);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);
		
		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = Manhattan( nextnpos - Vec2i(pj->ngoalx, pj->ngoalz) );
			nextn->F = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened ) 
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}

	if(walkable_sw && walkable_s && walkable_w)
	{
		Vec2i nextnpos(npos.x - 1, npos.y + 1);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);
		
		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = Manhattan( nextnpos - Vec2i(pj->ngoalx, pj->ngoalz) );
			nextn->F = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened ) 
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}

	if(walkable_se && walkable_s && walkable_e)
	{
		Vec2i nextnpos(npos.x + 1, npos.y + 1);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);
		
		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = Manhattan( nextnpos - Vec2i(pj->ngoalx, pj->ngoalz) );
			nextn->F = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened ) 
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}
}