

#include "unit.h"
#include "building.h"
#include "player.h"
#include "chat.h"
#include "projectile.h"
#include "main.h"
#include "selection.h"
#include "gui.h"
#include "main.h"
#include "model.h"
#include "sound.h"

bool CUnit::CheckTargetAvailability()
{
	CUnit* u;
	CBuilding* b;

	if(targetU)
	{
		u = &g_unit[target];

		if(!u->on)
			return false;

		if(u->hp <= 0.0f)
			return false;
	}
	else
	{
		b = &g_building[target];

		if(!b->on)
			return false;

		if(b->hp <= 0.0f)
			return false;
	}

	return true;
}

bool CUnit::FindTarget()
{
	if(g_player[g_localP].activity == ACTIVITY_NONE)
	{
		ResetMode();
		return false;
	}
	//Chat("find tar");

	bool isU = false;
	float nearestD = 9999999999.0f;
	int nearest = -1;

	CUnit* u;
	CBuilding* b;
	Vec3f p;
	float d;

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hp <= 0.0f)
			continue;

		if(u->owner < 0)
			continue;

		if(u->hidden())
			continue;

		if(g_diplomacy[owner][u->owner])
			continue;

		p = u->camera.Position();
		d = Magnitude2(p - camera.Position());

		if(nearest >= 0 && d > nearestD)
			continue;

		nearest = i;
		nearestD = d;
		isU = true;
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(b->hp <= 0.0f)
			continue;

		if(g_diplomacy[owner][b->owner])
			continue;

		p = b->pos;
		d = Magnitude2(p - camera.Position());

		if(nearest >= 0 && d > nearestD)
			continue;

		nearest = i;
		nearestD = d;
		isU = false;
	}

	if(nearest < 0)
		return false;

	target = nearest;
	targetU = isU;
	ResetGoal();
	nopath = false;

	if(isU)
		goal = g_unit[target].camera.Position();
	else
		goal = g_building[target].pos;

	return true;
}

Vec3f CUnit::EnemyPosition(int id, bool isU)
{
	if(isU)
		return g_unit[id].camera.Position();
	else
		return g_building[id].pos;
}

bool CUnit::WithinRange(Vec3f p)
{
	CUnitType* t = &g_unitType[type];
	float range = t->range;
	
	if(Magnitude2(p - camera.Position()) >= range*range)
		return false;

	return true;
}

bool CUnit::WithinView(Vec3f p)
{
	Vec3f d = p - camera.Position();
	float yaw = GetYaw(d.x, d.z);
	float yaw2 = yaw - 360.0f;
	float yaw3 = yaw + 360.0f;

	if(fabs(camera.Yaw() - yaw) <= UNIT_FOV || fabs(camera.Yaw() - yaw2) <= UNIT_FOV || fabs(camera.Yaw() - yaw3) <= UNIT_FOV)
		return true;

	return false;
}

void ClearAttackers(int id, bool isU)
{
	CUnit* u;

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(!u->IsMilitary())
			continue;

		if(u->targetU != isU)
			continue;

		if(u->target != id)
			continue;

		u->target = -1;
	}
}

void CUnit::Damage(int id, bool isU, float damage)
{
	CUnit* u;
	CBuilding* b;

	if(isU)
	{
		u = &g_unit[id];

		if(u->hp <= 0.0f)
			return;

		u->hp -= damage;

		if(u->hp <= 0.0f)
		{
			//char msg[128];
			//sprintf(msg, "dead %d p=%d", UnitID(this), u->owner);
			//Chat(msg);

			u->ResetMode();
			//u->camera.Stop();
			int oldowner = u->owner;

			if(u->type == TRUCK)
			{
				u->owner = owner;
				u->hp = g_unitType[u->type].hitpoints;
			}
			else
			{
				//u->on = false;
				u->framesleft = DEATH_FRAMES;

				CUnitType* t = &g_unitType[u->type];

				u->emitter.clear();
				for(int i=0; i<t->deademitter.size(); i++)
				{
					//Chat("new emitter counter");
					u->emitter.push_back(EmitterCounter());
				}
			}

			if(target == id && targetU == isU)
			{
				underOrder = false;
			}
			
			ClearAttackers(id, isU);
			CheckGameOver(oldowner);
		}
	}
	else
	{
		b = &g_building[id];

		b->hp -= damage;

		if(b->hp <= 0.0f)
		{
			//b->on = false;
			int oldowner = b->owner;
			b->convert(owner);
			ClearAttackers(id, isU);
			
			if(target == id && targetU == isU)
			{
				underOrder = false;
			}
			
			CheckGameOver(oldowner);
			//b->Explode();
		}
	}

	RecheckSelection();
	//RedoLeftPanel();
}

void CUnit::Shoot(int id, bool isU)
{
	LastNum("shoot");

	Vec3f p = EnemyPosition(id, isU);
	//last = GetTickCount();
	CUnitType* t = &g_unitType[type];
	framesleft = t->shotdelay/1000.0f * FRAME_RATE;

	if(t->atksnd.size() > 0)
		t->atksnd[ rand()%t->atksnd.size() ].play();

	NewProjectile(camera.Position() + Vec3f(0, 3.765f, 0), p + Vec3f(0, 3.765f, 0), GUNPROJ);
	Damage(id, isU, t->damage);
}

bool CUnit::canturn()
{
	if(Magnitude2(camera.Velocity()) <= 0.5f)
		return true;

	return false;
}

void CUnit::Aim(int* id, bool* isU)
{
	float nearestD = 9999999999.0f;
	(*id) = -1;

	CUnit* u;
	CBuilding* b;
	Vec3f p;
	float d;

	bool canTurn = canturn();

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->owner < 0)
			continue;

		if(u->hp <= 0.0f)
			continue;

		if(g_diplomacy[owner][u->owner])
			continue;

		p = u->camera.Position();

		if(!WithinRange(p))
			continue;

		if(!canTurn && !WithinView(p))
			continue;

		d = Magnitude2(p - camera.Position());

		if(d > nearestD)
			continue;

		if(canTurn && !WithinView(p))
			camera.View(p);

		(*id) = i;
		nearestD = d;
		(*isU) = true;
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		if(b->hp <= 0.0f)
			continue;

		if(g_diplomacy[owner][b->owner])
			continue;

		p = b->pos;
		
		if(!WithinRange(p))
			continue;
		
		if(!canTurn && !WithinView(p))
			continue;

		d = Magnitude2(p - camera.Position());

		if(d > nearestD)
			continue;

		if(canTurn && !WithinView(p))
			camera.View(p);

		(*id) = i;
		nearestD = d;
		(*isU) = false;
	}
}

bool CUnit::Shoot()
{
	if(!CheckTargetAvailability())
		target = -1;

	//CUnitType* t = &g_unitType[type];
	//int shotdelay = t->shotdelay;
	
	//if(GetTickCount() - last < shotdelay)
	//	return false;

	if(framesleft > 0)
	{
		framesleft --;
		return false;
	}

	if(target >= 0)
	{
		Vec3f p = EnemyPosition(target, targetU);

		if(WithinRange(p))
		{
			if(WithinView(p))
			{
				//char msg[128];
				//sprintf(msg, "withinv %d", UnitID(this));
				//Chat(msg);

				Shoot(target, targetU);
				return true;
			}
			else if(canturn())
			{
				//char msg[128];
				//sprintf(msg, "turn %d", UnitID(this));
				//Chat(msg);

				camera.View(p);
			}
		}
	}

	int id = -1;
	bool isU = false;

	Aim(&id, &isU);

	if(id < 0)
		return false;
	
	Shoot(id, isU);

	return true;
}

void CUnit::updatedead()
{
	framesleft --;

	//Chat("upd dead");

	if(framesleft <= 0)
	{
	//Chat("framesleft <= 0");
		on = false;
		return;
	}

	CUnitType* t = &g_unitType[type];

	if(t->walker)
	{
		PlayAnimation(frame[BODY_LOWER], 29.0f, 59.0f, false, 1.0f);
	}

	for(int i=0; i<t->deademitter.size(); i++)
	{
		EmitterPlace* ep = &t->deademitter[i];
		CParticleType* pt = &g_particleType[ep->type];
		if(emitter[i].EmitNext(pt->delay))
		{
			//Chat("emit next");
			EmitParticle(ep->type, camera.Position() + ep->offset);
		}
	}
}

void CUnit::UpdateMilitary()
{
	if(hp <= 0.0f)
	{
		updatedead();

		//char msg[128];
		//sprintf(msg, "updded %d @ %d", UnitID(this), (int)GetTickCount());
		//Chat(msg);

		return;
	}

	LastNum("pre shoot();");
	Shoot();

	//if(target >= 0)
	{
		//Chat("has targ");
	}

	if(target < 0 && !passive && !underOrder)
	{
	LastNum("pre findtarget();");
		FindTarget();
	}

	if(target < 0)
		return;
	
	LastNum("pre enemyposition();");
	Vec3f p = EnemyPosition(target, targetU);


	if(WithinRange(p))
	{
	LastNum("pre resetgoal();");
		ResetGoal();
	}
	else
	{
	//LastNum("pre set goal = p;");
	char msg[128];
	sprintf(msg, "pre set goal = p; %f,%f,%f", p.x, p.y, p.z);
	LastNum(msg);
		goal = p;
	}
}