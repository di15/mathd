

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
#include "pathblocked.h"

void ReconstructPath(list<Vec2i> &path, PathNode* bestS, Vec2i &subgoal, int cmgoalx, int cmgoalz)
{
	// Reconstruct the path, following the path steps
	// Keep adding steps as long as it is not the first one (one without a .previous)
	//for(tS = bestS; tS->previous; tS = tS->previous)
	for(PathNode* tS = bestS; tS; tS = tS->previous)
		path.push_front(Vec2i(tS->cx * PATHNODE_SIZE + PATHNODE_SIZE/2, tS->cz * PATHNODE_SIZE + PATHNODE_SIZE/2));

#if 1
	path.push_back(Vec2i(cmgoalx, cmgoalz));
#endif

	if(path.size() > 0)
		subgoal = *path.begin();
}

void ReconstructPathJPS(list<Vec2i> &path, PathNode* bestS, Vec2i &subgoal, int cmgoalx, int cmgoalz)
{
	g_log<<"reconstruct path jps"<<endl;
	g_log.flush();

	// Reconstruct the path, following the path steps
	// Keep adding steps as long as it is not the first one (one without a .previous)
	//for(tS = bestS; tS->previous; tS = tS->previous)
	for(PathNode* tS = bestS; tS; tS = tS->previous)
	{
		path.push_front(Vec2i(tS->cx * PATHNODE_SIZE + PATHNODE_SIZE/2, tS->cz * PATHNODE_SIZE + PATHNODE_SIZE/2));

		{
			g_log<<"reverse order: "<<tS->cx<<","<<tS->cz<<" exp "<<(int)tS->expansion<<" F:"<<tS->F<<"("<<(tS->F/PATHNODE_SIZE)<<") totalD:"<<tS->totalD<<"("<<(tS->totalD/PATHNODE_SIZE)<<") comment: "<<tS->comment<<endl;
		}
		
		if(!tS->previous)
			continue;

		PathNode* prevS = tS->previous;

		if(tS->expansion == EXP_N || tS->expansion == EXP_E || tS->expansion == EXP_W || tS->expansion == EXP_S)
			continue;
		else if(tS->expansion == EXP_NW || tS->expansion == EXP_NE || tS->expansion == EXP_SW || tS->expansion == EXP_SE)
			continue;
		else if(tS->expansion == EXP_NW_N || tS->expansion == EXP_NE_N)
		{//EXP_NW_N
			int dx = abs(tS->cx - prevS->cx);

			int midcx = tS->cx;
			int midcz = prevS->cz - dx;

			path.push_front(Vec2i(midcx * PATHNODE_SIZE + PATHNODE_SIZE/2, midcz * PATHNODE_SIZE + PATHNODE_SIZE/2));
			
			{
				g_log<<"reverse order: "<<midcx<<","<<midcz<<" mid"<<endl;
			}
		}
		else if(tS->expansion == EXP_SW_S || tS->expansion == EXP_SE_S)
		{
			int dx = abs(tS->cx - prevS->cx);

			int midcx = tS->cx;
			int midcz = prevS->cz + dx;

			path.push_front(Vec2i(midcx * PATHNODE_SIZE + PATHNODE_SIZE/2, midcz * PATHNODE_SIZE + PATHNODE_SIZE/2));
			
			{
				g_log<<"reverse order: "<<midcx<<","<<midcz<<" mid"<<endl;
			}
		}
		else if(tS->expansion == EXP_NW_W || tS->expansion == EXP_SW_W)
		{
			int dz = abs(tS->cz - prevS->cz);

			int midcx = prevS->cx - dz;
			int midcz = tS->cz;

			path.push_front(Vec2i(midcx * PATHNODE_SIZE + PATHNODE_SIZE/2, midcz * PATHNODE_SIZE + PATHNODE_SIZE/2));
			
			{
				g_log<<"reverse order: "<<midcx<<","<<midcz<<" mid"<<endl;
			}
		}
		else if(tS->expansion == EXP_NE_E || tS->expansion == EXP_SE_E)
		{
			int dz = abs(tS->cz - prevS->cz);

			int midcx = prevS->cx + dz;
			int midcz = tS->cz;

			path.push_front(Vec2i(midcx * PATHNODE_SIZE + PATHNODE_SIZE/2, midcz * PATHNODE_SIZE + PATHNODE_SIZE/2));
			
			{
				g_log<<"reverse order: "<<midcx<<","<<midcz<<" mid"<<endl;
			}
		}
	}

#if 1
	path.push_back(Vec2i(cmgoalx, cmgoalz));
#endif

	if(path.size() > 0)
		subgoal = *path.begin();
}