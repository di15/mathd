#include "unit.h"
#include "../render/shader.h"
#include "unittype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../render/model.h"
#include "../math/hmapmath.h"
#include "unitmove.h"
#include "sim.h"
#include "labourer.h"
#include "../debug.h"
#include "../math/frustum.h"
#include "building.h"
#include "labourer.h"
#include "../econ/demand.h"
#include "../math/vec4f.h"

Unit g_unit[UNITS];

Unit::Unit()
{
	on = false;
	threadwait = false;
}

Unit::~Unit()
{
	destroy();
}

void Unit::destroy()
{
	on = false;
	threadwait = false;
}

void Unit::resetpath()
{
	path.clear();
	subgoal = cmpos;
	goal = cmpos;
	pathblocked = false;
}

void DrawUnits()
{
	for(int i=0; i<UNITS; i++)
	{
		StartTimer(TIMER_DRAWUMAT);

		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UType* t = &g_utype[u->type];

		Vec3f vmin(u->drawpos.x - t->size.x/2, u->drawpos.y, u->drawpos.z - t->size.x/2);
		Vec3f vmax(u->drawpos.x + t->size.x/2, u->drawpos.y + t->size.y, u->drawpos.z + t->size.x/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		Model* m = &g_model[t->model];

		StopTimer(TIMER_DRAWUMAT);

		m->draw(u->frame[BODY_LOWER], u->drawpos, u->rotation.y);
	}
}

int NewUnit()
{
	for(int i=0; i<UNITS; i++)
		if(!g_unit[i].on)
			return i;

	return -1;
}

// starting belongings for labourer
void StartBel(Unit* u)
{
	Zero(u->belongings);
	u->car = -1;
	u->home = -1;

	if(u->type == UNIT_LABOURER)
	{
		//if(u->owner >= 0)
		{
			//u->belongings[ RES_FUNDS ] = 100;
			//u->belongings[ RES_FUNDS ] = CYCLE_FRAMES * LABOURER_FOODCONSUM * 30;
			u->belongings[ RES_FUNDS ] = CYCLE_FRAMES/SIM_FRAME_RATE * LABOURER_FOODCONSUM * 10;
		}

		u->belongings[ RES_RETFOOD ] = STARTING_RETFOOD;
		u->belongings[ RES_LABOUR ] = STARTING_LABOUR;
	}
}

bool PlaceUnit(int type, Vec2i cmpos, int owner, int *reti)
{
	int i = NewUnit();

	if(i < 0)
		return false;

	if(reti)
		*reti = i;

#if 0
	bool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;

	/*
	The draw (floating-point) position vectory is used for drawing.
	*/
	Vec3f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec3i cmpos;
	Vec3f facing;
	Vec2f rotation;

	deque<Vec2i> path;
	Vec2i goal;

	int step;
	int target;
	int target2;
	bool targetu;
	bool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	bool passive;
	Vec2i prevpos;
	int taskframe;
	bool pathblocked;
	int frameslookjobago;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//std::vector<TransportJob> bids;

	float frame[2];
#endif

	Unit* u = &g_unit[i];
	UType* t = &g_utype[type];

	u->on = true;
	u->type = type;
	u->cmpos = cmpos;
	u->drawpos = Vec3f(cmpos.x, g_hmap.accheight(cmpos.x, cmpos.y), cmpos.y);
	u->owner = owner;
	u->path.clear();
	u->goal = cmpos;
	u->target = -1;
	u->target2 = -1;
	u->targetu = false;
	u->underorder = false;
	u->fuelstation = -1;
	//u->home = -1;
	StartBel(u);
	u->hp = t->starthp;
	u->passive = false;
	u->prevpos = u->cmpos;
	u->taskframe = 0;
	u->pathblocked = false;
	u->frameslookjobago = 0;
	u->supplier = -1;
	u->reqamt = 0;
	u->targtype = -1;
	u->frame[BODY_LOWER] = 0;
	u->frame[BODY_UPPER] = 0;
	u->subgoal = u->goal;

	u->mode = UMODE_NONE;
	u->pathdelay = 0;
	u->lastpath = g_simframe;

	u->cdtype = CONDUIT_NONE;
	u->driver = -1;
	//u->framesleft = 0;
	u->cyframes = WORK_DELAY-1;

	u->fillcollider();

	return true;
}

void FreeUnits()
{
	for(int i=0; i<UNITS; i++)
	{
		g_unit[i].destroy();
		g_unit[i].on = false;
	}
}

bool Unit::hidden() const
{
#if 0
	if(mode == NORMJOB)
		return true;
	if(mode == CONJOB)
		return true;
	if(mode == PIPEJOB)
		return true;
	if(mode == ROADJOB)
		return true;
	if(mode == POWLJOB)
		return true;
	if(mode == DRIVING)
		return true;
	if(mode == RESTING)
		return true;
	if(mode == SHOPPING)
		return true;
#endif
	return false;
}

void AnimUnit(Unit* u)
{
	UType* t = &g_utype[u->type];

	if(u->type == UNIT_ROBOSOLDIER || u->type == UNIT_LABOURER)
	{
		if(u->prevpos == u->cmpos)
		{
			u->frame[BODY_LOWER] = 0;
			return;
		}

		PlayAni(u->frame[BODY_LOWER], 0, 29, true, 1.0f);
	}
}

void UpdAI(Unit* u)
{
	//return;	//do nothing for now?

	if(u->type == UNIT_LABOURER)
		UpdLab(u);
}

void UpdUnits()
{
	for(int i = 0; i < UNITS; i++)
	{
		StartTimer(TIMER_UPDUONCHECK);

		Unit* u = &g_unit[i];

		if(!u->on)
		{
			StopTimer(TIMER_UPDUONCHECK);
			continue;
		}

		StopTimer(TIMER_UPDUONCHECK);

		StartTimer(TIMER_UPDUNITAI);
		UpdAI(u);
		StopTimer(TIMER_UPDUNITAI);
		StartTimer(TIMER_MOVEUNIT);
		MoveUnit(u);
		StopTimer(TIMER_MOVEUNIT);
		StartTimer(TIMER_ANIMUNIT);
		AnimUnit(u);
		StopTimer(TIMER_ANIMUNIT);
	}
}

void ResetPath(Unit* u)
{
	u->path.clear();
}

void ResetGoal(Unit* u)
{
	u->goal = u->subgoal = u->cmpos;
	ResetPath(u);
}

void ResetMode(Unit* u)
{
	//LastNum("resetmode 1");
	if(u->type == UNIT_LABOURER)
	{
		//LastNum("resetmode 1a");
		if(u->mode == UMODE_BLJOB)
			RemWorker(u);

		//if(hidden())
		//	relocate();
	}
	else if(u->type == UNIT_TRUCK)
	{
#if 0
		if(u->mode == UMODE_GOSUP
		                //|| mode == GOINGTOREFUEL
		                //|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
		  )
		{
			if(u->supplier >= 0)
			{
				Building* b = &g_building[u->supplier];
				b->transporter[u->transportRes] = -1;
			}
		}
		if((mode == GOINGTOSUPPLIER || mode == GOINGTODEMANDERB) && targtype == GOINGTODEMANDERB)
		{
			if(target >= 0)
			{
				CBuilding* b = &g_building[target];
				b->transporter[transportRes] = -1;
			}
		}
		else if((mode == GOINGTOSUPPLIER || mode == GOINGTODEMROAD) && targtype == GOINGTODEMROAD)
		{
			RoadAt(target, target2)->transporter[transportRes] = -1;
		}
		else if((mode == GOINGTOSUPPLIER || mode == GOINGTODEMPOWL) && targtype == GOINGTODEMPOWL)
		{
			PowlAt(target, target2)->transporter[transportRes] = -1;
		}
		else if((mode == GOINGTOSUPPLIER || mode == GOINGTODEMPIPE) && targtype == GOINGTODEMPIPE)
		{
			PipeAt(target, target2)->transporter[transportRes] = -1;
		}
#endif
		u->targtype = TARG_NONE;

		if(u->driver >= 0)
		{
			//LastNum("resetmode 1b");
			//g_unit[u->driver].Disembark();
			u->driver = -1;
		}
	}

	//LastNum("resetmode 2");

	//transportAmt = 0;
	u->target = u->target2 = -1;
	u->supplier = -1;
	u->mode = UMODE_NONE;
	ResetGoal(u);

	//LastNum("resetmode 3");
}

void ResetTarget(Unit* u)
{
	u->target = -1;
	ResetMode(u);
}

void DrawUOv(Matrix* mvp)
{
	Player* py = &g_player[g_curP];

	if(py->zoom < 0.6f)
		return;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != UNIT_LABOURER && u->type != UNIT_TRUCK)
			continue;

		if(!g_frustum.pointin(u->drawpos.x, u->drawpos.y, u->drawpos.z))
			continue;

		Vec4f screenpos = ScreenPos(mvp, u->drawpos, py->width, py->height, true);
		RichText rt;

		std::string mode;
		Building* b;
		BlType* bt;
		ConduitType* ct;

		switch(u->mode)
		{
		case UMODE_NONE:
			mode = "Idle";	
			break;
		case UMODE_GOBLJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "To job (" + std::string(bt->name) + ")";	
			break;
		case UMODE_BLJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "At job (" + std::string(bt->name) + ")";	
			break;
		case UMODE_GOCSTJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "To job (" + std::string(bt->name) + " construction)";
			break;
		case UMODE_CSTJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "At job (" + std::string(bt->name) + " construction)";
			break;
		case UMODE_GOCDJOB:
			ct = &g_cotype[u->cdtype];
			mode = "To job (" + std::string(ct->name) + " construction)";
			break;
		case UMODE_CDJOB:
			ct = &g_cotype[u->cdtype];
			mode = "At job (" + std::string(ct->name) + " construction)";
			break;
		case UMODE_GOSHOP:
			mode = "To store";
			break;
		case UMODE_SHOPPING:
			mode = "At store";
			break;
		case UMODE_GOREST:
			mode = "To home";
			break;
		case UMODE_RESTING:
			mode = "At home";
			break;
		case UMODE_GOTRANSP:
			mode = "To job (Trucking)";
			break;
		case UMODE_DRTRANSP:
			mode = "At job (Trucking)";
			break;
		case UMODE_GOSUP:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "To supplier (" + std::string(bt->name) + ")";
			break;
		case UMODE_GODEMB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "To demander (" + std::string(bt->name) + ")";
			break;
		case UMODE_GOREFUEL:
			mode = "To refuel";
			break;
		case UMODE_REFUELING:
			mode = "Refueling";
			break;
		case UMODE_ATDEMB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "Offloading (" + std::string(bt->name) + ")";
			break;
		case UMODE_ATSUP:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "Loading up (" + std::string(bt->name) + ")";
			break;
		case UMODE_GODEMCD:
			ct = &g_cotype[u->cdtype];
			mode = "To demander (" + std::string(ct->name) + ")";
			break;
		case UMODE_ATDEMCD:
			ct = &g_cotype[u->cdtype];
			mode = "Offloading (" + std::string(ct->name) + ")";
			break;
		default: 
			mode = "?";
			break;
		}

		rt.m_part.push_back(RichTextP(UString(mode.c_str())));
		DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, &rt);
		
		RichText rt2;
		
		if(u->type == UNIT_LABOURER)
		{
			char food[16];
			char labour[16];
			char funds[16];
			
			sprintf(food, "%d ", u->belongings[RES_RETFOOD]);
			sprintf(labour, "%d ", u->belongings[RES_LABOUR]);
			sprintf(funds, "%d ", u->belongings[RES_FUNDS]);
			
			rt2.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_RETFOOD));
			rt2.m_part.push_back(RichTextP(UString(food)));
			rt2.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_LABOUR));
			rt2.m_part.push_back(RichTextP(UString(labour)));
			rt2.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_DOLLARS));
			rt2.m_part.push_back(RichTextP(UString(funds)));
		}
		else if(u->type == UNIT_TRUCK)
		{
		}

		Font* f = &g_font[MAINFONT8];
		DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y + f->gheight, &rt2);
	}

	RichText rt("lkajslkdlads");
	DrawCenterShadText(MAINFONT32, 100,100, &rt);
}