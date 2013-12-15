

#include "transportation.h"
#include "building.h"
#include "unit.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "resource.h"
#include "player.h"
#include "main.h"
#include "chat.h"
#include "job.h"
#include "transaction.h"
#include "ai.h"

vector<int> g_freetrucks;
vector<ResSource> g_sourceBs[PLAYERS][RESOURCES];
float g_cheapfuel[PLAYERS];

void FillResSources(int res, int player)
{
	g_sourceBs[player][res].clear();

	for(int j=0; j<BUILDINGS; j++)
	{
		//if(j == building)
		//	continue;

		CBuilding* b = &g_building[j];

		if(!b->on)
			continue;

		CBuildingType* t = &g_buildingType[b->type];

		if(t->output[res] <= 0.0f)
			continue;

#ifdef TRUCK_DEBUG2
		if(res == CEMENT)
		{
			g_log<<"fill cement 1"<<endl;
		}
#endif

		if(!b->finished)
			continue;

		if(!g_diplomacy[b->owner][player])
			continue;
		
#ifdef TRUCK_DEBUG2
		if(res == CEMENT)
		{
			g_log<<"fill cement 2"<<endl;
		}
#endif

		float avail = b->stock[res];

		CPlayer* p = &g_player[b->owner];

		avail += p->global[res];
		
#ifdef TRUCK_DEBUG2
		if(res == CEMENT)
		{
			g_log<<"fill cement 3"<<endl;
		}
#endif

		if(avail <= 0.0f)
			continue;
		
#ifdef TRUCK_DEBUG2
		if(res == CEMENT)
		{
			g_log<<"fill cement 4"<<endl;
		}
#endif

		ResSource rs;
		rs.amt = avail;
		rs.supplier = j;
		g_sourceBs[player][res].push_back(rs);
	}

	if(g_sourceBs[player][res].size() <= 0)
	{
		//Chat("0 res sources");
		ResSource rs;
		rs.amt = -1;
		rs.supplier = -1;
		g_sourceBs[player][res].push_back(rs);
	}
}

void CalcTrip(TransportJob* tj, int clientplayer, int transporter, int modeto, int supplier, int modefrom, int target, int target2)
{
	//TransportJob tj;
	tj->path.clear();
	tj->pathlen = 0;

	if(modefrom == GOINGTODEMANDERB && g_building[target].type == g_building[supplier].type)
		return;

	CUnit* u = &g_unit[transporter];

	Vec3f orig = u->camera.Position();
	
	//don't forget to resetmode();
	u->mode = modeto;
	u->supplier = supplier;
	u->goal = g_building[supplier].pos;
	tj->supplier = supplier;

	if(!u->Pathfind())
	{
#ifdef TRUCK_DEBUG
		g_log<<"no path 1 to "<<g_buildingType[g_building[u->supplier].type].name<<" unit#"<<transporter<<endl;
		//Chat("no path to sup");
#endif
		if(modefrom == GOINGTODEMANDERB)
		{
			AIPlotRoad(g_building[target].owner, u->camera.Position().x/TILE_SIZE, u->camera.Position().x/TILE_SIZE, u->camera.Position().z/TILE_SIZE, u->camera.Position().z/TILE_SIZE, u->camera.Position(), supplier);

			//AIPlotRoad(g_building[target].owner, supplier, target);
			//AIPlotRoad(g_building[target].owner, target, supplier);
		}

		tj->path.clear();
		//don't forget to resetmode();
		return;
	}

	int stepbefore = u->stepbeforearrival();
	for(int i=0; i<=stepbefore; i++)
		tj->path.push_back(u->path[i]);
	tj->pathlen += u->pathlength();
	u->camera.MoveTo(u->path[stepbefore]);

	//g_log<<"CalcTrip to pathsize="<<u->path.size()<<endl;
	//g_log.flush();

		//PlaceUnit(CARLYLE, g_localP, tj->path[0]);
	
	//don't forget to resetmode();
	u->mode = modefrom;
	u->target = target;
	u->target2 = target2;
	if(modefrom == GOINGTODEMANDERB)
		u->goal = g_building[target].pos;
	tj->targtype = modefrom;
	tj->target = target;
	tj->target2 = target2;
	
	if(!u->Pathfind())
	{
#ifdef TRUCK_DEBUG
		g_log<<"no path 2"<<endl;
		//Chat("no path to dem");
#endif
		
		if(modefrom == GOINGTODEMANDERB)
		{
			AIPlotRoad(g_building[target].owner, u->camera.Position().x/TILE_SIZE, u->camera.Position().x/TILE_SIZE, u->camera.Position().z/TILE_SIZE, u->camera.Position().z/TILE_SIZE, u->camera.Position(), target);

			//AIPlotRoad(g_building[target].owner, supplier, target);
			//AIPlotRoad(g_building[target].owner, target, supplier);
		}

		tj->path.clear();
		//don't forget to resetmode();
		u->camera.MoveTo(orig);
		return;
	}
	
	//g_log<<"CalcTrip from pathsize="<<u->path.size()<<endl;
	//g_log.flush();

	stepbefore = u->stepbeforearrival();
	for(int i=0; i<=stepbefore; i++)
		tj->path.push_back(u->path[i]);
	tj->pathlen += u->pathlength();
	
		//PlaceUnit(CARLYLE, g_localP, tj->path[1]);

	u->camera.MoveTo(orig);

	//Chat("calc'd trip");
	
	tj->transporter = transporter;
	//don't forget to resetmode();
	/*
	if(modefrom == GOINGTODEMANDERB)
	{
		g_log<<"CalcTrip from "<<g_buildingType[g_building[supplier].type].name<<" to "<<g_buildingType[g_building[target].type].name<<endl;
		g_log.flush();
	}*/

	//g_log<<"CalcTrip SUCCESS"<<endl;
	//g_log.flush();

	return;
}

void CheapestFuel()
{
	bool found[PLAYERS];
	for(int i=0; i<PLAYERS; i++)
	{
		//g_cheapfuel[i] = 999999;
		found[i] = false;
		g_cheapfuel[i] = 0;
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		CBuildingType* t = &g_buildingType[b->type];

		if(t->output[ZETROL] <= 0.0f)
			continue;

		CPlayer* p = &g_player[b->owner];

		if(b->stock[ZETROL] + p->global[ZETROL] <= 0.0f)
			continue;

		for(int j=0; j<PLAYERS; j++)
		{
			if(!g_diplomacy[b->owner][j])
				continue;

			if(j == b->owner)
				g_cheapfuel[j] = 0;
			else if(!found[j] || p->price[ZETROL] < g_cheapfuel[j])
				g_cheapfuel[j] = p->price[ZETROL];
		}
	}
}

void TripCost(TransportJob* tj)
{
	CUnit* u = &g_unit[tj->transporter];
	int owner = u->owner;
	tj->fuelcost = tj->pathlen * g_cheapfuel[owner] / FUEL_DISTANCE * TRUCK_CONSUMPTIONRATE;
	CPlayer* p = &g_player[owner];
	CBuilding* supb = &g_building[tj->supplier];
	CPlayer* supp = &g_player[supb->owner];
	CUnitType* t = &g_unitType[u->type];
	float speed = t->speed;
	float driveframes = tj->pathlen / t->speed;
	//tj->clientcharge = tj->pathlen * p->transportprice / TILE_SIZE;
	float payments = driveframes * 1000.0f / FRAME_RATE / DRIVE_WORK_DELAY;

	//g_log<<"payments = "<<payments<<" pathlen/TILE_SIZE = "<<(tj->pathlen/TILE_SIZE)<<endl;
	//g_log.flush();

	tj->totalcosttoclient = tj->clientcharge + p->price[tj->restype]*tj->resamt;
	tj->clientcharge = payments * p->transportprice;
	tj->driverwagepaid = payments * p->truckwage;
	tj->netprofit = tj->clientcharge - tj->fuelcost - tj->driverwagepaid;
}

// bid for transportation of resources to X
void TBid(int target, int target2, int targtype, int res, float amt)
{
#ifdef TRUCK_DEBUG
	g_log<<"-------------------"<<endl;
	g_log<<"TBid("<<target<<", "<<target2<<", "<<targtype<<", "<<res<<", "<<amt<<");"<<endl;
	g_log.flush();
#endif

	TransportJob cheapestTJ;
	cheapestTJ.path.clear();
	int player = -1;

	if(targtype == GOINGTODEMANDERB)
	{
		//char msg[128];
		//sprintf(msg, "bid %s", g_buildingType[g_building[target].type].name);
		//Chat(msg);
		player = g_building[target].owner;
		
#ifdef TRUCK_DEBUG
		g_log<<"demander b = "<<g_buildingType[g_building[target].type].name<<endl;
		g_log.flush();
#endif
	}
	else if(targtype == GOINGTODEMROAD)
	{
		//Chat("bid road");
		player = RoadAt(target, target2)->owner;
	}
	else if(targtype == GOINGTODEMPOWL)
	{
		//Chat("bid powl");
		player = PowlAt(target, target2)->owner;
	}
	else if(targtype == GOINGTODEMPIPE)
	{
		//Chat("bid pipe");
		player = PipeAt(target, target2)->owner;
	}

	vector<ResSource> *sourceBs = &g_sourceBs[player][res];
	
	if((*sourceBs).size() <= 0)
		FillResSources(res, player);

	// no available sources/paths?
	if((*sourceBs).size() > 0 && (*sourceBs)[0].supplier < 0)
	{
		
#ifdef TRUCK_DEBUG
		g_log<<"sources = none"<<endl;
		g_log.flush();
#endif
		return;
	}

	for(int i=0; i<g_freetrucks.size(); i++)
	{
		vector<TransportJob> TJs;
		int ID = g_freetrucks[i];
		CUnit* u = &g_unit[ID];

		
#ifdef TRUCK_DEBUG
		g_log<<"sources = "<<(*sourceBs).size()<<endl;
		g_log.flush();
#endif

		for(int j=0; j<(*sourceBs).size(); j++)
		{
			int sourceID = (*sourceBs)[j].supplier;

			//CBuilding* supb = &g_building[sourceID];

			//if(supb->transporter[res] >= 0)
			//	continue;
			/*
			if(targtype == GOINGTODEMANDERB)
			{
				g_log<<"CalcTrip of "<<g_resource[res].name<<" from "<<g_buildingType[g_building[sourceID].type].name<<" to "<<g_buildingType[g_building[target].type].name<<endl;
				g_log.flush();
			}*/

			TransportJob tj;
			
			CalcTrip(&tj, player, ID, GOINGTOSUPPLIER, sourceID, targtype, target, target2);

			if(tj.path.size() <= 0)
			{
#ifdef TRUCK_DEBUG
				g_log<<"UNSUCCESSFULL truck"<<ID<<endl;
				g_log.flush();
#endif
				continue;
			}
			
#ifdef TRUCK_DEBUG
			g_log<<"SUCCESS truck"<<ID<<" pathsize="<<tj.path.size()<<endl;
			g_log.flush();
#endif

			tj.restype = res;
			tj.resamt = min((*sourceBs)[j].amt, amt);
			TripCost(&tj);
		

			if(cheapestTJ.resamt <= 0 || tj.totalcosttoclient <= 0.0f || tj.resamt / tj.totalcosttoclient > cheapestTJ.resamt / cheapestTJ.totalcosttoclient)
			{
				cheapestTJ = tj;
			}
		}
	}
	
#ifdef TRUCK_DEBUG
	g_log<<"endbid----------------"<<endl;
	g_log.flush();
#endif

	// no available sources/paths?
	if(cheapestTJ.path.size() <= 0)
		return;

	CUnit* u = &g_unit[cheapestTJ.transporter];
	u->bids.push_back(cheapestTJ);
}

void ManageTrips()
{
	//g_freetrucks.clear();
	//return; 
	
	//LastNum("pre cheap fuel");

	CheapestFuel();

	for(int i=0; i<PLAYERS; i++)
		for(int j=0; j<RESOURCES; j++)
			g_sourceBs[i][j].clear();

	//LastNum("pre buildings trip");

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		CBuildingType* t = &g_buildingType[b->type];
		//CPlayer* p = &g_player[b->owner];
		
		// bid for transportation of resources to finished buildings
		if(b->finished)
		{
			//if(b->type == FACTORY)
			{
			//	char msg[128];
			//	sprintf(msg, "bid Bf 1 %s", t->name);
			//	Chat(msg);
			}
			//if(b->excessoutput())
			//	continue;
			//if(b->type == FACTORY)
			//	Chat("bid 1.1");
			//if(b->prodquota <= 0.0f)
			//	continue;
			//if(b->type == FACTORY)
			//	Chat("bid 1.2");
			for(int res=0; res<RESOURCES; res++)
			{
				CResource* r = &g_resource[res];
				if(r->capacity)
					continue;
				if(!r->physical)
					continue;
				if(res == LABOUR)
					continue;
				//if(b->type == FACTORY)
				//	Chat("bid Bf 2");
				if(b->transporter[res] >= 0)
					continue;
				//if(b->type == FACTORY)
				//	Chat("bid Bf 3");
				float netreq = b->netreq(res);
				if(netreq <= 0.0f)
					continue;
				//if(b->type == FACTORY)
				{
				//	char msg[128];
				//	sprintf(msg, "bid Bf 5 %s", t->name);
				//	Chat(msg);
				}
				TBid(i, -1, GOINGTODEMANDERB, res, netreq);
			}
		}
		// bid for transportation of resources to building construction project
		else
		{
			for(int res=0; res<RESOURCES; res++)
			{
				CResource* r = &g_resource[res];
				//Chat("1");
				if(r->capacity)
					continue;
				//Chat("2");
				if(!r->physical)
					continue;
				if(res == LABOUR)
					continue;
				//if(res == CEMENT)
				//	Chat("3");
				if(b->transporter[res] >= 0)
					continue;
				//if(res == CEMENT)
				//	Chat("4");
				float netreq = b->netreq(res);
				//if(res == CEMENT)
				//	Chat("5");
				if(netreq <= 0.0f)
					continue;
				//if(res == CEMENT)
				//	Chat("6");
				TBid(i, -1, GOINGTODEMANDERB, res, netreq);
			}
		}
	}

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CRoad* road = RoadAt(x, z);
			if(!road->on)
				continue;
			if(road->finished)
				continue;
#ifdef TRUCK_DEBUG
				g_log<<"road bid 0"<<endl;
				g_log.flush();
#endif
			for(int res=0; res<RESOURCES; res++)
			{
				CResource* r = &g_resource[res];
#ifdef TRUCK_DEBUG
				g_log<<"road bid 1"<<endl;
				g_log.flush();
#endif
				if(r->capacity)
					continue;
#ifdef TRUCK_DEBUG
				g_log<<"road bid 2"<<endl;
				g_log.flush();
#endif
				if(res == LABOUR)
					continue;
#ifdef TRUCK_DEBUG
				g_log<<"road bid 3"<<endl;
				g_log.flush();
#endif
				if(road->transporter[res] >= 0)
					continue;
#ifdef TRUCK_DEBUG
				g_log<<"road bid 4 res="<<g_resource[res].name<<endl;
				g_log.flush();
#endif
				float netreq = road->netreq(res);
				if(netreq <= 0.0f)
					continue;
#ifdef TRUCK_DEBUG
				g_log<<"road bid 5"<<endl;
				g_log.flush();
#endif
				//Chat("bid r");
				TBid(x, z, GOINGTODEMROAD, res, netreq);
			}
		}
	
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CPowerline* powl = PowlAt(x, z);
			if(!powl->on)
				continue;
			for(int res=0; res<RESOURCES; res++)
			{
				CResource* r = &g_resource[res];
				if(r->capacity)
					continue;
				if(res == LABOUR)
					continue;
				if(powl->transporter[res] >= 0)
					continue;
				float netreq = powl->netreq(res);
				if(netreq <= 0.0f)
					continue;
				//Chat("bid r");
				TBid(x, z, GOINGTODEMPOWL, res, netreq);
			}
		}
	
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CPipeline* pipe = PipeAt(x, z);
			if(!pipe->on)
				continue;
			for(int res=0; res<RESOURCES; res++)
			{
				CResource* r = &g_resource[res];
				if(r->capacity)
					continue;
				if(res == LABOUR)
					continue;
				if(pipe->transporter[res] >= 0)
					continue;
				float netreq = pipe->netreq(res);
				if(netreq <= 0.0f)
					continue;
				//Chat("bid r");
				TBid(x, z, GOINGTODEMPIPE, res, netreq);
			}
		}

	//LastNum("pre pick up bids");

	// pick which bid to take up

	bool newjobs = false;

	for(int i=0; i<g_freetrucks.size(); i++)
	{
				//Chat("free truck");

		int truckID = g_freetrucks[i];
		CUnit* u = &g_unit[truckID];
		u->ResetMode();
		u->path.clear();
		u->step = 0;

		
	//LastNum("pick up bids 0");

		TransportJob* bestbid = NULL;
		bool needfuel = u->NeedFood();

		for(int j=0; j<u->bids.size(); j++)
		{
			TransportJob* bid = &u->bids[j];
			//if(bestbid == NULL || bid->netprofit > bestbid->netprofit || (needfuel && bid->targtype == GOINGTODEMANDERB && g_buildingType[g_building[bid->target].type].output[ZETROL] > 0.0f))
			if(bestbid == NULL || bid->netprofit > bestbid->netprofit || (needfuel && bid->targtype == GOINGTODEMANDERB && g_building[bid->target].type == REFINERY))
				bestbid = bid;
		}


		//char msg[128];
		//sprintf(msg, "bids truck #%d", truckID);
		//Chat(msg);
		
	//LastNum("pick up bids 1");

		if(bestbid != NULL
			//&& bestbid->netprofit < 0.0f
			)
		{
			//Chat("unpr");
			int truckplayer = u->owner;
			CPlayer* truckp = &g_player[truckplayer];

			if(rand()%2 == 1)
				truckp->transportprice += 0.01f;
			else
			{
				truckp->truckwage -= 0.01f;
				if(truckp->truckwage <= 0.0f)
					truckp->truckwage = 0.01;
			}
			
			//char msg[128];
			//sprintf(msg, "trnc %f", truckp->transportprice);
			//Chat(msg);
		}

		if(bestbid != NULL && bestbid->netprofit > 0.0f)
		{
			
			//sprintf(msg, "best bid truck #%d", truckID);
			//Chat(msg);

				//Chat("take up bid");
				
			int truckplayer = u->owner;
			int client = -1;
			
	//LastNum("pick up bids 1.1");

			if(bestbid->targtype == GOINGTODEMANDERB)
			{
				//g_log<<"bestbid->target = "<<bestbid->target<<endl;
				//g_log.flush();

				CBuilding* b = &g_building[bestbid->target];
				client = b->owner;
			}
			else if(bestbid->targtype == GOINGTODEMROAD)
			{
				CRoad* road = RoadAt(bestbid->target, bestbid->target2);
				client = road->owner;
			}
			else if(bestbid->targtype == GOINGTODEMPOWL)
			{
				CPowerline* powl = PowlAt(bestbid->target, bestbid->target2);
				client = powl->owner;
			}
			else if(bestbid->targtype == GOINGTODEMPIPE)
			{
				CPipeline* pipe = PipeAt(bestbid->target, bestbid->target2);
				client = pipe->owner;
			}

			CPlayer* truckp = &g_player[truckplayer];
			g_log<<"payments = "<<(bestbid->driverwagepaid/truckp->truckwage)<<" pathlen = "<<bestbid->pathlen<<" pathlen/TILE_SIZE = "<<(bestbid->pathlen/TILE_SIZE)<<endl;
			//g_log.flush();
			
	//LastNum("pick up bids 1.2");

			// subtract cost from client
			if(client >= 0 && truckplayer != client)
			{
				CPlayer* clientp = &g_player[client];
				clientp->global[CURRENC] -= bestbid->clientcharge;
				//CPlayer* truckp = &g_player[truckplayer];
				truckp->global[CURRENC] += bestbid->clientcharge;
				
				clientp->recentth.consumed[CURRENC] += bestbid->clientcharge;
				truckp->recentth.produced[CURRENC] += bestbid->clientcharge;

				NewTransx(u->camera.Position(), CURRENC, bestbid->clientcharge);

				char msg[128];
				sprintf(msg, "transport charge %d %s", (int)bestbid->resamt, g_resource[bestbid->restype].name);
				LogTransx(client, -bestbid->clientcharge, msg);
				LogTransx(truckplayer, bestbid->clientcharge, msg);

				// to do: check if went over budget, game over, etc.
				if(clientp->global[CURRENC] <= 0.0f)
					Bankrupt(client, "transport costs");
			}
			
	//LastNum("pick up bids 2");

			// to do... take up bid

			u->target = bestbid->target;
			u->target2 = bestbid->target2;
			u->supplier = bestbid->supplier;
			u->transportRes = bestbid->restype;
			u->transportAmt = 0;
			u->targtype = bestbid->targtype;
			CBuilding* supb = &g_building[bestbid->supplier];
			//supb->transporter[bestbid->restype] = truckID;
			u->mode = GOINGTOSUPPLIER;
			u->goal = supb->pos;

			if(bestbid->targtype == GOINGTODEMANDERB)
			{
				//LastNum("before take bid");

				CBuilding* b = &g_building[bestbid->target];
				b->transporter[bestbid->restype] = truckID;
				u->reqAmt = min(bestbid->resamt, b->netreq(bestbid->restype));

				//LastNum("after take bid");

				//g_log<<"after bid u->mode = "<<u->mode<<endl;
				//g_log.flush();

				//newjobs = true;
			}
			else if(bestbid->targtype == GOINGTODEMROAD)
			{
				CRoad* road = RoadAt(bestbid->target, bestbid->target2);
				road->transporter[bestbid->restype] = truckID;
				u->reqAmt = min(bestbid->resamt, road->netreq(bestbid->restype));
			}
			else if(bestbid->targtype == GOINGTODEMPOWL)
			{
				CPowerline* powl = PowlAt(bestbid->target, bestbid->target2);
				powl->transporter[bestbid->restype] = truckID;
				u->reqAmt = min(bestbid->resamt, powl->netreq(bestbid->restype));
			}
			else if(bestbid->targtype == GOINGTODEMPIPE)
			{
				CPipeline* pipe = PipeAt(bestbid->target, bestbid->target2);
				pipe->transporter[bestbid->restype] = truckID;
				u->reqAmt = min(bestbid->resamt, pipe->netreq(bestbid->restype));
			}
			
			//newjobs = true;
			NewJob(GOINGTOTRUCK, truckID, -1);
		}

	//LastNum("pick up bids 4");

		u->bids.clear();
		u->frameslookjobago = 0;
	}

	//if(newjobs)
	//	NewJob();
}