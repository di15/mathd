

#include "gui.h"
#include "waves.h"
#include "unit.h"
#include "chat.h"
#include "main.h"

bool g_started;
long g_startTick;
long g_lastWave;
int g_waves = 0;

void Start()
{
	if(g_started)
		return;

	g_started = true;
	g_startTick = GetTickCount();
	g_lastWave = GetTickCount();
	g_waves = 0;
	//NextWave();
}

void Spawn(int ut)
{
	int i = NewUnit();
	CUnit* u = &g_unit[i];
	CUnitType* t = &g_unitType[ut];
	u->on = true;
	u->type = ut;
	u->camera.PositionCamera(0, 0, 0, 0, 0, 1, 0, 1, 0);
	u->hp = t->hitpoints;
	u->owner = 1;
	u->passive = false;
	u->ResetTarget();

	float left, top, right, bottom;
	float r = t->radius;
	float r2 = r*2.0f;
	float x, z;

	for(int shell=0; shell<50; shell++)
	{
		left = 0 + r + shell*r2;
		top = 0 + r + shell*r2;
		right = g_hmap.m_widthX*TILE_SIZE - r - shell*r2;
		bottom = g_hmap.m_widthZ*TILE_SIZE - r - shell*r2;

		// top row
		for(x=right, z=top; x>=left+r2; x-=r2)
		{
			u->camera.MoveTo(Vec3f(x, 0, z));
			if(!u->Collides2(false))
			{
				u->ResetMode();
				return;
			}
		}
		
		// left column
		for(x=left, z=top; z<=bottom-r2; z+=r2)
		{
			u->camera.MoveTo(Vec3f(x, 0, z));
			if(!u->Collides2(false))
			{
				u->ResetMode();
				return;
			}
		}

		// bottom row
		for(x=left, z=bottom; x<=right-r2; x+=r2)
		{
			u->camera.MoveTo(Vec3f(x, 0, z));
			if(!u->Collides2(false))
			{
				u->ResetMode();
				return;
			}
		}

		// right column
		for(x=right, z=bottom; z>=top+r2; z-=r2)
		{
			u->camera.MoveTo(Vec3f(x, 0, z));
			if(!u->Collides2(false))
			{
				u->ResetMode();
				return;
			}
		}
	}
}

void NextWave()
{
	if(g_waves == 0)
	{
		Spawn(BATTLECOMP);
	}
	else if(g_waves == 1)
	{
		Spawn(BATTLECOMP);
		Spawn(BATTLECOMP);
	}
	else if(g_waves == 2)
	{
		Spawn(BATTLECOMP);
		Spawn(BATTLECOMP);
		//Spawn(BATTLECOMP);
	}
	else if(g_waves >= 3)
	{
		Spawn(BATTLECOMP);
		Spawn(BATTLECOMP);
		//Spawn(BATTLECOMP);
		//Spawn(BATTLECOMP);
	}

	g_waves ++;
}

void UpdateTimes()
{
	if(!g_started)
		return;

	if(g_gameover)
		return;

	int elapsed = GetTickCount() - g_startTick;
	int seconds = elapsed/1000;
	int minutes = seconds/60;
	seconds = seconds - minutes*60;

	char time[32];
	sprintf(time, "%d:%02d", minutes, seconds);
	g_GUI.getview("game")->getwidget("time", TEXT)->text = time;

	elapsed = GetTickCount() - g_lastWave;

	int left = SPAWN_DELAY - elapsed;

	if(left <= 0)
	{
		left = 0;
		g_lastWave = GetTickCount();
		NextWave();
	}
	
	seconds = left/1000;
	minutes = seconds/60;
	seconds = seconds - minutes*60;
	
	sprintf(time, "%d:%02d", minutes, seconds);
	g_GUI.getview("game")->getwidget("wave", TEXT)->text = time;
}