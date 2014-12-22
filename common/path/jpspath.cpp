#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../sim/road.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../phys/trace.h"
#include "binheap.h"
#include "jpspath.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "jpsexpansion.h"
#include "pathjob.h"
#include "../render/transaction.h"

// Calculates a path.
void JPSPath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype, int cdtype,
             std::list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb,
             int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
			 int nminx, int nminy, int nmaxx, int nmaxy, bool bounded)
{
	UType* ut = &g_utype[utype];

	if(thisu)
	{
		RichText rt("pathf");
		NewTransx(thisu->drawpos, &rt);
	}
	
	if(!ignoreb)
	{
		switch(umode)
		{
		case UMODE_GODEMB:
		case UMODE_ATDEMB:
		case UMODE_GOBLJOB:
		case UMODE_BLJOB:
		case UMODE_GOCSTJOB:
		case UMODE_CSTJOB:
		case UMODE_GOSHOP:
		case UMODE_SHOPPING:
		case UMODE_GOREST:
		case UMODE_RESTING:
			ignoreb = &g_building[target];
			break;
		case UMODE_GOSUP:
		case UMODE_ATSUP:
			if(thisu)
				ignoreb = &g_building[thisu->supplier];
			break;
		case UMODE_GOREFUEL:
		case UMODE_REFUELING:
			if(thisu)
				ignoreb = &g_building[thisu->fuelstation];
			break;
		case UMODE_NONE:
		case UMODE_GOCDJOB:
		case UMODE_CDJOB:
		case UMODE_GODRIVE:
		case UMODE_DRIVE:
		case UMODE_GODEMCD:
		case UMODE_ATDEMCD:
			break;
		}
	}

	if(!ignoreu)
	{
		switch(umode)
		{
		case UMODE_GOSUP:
		case UMODE_GODEMB:
		case UMODE_GOREFUEL:
		case UMODE_REFUELING:
		case UMODE_ATDEMB:
		case UMODE_ATSUP:
		case UMODE_GOBLJOB:
		case UMODE_BLJOB:
		case UMODE_GOCSTJOB:
		case UMODE_CSTJOB:
		case UMODE_GOSHOP:
		case UMODE_SHOPPING:
		case UMODE_GOREST:
		case UMODE_RESTING:
		case UMODE_NONE:
		case UMODE_GOCDJOB:
		case UMODE_CDJOB:
		case UMODE_GODEMCD:
		case UMODE_ATDEMCD:
			break;
		case UMODE_GODRIVE:
		case UMODE_DRIVE:
			ignoreu = &g_unit[target];
			break;
		}
	}

	//could be cleaner, not rely on thisu->cdtype
	//if(targtype == TARG_CD && thisu)
	// TO DO
	if((umode == UMODE_GODEMCD ||
		umode == UMODE_ATDEMCD ||
		umode == UMODE_GOCDJOB) &&
		thisu)
	{
		CdType* ct = &g_cdtype[cdtype];
		//...and not muck around with cmgoalmin/max
		CdTile* ctile = GetCd(thisu->cdtype, target, target2, false);
		Vec2i ccmpos = Vec2i(target, target2)*TILE_SIZE + ct->physoff;
		cmgoalminx = ccmpos.x - TILE_SIZE/2;
		cmgoalminy = ccmpos.y - TILE_SIZE/2;
		cmgoalmaxx = cmgoalminx + TILE_SIZE;
		cmgoalmaxy = cmgoalminy + TILE_SIZE;
	}
	
#if 0
	if((umode == UMODE_GOSUP ||
		umode == UMODE_GODEMB ||
		umode == UMODE_GOREFUEL) &&
		thisu)
	{
		Building* b;
		
		if(umode == UMODE_GOSUP)
			b = &g_building[thisu->supplier];
		else if(umode == UMODE_GODEMB)
			b = &g_building[thisu->target];
		else if(umode == UMODE_GOREFUEL)
			b = &g_building[thisu->fuelstation];

		BlType* bt = &g_bltype[b->type];

		int btminx = b->tilepos.x - bt->widthx/2;
		int btminz = b->tilepos.y - bt->widthy/2;
		int btmaxx = btminx + bt->widthx - 1;
		int btmaxz = btminz + bt->widthy - 1;

		cmgoalminx = btminx * TILE_SIZE;
		cmgoalminy = btminz * TILE_SIZE;
		cmgoalmaxx = cmgoalminx + bt->widthx*TILE_SIZE - 1;
		cmgoalmaxy = cmgoalminy + bt->widthy*TILE_SIZE - 1;
	}
#endif

	PathJob* pj = new PathJob;
	pj->utype = utype;
	pj->umode = umode;
	pj->cmstartx = cmstartx;
	pj->cmstarty = cmstarty;
	pj->target = target;
	pj->target2 = target2;
	pj->targtype = targtype;
	pj->path = path;
	pj->subgoal = subgoal;
	pj->thisu = thisu ? thisu - g_unit : -1;
	pj->ignoreu = ignoreu ? ignoreu - g_unit : -1;
	pj->ignoreb = ignoreb ? ignoreb - g_building : -1;
	//pj->goalx = (cmgoalminx+cmgoalmaxx)/2;
	//pj->goalz = (cmgoalminy+cmgoalmaxy)/2;
	pj->goalx = cmgoalx;
	pj->goalz = cmgoalz;
	pj->goalx = pj->goalx / PATHNODE_SIZE;
	pj->goalz = pj->goalz / PATHNODE_SIZE;
	pj->goalminx = cmgoalminx;
	pj->goalminy = cmgoalminy;
	pj->goalmaxx = cmgoalmaxx;
	pj->goalmaxy = cmgoalmaxy;
	pj->roaded = ut->roaded;
	pj->landborne = ut->landborne;
	pj->seaborne = ut->seaborne;
	pj->airborne = ut->airborne;
	pj->callback = Callback_UnitPath;
	pj->pjtype = bounded ? PATHJOB_BOUNDJPS : PATHJOB_JPS;
	pj->cdtype = cdtype;
	pj->nminx = nminx;
	pj->nminy = nminy;
	pj->nmaxx = nmaxx;
	pj->nmaxy = nmaxy;
	pj->cmgoal = Vec2i(cmgoalx, cmgoalz);

	// Returns the path from location `<startX, startY>` to location `<endX, endY>`.
	//return function(finder, startNode, endNode, clearance, toClear)

	pj->process();
	delete pj;
}
