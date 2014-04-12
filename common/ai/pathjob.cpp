

#include "pathjob.h"
#include "pathnode.h"
#include "collidertile.h"
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
#include "../window.h"
#include "partialpath.h"

void ClearNodes(list<PathNode*> &toclear)
{
	for(auto niter = toclear.begin(); niter != toclear.end(); niter++)
	{
		PathNode* n = *niter;
		n->opened = false;
		n->closed = false;
	}

	toclear.clear();
}

bool PathJob::process()
{
	long long frames = g_simframe - wt->lastpath;

	int delay = frames * 1000 / DRAW_FRAME_RATE;

	if(delay < PATH_DELAY)
		return false;

	wt->lastpath = g_simframe;

	wt->openlist.resetelems();

	SnapToNode(this);

	PathNode* node;
	searchdepth = 0;
	closest = 0;
	closestnode = NULL;

	while( wt->openlist.hasmore() ) 
	{
		searchdepth ++;

		if(pjtype == PATHJOB_QUICKPARTIAL && searchdepth > maxsearch)
			break;

		// Pops the lowest F-cost node, moves it in the closed list
		node = wt->openlist.deletemin();
		int i = node - wt->pathnode;

		Vec2i npos = PathNodePos(wt, node);

		node->closed = true;

		// If the popped node is the endNode, return it
		if( AtGoal(this, node) ) 
		{
			ReconstructPath(this, node);
			ClearNodes(wt->toclear);

			if(callback)
				callback(true, this);

			return true;
		}

		// otherwise, identify successors of the popped node
		if(pjtype == PATHJOB_JPS)
			IdentifySuccessors_JPS(this, node);
		else if(pjtype == PATHJOB_QUICKPARTIAL)
			IdentifySuccessors_QP(this, node);
	}

	if(pjtype == PATHJOB_QUICKPARTIAL && closestnode)
		ReconstructPath(this, closestnode);

	ClearNodes(wt->toclear);

	// No path found
	if(callback)
		callback(false, this);

	return true;
}

void Callback_UnitPath(bool result, PathJob* pj)
{
	short ui = pj->thisu;

	if(ui < 0)
		return;

	Unit* u = &g_unit[ui];

	u->threadwait = false;
	u->pathblocked = !result;
}