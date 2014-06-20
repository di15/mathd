
#include "unittype.h"
#include "../texture.h"
#include "resources.h"
#include "../render/model.h"

UnitT g_unitT[UNIT_TYPES];

#if 0
void DefU(int type, const char* texrelative, Vec3i size, Vec2i bilbsize, const char* name, int starthp, bool landborne, bool walker, bool roaded, bool seaborne, bool airborne)
#else
void DefU(int type, const char* modelrelative, Vec3f scale, Vec3f translate, Vec3i size, const char* name, int starthp, bool landborne, bool walker, bool roaded, bool seaborne, bool airborne, int cmspeed, bool military)
#endif
{
	UnitT* t = &g_unitT[type];
#if 0
	QueueTexture(&t->texindex, texrelative, true);
	t->bilbsize = bilbsize;
#endif
	QueueModel(&t->model, modelrelative, scale, translate);
	t->size = size;
	strcpy(t->name, name);
	t->starthp = starthp;
	Zero(t->cost);
	t->landborne = landborne;
	t->walker = walker;
	t->roaded = roaded;
	t->seaborne = seaborne;
	t->airborne = airborne;
	t->cmspeed = cmspeed;
	t->military = true;
}