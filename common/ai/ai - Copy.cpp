#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"
#include "../sim/building.h"
#include "../sim/buildingtype.h"
#include "../sim/build.h"
#include "../sim/sim.h"
#include "../econ/utility.h"
#include "../sim/unit.h"

void UpdAI()
{
	//return;	//do nothing for now

	for(int i=0; i<PLAYERS; i++)
	{
		if(i == g_localP)
			continue;
        
		Player* p = &g_player[i];
        
		//if(p->activity == ACTIVITY_NONE)
		//	continue;

		if(!p->ai)
			continue;
        
		UpdAI(p);
		//BuildPipe(i);
		//BuildPow(i);
		//AdjustPQuotas(i);	//remove this
	}
}

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
    
	//for(int x=0; x<g_map.m_widthX; x++)
	//	for(int z=0; z<g_map.m_widthZ; z++)
	//		OpenAt(x, z) = true;
    
	g_curpath ++;
    
	//g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz, -1));
	//if(startBstartz > 0)
	{
		int z=startBstartz;
		for(int x=max(0,startBstartx); x<=min(startBendx,g_map.m_widthX-1); x++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	//if(startBendx < g_map.m_widthX)
	{
		int x=startBendx;
		for(int z=startBstartz; z<=min(startBendz,g_map.m_widthZ-1); z++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	//if(startBendz < g_map.m_widthZ)
	{
		int z=startBendz;
		for(int x=min(startBendx,g_map.m_widthX-1); x>=max(0,startBstartx); x--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	if(startBstartx >= 0)
	{
		int x=startBstartx;
		for(int z=min(startBendz,g_map.m_widthZ-1); z>=startBstartz; z--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	int i;
	int best;
	CTryStep* tS;
	CTryStep* bestS;
    
	//int searchdepth = 0;
    
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
		if(bestS->x < g_map.m_widthX-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x + 1, bestS->z, best, runningD, stepD));
		if(bestS->z > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z - 1, best, runningD, stepD));
		if(bestS->z < g_map.m_widthZ-1)
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
    
	//for(int x=0; x<g_map.m_widthX; x++)
	//	for(int z=0; z<g_map.m_widthZ; z++)
	//		OpenAt(x, z) = true;
    
	g_curpath ++;
    
	//g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz, -1));
	//if(startBstartz > 0)
	{
		int z=startBstartz;
		for(int x=startBstartx; x<=min(startBendx,g_map.m_widthX-1); x++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	if(startBendx < g_map.m_widthX)
	{
		int x=startBendx;
		for(int z=startBstartz; z<=min(startBendz,g_map.m_widthZ-1); z++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	if(startBendz < g_map.m_widthZ)
	{
		int z=startBendz;
		for(int x=min(startBendx,g_map.m_widthX-1); x>=startBstartx; x--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	//if(startBstartx > 0)
	{
		int x=startBstartx;
		for(int z=min(startBendz,g_map.m_widthZ-1); z>=startBstartz; z--)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	int i;
	int best;
	CTryStep* tS;
	CTryStep* bestS;
    
	//int searchdepth = 0;
    
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
		if(bestS->x < g_map.m_widthX-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x + 1, bestS->z, best, runningD, stepD));
		if(bestS->z > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z - 1, best, runningD, stepD));
		if(bestS->z < g_map.m_widthZ-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z + 1, best, runningD, stepD));
	}
    
	//Chat("rpath failed3");
	//Chat("powpath failed3");
	return false;
}


bool AIPlotRoad(int owner, int startBui, int endBui)
{
	//const float tileSize = TILE_SIZE;
	CBuilding* startB = &g_building[startBui];
	const Vec3f pos = startB->pos;
	//CBuilding* endB = &g_building[endBui];
	//const Vec3f goal = endB->pos;
    
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
    
	//for(int x=0; x<g_map.m_widthX; x++)
	//	for(int z=0; z<g_map.m_widthZ; z++)
	//		OpenAt(x, z) = true;
    
	g_curpath ++;
    
	//g_tryStep.push_back(CTryStep(startx, startz, endx, endz, startx, startz, -1));
	if(startBstartz >= 0)
	{
		int z=startBstartz;
		for(int x=startBstartx+1; x<startBendx+1; x++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	if(startBendx+1 < g_map.m_widthX)
	{
		int x=startBendx+1;
		for(int z=startBstartz+1; z<startBendz+1; z++)
			g_tryStep.push_back(CTryStep(x, z, endx, endz, x, z, -1, 0, 0));
	}
    
	if(startBendz+1 < g_map.m_widthZ)
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
		if(bestS->x+1 < g_map.m_widthX)
			roadsaround.push_back(Vec2i(bestS->x+1, bestS->z));
		if(bestS->z > 0)
			roadsaround.push_back(Vec2i(bestS->x, bestS->z-1));
		if(bestS->z+1 < g_map.m_widthZ)
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
		if(bestS->x < g_map.m_widthX-1)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x + 1, bestS->z, best, runningD, stepD));
		if(bestS->z > 0)
			g_tryStep.push_back(CTryStep(startx, startz, endx, endz, bestS->x, bestS->z - 1, best, runningD, stepD));
		if(bestS->z < g_map.m_widthZ-1)
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
    
	for(int x=0; x<g_map.m_widthX; x++)
		for(int z=0; z<g_map.m_widthZ; z++)
		{
			//CRoad* r = RoadAt(x, z);
            
			//if(r->on && !r->finished)
			//	return false;
            
			//CPowerline* pow = PowlAt(x, z);
            
			///if(pow->on && !pow->finished)
			//	return false;
            
			//CPipeline* pipe = PipeAt(x, z);
            
			//if(pipe->on && !pipe->finished)
			//	return false;
		}
    
	return true;
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
	Vec3f vCenter = Vec3f(g_map.m_widthX*TILE_SIZE/2, 0, g_map.m_widthZ*TILE_SIZE/2);
    
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
    
	BuildingType* bt = &g_buildingType[btype];
    
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
			Building* b = &g_building[i];
            
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
			Building* b = &g_building[i];
            
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

#if 0
//buy buildings for sale
void BuyBs(int player)
{
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];
        
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
#endif

void BuildStuff(int player)
{
	Player* p = &g_player[player];
    
	if(p->bbframesago < BUILDSTUFF_DELAY)
		return;
    
	BuiltStuff(player);
    
	//BuyBs(player);
    
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
        
		//CPlayer* p2 = &g_player[b->owner];
        
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

void UpdAI(Player* p)
{
	//if(p != g_player)
	//	return;

	p->bbframesago++;
	p->buframesago++;
	p->priceadjframes++;
    
	//if(p->activity != ACTIVITY_NOBUILDBU)
	{
		//if(p->activity != ACTIVITY_NOBUILDB)
		{
            //LastNum("build s");
			BuildStuff(player);
		}
        
		//if(p->activity != ACTIVITY_NOBUILDU)
		{
            //LastNum("manuf s");
			ManufStuff(player);
		}
	}
    
	if(p->priceadjframes >= CYCLE_FRAMES)
	{
        //LastNum("adj pr");
		AdjustPrices(player);
		p->priceadjframes = 0;
	}
}
