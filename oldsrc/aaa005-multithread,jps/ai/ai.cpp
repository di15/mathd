

#include "main.h"
#include "ai.h"
#include "unit.h"
#include "building.h"
#include "player.h"
#include "resource.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "physics.h"
#include "pathfinding.h"
#include "chat.h"

vector<int> g_nearb;

bool AIPlotPipe(int owner, int startBui, int endBui)
{
	g_tryStep.clear();

	const float tileSize = TILE_SIZE;
	CBuilding* startB = &g_building[startBui];
	const Vec3f pos = startB->pos;
	CBuilding* endB = &g_building[endBui];
	const Vec3f goal = endB->pos;

	Vec3f vTile = pos;
	CBuildingType* t = &g_buildingType[startB->type];
	if(t->widthX%2 == 1)
		vTile.x += 0.5f*TILE_SIZE;
	if(t->widthZ%2 == 1)
		vTile.z += 0.5f*TILE_SIZE;
	//int startBstartx = vTile.x/TILE_SIZE - t->widthX/2 - 2;
	int startBstartx = vTile.x/TILE_SIZE - t->widthX/2 - 1;
	int startBendx = startBstartx + t->widthX;
	int startBstartz = vTile.z/TILE_SIZE - t->widthZ/2;
	int startBendz = startBstartz + t->widthZ;
	/*
	vTile = goal;
	t = &g_buildingType[endB->type];
	if(t->widthX%2 == 1)
		vTile.x += 0.5f*TILE_SIZE;
	if(t->widthZ%2 == 1)
		vTile.z += 0.5f*TILE_SIZE;
	int endBstartx = vTile.x/TILE_SIZE - t->widthX/2 - 1;
	int endBendx = endBstartx + t->widthX - 1;
	int endBstartz = vTile.z/TILE_SIZE - t->widthZ/2;
	int endBendz = endBstartz + t->widthZ - 1;*/

	const int startx = pos.x / tileSize;
	const int startz = pos.z / tileSize;
	const int endx = goal.x / tileSize;
	const int endz = goal.z / tileSize;

	//for(int x=0; x<g_hmap.m_widthX; x++)
	//	for(int z=0; z<g_hmap.m_widthZ; z++)
	//		OpenAt(x, z) = true;

	g_curpath ++;

	//g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz, -1));
	//if(startBstartz > 0)
	{
		int z=startBstartz;
		for(int x=max(0,startBstartx); x<=min(startBendx,g_hmap.m_widthX-1); x++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
	
	//if(startBendx < g_hmap.m_widthX)
	{
		int x=startBendx;
		for(int z=startBstartz; z<=min(startBendz,g_hmap.m_widthZ-1); z++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
	
	//if(startBendz < g_hmap.m_widthZ)
	{
		int z=startBendz;
		for(int x=min(startBendx,g_hmap.m_widthX-1); x>=max(0,startBstartx); x--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}

	if(startBstartx >= 0)
	{
		int x=startBstartx;
		for(int z=min(startBendz,g_hmap.m_widthZ-1); z>=startBstartz; z--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}

	int i;
	int best;
	CTryStep* tS;
	CTryStep* bestS;

	int searchdepth = 0;

	//g_log<<"aiplotpow1 "<<owner<<endl;
	//g_log.flush();

	//Chat("aiplotroad1");

	while(true)
	{
		//searchdepth ++;

		//if(searchdepth > MAX_SEARCH_DEPTH)
		{
			//Chat("rpath failed1");
		//	return false;
		}

		best = -1;
		bestS = NULL;

		for(i=0; i<g_tryStep.size(); i++)
		{
			tS = &g_tryStep[i];
			//if(!OpenAt(tS->x, tS->z))
			if(OpenAt(tS->x, tS->z) == g_curpath)
				continue;

			if(bestS != NULL && tS->F > bestS->F)
				continue;

			bestS = tS;
			best = i;
		}

		if(best < 0)
		{
			//Chat("rpath failed2");
	//Chat("powpath failed2");
			return false;
		}
		
		//OpenAt(bestS->x, bestS->z) = false;
		OpenAt(bestS->x, bestS->z) = g_curpath;

		//Vec3f vBestS = TryStepToPosition(bestS->x, bestS->z);

		/*
		if(vBestS.y <= WATER_HEIGHT)
			continue;
		
		if(bestS->previous >= 0 && !TryStep(vBestS, ignoreUnit, ignoreBuilding, roadVehicle))
			continue;*/
		
		if(!PipeHasRoad(bestS->x, bestS->z))
			continue;

		bool reached = false;

		if(!PipelinePlaceable(bestS->x, bestS->z))
		{
			if(g_colliderType == COLLIDER_BUILDING)
			{
				CBuilding* b = &g_building[g_lastCollider];

				if(!g_diplomacy[b->owner][owner])
					continue;

				if(b->pipenetw == endB->pipenetw && endB->pipenetw >= 0)
					reached = true;
			}
			else
				continue;
		}
		/*
		if(bestS->previous < 0)
		{
			if(!RoadLevel(bestS->x, bestS->z, bestS->x, bestS->z, 0, 0, 0, 0, NULL))
				continue;
		}
		else
		//if(bestS->previous >= 0)
		{
			tS = &g_tryStep[bestS->previous];
			float dx = bestS->x - tS->x;
			float dz = bestS->z - tS->z;

			if(!RoadLevel(bestS->x, bestS->z, bestS->x, bestS->z, dx, dz, 1, 1, NULL))
				continue;
		}*/

		CPipeline* pipehere = PipeAt(bestS->x, bestS->z);

		if(pipehere->on)
		{
			if(pipehere->netw == endB->pipenetw && endB->pipenetw >= 0)
			{
				//g_log<<"reached1"<<endl;
				//g_log.flush();
	//Chat("reached1");
				reached = true;
			}
		}

		/*
		//if(endB->roadnetw.size() <= 0)
		{
			if(bestS->x >= endBstartx-1 && bestS->x <= endBendx && bestS->z == endBstartz)
			{
						//g_log<<"reached2"<<endl;
						//g_log.flush();
	//Chat("reached2");
				reached = true;
			}
		
			if(bestS->x >= endBstartx-1 && bestS->x <= endBendx && bestS->z == endBendz+1)
			{
						//g_log<<"reached3"<<endl;
						//g_log.flush();
	//Chat("reached3");
				reached = true;
			}
		
			if(bestS->x == endBstartx-1 && bestS->z >= endBstartz && bestS->z <= endBendz+1)
			{
						//g_log<<"reached4"<<endl;
						//g_log.flush();
	//Chat("reached4");
				reached = true;
			}
		
			if(bestS->x == endBendx+1 && bestS->z >= endBstartz && bestS->z <= endBendz+1)
			{
						//g_log<<"reached5"<<endl;
						//g_log.flush();
	//Chat("reached5");
				reached = true;
			}
		}
		*/

		if(PipelineAdjacent(endBui, bestS->x, bestS->z))
			reached = true;

		if(reached)
		{
			tS = bestS;
			deque<CTryStep> path;

			for(tS=bestS; true; tS=&g_tryStep[tS->previous])
			{
				path.push_front(*tS);

				if(tS->previous < 0)
					break;
			}

			//g_log<<"plotting pow length="<<path.size()<<endl;
			//g_log.flush();
			//char msg[128];
			//sprintf(msg, "plotting pow length=%d", (int)path.size());
	//Chat(msg);

			for(int i=0; i<path.size(); i++)
			{
				
			//g_log<<"plotting pow at "<<path[i].x<<","<<path[i].z<<endl;
			//g_log.flush();

				
			//sprintf(msg, "plotting pow at %d,%d", (int)path[i].x, (int)path[i].z);
	//Chat(msg);

				PlacePipeline(path[i].x, path[i].z, owner, false);

				//g_log<<"plot pipe "<<path[i].x<<","<<path[i].z<<endl;
				//g_log.flush();
			}

			
			//g_log<<"repowing"<<endl;
			//g_log.flush();

			if(path.size() > 0)
				RePipe();
				
			//g_log<<"repowed"<<endl;
			//g_log.flush();

			return true;
		}

		
		float runningD = 0;
		float stepD = 1;

		if(bestS->previous >= 0)
		{
			tS = &g_tryStep[bestS->previous];
			runningD = tS->runningD;
		}

		if(bestS->x > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x - 1, bestS->z, best, runningD, stepD));
		if(bestS->x < g_hmap.m_widthX-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x + 1, bestS->z, best, runningD, stepD));
		if(bestS->z > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z - 1, best, runningD, stepD));
		if(bestS->z < g_hmap.m_widthZ-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z + 1, best, runningD, stepD));
	}

	//Chat("rpath failed3");
	//Chat("powpath failed3");
	return false;
}

bool AIPlotPowl(int owner, int startBui, int endBui)
{
	g_tryStep.clear();

	const float tileSize = TILE_SIZE;
	CBuilding* startB = &g_building[startBui];
	const Vec3f pos = startB->pos;
	CBuilding* endB = &g_building[endBui];
	const Vec3f goal = endB->pos;

	Vec3f vTile = pos;
	CBuildingType* t = &g_buildingType[startB->type];
	if(t->widthX%2 == 1)
		vTile.x += 0.5f*TILE_SIZE;
	if(t->widthZ%2 == 1)
		vTile.z += 0.5f*TILE_SIZE;
	int startBstartx = vTile.x/TILE_SIZE - t->widthX/2;
	int startBendx = startBstartx + t->widthX;
	int startBstartz = vTile.z/TILE_SIZE - t->widthZ/2;
	int startBendz = startBstartz + t->widthZ;
	/*
	vTile = goal;
	t = &g_buildingType[endB->type];
	if(t->widthX%2 == 1)
		vTile.x += 0.5f*TILE_SIZE;
	if(t->widthZ%2 == 1)
		vTile.z += 0.5f*TILE_SIZE;
	int endBstartx = vTile.x/TILE_SIZE - t->widthX/2 - 1;
	int endBendx = endBstartx + t->widthX;
	int endBstartz = vTile.z/TILE_SIZE - t->widthZ/2 - 1;
	int endBendz = endBstartz + t->widthZ;
	*/
	const int startx = pos.x / tileSize;
	const int startz = pos.z / tileSize;
	const int endx = goal.x / tileSize;
	const int endz = goal.z / tileSize;

	//for(int x=0; x<g_hmap.m_widthX; x++)
	//	for(int z=0; z<g_hmap.m_widthZ; z++)
	//		OpenAt(x, z) = true;

	g_curpath ++;

	//g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz, -1));
	//if(startBstartz > 0)
	{
		int z=startBstartz;
		for(int x=startBstartx; x<=min(startBendx,g_hmap.m_widthX-1); x++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
	
	if(startBendx < g_hmap.m_widthX)
	{
		int x=startBendx;
		for(int z=startBstartz; z<=min(startBendz,g_hmap.m_widthZ-1); z++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
	
	if(startBendz < g_hmap.m_widthZ)
	{
		int z=startBendz;
		for(int x=min(startBendx,g_hmap.m_widthX-1); x>=startBstartx; x--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}

	//if(startBstartx > 0)
	{
		int x=startBstartx;
		for(int z=min(startBendz,g_hmap.m_widthZ-1); z>=startBstartz; z--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}

	int i;
	int best;
	CTryStep* tS;
	CTryStep* bestS;

	int searchdepth = 0;

	//g_log<<"aiplotpow1 "<<owner<<endl;
	//g_log.flush();

	//Chat("aiplotroad1");

	while(true)
	{
		//searchdepth ++;

		//if(searchdepth > MAX_SEARCH_DEPTH)
		{
			//Chat("rpath failed1");
		//	return false;
		}

		best = -1;
		bestS = NULL;

		for(i=0; i<g_tryStep.size(); i++)
		{
			tS = &g_tryStep[i];
			//if(!OpenAt(tS->x, tS->z))
			if(OpenAt(tS->x, tS->z) == g_curpath)
				continue;

			if(bestS != NULL && tS->F > bestS->F)
				continue;

			bestS = tS;
			best = i;
		}

		if(best < 0)
		{
			//Chat("rpath failed2");
	//Chat("powpath failed2");
			return false;
		}
		
		//OpenAt(bestS->x, bestS->z) = false;
		OpenAt(bestS->x, bestS->z) = g_curpath;

		//Vec3f vBestS = TryStepToPosition(bestS->x, bestS->z);

		/*
		if(vBestS.y <= WATER_HEIGHT)
			continue;
		
		if(bestS->previous >= 0 && !TryStep(vBestS, ignoreUnit, ignoreBuilding, roadVehicle))
			continue;*/

		if(!PowlHasRoad(bestS->x, bestS->z))
			continue;
		
		bool reached = false;

		if(!PowerlinePlaceable(bestS->x, bestS->z))
		{
			if(g_colliderType == COLLIDER_BUILDING)
			{
				CBuilding* b = &g_building[g_lastCollider];

				if(!g_diplomacy[b->owner][owner])
					continue;

				if(b->pownetw == endB->pownetw && endB->pownetw >= 0)
					reached = true;
			}
			else
				continue;
		}
		/*
		if(bestS->previous < 0)
		{
			if(!RoadLevel(bestS->x, bestS->z, bestS->x, bestS->z, 0, 0, 0, 0, NULL))
				continue;
		}
		else
		//if(bestS->previous >= 0)
		{
			tS = &g_tryStep[bestS->previous];
			float dx = bestS->x - tS->x;
			float dz = bestS->z - tS->z;

			if(!RoadLevel(bestS->x, bestS->z, bestS->x, bestS->z, dx, dz, 1, 1, NULL))
				continue;
		}*/

		CPowerline* powlhere = PowlAt(bestS->x, bestS->z);

		if(powlhere->on)
		{
			if(powlhere->netw == endB->pownetw && endB->pownetw >= 0)
			{
				//g_log<<"reached1"<<endl;
				//g_log.flush();
	//Chat("reached1");
				reached = true;
			}
		}
		/*
		//if(endB->roadnetw.size() <= 0)
		{
			if(bestS->x >= endBstartx && bestS->x <= endBendx+1 && bestS->z == endBstartz)
			{
						//g_log<<"reached2"<<endl;
						//g_log.flush();
	//Chat("reached2");
				reached = true;
			}
		
			if(bestS->x >= endBstartx && bestS->x <= endBendx+1 && bestS->z == endBendz+1)
			{
						//g_log<<"reached3"<<endl;
						//g_log.flush();
	//Chat("reached3");
				reached = true;
			}
		
			if(bestS->x == endBstartx-1 && bestS->z >= endBstartz && bestS->z <= endBendz+1)
			{
						//g_log<<"reached4"<<endl;
						//g_log.flush();
	//Chat("reached4");
				reached = true;
			}
		
			if(bestS->x == endBendx+1 && bestS->z >= endBstartz && bestS->z <= endBendz+1)
			{
						//g_log<<"reached5"<<endl;
						//g_log.flush();
	//Chat("reached5");
				reached = true;
			}
		}
		*/
		if(PowerlineAdjacent(endBui, bestS->x, bestS->z))
			reached = true;

		if(reached)
		{
			tS = bestS;
			deque<CTryStep> path;

			for(tS=bestS; true; tS=&g_tryStep[tS->previous])
			{
				path.push_front(*tS);

				if(tS->previous < 0)
					break;
			}

			//g_log<<"plotting pow length="<<path.size()<<endl;
			//g_log.flush();
			//char msg[128];
			//sprintf(msg, "plotting pow length=%d", (int)path.size());
	//Chat(msg);

			for(int i=0; i<path.size(); i++)
			{
				
			//g_log<<"plotting pow at "<<path[i].x<<","<<path[i].z<<endl;
			//g_log.flush();

				
			//sprintf(msg, "plotting pow at %d,%d", (int)path[i].x, (int)path[i].z);
	//Chat(msg);

				PlacePowerline(path[i].x, path[i].z, owner, false);
			}

			
			//g_log<<"repowing"<<endl;
			//g_log.flush();

			if(path.size() > 0)
				RePow();
				
			//g_log<<"repowed"<<endl;
			//g_log.flush();

			return true;
		}

		
		float runningD = 0;
		float stepD = 1;

		if(bestS->previous >= 0)
		{
			tS = &g_tryStep[bestS->previous];
			runningD = tS->runningD;
		}

		if(bestS->x > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x - 1, bestS->z, best, runningD, stepD));
		if(bestS->x < g_hmap.m_widthX-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x + 1, bestS->z, best, runningD, stepD));
		if(bestS->z > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z - 1, best, runningD, stepD));
		if(bestS->z < g_hmap.m_widthZ-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z + 1, best, runningD, stepD));
	}

	//Chat("rpath failed3");
	//Chat("powpath failed3");
	return false;
}


bool AIPlotRoad(int owner, int startBui, int endBui)
{	
	const float tileSize = TILE_SIZE;
	CBuilding* startB = &g_building[startBui];
	const Vec3f pos = startB->pos;
	CBuilding* endB = &g_building[endBui];
	const Vec3f goal = endB->pos;

	Vec3f vTile = pos;
	CBuildingType* t = &g_buildingType[startB->type];
	if(t->widthX%2 == 1)
		vTile.x += 0.5f*TILE_SIZE;
	if(t->widthZ%2 == 1)
		vTile.z += 0.5f*TILE_SIZE;
	int startBstartx = vTile.x/TILE_SIZE - t->widthX/2 - 1;
	int startBendx = startBstartx + t->widthX;
	int startBstartz = vTile.z/TILE_SIZE - t->widthZ/2 - 1;
	int startBendz = startBstartz + t->widthZ;

	return AIPlotRoad(owner, startBstartx, startBendx, startBstartz, startBendz, pos, endBui);
}

bool AIPlotRoad(int owner, int startBstartx, int startBendx, int startBstartz, int startBendz,  Vec3f pos, int endBui)
{	
	const float tileSize = TILE_SIZE;
	CBuilding* endB = &g_building[endBui];
	const Vec3f goal = endB->pos;

	Vec3f vTile = goal;
	CBuildingType* t = &g_buildingType[endB->type];
	if(t->widthX%2 == 1)
		vTile.x += 0.5f*TILE_SIZE;
	if(t->widthZ%2 == 1)
		vTile.z += 0.5f*TILE_SIZE;
	int endBstartx = vTile.x/TILE_SIZE - t->widthX/2 - 1;
	int endBendx = endBstartx + t->widthX;
	int endBstartz = vTile.z/TILE_SIZE - t->widthZ/2 - 1;
	int endBendz = endBstartz + t->widthZ;

//	return AIPlotRoad(owner, startBstartx, startBendx, startBstartz, startBendz, endBstartx, endBendx, endBstartz, endBendz, pos, goal);
//}

//bool AIPlotRoad(int owner, int startBstartx, int startBendx, int startBstartz, int startBendz, int endBstartx, int endBendx, int endBstartz, int endBendz, Vec3f pos, Vec3f goal)
//{
	g_tryStep.clear();

	//const float tileSize = TILE_SIZE;

	const int startx = pos.x / tileSize;
	const int startz = pos.z / tileSize;
	const int endx = goal.x / tileSize;
	const int endz = goal.z / tileSize;

	//for(int x=0; x<g_hmap.m_widthX; x++)
	//	for(int z=0; z<g_hmap.m_widthZ; z++)
	//		OpenAt(x, z) = true;

	g_curpath ++;

	//g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz, -1));
	if(startBstartz >= 0)
	{
		int z=startBstartz;
		for(int x=startBstartx+1; x<startBendx+1; x++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
	
	if(startBendx+1 < g_hmap.m_widthX)
	{
		int x=startBendx+1;
		for(int z=startBstartz+1; z<startBendz+1; z++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
	
	if(startBendz+1 < g_hmap.m_widthZ)
	{
		int z=startBendz+1;
		for(int x=startBendx; x>startBstartx; x--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}

	if(startBstartx >= 0)
	{
		int x=startBstartx;
		for(int z=startBendz; z>startBstartz; z--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}

	int i;
	int best;
	CTryStep* tS;
	CTryStep* bestS;

	int searchdepth = 0;

	//g_log<<"aiplotroad1 "<<owner<<endl;
	//g_log.flush();

	while(true)
	{
		searchdepth ++;

		//if(searchdepth > MAX_SEARCH_DEPTH)
		{
			//Chat("rpath failed1");
		//	return false;
		}

		best = -1;
		bestS = NULL;

		for(i=0; i<g_tryStep.size(); i++)
		{
			tS = &g_tryStep[i];
			//if(!OpenAt(tS->x, tS->z))
			if(OpenAt(tS->x, tS->z) == g_curpath)
				continue;

			if(bestS != NULL && tS->F > bestS->F)
				continue;

			bestS = tS;
			best = i;
		}

		if(best < 0)
		{
			//Chat("rpath failed2");
			return false;
		}
		
		//OpenAt(bestS->x, bestS->z) = false;
		OpenAt(bestS->x, bestS->z) = g_curpath;

		//Vec3f vBestS = TryStepToPosition(bestS->x, bestS->z);

		/*
		if(vBestS.y <= WATER_HEIGHT)
			continue;
		
		if(bestS->previous >= 0 && !TryStep(vBestS, ignoreUnit, ignoreBuilding, roadVehicle))
			continue;*/

		if(!RoadPlaceable(bestS->x, bestS->z))
		{
			/*
			if(g_colliderType = COLLIDER_BUILDING)
			{
				if(g_lastCollider != startBui && g_lastCollider != endBui)
					continue;
				else if(g_lastCollider == endBui && bestS->previous < 0)
					continue;
			}
			else
				continue;*/
			continue;
		}

		if(bestS->previous < 0)
		{
			if(!RoadLevel(bestS->x, bestS->z, bestS->x, bestS->z, 0, 0, 0, 0, NULL))
				continue;
		}
		else
		//if(bestS->previous >= 0)
		{
			tS = &g_tryStep[bestS->previous];
			float dx = bestS->x - tS->x;
			float dz = bestS->z - tS->z;

			if(!RoadLevel(bestS->x, bestS->z, bestS->x, bestS->z, dx, dz, 1, 1, NULL))
				continue;
		}

		bool reached = false;

		vector<Vec2i> roadsaround;

		roadsaround.push_back(Vec2i(bestS->x, bestS->z));

		if(bestS->x > 0)
			roadsaround.push_back(Vec2i(bestS->x-1, bestS->z));
		if(bestS->x+1 < g_hmap.m_widthX)
			roadsaround.push_back(Vec2i(bestS->x+1, bestS->z));
		if(bestS->z > 0)
			roadsaround.push_back(Vec2i(bestS->x, bestS->z-1));
		if(bestS->z+1 < g_hmap.m_widthZ)
			roadsaround.push_back(Vec2i(bestS->x, bestS->z+1));

		for(int j=0; j<roadsaround.size(); j++)
		{
			CRoad* roadhere = RoadAt(roadsaround[j].x, roadsaround[j].y);

			if(roadhere->on)
			{
				for(int i=0; i<endB->roadnetw.size(); i++)
				{
					if(endB->roadnetw[i] == roadhere->netw && roadhere->netw >= 0)
					{
						//g_log<<"reached1"<<endl;
						//g_log.flush();
						reached = true;
						break;
					}
				}
			}
			if(reached)
				break;
		}

		if(endB->roadnetw.size() <= 0)
		{
			if(bestS->x > endBstartx && bestS->x <= endBendx && bestS->z == endBstartz)
			{
						//g_log<<"reached2"<<endl;
						//g_log.flush();
				reached = true;
			}
		
			if(bestS->x > endBstartx && bestS->x <= endBendx && bestS->z == endBendz+1)
			{
						//g_log<<"reached3"<<endl;
						//g_log.flush();
				reached = true;
			}
		
			if(bestS->x == endBstartx && bestS->z > endBstartz && bestS->z <= endBendz)
			{
						//g_log<<"reached4"<<endl;
						//g_log.flush();
				reached = true;
			}
		
			if(bestS->x == endBendx+1 && bestS->z > endBstartz && bestS->z <= endBendz)
			{
						//g_log<<"reached5"<<endl;
						//g_log.flush();
				reached = true;
			}
		}

		if(reached)
		{
			tS = bestS;
			deque<CTryStep> path;

			for(tS=bestS; true; tS=&g_tryStep[tS->previous])
			{
				path.push_front(*tS);

				if(tS->previous < 0)
					break;
			}

			//g_log<<"plotting road length="<<path.size()<<endl;
			//g_log.flush();

			for(int i=0; i<path.size(); i++)
			{
				
			//g_log<<"plotting road at "<<path[i].x<<","<<path[i].z<<endl;
			//g_log.flush();

				PlaceRoad(path[i].x, path[i].z, owner, false);
			}

			if(path.size() > 0)
				ReRoadNetw();

			return true;
		}

		
		float runningD = 0;
		float stepD = 1;

		if(bestS->previous >= 0)
		{
			tS = &g_tryStep[bestS->previous];
			runningD = tS->runningD;
		}

		if(bestS->x > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x - 1, bestS->z, best, runningD, stepD));
		if(bestS->x < g_hmap.m_widthX-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x + 1, bestS->z, best, runningD, stepD));
		if(bestS->z > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z - 1, best, runningD, stepD));
		if(bestS->z < g_hmap.m_widthZ-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z + 1, best, runningD, stepD));
	}

	//Chat("rpath failed3");
	return false;
}

//all constructions finished?
bool AllConstF()
{
	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			return false;
	}

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CRoad* r = RoadAt(x, z);

			//if(r->on && !r->finished)
			//	return false;

			CPowerline* pow = PowlAt(x, z);

			///if(pow->on && !pow->finished)
			//	return false;

			CPipeline* pipe = PipeAt(x, z);

			//if(pipe->on && !pipe->finished)
			//	return false;
		}

	return true;
}

void BuiltStuff(int player)
{
	CPlayer* p = &g_player[player];
	int framesago = p->bbframesago;

	for(int i=0; i<PLAYERS; i++)
	{
		if(i == g_localP)
			continue;

		if(!g_diplomacy[player][i])
			continue;

		g_player[i].bbframesago -= framesago;

		if(i == player)
			g_player[i].bbframesago -= 2;
	}
}

int PlaceBAround(int player, int btype, Vec3f vAround)
{
	//g_log<<"PlaceBAround "<<player<<endl;
	//g_log.flush();

	CBuildingType* t = &g_buildingType[btype];
	int shell = 1;
	
	//char msg[128];
	//sprintf(msg, "place b a %f,%f,%f", vAround.x/16, vAround.y/16, vAround.z/16);
	//Chat(msg);

	do
	{
		vector<Vec3f> canplace;
		Vec3f vTile;
		int tilex, tilez;
		int left, right, top, bottom;
		left = vAround.x/TILE_SIZE - shell;
		top = vAround.z/TILE_SIZE - shell;
		right = vAround.x/TILE_SIZE + shell;
		bottom = vAround.z/TILE_SIZE + shell;

		tilez = top;
		for(tilex=left; tilex<right; tilex++)
		{
			vTile = Vec3f(tilex*TILE_SIZE, 0, tilez*TILE_SIZE);
			if(t->widthX%2 == 1)
				vTile.x += 0.5f*TILE_SIZE;
			if(t->widthZ%2 == 1)
				vTile.z += 0.5f*TILE_SIZE;
			int startx = vTile.x/TILE_SIZE - t->widthX/2;
			int endx = startx + t->widthX - 1;
			int startz = vTile.z/TILE_SIZE - t->widthZ/2;
			int endz = startz + t->widthZ - 1;

			if(startx < 0)
				continue;
			else if(endx >= g_hmap.m_widthX)
				continue;
			if(startz < 0)
				continue;
			else if(endz >= g_hmap.m_widthZ)
				continue;
			
			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, vTile))
				continue;
			canplace.push_back(vTile);
		}
		
		tilex = right;
		for(tilez=top; tilez<bottom; tilez++)
		{
			vTile = Vec3f(tilex*TILE_SIZE, 0, tilez*TILE_SIZE);
			if(t->widthX%2 == 1)
				vTile.x += 0.5f*TILE_SIZE;
			if(t->widthZ%2 == 1)
				vTile.z += 0.5f*TILE_SIZE;
			int startx = vTile.x/TILE_SIZE - t->widthX/2;
			int endx = startx + t->widthX - 1;
			int startz = vTile.z/TILE_SIZE - t->widthZ/2;
			int endz = startz + t->widthZ - 1;

			if(startx < 0)
				continue;
			else if(endx >= g_hmap.m_widthX)
				continue;
			if(startz < 0)
				continue;
			else if(endz >= g_hmap.m_widthZ)
				continue;
			
			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, vTile))
				continue;
			canplace.push_back(vTile);
		}

		tilez = bottom;
		for(tilex=right; tilex>left; tilex--)
		{
			vTile = Vec3f(tilex*TILE_SIZE, 0, tilez*TILE_SIZE);
			if(t->widthX%2 == 1)
				vTile.x += 0.5f*TILE_SIZE;
			if(t->widthZ%2 == 1)
				vTile.z += 0.5f*TILE_SIZE;
			int startx = vTile.x/TILE_SIZE - t->widthX/2;
			int endx = startx + t->widthX - 1;
			int startz = vTile.z/TILE_SIZE - t->widthZ/2;
			int endz = startz + t->widthZ - 1;

			if(startx < 0)
				continue;
			else if(endx >= g_hmap.m_widthX)
				continue;
			if(startz < 0)
				continue;
			else if(endz >= g_hmap.m_widthZ)
				continue;
			
			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, vTile))
				continue;
			canplace.push_back(vTile);
		}

		tilex = left;
		for(tilez=bottom; tilez>top; tilez--)
		{
			vTile = Vec3f(tilex*TILE_SIZE, 0, tilez*TILE_SIZE);
			if(t->widthX%2 == 1)
				vTile.x += 0.5f*TILE_SIZE;
			if(t->widthZ%2 == 1)
				vTile.z += 0.5f*TILE_SIZE;
			int startx = vTile.x/TILE_SIZE - t->widthX/2;
			int endx = startx + t->widthX - 1;
			int startz = vTile.z/TILE_SIZE - t->widthZ/2;
			int endz = startz + t->widthZ - 1;
			

			if(startx < 0)
				continue;
			else if(endx >= g_hmap.m_widthX)
				continue;
			if(startz < 0)
				continue;
			else if(endz >= g_hmap.m_widthZ)
				continue;
			
			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, vTile))
				continue;
			canplace.push_back(vTile);
		}

		if(canplace.size() > 0)
		{
			//Chat("placing");
			//g_log<<"placeb t="<<btype<<" "<<vTile.x<<","<<vTile.y<<","<<vTile.z<<"("<<(vTile.x/16)<<","<<(vTile.y/16)<<","<<(vTile.z/16)<<")"<<endl;
			//g_log.flush();
			vTile = canplace[ rand()%canplace.size() ];
			vTile.y = Bilerp(vTile.x, vTile.z);
			int ID = PlaceB(player, btype, vTile);

			return ID;
		}

		//char msg[128];
		//sprintf(msg, "shell %d", shell);
		//Chat(msg);

		shell++;
	}while(shell < g_hmap.m_widthX || shell < g_hmap.m_widthZ);

	return -1;
}

int PlaceNear(int player, int btype, int near1)
{
	Vec3f vAround = g_building[near1].pos;
	
	//g_log<<"PlaceNear "<<player<<endl;
	//g_log.flush();

	return PlaceBAround(player, btype, vAround);
}

//place near labourers
bool PlaceNearL(int player, int btype)
{
	int counted = 0;
	Vec3f vCenter(0,0,0);

	for(int i=0; i<UNITS; i++)
	{
		CUnit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hp <= 0.0f)
			continue;

		if(u->type != LABOURER)
			continue;

		vCenter = (vCenter * ((float)counted) + u->camera.Position())/(float)(counted+1);
		counted++;
	}

	if(counted <= 0)
		return false;

	//g_log<<"placenearl "<<player;
	//g_log.flush();

	PlaceBAround(player, btype, vCenter);
	
	//char msg[128];
	//sprintf(msg, "place n l %f,%f,%f", vCenter.x, vCenter.y, vCenter.z);
	//Chat(msg);

	return true;
}

//place centrally/randomly
bool PlaceCRandomly(int player, int btype)
{
	Vec3f vCenter = Vec3f(g_hmap.m_widthX*TILE_SIZE/2, 0, g_hmap.m_widthZ*TILE_SIZE/2);
	
	if(PlaceBAround(player, btype, vCenter) >= 0)
		return true;

	return false;
}

bool PlaceRCentral(int player, int btype)
{	
	g_nearb.clear();

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		if(!g_diplomacy[b->owner][player])
			continue;

		g_nearb.push_back(i);
	}

	if(g_nearb.size() > 0)
	{
		return PlaceNearB(player, btype);
	}

	if(!PlaceNearL(player, btype))
		return PlaceCRandomly(player, btype);

	return false;
}

void ConnectAll(int player, int startBui)
{
	for(int i=0; i<BUILDINGS; i++)
	{
		if(i == startBui)
			continue;

		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;
		
		AIPlotRoad(player, startBui, i);
	}

	BuildPow(player);
	BuildPipe(player);
}

bool PlaceNearB(int player, int btype)
{
	
	//g_log<<"PlaceNearB1 "<<player<<endl;
	//g_log.flush();
	bool result = false;

	if(g_nearb.size() <= 0)
	{
	//g_log<<"PlaceNearB1 goto placercen "<<player<<endl;
	//g_log.flush();

		result = PlaceRCentral(player, btype);
		return result;
	}

	
	//g_log<<"PlaceNearB2 "<<player<<endl;
	//g_log.flush();

	int near1b = g_nearb[ rand()%g_nearb.size() ];

	
	//g_log<<"PlaceNearB3 "<<player<<endl;
	//g_log.flush();

	int startBui = PlaceNear(player, btype, near1b);

	if(startBui >= 0)
	{
	//g_log<<"PlaceNearB aiplotroad "<<player<<endl;
	//g_log.flush();
	
		//AIPlotRoad(player, startBui, near1b);

		for(int i=0; i<BUILDINGS; i++)
		{
			if(i == startBui)
				continue;

			CBuilding* b = &g_building[i];

			if(!b->on)
				continue;

			AIPlotRoad(player, startBui, i);
		}

		return true;
	}

	//g_log<<"PlaceNearB4 "<<player<<endl;
	//g_log.flush();

	return false;
}

void BuildPow(int player)
{
	//extend powerlines to player's consumers

	vector<int> withoutpow;
	vector<int> powsrc;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		CBuildingType* bt = &g_buildingType[b->type];

		if(b->owner == player && bt->input[ELECTRICITY] > 0.0f && b->pownetw < 0)
		{
			withoutpow.push_back(i);
		}

		if(g_diplomacy[b->owner][player] && bt->output[ELECTRICITY] > 0.0f)
		{
			powsrc.push_back(i);
		}
	}

	if(withoutpow.size() > 0 && powsrc.size() > 0)
	{
		for(int i=0; i<withoutpow.size(); i++)
		{
			for(int j=0; j<powsrc.size(); j++)
			{
				AIPlotPowl(player, withoutpow[i], powsrc[j]);
			}
		}
	}
	else
	{
		//Chat("no need to pow 1");
	}

	withoutpow.clear();
	powsrc.clear();

	// extend powerlines from player's powerstations to other clients

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		CBuildingType* bt = &g_buildingType[b->type];

		if(g_diplomacy[b->owner][player] && bt->input[ELECTRICITY] > 0.0f)
		{
			withoutpow.push_back(i);
		}

		if(b->owner == player && bt->output[ELECTRICITY] > 0.0f)
		{
			powsrc.push_back(i);
		}
	}

	if(withoutpow.size() > 0 && powsrc.size() > 0)
	{
		for(int i=0; i<withoutpow.size(); i++)
		{
			for(int j=0; j<powsrc.size(); j++)
			{
				AIPlotPowl(player, withoutpow[i], powsrc[j]);
			}
		}
	}
	else
	{
		//Chat("no need to pow 2");
	}
}

void BuildPipe(int player)
{
	//extend powerlines to player's consumers

	vector<int> withoutcrud;
	vector<int> crudsrc;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		CBuildingType* bt = &g_buildingType[b->type];

		if(b->owner == player && bt->input[CRUDE] > 0.0f)
		{
			withoutcrud.push_back(i);
		}

		if(g_diplomacy[b->owner][player] && bt->output[CRUDE] > 0.0f)
		{
			crudsrc.push_back(i);
		}
	}

	if(withoutcrud.size() > 0 && crudsrc.size() > 0)
	{
		for(int i=0; i<withoutcrud.size(); i++)
		{
			for(int j=0; j<crudsrc.size(); j++)
			{
				AIPlotPipe(player, withoutcrud[i], crudsrc[j]);
			}
		}
	}
	else
	{
		//Chat("no need to pipe 1");
	}

	withoutcrud.clear();
	crudsrc.clear();

	// extend powerlines from player's powerstations to other clients

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		CBuildingType* bt = &g_buildingType[b->type];

		if(g_diplomacy[b->owner][player] && bt->input[CRUDE] > 0.0f)
		{
			withoutcrud.push_back(i);
		}

		if(b->owner == player && bt->output[CRUDE] > 0.0f)
		{
			crudsrc.push_back(i);
		}
	}

	if(withoutcrud.size() > 0 && crudsrc.size() > 0)
	{
		for(int i=0; i<withoutcrud.size(); i++)
		{
			for(int j=0; j<crudsrc.size(); j++)
			{
				AIPlotPipe(player, withoutcrud[i], crudsrc[j]);
			}
		}
	}
	else
	{
		//Chat("no need to pipe 2");
	}
}

bool AIBuild(int player, int btype)
{
	g_nearb.clear();

	//g_log<<"ai build "<<player<<endl;
	//g_log.flush();

	CBuildingType* bt = &g_buildingType[btype];
	
	bool meetres = true;
	CPlayer* p = &g_player[player];
	float needres[RESOURCES];
	
	for(int i=0; i<RESOURCES; i++)
	{
		needres[i] = 0;
		if(bt->cost[i] <= 0.0f)
			continue;

		if(i == LABOUR)
			continue;

		if(bt->cost[i] > p->global[i])
		{
			meetres = false;
			needres[i] = bt->cost[i] - p->global[i];
			//break;
		}
	}

	if(!meetres)
	{
		bool havetrucks = false;

		for(int i=0; i<UNITS; i++)
		{
			CUnit* u = &g_unit[i];

			if(!u->on)
				continue;

			if(u->type != TRUCK)
				continue;

			if(!g_diplomacy[player][u->owner])
				continue;

			havetrucks = true;
			break;
		}

		if(!havetrucks)
			return false;

		for(int i=0; i<BUILDINGS; i++)
		{
			CBuilding* b = &g_building[i];

			if(!b->on)
				continue;

			if(!g_diplomacy[b->owner][player])
				continue;
			
			CBuildingType* supt = &g_buildingType[b->type];

			for(int j=0; j<RESOURCES; j++)
			{
				if(supt->output[j] <= 0.0f)
					continue;

				if(needres[j] <= 0.0f)
					continue;

				needres[j] = 0;
			}
		}

		for(int i=0; i<RESOURCES; i++)
		{
			if(needres[i] > 0.0f)
				return false;
		}
	}

	//cost estimate
	float estcost = 0;
	for(int i=0; i<RESOURCES; i++)
	{
		if(bt->cost[i] <= 0.0f)
			continue;

		if(i == LABOUR)
		{
			estcost += 0.10f * bt->cost[i];
			continue;
		}

		float remain = bt->cost[i] - p->global[i];

		if(remain <= 0.0f)
			continue;

		float cheapest = 99999.0f;
		bool found = false;

		for(int j=0; j<BUILDINGS; j++)
		{
			CBuilding* b = &g_building[j];

			if(!b->on)
				continue;

			if(!g_diplomacy[player][b->owner])
				continue;

			CBuildingType* supbt = &g_buildingType[b->type];

			if(supbt->output[i] <= 0.0f)
				continue;

			if(b->owner == player)
			{
				found = true;
				cheapest = 0;
				break;
			}

			CPlayer* supp = &g_player[b->owner];

			if(!found || supp->price[i] < cheapest)
			{
				found = true;
				cheapest = supp->price[i];
			}
		}

		if(!found)
			return false;

		estcost += cheapest * bt->cost[i];
	}

	if(estcost >= p->global[CURRENC] - MINIMUM_RESERVE)
		return false;

	vector<int> suppliesr;

	for(int i=0; i<RESOURCES; i++)
	{
		if(bt->output[i] <= 0.0f)
			continue;

		if(!g_resource[i].physical)
			continue;

		suppliesr.push_back(i);
	}

	
	//g_log<<"ai builda "<<player<<endl;
	//g_log.flush();

	vector<int> demandert;

	for(int i=0; i<BUILDING_TYPES; i++)
	{
		CBuildingType* bt2 = &g_buildingType[i];

		for(int j=0; j<suppliesr.size(); j++)
		{
			if(bt2->input[ suppliesr[j] ] > 0.0f)
			{
				demandert.push_back(i);
				break;
			}
		}
	}

	bool result = false;

	//g_log<<"ai buildb "<<player<<endl;
	//g_log.flush();

	if(demandert.size() <= 0)
	{
		//build near suppliers

		vector<int> demandsr;

		for(int i=0; i<RESOURCES; i++)
		{
			if(bt->input[i] <= 0.0f)
				continue;

			if(!g_resource[i].physical)
				continue;

			demandsr.push_back(i);
		}

		
	//g_log<<"ai buildc "<<player<<endl;
	//g_log.flush();
		
		vector<int> suppliert;

		for(int i=0; i<BUILDING_TYPES; i++)
		{
			CBuildingType* bt2 = &g_buildingType[i];

			for(int j=0; j<demandsr.size(); j++)
			{
				if(bt2->output[ demandsr[j] ] > 0.0f)
				{
					suppliert.push_back(i);
					break;
				}
			}
		}

		
	//g_log<<"ai buildd "<<player<<endl;
	//g_log.flush();

		if(suppliert.size() <= 0)
		{
			// just pick a random central spot
			result = PlaceRCentral(player, btype);
			return result;
		}

		
	//g_log<<"ai builde "<<player<<endl;
	//g_log.flush();

		for(int i=0; i<BUILDINGS; i++)
		{
			CBuilding* b = &g_building[i];

			if(!b->on)
				continue;

			if(!g_diplomacy[player][b->owner])
				continue;

			bool found = false;
			for(int j=0; j<suppliert.size(); j++)
			{
				if(suppliert[j] == i)
				{
					found = true;
					break;
				}
			}

			if(!found)
				continue;

			g_nearb.push_back(i);
		}

		
	//g_log<<"ai buildf "<<player<<endl;
	//g_log.flush();
		result = PlaceNearB(player, btype);
	}
	else
	{
		
	//g_log<<"ai buildg "<<player<<endl;
	//g_log.flush();

		for(int i=0; i<BUILDINGS; i++)
		{
			CBuilding* b = &g_building[i];
			
			if(!b->on)
				continue;

			if(!g_diplomacy[player][b->owner])
				continue;
			
	//g_log<<"ai buildh "<<player<<endl;
	//g_log.flush();

			bool found = false;
			for(int j=0; j<demandert.size(); j++)
			{
				
	//g_log<<"ai buildhh "<<player<<endl;
	//g_log.flush();

				if(demandert[j] == i)
				{
					found = true;
					break;
				}
			}

			if(!found)
				continue;

			g_nearb.push_back(i);
		}
		

	//g_log<<"ai buildi "<<player<<endl;
	//g_log.flush();
		result = PlaceNearB(player, btype);
	}
	
	BuildPow(player);
	BuildPipe(player);
	
	//g_log<<"ai build end "<<player<<endl;
	//g_log.flush();

	return result;
}

void BuyBs(int player)
{
	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		if(b->owner == player)
			continue;

		if(!g_diplomacy[player][b->owner])
			continue;

		if(b->forsale)
		{ 
			if(b->saleprice <= 0.0f)
			{
				if(b->trybuy(player))
					ConnectAll(player, i);
			}
			else
			{
				float earn = 0;

				if(b->cycleh.size() > 0)
					earn = b->cycleh[ b->cycleh.size()-1 ].produced[CURRENC] - b->cycleh[ b->cycleh.size()-1 ].consumed[CURRENC];
				else
					earn = b->recenth.produced[CURRENC] - b->recenth.consumed[CURRENC];

				if(earn * 10.0f > b->saleprice)
				{
					if(b->trybuy(player))
						ConnectAll(player, i);
				}
			}
		}
	}
}

void BuildStuff(int player)
{
	CPlayer* p = &g_player[player];

	if(p->bbframesago < BUILDSTUFF_DELAY)
		return;

	BuiltStuff(player);

	BuyBs(player);

	if(!AllConstF())
		return;

	//g_log<<"build stuff "<<player<<endl;
	//g_log.flush();

	int havenum[BUILDING_TYPES];
	int anyphavenum[BUILDING_TYPES];

	for(int i=0; i<BUILDING_TYPES; i++)
	{
		havenum[i] = 0;
		anyphavenum[i] = 0;
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		anyphavenum[b->type]++;

		CPlayer* p2 = &g_player[b->owner];

		if(!g_diplomacy[player][b->owner])
			continue;

		havenum[b->type]++;
	}

	int leasthave = 999999;
	int leasthaveB = -1;
	vector<int> leasthaves;
	
	bool noapt = false;
	bool noshopcplx = false;
	bool nopow = false;
	bool noderk = false;
	bool norefy = false; 
	bool nofac = false;
	bool nomine = false;
	bool nosmelt = false;
	bool noquarry = false;
	bool nocemplant = false;
	bool nofarm = false;
	bool nochem = false;

	for(int i=0; i<BUILDING_TYPES; i++)
	{
		if(leasthaveB >= 0 && havenum[i] > leasthave)
			continue;

		if(i == RNDFACILITY)
			continue;

		if(havenum[i] < leasthave)
		{
			leasthaves.clear();
			noapt = false;
			noshopcplx = false;
			nopow = false;
			noderk = false;
			norefy = false; 
			nofac = false;
			nomine = false;
			nosmelt = false;
			noquarry = false;
			nocemplant = false;
			nofarm = false;
			nochem = false;
		}

		if(i == APARTMENT)	noapt = true;
		if(i == SHOPCPLX)	noshopcplx = true;
		if(i == REACTOR)	nopow = true;
		if(i == REFINERY)	norefy = true;
		if(i == DERRICK)	noderk = true;
		if(i == FACTORY)	nofac = true;
		if(i == MINE)		nomine = true;
		if(i == SMELTER)	nosmelt = true;
		if(i == QUARRY)		noquarry = true;
		if(i == CEMPLANT)	nocemplant = true;
		if(i == FARM)		nofarm = true;
		if(i == CHEMPLANT)	nochem = true;

		leasthave = havenum[i];
		leasthaveB = i;
		leasthaves.push_back(i);
	}

	int numlab = 0;

	for(int i=0; i<UNITS; i++)
	{
		CUnit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != LABOURER)
			continue;

		numlab++;
	}

	CBuildingType* aptt = &g_buildingType[APARTMENT];

	if(aptt->output[HOUSING]*anyphavenum[APARTMENT] < numlab)
		noapt = true;
	
	//g_log<<"build stuff1.5 "<<player<<endl;
	//g_log.flush();
	
	//if(leasthave <= 0)
	if(leasthave < numlab/(1.5f*g_buildingType[APARTMENT].output[HOUSING]) || leasthave == 0)
	{
		//else 
		if(noapt)
			AIBuild(player, APARTMENT);
		else if(noshopcplx)
			AIBuild(player, SHOPCPLX);
		else if(noquarry)
			AIBuild(player, QUARRY);
		else if(nocemplant)
			AIBuild(player, CEMPLANT);
		//else if(nomine)
		//	AIBuild(player, MINE);
		//else if(nosmelt)
		//	AIBuild(player, SMELTER);
		else if(nofac)
			AIBuild(player, FACTORY);
		else if(nopow)
			AIBuild(player, REACTOR);
		else if(noderk)
			AIBuild(player, DERRICK);
		else if(norefy)
			AIBuild(player, REFINERY);
		else if(nofarm)
			AIBuild(player, FARM);
		else if(nochem)
			AIBuild(player, CHEMPLANT);
		else
			AIBuild(player, leasthaves[ rand()%leasthaves.size() ]);
		
		//if(rand()%2 == 0)
		//	AIBuild(player, DERRICK);
		//else
		//	AIBuild(player, REFINERY);
	}

	
	//g_log<<"build stuff2 "<<player<<endl;
	//g_log.flush();

	
	//g_log<<"build stuff3 "<<player<<endl;
	//g_log.flush();
	BuildPipe(player);
	BuildPow(player);
}

void BuildUnits(int player, int utype, int count, bool mostimportant=false)
{
	int cheapestID = -1;
	float cheapestPr = 99999999;

	//char msg[128];
	//sprintf(msg, "build x%d #%s p%d", count, g_unitType[utype].name, player);
	//Chat(msg);

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		if(!g_diplomacy[player][b->owner])
			continue;

		//Chat("dip");

		CBuildingType* t = &g_buildingType[b->type];

		bool found = false;
		for(int j=0; j<t->buildable.size(); j++)
		{
			//char msg[128];
			//sprintf(msg, "build %d", t->buildable[j]);
			//Chat(msg);

			if(t->buildable[j] == utype)
			{
				//Chat("found1");
				found = true;
				break;
			}
		}

		if(!found)
			continue;

		//Chat("found");

		CPlayer* p = &g_player[b->owner];

		if(p->uprice[utype] < cheapestPr || b->owner == player)
		{
			cheapestID = i;
			
			if(b->owner != player)
				cheapestPr = p->uprice[utype];
		}
	}

	if(cheapestID < 0)
		return;

	//Chat("2");

	if(g_building[cheapestID].owner != player)
	{
		CPlayer* p = &g_player[player];
		if(p->global[CURRENC] <= cheapestPr * count)
		{
			count = p->global[CURRENC] / cheapestPr;

			if(p->global[CURRENC] <= cheapestPr * count)
				count --;
		}
	}

	CBuilding* b = &g_building[cheapestID];

	
	if(mostimportant)
	{
		for(int i=0; i<b->produ.size(); i++)
		{
			ManufJob* mj = &b->produ[i];
			
			//if(mj->owner == player && mj->utype != TRUCK)
			if(mj->owner != g_localP && mj->utype != utype)
			{
				b->DequeueU(mj->utype, mj->owner);
				i--;
			}
		}
	}

	for(int i=0; i<count; i++)
	{
		
		//Chat("3");

		b->QueueU(utype, player);
	}
}

void DeclareWars(int player)
{
	//g_log<<"declare wars"<<endl;

	CPlayer* p = &g_player[player];

	int maxcount[PLAYERS][UNIT_TYPES];
	
	for(int i=0; i<UNIT_TYPES; i++)
		for(int j=0; j<PLAYERS; j++)
			maxcount[j][i] = 0;

	for(int i=0; i<UNITS; i++)
	{
		CUnit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hp <= 0.0f)
			continue;

		CUnitType* t = &g_unitType[u->type];

		//if(t->damage <= 0.0f)
		//	continue;

		maxcount[u->owner][u->type] ++;
	}

	float playerdmghp[PLAYERS];
	float totalenemydmghp = 0;

	for(int i=0; i<PLAYERS; i++)
	{
		playerdmghp[i] = 0;

		for(int j=0; j<UNIT_TYPES; j++)
		{
			CUnitType* t = &g_unitType[j];

			if(t->damage <= 0.0f)
				continue;

			float add = 1000.0f * t->damage / t->shotdelay + t->hitpoints;

			playerdmghp[i] += add * maxcount[i][j];

			if(!g_diplomacy[player][i])
				totalenemydmghp += add * maxcount[i][j];
		}

		
	//g_log<<"play "<<i<<" dmghp "<<playerdmghp[i]<<endl;
	}
	
	if(playerdmghp[player] < MINIMUM_SUPERIORITY)
	{
		//g_log<<"less than min sup"<<endl;
		return;
	}

	if(playerdmghp[player] < totalenemydmghp)
	{
		//g_log<<"less than enemy dmghp"<<endl;
		return;
	}

	for(int i=0; i<PLAYERS; i++)
	{
		if(i == player)
			continue;

		if(g_player[i].activity == ACTIVITY_NONE)
			continue;
		
		if(playerdmghp[i] <= 0.0f)
			playerdmghp[i] = 1;

		//g_log<<"make war?"<<(playerdmghp[player] / playerdmghp[i])<<" >= "<<SUPERIORITY_RATIO<<endl;

		if(playerdmghp[player] / playerdmghp[i] >= SUPERIORITY_RATIO)
		{
			//g_log<<"make war with "<<facnamestr[i]<<endl;
			MakeWar(player, i);
		}
	}
}

void ManufStuff(int player)
{
	CPlayer* p = &g_player[player];

	//char msg[128];
	//sprintf(msg, "manuf %d [%d/%d]", player, p->buframesago, (int)BUILDUNITS_DELAY);
	//Chat(msg);

	
	if(p->buframesago < BUILDUNITS_DELAY)
		return;

	//sprintf(msg, "manuf2 %d", player);
	//Chat(msg);
	
	p->buframesago = BUILDUNITS_DELAY/2;
	//int framesago = p->buframesago;
	/*
	for(int i=0; i<PLAYERS; i++)
	{
		if(i == g_localP)
			continue;

		if(!g_diplomacy[player][i])
			continue;

		//g_player[i].buframesago -= framesago;

		if(i == player)
		{
			g_player[i].buframesago -= 2;
			g_player[i].buframesago -= framesago;
		}
	}*/

	int maxcount[PLAYERS][UNIT_TYPES];
	int inmanuf[UNIT_TYPES];
	
	for(int i=0; i<UNIT_TYPES; i++)
	{
		inmanuf[i] = 0;
		for(int j=0; j<PLAYERS; j++)
			maxcount[j][i] = 0;
	}

	for(int i=0; i<UNITS; i++)
	{
		CUnit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hp <= 0.0f)
			continue;

		CUnitType* t = &g_unitType[u->type];

		//if(t->damage <= 0.0f)
		//	continue;

		if(u->owner < 0)
			continue;

		maxcount[u->owner][u->type] ++;
	}

	int ownedb = 0;
	float recentearn = 0;

	if(p->truckh.size() > 0)
		recentearn += p->truckh[ p->truckh.size()-1 ].produced[CURRENC] - p->truckh[ p->truckh.size()-1 ].consumed[CURRENC];
	else
		recentearn += p->recentth.produced[CURRENC] - p->recentth.consumed[CURRENC];

	float cheapestTruck = 99999;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		if(b->owner == player)
		{
			ownedb ++;

			if(b->cycleh.size() > 0)
				recentearn += b->cycleh[ b->cycleh.size()-1 ].produced[CURRENC] - b->cycleh[ b->cycleh.size()-1 ].consumed[CURRENC];
			else
				recentearn += b->recenth.produced[CURRENC] - b->recenth.consumed[CURRENC];
		}

		for(int j=0; j<b->produ.size(); j++)
		{
			ManufJob* mj = &b->produ[j];

			maxcount[mj->owner][mj->utype] ++;

			if(mj->owner == player)
				inmanuf[mj->utype] ++;
		}

		if(g_diplomacy[b->owner][player])
		{
			CBuildingType* t = &g_buildingType[b->type];

			for(int j=0; j<t->buildable.size(); j++)
			{
				if(t->buildable[j] == TRUCK)
				{
					if(b->owner == player)
						cheapestTruck = 0.00f;
					else
					{
						CPlayer* bowner = &g_player[b->owner];

						if(bowner->uprice[TRUCK] < cheapestTruck)
							cheapestTruck = bowner->uprice[TRUCK];
					}
					break;
				}
			}
		}
	}

	int numtrucks = 0;
	for(int i=0; i<PLAYERS; i++)
	{
		// trucks by any player
		numtrucks += maxcount[i][TRUCK];
	}

	if(numtrucks > 0)
	{

		//sprintf(msg, "manuf3 %d", player);
		//Chat(msg);

		for(int i=0; i<UNIT_TYPES; i++)
		{
			CUnitType* t = &g_unitType[i];

			if(t->damage <= 0.0f)
				continue;

			int maxc = 0;

			for(int j=0; j<PLAYERS; j++)
			{
				if(j == player)
					continue;

				if(maxcount[j][i] > maxc)
					maxc = maxcount[j][i];
			}

			//Chat("build u?");

			//if(maxc > 0)
			if(maxc > maxcount[player][i]*3)
			{
				//BuildUnits(player, i, (maxc - maxcount[player][i])/6);
				BuildUnits(player, i, 1);
				p->buframesago = 0;
			}

			int cheapestB = -1;
			float cheapestC = 999999.0f;

			for(int j=0; j<BUILDINGS; j++)
			{
				CBuilding* b = &g_building[j];

				if(!b->on)
					continue;

				CBuildingType* bt = &g_buildingType[b->type];

				bool found = false;
				for(int k=0; k<bt->buildable.size(); k++)
				{
					if(bt->buildable[k] == i)
					{
						found = true;
						break;
					}
				}

				if(!found)
					continue;

				if(!g_diplomacy[b->owner][player])
					continue;

				CPlayer* bp = &g_player[b->owner];

				if(cheapestB < 0 || bp->uprice[i] <= cheapestC)
				{
					cheapestB = j;
					cheapestC = bp->uprice[i];
				}
			}

			if(cheapestB < 0)
				continue;
			
			float cashhandy = p->global[CURRENC] - MINIMUM_RESERVE + recentearn;
			if(ownedb <= 0)
				cashhandy -= 1.00;
			//int extra = (p->global[CURRENC] - MINIMUM_RESERVE - (1-ownedb)*0.25f) / cheapestC;
			int extra = cashhandy / cheapestC;
			extra = min(extra, 1);
			extra = max(0, extra - inmanuf[i]);
			BuildUnits(player, i, extra);

			//char msg[128];
			//sprintf(msg, "build %d extra %s (%f - 0.1f) / %f", extra, g_unitType[i].name, p->global[CURRENC], cheapestC);
			//Chat(msg);

			//Chat("build u!");
		
			if(extra > 0)
				p->buframesago = 0;
		}
	}
	
	int numb = 0;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		numb ++;
	}

	numtrucks = 0;

	for(int j=0; j<PLAYERS; j++)
	{
		//if(j == player)
		//	continue;

		if(!g_diplomacy[player][j])
			continue;

		// trucks by allied players
		numtrucks += maxcount[j][TRUCK];
	}

	//if(numb*3 / 5 > numtrucks)
	if(numb / 5 > numtrucks || (numtrucks < 3 && numb*3/5 > numtrucks))
	{
		bool urgent = false;
		if(numtrucks <= 0)
			urgent = true;

		if(p->global[CURRENC] - cheapestTruck > MINIMUM_RESERVE)
			//BuildUnits(player, TRUCK, 1, true);
			BuildUnits(player, TRUCK, 1, urgent);
		p->buframesago = 0;
		//Chat("new truck");
	}

	DeclareWars(player);
}

void AdjustPrices(int player)
{
	CPlayer* p = &g_player[player];

	float cheapcg = 99999.0f;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		CBuildingType* t = &g_buildingType[b->type];

		if(t->output[CONSUMERGOODS] <= 0.0f)
			continue;

		CPlayer* p = &g_player[b->owner];

		if(p->price[CONSUMERGOODS] < cheapcg)
			cheapcg = p->price[CONSUMERGOODS];
	}

	int numlab = 0;
	float labcashres = 0;
	float needforcong = 0;
	for(int i=0; i<UNITS; i++)
	{
		CUnit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != LABOURER)
			continue;

		numlab ++;
		labcashres += u->currency;
		needforcong += cheapcg * (MULTIPLY_FUEL - u->fuel);
	}

	for(int i=0; i<RESOURCES; i++)
	{
		if(p->priceh[i].size() > 0)
		{
			PriceH* ph0 = &p->priceh[i][ p->priceh[i].size() - 1 ];
			PriceH* ph1 = &p->recentph[i];

			float pricech = ph1->price - ph0->price;
			float earnch = ph1->earnings - ph0->earnings;

			if(i == HOUSING)
			{
				if((labcashres - needforcong) / (float)numlab < p->price[i])
					p->price[i] -= 0.01f;
				else
					p->price[i] += 0.01f;
			}
			else
			{
				if(earnch > 0.0f)
				{
					if(pricech > 0.0f)
						p->price[i] += 0.01f;
					else if(pricech < 0.0f)
						p->price[i] -= 0.01f;
				}
				else if(earnch < 0.0f)
				{
					if(pricech > 0.0f)
						p->price[i] += 0.01f;
					else if(pricech < 0.0f)
						p->price[i] -= 0.01f;
				}
			}

			if(p->price[i] < 0.01f)
				p->price[i] = 0.01f;
		}
		else
		{
			p->price[i] += 0.01f;
		}

		p->priceh[i].push_back(p->recentph[i]);
		p->recentph[i].reset();
		p->recentph[i].price = p->price[i];
	}
}

void UpdateAI(int player)
{
	CPlayer* p = &g_player[player];
	p->bbframesago++;
	p->buframesago++;
	p->priceadjframes++;

	if(p->activity != ACTIVITY_NOBUILDBU)
	{
		if(p->activity != ACTIVITY_NOBUILDB)
		{
	LastNum("build s");
			BuildStuff(player);
		}

		if(p->activity != ACTIVITY_NOBUILDU)
		{
	LastNum("manuf s");
			ManufStuff(player);
		}
	}

	if(p->priceadjframes >= CYCLE_FRAMES)
	{
	LastNum("adj pr");
		AdjustPrices(player);
		p->priceadjframes = 0;
	}
}

void AdjustPQuotas(int player)
{
	//float needcong = 0;
	
	float totalprod[RESOURCES];
	float totalneed[RESOURCES];
	Zero(totalprod);
	Zero(totalneed);

	for(int i=0; i<UNITS; i++)
	{
		CUnit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type == LABOURER)
		{
			//needcong += LABOURER_FUEL;
			//needcong += MULTIPLY_FUEL;
			totalneed[CONSUMERGOODS] += MULTIPLY_FUEL;
		}
		else if(u->type == TRUCK)
		{
			totalneed[ZETROL] += TRUCK_FUEL;
		}
	}

	//float totalprodneed = 0;
	//float totalfarmprod = 0;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		if(!g_diplomacy[player][b->owner])
			continue;

		CBuildingType* t = &g_buildingType[b->type];
		CPlayer* p = &g_player[b->owner];

		//if(t->output[PRODUCE] > 0.0f)
		{
		//	totalfarmprod += t->output[PRODUCE] * b->prodquota + b->stock[PRODUCE] + p->global[PRODUCE];
		}
		//if(t->input[PRODUCE] > 0.0f)
		{
		}

		if(b->finished)
		{
			for(int j=0; j<RESOURCES; j++)
			{
				if(t->output[j] > 0.0f)
				{
					totalprod[j] += t->output[j] * b->prodquota + b->stock[j];

					if(!g_resource[j].capacity)
						totalprod[j] += p->global[j];
				}
				if(t->input[j] > 0.0f)
				{
					totalneed[j] += t->input[j] * b->prodquota - b->stock[j];
				}
			}
		}
		else
		{
			for(int j=0; j<RESOURCES; j++)
			{
				if(t->cost[j] > 0.0f)
				{
					totalneed[j] += t->cost[j] - b->conmat[j];
				}
			}
		}
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on || !b->finished)
			continue;

		if(b->owner != player || b->owner == g_localP)
			continue;

		CPlayer* p = &g_player[b->owner];

		if(p->activity == ACTIVITY_NONE)
			continue;

		CBuildingType* t = &g_buildingType[b->type];

		bool lackingout = false;

		for(int res=0; res<RESOURCES; res++)
		{
			if(t->output[res] <= 0.0f)
				continue;

			//if(b->type == REFINERY)
			//{
			//	Chat("ref 1");
			//}
			

			float total = b->stock[res];

			if(!g_resource[res].capacity)
				total += p->global[res];

			//if(b->quotafilled[res] >= b->prodquota * t->output[res])
			if(t->output[res] > 0.0f)
			{
				
			//if(b->type == REFINERY)
			//{
			//	Chat("ref 2");
			//}
				
				//if(total <= 0.0f || (res == CONSUMERGOODS && total <= needcong))
				//if(total <= 5.0f || (res == CONSUMERGOODS && total <= needcong))
				if(total <= 5.0f || totalneed[res] - totalprod[res] >= 0.0f)
				{
					
			//if(b->type == REFINERY)
			//{
			//	Chat("ref 3");
			//}

					lackingout = true;
					break;
				}
			}
		}

		if(lackingout)
			b->prodquota += 1.0f;
		else
			b->prodquota -= 1.0f;

		if(b->prodquota <= 0.0f)
			//b->prodquota = 1.0f;
			b->prodquota = 0.0f;
	}
}

void UpdateAI()
{
	for(int i=0; i<PLAYERS; i++)
	{
		if(i == g_localP)
			continue;

		CPlayer* p = &g_player[i];

		if(p->activity == ACTIVITY_NONE)
			continue;

		UpdateAI(i);
		//BuildPipe(i);
		//BuildPow(i);
		//AdjustPQuotas(i);	//remove this
	}

	
	LastNum("end up ai");
}