

#include "main.h"
#include "unit.h"
#include "building.h"
#include "map.h"
#include "road.h"
#include "chat.h"
#include "powerline.h"
#include "pipeline.h"
#include "physics.h"
#include "pathfinding.h"
#include "selection.h"
#include "transaction.h"

bool CUnit::CheckIfArrived()
{
	CBuilding* b;
	CBuildingType* bt;
	float hwx, hwz;
	Vec3f p;
	float r = g_unitType[type].radius;
	CUnit* u;
	CUnitType* ut;
	float r2;

#ifdef PATH_DEBUG
	int uID = UnitID(this);

	if(uID == 5)
	{
		g_log<<"7 checkifarrived"<<endl;
	}
#endif

	switch(mode)
	{
	case GOINGTONORMJOB:
	case GOINGTOCONJOB:
	case GOINGTOSHOP:
	case GOINGTOREST:
	case GOINGTODEMANDERB:
		b = &g_building[target];
		bt = &g_buildingType[b->type];
		p = b->pos;
		hwx = bt->widthX*TILE_SIZE/2;
		hwz = bt->widthZ*TILE_SIZE/2;
		if(fabs(p.x-camera.Position().x) <= hwx+r && fabs(p.z-camera.Position().z) <= hwz+r)
			return true;
		break;
	case GOINGTOROADJOB:
	case GOINGTODEMROAD:
		r2 = TILE_SIZE;
		p = RoadPosition(target, target2);
		if(fabs(p.x-camera.Position().x) <= r2+r && fabs(p.z-camera.Position().z) <= r2+r)
			return true;
		break;
	case GOINGTOPIPEJOB:
	case GOINGTODEMPIPE:
		r2 = TILE_SIZE/2;
		p = PipelinePhysPos(target, target2);
		if(fabs(p.x-camera.Position().x) <= r2+r && fabs(p.z-camera.Position().z) <= r2+r)
			return true;
		break;
	case GOINGTOPOWLJOB:
	case GOINGTODEMPOWL:
		r2 = TILE_SIZE/2;
		p = PowerlinePosition(target, target2);
		if(fabs(p.x-camera.Position().x) <= r2+r && fabs(p.z-camera.Position().z) <= r2+r)
			return true;
#ifdef PATH_DEBUG
	if(uID == 5)
	{
		g_log<<"7 checkifarrived dx,dz: "<<(fabs(p.x-camera.Position().x)-r2-r)<<","<<(fabs(p.z-camera.Position().z)-r2-r)<<endl;
	}
#endif
		break;
	case GOINGTOSUPPLIER:
		b = &g_building[supplier];
		bt = &g_buildingType[b->type];
		p = b->pos;
		hwx = bt->widthX*TILE_SIZE/2;
		hwz = bt->widthZ*TILE_SIZE/2;
		if(fabs(p.x-camera.Position().x) <= hwx+r && fabs(p.z-camera.Position().z) <= hwz+r)
			return true;
		break;
	case GOINGTOREFUEL:
		b = &g_building[fuelStation];
		bt = &g_buildingType[b->type];
		p = b->pos;
		hwx = bt->widthX*TILE_SIZE/2;
		hwz = bt->widthZ*TILE_SIZE/2;
		if(fabs(p.x-camera.Position().x) <= hwx+r && fabs(p.z-camera.Position().z) <= hwz+r)
			return true;
		break;/*
	case GOINGTOROADJOB:
		p = RoadPosition(target, target2);
		hwx = TILE_SIZE/2;
		hwz = TILE_SIZE/2;
		if(fabs(p.x-camera.Position().x) < hwx+r && fabs(p.z-camera.Position().z) < hwz+r)
			return true;
		break;
	case GOINGTOPOWLJOB:
		p = PowerlinePosition(target, target2);
		hwx = TILE_SIZE/2;
		hwz = TILE_SIZE/2;
		if(fabs(p.x-camera.Position().x) < hwx+r && fabs(p.z-camera.Position().z) < hwz+r)
			return true;
		break;
	case GOINGTOPIPEJOB:
		p = PipelinePhysPos(target, target2);
		hwx = TILE_SIZE/2;
		hwz = TILE_SIZE/2;
		if(fabs(p.x-camera.Position().x) < hwx+r && fabs(p.z-camera.Position().z) < hwz+r)
			return true;
		break;*/
	case GOINGTOTRUCK:
		u = &g_unit[target];
		ut = &g_unitType[u->type];
		p = u->camera.Position();
		r2 = ut->radius;
		if(fabs(p.x-camera.Position().x) <= r2+r && fabs(p.z-camera.Position().z) <= r2+r)
		{
#ifdef PATH_DEBUG
	if(uID == 5)
		g_log<<"ARRIVED"<<endl;
#endif
			return true;
		}
#ifdef PATH_DEBUG
	if(uID == 5)
	{
		g_log<<"7 checkifarrived truck dx,dz: "<<(fabs(p.x-camera.Position().x)-r2-r)<<","<<(fabs(p.z-camera.Position().z)-r2-r)<<endl;
	}
#endif
		break;
	default: break;
	};

	return false;
}

void CUnit::OnArrived()
{
	switch(mode)
	{
	case GOINGTONORMJOB:		mode = NORMJOB;		freecollidercells();	ResetGoal();	g_building[target].addoccupier(this);		break;
	case GOINGTOCONJOB:			mode = CONJOB;		freecollidercells();	ResetGoal();	break;
	case GOINGTOROADJOB:		mode = ROADJOB;		freecollidercells();	ResetGoal();	break;
	case GOINGTOPOWLJOB:		mode = POWLJOB;		freecollidercells();	ResetGoal();	break;
	case GOINGTOPIPEJOB:		mode = PIPEJOB;		freecollidercells();	ResetGoal();	break;
	case GOINGTOSHOP:			mode = SHOPPING;	freecollidercells();	ResetGoal();	break;
	case GOINGTOREST:			mode = RESTING;		freecollidercells();	ResetGoal();	break;
	case GOINGTOTRUCK:			ArrivedAtTruck();	freecollidercells();	ResetGoal();	break;
	case GOINGTODEMANDERB:	
		if(driver >= 0)
			g_unit[driver].Disembark();
		driver = -1;
		mode = ATDEMANDERB;	
		ResetGoal();	
		break;
	case GOINGTODEMROAD:
		if(driver >= 0)
			g_unit[driver].Disembark();
		driver = -1;
		mode = ATDEMROAD;	
		ResetGoal();	
		break;
	case GOINGTODEMPOWL:
		if(driver >= 0)
			g_unit[driver].Disembark();
		driver = -1;
		mode = ATDEMPOWL;	
		ResetGoal();	
		break;
	case GOINGTODEMPIPE:
		if(driver >= 0)
			g_unit[driver].Disembark();
		driver = -1;
		mode = ATDEMPIPE;	
		ResetGoal();	
		break;
	case GOINGTOSUPPLIER:
		if(driver >= 0)
			g_unit[driver].Disembark();
		driver = -1;
		mode = ATSUPPLIER;	
		ResetGoal();	
		break;
	case GOINGTOREFUEL:
		if(driver >= 0)
			g_unit[driver].Disembark();
		driver = -1;
		mode = REFUELING;	
		ResetGoal();	
		break;
	default: break;
	};

	//if(type == TRUCK && UnitSelected(this))
	//	RedoLeftPanel();

	RecheckSelection();
}

void CUnit::ExchangeMomentum(int i)
{
	CUnit* u = &g_unit[i];

	Vec3f temp = camera.Velocity();
	camera.Velocity( u->camera.Velocity() );
	u->camera.Velocity(temp);
}

bool CUnit::AtGoal()
{
	CUnitType* t = &g_unitType[type];

	//if(Magnitude3(goal - camera.Position()) <= GOAL_PROXIMITY*GOAL_PROXIMITY)
	if(Magnitude3(goal - camera.Position()) <= t->speed*t->speed*2.0f)
		return true;

	return false;
}

bool CUnit::walking()
{
	CUnitType* t = &g_unitType[type];

	if(t->walker && Magnitude2(camera.Position() - oldPos) >= t->speed/1.1f && Magnitude3(camera.Position() - subgoal) > t->speed*t->speed*2.0f && CanMove())
		return true;

	return false;
}

void CUnit::Movement()
{
	//if(type == LABOURER)
	//	Chat("in move");
	/*
	if(type == TRUCK)
		{
			char msg[128];
			//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
			//Vec3f vw = camera.View() - camera.Position();
			//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
			Vec3f vel = camera.Velocity();
			//Vec3f offg = subgoal - camera.Position();
			//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
			sprintf(msg, "velbeforemov=%f,%f,%f", vel.x, vel.y, vel.z);
			int unID = UnitID(this);

			g_log<<"u["<<unID<<"]: "<<msg<<endl;
			g_log.flush();

			Chat(msg);
		}*/

	if(hidden())
		return;

	LastNum("in move");

	int last = path.size() - 1;

	if(last >= 0 && path[last] != goal)
	{
		//if(type == TRUCK)
		{
			//Chat("truck path1");
		}

		//if(canpath())
		{
	LastNum("pre pathfind 1");/*
	if(IsMilitary())
	{
		NewTransx(camera.Position(), "path 1");
		Chat("mil path 1");
	}
	*/
			if(!Pathfind(false))
			{
				
				if(type == TRUCK)
				{
					//Chat("truck !path1");
				}
				if(type == LABOURER)
				{
					ResetMode();
				}
			}
		}
		//else
		{
			//ResetPath();
			//path.clear();
			//path.push_back(goal);
			//subgoal = goal;
			//step = 0;
		}
	}
	else if(last < 0 && !AtGoal())
	{
		//if(type == TRUCK)
		{
			//Chat("truck path2");
		}

		//if(canpath())
		{
	LastNum("pre pathfind 2");/*
	if(IsMilitary())
	{
		NewTransx(camera.Position(), "path 2");
		Chat("mil path 2");
	}*/

			if(!Pathfind(false))
			{
				
				if(type == TRUCK)
				{
					//Chat("truck !path2");
				}
				if(type == LABOURER)
				{
					ResetMode();
				}
			}
		}
		//else
		{
			//ResetPath();
			//path.clear();
			//path.push_back(goal);
			//subgoal = goal;
			//step = 0;
		}
	}
	else if(last < 0 && AtGoal())
	{
	LastNum("pre repath");
		repath();
	}

	
	LastNum("in move 2");

	CUnitType* t = &g_unitType[type];

	Vec3f old = camera.Position();

	/*
		if(UnitID(this) == 0)
		{
			g_log<<"pathsize1="<<path.size()<<endl;
			g_log.flush();
		}
		*/

	
	/*	if(UnitID(this) == 0)
		{
			g_log<<"pathsize2="<<path.size()<<endl;
			g_log.flush();
		}
		*/
	/*
	if(type == TRUCK)
		{
			char msg[128];
			//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
			//Vec3f vw = camera.View() - camera.Position();
			//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
			Vec3f vel = camera.Velocity();
			//Vec3f offg = subgoal - camera.Position();
			//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
			sprintf(msg, "velbeforestep=%f,%f,%f", vel.x, vel.y, vel.z);
			int unID = UnitID(this);

			g_log<<"u["<<unID<<"]: "<<msg<<endl;
			g_log.flush();

			Chat(msg);
		}*/
	
	freecollidercells();

	camera.Step();
	Vec3f pos = camera.Position();
	pos.y = Bilerp(pos.x, pos.z);
	camera.MoveTo(pos);
	/*
	if(type == TRUCK)
		{
			char msg[128];
			//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
			//Vec3f vw = camera.View() - camera.Position();
			//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
			//Vec3f vel = camera.Velocity();
			Vec3f sc = camera.Position() - old;
			//Vec3f offg = subgoal - camera.Position();
			//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
			sprintf(msg, "stepchange=%f,%f,%f", sc.x, sc.y, sc.z);
			int unID = UnitID(this);

			g_log<<"u["<<unID<<"]: "<<msg<<endl;
			g_log.flush();

			Chat(msg);
		}*/
	
	if(IsMilitary() && Magnitude3(camera.Position() - goal) <= t->radius)
	{
		ResetGoal();
	}
	
	//if(last >= 0 && Magnitude3(camera.Position() - subgoal) <= GOAL_PROXIMITY*GOAL_PROXIMITY)
	else if(last >= 0 && Magnitude3(camera.Position() - subgoal) <= t->radius*t->radius/5.0f)
	//if(Magnitude3(camera.Position() - subgoal) <= Magnitude3(camera.Velocity())
	{
		camera.MoveTo(subgoal);

		if(last >= 0)
		{
			step ++;
			if(step >= path.size())
			{
				frame[BODY_LOWER] = 0;
				ResetGoal();
			}
			else
				subgoal = path[step];
		}

		//if(type == TRUCK)
		{
		//	char msg[128];
		//	sprintf(msg, "truck next subg @ %d", (int)GetTickCount());
		//	Chat(msg);
		}
	}
	//if(subgoal != camera.Position())
	//if(Magnitude3(camera.Position() - subgoal) > GOAL_PROXIMITY*GOAL_PROXIMITY && CanMove())

	if(Magnitude3(camera.Position() - subgoal) > t->radius*t->radius/5.0f && CanMove())
	{
		subgoal.y = Bilerp(subgoal.x, subgoal.z);
		camera.View(subgoal);
		camera.Accelerate(g_unitType[type].speed);
		
		/*if(type == TRUCK)
		{
			char msg[128];
			//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
			Vec3f vw = camera.View() - camera.Position();
			//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
			Vec3f vel = camera.Velocity();
			Vec3f offg = subgoal - camera.Position();
			sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);

			int unID = UnitID(this);

			g_log<<"u["<<unID<<"]: "<<msg<<endl;
			g_log.flush();

			Chat(msg);
		}*/
	}

	if(type == TRUCK)
		Accum(Magnitude(camera.Velocity()));

	
		//if(type == TRUCK)
		{
			//char msg[128];
			//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
			//Chat(msg);
		}
		
		
	fillcollidercells();

	//if(Collides())
	if(collidesfast())
	{
		frame[BODY_LOWER] = 0;
		bool arrived = CheckIfArrived();
		
		if(!arrived && g_colliderType == COLLIDER_UNIT)
		{
			Vec3f thisv = camera.Velocity();
			CUnit* u = &g_unit[g_lastCollider];
			camera.Velocity(u->camera.Velocity());
			u->camera.Velocity(thisv);
		}

		freecollidercells();
		camera.MoveTo(old);
		fillcollidercells();
		//camera.Stop();
		ResetPath();

		/*
		if(UnitID(this) == 0)
		{
			g_log<<"pathsize3="<<path.size()<<endl;
			
			g_log<<"g_colt = "<<g_colliderType<<endl;
			g_log<<"g_lastc = "<<g_lastCollider<<endl;

			if(g_colliderType == COLLIDER_BUILDING)
			{
				g_log<<"colb = "<<g_buildingType[g_building[g_lastCollider].type].name<<endl;
			}

			g_log.flush();
		}
		*/

		if(arrived)
			OnArrived();
	}
	else if(walking())
	{
		PlayAnimation(frame[BODY_LOWER], 0, 29, true, 1.5f);
	}
	
	//if(Magnitude3(camera.Position() - goal) <= GOAL_PROXIMITY*GOAL_PROXIMITY)
	if(Magnitude3(camera.Position() - goal) <= t->speed*t->speed*2.0f)
	{
		if(target < 0)
			underOrder = false;
		frame[BODY_LOWER] = 0;
	}
	/*
	if(type == TRUCK)
		{
			char msg[128];
			//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
			//Vec3f vw = camera.View() - camera.Position();
			//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
			Vec3f vel = camera.Velocity();
			//Vec3f offg = subgoal - camera.Position();
			//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
			sprintf(msg, "velbeforefriction=%f,%f,%f", vel.x, vel.y, vel.z);
			int unID = UnitID(this);

			g_log<<"u["<<unID<<"]: "<<msg<<endl;
			g_log.flush();

			Chat(msg);
		}*/

	//camera.LimitHVel(t->speed);
	camera.Friction();
	//camera.Friction();
	/*
	if(type == TRUCK)
		{
			char msg[128];
			//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
			//Vec3f vw = camera.View() - camera.Position();
			//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
			Vec3f vel = camera.Velocity();
			//Vec3f offg = subgoal - camera.Position();
			//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
			sprintf(msg, "velafterfriction=%f,%f,%f", vel.x, vel.y, vel.z);
			int unID = UnitID(this);

			g_log<<"u["<<unID<<"]: "<<msg<<endl;
			g_log.flush();

			Chat(msg);
		}*/
}