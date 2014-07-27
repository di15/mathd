#include "labourer.h"
#include "../platform.h"
#include "../econ/demand.h"
#include "unittype.h"
#include "player.h"

bool NeedFood(Unit* u)
{
	return false;
}

bool FindFood(Unit* u)
{
	return false;
}

bool NeedRest(Unit* u)
{
	return false;
}

void GoHome(Unit* u)
{
}

bool FindRest(Unit* u)
{
	return false;
}

bool FindJob(Unit* u)
{
	return false;
}

// go to construction job
void GoToCstJob(Unit* u)
{
}

// do construction job
void DoCstJob(Unit* u)
{
}

// go to building job
void GoToBlJob(Unit* u)
{
}

// do building job
void DoBlJob(Unit* u)
{
}

// go to conduit (construction) job
void GoToCdJob(Unit* u)
{
}

// do conduit (construction) job
void DoCdJob(Unit* u)
{
}

// go shop
void GoShop(Unit* u)
{
}

// go rest
void GoRest(Unit* u)
{
}

// do shop
void DoShop(Unit* u)
{
}

// do rest
void DoRest(Unit* u)
{
}

// go to transport for drive job
void GoToTra(Unit* u)
{
}

// do transport drive job
void DoDrTra(Unit* u)
{
}


void UpdLab(Unit* u)
{
	u->belongings[RES_RETFOOD] -= LABOURER_FOODCONSUM;
	u->frameslookjobago ++;
    
	if(u->belongings[RES_RETFOOD] <= 0)
	{
		//Chat("Starvation!");
		u->destroy();
		return;
	}

	switch(u->mode)
	{
	case UMODE_NONE:
		{
			if(NeedFood(u) /* && rand()%5==1 */ && FindFood(u))
			{/*
			 if(uID == 2)
			 {
			 g_log<<"u[2]findfood"<<endl;
			 g_log.flush();
			 }*/
			}
			else if(NeedRest(u))
			{/*
			 if(uID == 2)
			 {
			 g_log<<"u[2]needrest"<<endl;
			 g_log.flush();
			 }*/
				/*
				if(UnitID(this) == 0)
				{
				g_log<<"0 needrest"<<endl;
				g_log.flush();
				}
				*/
				if(u->home >= 0)
				{/*
				 if(UnitID(this) == 0)
				 {
				 g_log<<"home >= 0"<<endl;
				 g_log.flush();
				 }
				 */
					//Chat("go home");
					GoHome(u);
				}
				else
				{
					/*
					if(UnitID(this) == 0)
					{
					g_log<<"findrest"<<endl;
					g_log.flush();
					}*/

					//Chat("find rest");
					FindRest(u);
				}
			}
			else if(u->belongings[RES_LABOUR] > 0)
			{/*
			 if(uID == 2)
			 {
			 g_log<<"u[2]findjob"<<endl;
			 g_log.flush();
			 }*/
				if(!FindJob(u))
				{
					//if(rand()%(FRAME_RATE*2) == 1)
					{
						//move randomly?
						//goal = camera.Position() + Vec3f(rand()%TILE_SIZE - TILE_SIZE/2, 0, rand()%TILE_SIZE - TILE_SIZE/2);
					}
				}
			}
		}break;

#if 0
#define UMODE_NONE					0
#define UMODE_GOINGTOBLJOB			1
#define UMODE_BLJOB					2
#define UMODE_GOINGTOCSTJOB			3	//going to construction job
#define UMODE_CSTJOB				4
#define UMODE_GOINGTOCDJOB			5	//going to conduit job
#define UMODE_CDJOB					6
#define UMODE_GOINGTOSHOP			7
#define UMODE_SHOPPING				8
#define UMODE_GOINGTOREST			9
#define UMODE_RESTING				10
#define	UMODE_GOINGTOTRANSP			11	//going to transport job
#define UMODE_TRANSPDRV				12	//driving transport
#define UMODE_GOINGTOSUPPLIER		13	//transporter going to supplier
#define UMODE_GOINGTODEMANDERB		14	//transporter going to demander bl
#define UMODE_GOINGTOREFUEL			15
#define UMODE_REFUELING				16
#define UMODE_ATDEMANDERB			17
#define UMODE_ATSUPPLIER			18
#define UMODE_GOINGTODEMCD			19	//going to demander conduit
#define UMODE_ATDEMCD				20	//at demander conduit
#endif

	case UMODE_GOINGTOCSTJOB:		GoToCstJob(u);		break;
	case UMODE_CSTJOB:				DoCstJob(u);		break;
	case UMODE_GOINGTOBLJOB:		GoToBlJob(u);		break;
	case UMODE_BLJOB:				DoBlJob(u);			break;
	case UMODE_GOINGTOCDJOB:		GoToCdJob(u);		break;
	case UMODE_CDJOB:				DoCdJob(u);			break;
	case UMODE_GOINGTOSHOP:			GoShop(u);			break;
	case UMODE_GOINGTOREST:			GoRest(u);			break;
	case UMODE_SHOPPING:			DoShop(u);			break;
	case UMODE_RESTING:				DoRest(u);			break;
	case UMODE_GOINGTOTRANSP:		GoToTra(u);			break;
	case UMODE_TRANSPDRV:			DoDrTra(u);			break;
	default: break;
	}
}
