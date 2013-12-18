


#include "platform.h"
#include "debug.h"
//#include "main.h"
//#include "gui.h"
//#include "unit.h"
//#include "pathfinding.h"
//#include "physics.h"
//#include "building.h"

Profile g_profile[PROFILES];
ofstream g_profF;

void LastNum(const char* l)
{
	return;

	ofstream last;
	last.open("last.txt", ios_base::out);
	last<<l;
	last.flush();
}

#if 0
void UDebug(int i)
{
	return;

	CUnit* u = &g_unit[i];
	CUnitType* t = &g_unitType[u->type];

	g_log<<"UNIT DEBUG: "<<t->name<<" ("<<i<<")"<<endl;
	g_log<<"path size: "<<u->path.size()<<endl;
	
	if(u->collidesfast())
	{
		g_log<<"COLLIDES: type:"<<g_colliderType<<" ID:"<<g_lastCollider<<endl;

		if(g_colliderType == COLLIDER_BUILDING)
		{
			CBuilding* b = &g_building[g_lastCollider];
			CBuildingType* bt = &g_buildingType[b->type];

			g_log<<"COLLIDER B: "<<bt->name<<endl;

			if(u->confirmcollision(g_colliderType, g_lastCollider))
			{
				g_log<<"CONFIRMED COLLISION"<<endl;

				Vec3f p = u->camera.Position();
				Vec3f p2 = b->pos;

				float r = t->radius;
				float hwx = bt->widthX*TILE_SIZE/2.0f;
				float hwz = bt->widthZ*TILE_SIZE/2.0f;

				g_log<<"COLLISION DX,DZ: "<<(fabs(p2.x-p.x)-r-hwx)<<","<<(fabs(p2.z-p.z)-r-hwz)<<endl;
			}
		}
	}
}

void StartProfile(int id)
{
	return;

	if(g_mode != PLAY)
		return;

	g_profile[id].starttick = GetTickCount();
}

void EndProfile(int id)
{
	return;

	if(g_mode != PLAY)
		return;

	if(id == FRAME || g_profile[id].lastframe < g_profile[FRAME].lastframe)
	{
		//g_profile[id].averagems = ( g_profile[id].lastframeaverage + g_profile[id].averagems*g_profile[id].frames ) / (g_profile[id].frames+1);
		g_profile[id].averagems = ( g_profile[id].lastframetotal + g_profile[id].averagems*g_profile[id].frames ) / (g_profile[id].frames+1);
		g_profile[id].frames+=1.0;
		//g_profile[id].timescountedperframe = 0;
		g_profile[id].lastframetotal = 0;
		g_profile[id].lastframe = g_profile[FRAME].lastframe;

		//g_log<<g_profile[id].name<<" "<<g_profile[id].averagems<<"ms"<<endl;
	}
	if(id == FRAME)
		g_profile[id].lastframe++;
	
	double elapsed = GetTickCount() - g_profile[id].starttick;
	g_profile[id].lastframetotal += elapsed;
	//g_profile[id].lastframeaverage = ( elapsed + g_profile[id].lastframeaverage*g_profile[id].timescountedperframe ) / (g_profile[id].timescountedperframe+1);
	//g_profile[id].timescountedperframe+=1.0f;
}

void WriteProfiles(int in, int layer)
{
	if(in == -1)
		g_profF.open("profiles.txt", ios_base::out);

	double totalms = 0;
	double percentage;

	for(int j=0; j<PROFILES; j++)
	{
		if(g_profile[j].inside != in)
			continue;

		totalms += g_profile[j].averagems;
	}

	for(int j=0; j<PROFILES; j++)
	{
		if(g_profile[j].inside != in)
			continue;

		percentage = 100.0 * g_profile[j].averagems / totalms;

		for(int k=0; k<layer; k++)
			g_profF<<"\t";

		g_profF<<g_profile[j].name<<" "<<g_profile[j].averagems<<"ms, "<<percentage<<"%"<<endl;

		WriteProfiles(j, layer+1);
	}

	if(in == -1)
		g_profF.flush();
}

void Profile(int id, int inside, char* name)
{
	g_profile[id].inside = inside;
	strcpy(g_profile[id].name, name);
}

void InitProfiles()
{
	Profile(FRAME, -1, "Frame");
	Profile(UPDATE, FRAME, "Update();");
	Profile(DRAW, FRAME, "Draw();");
	Profile(UPDATEUNITS, UPDATE, "UpdateUnits();");
	Profile(UPDATEBUILDINGS, UPDATE, "UpdateBuildings();");
	Profile(DRAWBUILDINGS, DRAW, "DrawBuildings();");
	Profile(DRAWUNITS, DRAW, "DrawUnits();");
	Profile(SORTPARTICLES, DRAW, "SortParticles();");
	Profile(DRAWPARTICLES, DRAW, "DrawParticles();");
	Profile(DRAWMAP, DRAW, "DrawMap();");
	Profile(SHADOWS, DRAW, "Shadows");
	Profile(DRAWSKY, DRAW, "DrawSky();");
	Profile(DRAWPOWERLINES, DRAW, "DrawPowerlines();");
	Profile(DRAWROADS, DRAW, "DrawRoads();");
	Profile(DRAWMODEL1, DRAWBUILDINGS, "Draw model 1");
	Profile(DRAWMODEL2, DRAWBUILDINGS, "Draw model 2");
	Profile(DRAWMODEL3, DRAWBUILDINGS, "Draw model 3");
}

void UpdateFPS()
{
	//char msg[128];
	//sprintf(msg, "FPS: %f, %fms", 1000.0f / (float)g_profile[FRAME].lastframetotal, (float)g_profile[FRAME].lastframetotal);
	//g_GUI.getview("chat")->gettext("fps")->text = msg;
}

#endif