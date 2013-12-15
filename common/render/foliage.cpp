


#include "model.h"
#include "scenery.h"
#include "map.h"
#include "water.h"

SceneryType g_sceneryType[SCENERY_TYPES];
Scenery g_scenery[SCENERY];

int g_scT = -1;

Scenery::Scenery()
{
	on = false;
}

void SDefine(int type, const char* name, const char* model, Vec3f scale, Vec3f translate, float hsize, float vsize)
{
	SceneryType* t = &g_sceneryType[type];

	strcpy(t->name, name);
	QueueModel(&t->model, model, scale, translate);
	t->hsize = hsize;
	t->vsize = vsize;
}

int NewScenery()
{
	for(int i=0; i<SCENERY; i++)
	{
		if(!g_scenery[i].on)
			return i;
	}

	return -1;
}

void PlaceScenery()
{
	if(g_scT < 0)
		return;

	int i = NewScenery();

	if(i < 0)
		return;

	Scenery* s = &g_scenery[i];
	s->on = true;
	s->pos = g_vMouse;
	s->type = g_scT;
	s->yaw = DEGTORAD((rand()%360));

	if(s->pos.y <= WATER_HEIGHT)
		s->on = false;
}

void InitScenery()
{
	SDefine(TREE1, "tree1", "models\\tree\\tree1.ms3d", Vec3f(1,1,1), Vec3f(0,0,0), 4, 15);
}

void DrawScenery()
{
	for(int i=0; i<SCENERY; i++)
	{
		Scenery* s = &g_scenery[i];

		if(!s->on)
			continue;

		SceneryType* t = &g_sceneryType[s->type];

		CModel* m = &g_model[t->model];

		m->draw(0, s->pos, s->yaw);
	}
}

void ClearScenery(float x, float z, float hwx, float hwz)
{
	for(int i=0; i<SCENERY; i++)
	{
		Scenery* s = &g_scenery[i];

		if(!s->on)
			continue;

		if(fabs(s->pos.x-x) <= hwx && fabs(s->pos.z-z) <= hwz)
			s->on = false;
	}
}

void ClearScenery()
{
	for(int i=0; i<SCENERY; i++)
	{
		g_scenery[i].on = false;
	}
}