

#include "transport.h"
#include "building.h"
#include "unit.h"
#include "bltype.h"
#include "utype.h"
#include "conduit.h"
#include "resources.h"
#include "player.h"
#include "job.h"
#include "../render/transaction.h"
#include "../ai/ai.h"
#include "simdef.h"
#include "../econ/utility.h"

//not engine
#include "../../game/gui/chattext.h"

int BestSup(Vec2i demcmpos, int rtype, int* retutil, Vec2i* retcmpos)
{
	int bestsup = -1;
	int bestutil = -1;
	Resource* r = &g_resource[rtype];

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];
		BlType* bt = &g_bltype[b->type];

		if(bt->output[rtype] <= 0)
			continue;

		Player* supp = &g_player[b->owner];

		if(supp->global[rtype] + b->stocked[rtype] <= 0)
			continue;

		//check if distance is better or if there's no best yet

		Vec2i bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

		int dist = Magnitude(bcmpos - demcmpos);

		//if(dist > bestdist && bestdemb)
		//	continue;

		int margpr = b->price[rtype];

		int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

		if(util <= bestutil && bestsup >= 0)
			continue;

		bestsup = bi;
		bestutil = util;
		*retutil = util;
		*retcmpos = bcmpos;
	}

	return bestsup;
}

// bid for transportation of resources to X
void TBid(int target, int target2, int targtype, int cdtype, int res, int amt)
{
#ifdef TRUCK_DEBUG
	g_log<<"-------------------"<<endl;
	g_log<<"TBid("<<target<<", "<<target2<<", "<<targtype<<", "<<res<<", "<<amt<<");"<<endl;
	g_log.flush();
#endif
    
	Vec2i dempos;
	int demplayer = -1;
	int bestsup = -1;
    
	if(targtype == UMODE_GODEMB)
	{
		//char msg[128];
		//sprintf(msg, "bid %s", g_bltype[g_building[target].type].name);
		//Chat(msg);
		demplayer = g_building[target].owner;
        
#ifdef TRUCK_DEBUG
		g_log<<"demander b = "<<g_bltype[g_building[target].type].name<<endl;
		g_log.flush();
#endif
	}
	else if(targtype == UMODE_GODEMCD)
	{
		//Chat("bid road");
		CdTile* ctile = GetCo(cdtype, target, target2, false);
		demplayer = ctile->owner;
		
	}
    
	// no available sources/paths?

#if 0
	bool found = false;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[res] <= 0)
			continue;

		Player* bp = &g_player[b->owner];

		if(b->stocked[res] + bp->global[res] <= 0)
			continue;

		found = true;
	}

	if(!found)
		return;
#else
	int suputil;
	Vec2i suppos;
	bestsup = BestSup(dempos, res, &suputil, &suppos);

	if(bestsup < 0)
		return;
#endif

	int bestutil = -1;
	int bestunit = -1;
    
	for(int ui=0; ui<UNITS; ui++)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(u->type != UNIT_TRUCK)
			continue;

		if(u->mode != UMODE_NONE)
			continue;

		Player* up = &g_player[u->owner];

		//warning: possible overflow using Magnitude with distance of large maps, definitely on 255x255 tile maps.
		int trucktosup = Magnitude(u->cmpos - suppos);	//truck to supplier distance
		int suptodem = Magnitude(suppos - dempos);	//supplier to demander distance

		int dist = suptodem + trucktosup;
		int price = up->transpcost;
		int util = PhUtil(price, dist);

		if(util < bestutil)
			continue;

		bestutil = util;
		bestunit = ui;
	}
    
	// no transporters?
	if(bestunit < 0)
		return;
    
	Unit* u = &g_unit[bestunit];
	u->mode = UMODE_GOSUP;
	u->goal = suppos;
	u->cargoreq = amt;
	u->cargotype = res;
	u->cargoamt = 0;
	u->target = target;
	u->target2 = target2;
	u->targtype = targtype;
	u->cdtype = cdtype;
	u->supplier = bestsup;
}

void ManageTrips()
{
	//g_freetrucks.clear();
	//return;
    
	//LastNum("pre cheap fuel");
    
	//LastNum("pre buildings trip");
    
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];
        
		if(!b->on)
			continue;
        
		//BlType* t = &g_bltype[b->type];
		//Player* p = &g_player[b->owner];
        
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
			for(int ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(r->capacity)
					continue;

				if(!r->physical)
					continue;

				if(ri == RES_LABOUR)
					continue;

				//if(b->type == FACTORY)
				//	Chat("bid Bf 2");

				if(b->transporter[ri] >= 0)
					continue;

				//if(b->type == FACTORY)
				//	Chat("bid Bf 3");

				int netreq = b->netreq(ri);

				if(netreq <= 0)
					continue;

				//if(b->type == FACTORY)
				{
                    //	char msg[128];
                    //	sprintf(msg, "bid Bf 5 %s", t->name);
                    //	Chat(msg);
				}

				TBid(i, -1, UMODE_GODEMB, -1, ri, netreq);
			}
		}
		// bid for transportation of resources to building construction project
		else
		{
			for(int ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];
				//Chat("1");

				if(r->capacity)
					continue;

				//Chat("2");

				if(!r->physical)
					continue;

				if(ri == RES_LABOUR)
					continue;

				//if(ri == CEMENT)
				//	Chat("3");

				if(b->transporter[ri] >= 0)
					continue;

				//if(ri == CEMENT)
				//	Chat("4");

				int netreq = b->netreq(ri);

				//if(ri == CEMENT)
				//	Chat("5");

				if(netreq <= 0)
					continue;

				//if(ri == CEMENT)
				//	Chat("6");

				TBid(i, -1, UMODE_GODEMB, -1, ri, netreq);
			}
		}
	}
    
	for(int x=0; x<g_map.m_widthX; x++)
		for(int z=0; z<g_map.m_widthZ; z++)
		{
			CdTile* road = RoadAt(x, z);
			if(!road->on)
				continue;
			if(road->finished)
				continue;
#ifdef TRUCK_DEBUG
            g_log<<"road bid 0"<<endl;
            g_log.flush();
#endif
			for(int ri=0; ri<RESOURCES; ri++)
			{
				CResource* r = &g_resource[ri];
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
				if(ri == LABOUR)
					continue;
#ifdef TRUCK_DEBUG
				g_log<<"road bid 3"<<endl;
				g_log.flush();
#endif
				if(road->transporter[ri] >= 0)
					continue;
#ifdef TRUCK_DEBUG
				g_log<<"road bid 4 ri="<<g_resource[ri].name<<endl;
				g_log.flush();
#endif
				int netreq = road->netreq(ri);
				if(netreq <= 0.0f)
					continue;
#ifdef TRUCK_DEBUG
				g_log<<"road bid 5"<<endl;
				g_log.flush();
#endif
				//Chat("bid r");
				TBid(x, z, GOINGTODEMROAD, ri, netreq);
			}
		}
    
	//LastNum("pre pick up bids");
    
	// pick which bid to take up
    
	//bool newjobs = false;
    
	for(int i=0; i<g_freetrucks.size(); i++)
	{
        //Chat("free truck");
        
		int truckID = g_freetrucks[i];
		Unit* u = &g_unit[truckID];
		u->ResetMode();
		u->path.clear();
		u->step = 0;
        
        
        //LastNum("pick up bids 0");
        
		TransportJob* bestbid = NULL;
		bool needfuel = u->NeedFood();
        
		for(int j=0; j<u->bids.size(); j++)
		{
			TransportJob* bid = &u->bids[j];
			//if(bestbid == NULL || bid->netprofit > bestbid->netprofit || (needfuel && bid->targtype == UMODE_GODEMB && g_bltype[g_building[bid->target].type].output[ZETROL] > 0.0f))
			if(bestbid == NULL || bid->netprofit > bestbid->netprofit || (needfuel && bid->targtype == UMODE_GODEMB && g_building[bid->target].type == REFINERY))
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
			Player* truckp = &g_player[truckplayer];
            
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
            
			if(bestbid->targtype == UMODE_GODEMB)
			{
				//g_log<<"bestbid->target = "<<bestbid->target<<endl;
				//g_log.flush();
                
				Building* b = &g_building[bestbid->target];
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
            
			Player* truckp = &g_player[truckplayer];
			//g_log<<"payments = "<<(bestbid->driverwagepaid/truckp->truckwage)<<" pathlen = "<<bestbid->pathlen<<" pathlen/TILE_SIZE = "<<(bestbid->pathlen/TILE_SIZE)<<endl;
			//g_log.flush();
            
            //LastNum("pick up bids 1.2");
            
			// subtract cost from client
			if(client >= 0 && truckplayer != client)
			{
				Player* clientp = &g_player[client];
				clientp->global[CURRENC] -= bestbid->clientcharge;
				//Player* truckp = &g_player[truckplayer];
				truckp->global[CURRENC] += bestbid->clientcharge;
                
				clientp->recentth.consumed[CURRENC] += bestbid->clientcharge;
				truckp->recentth.produced[CURRENC] += bestbid->clientcharge;
                
				NewTransx(u->camera.Position(), CURRENC, bestbid->clientcharge);
                
				//char msg[128];
				//sprintf(msg, "transport charge %d %s", (int)bestbid->resamt, g_resource[bestbid->restype].name);
				//LogTransx(client, -bestbid->clientcharge, msg);
				//LogTransx(truckplayer, bestbid->clientcharge, msg);
                
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
			Building* supb = &g_building[bestbid->supplier];
			//supb->transporter[bestbid->restype] = truckID;
			u->mode = UMODE_GOSUP;
			u->goal = supb->pos;
            
			if(bestbid->targtype == UMODE_GODEMB)
			{
				//LastNum("before take bid");
                
				Building* b = &g_building[bestbid->target];
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