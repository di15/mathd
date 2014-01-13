

#include "sim.h"
#include "../platform.h"
#include "../texture.h"
#include "../render/heightmap.h"
#include "../render/model.h"

void Queue()
{
	QueueTexture(&g_tiletexs[TILE_SAND], "textures/terrain/default/sand.jpg", false);
	QueueTexture(&g_tiletexs[TILE_GRASS], "textures/terrain/default/grass.jpg", false);
	QueueTexture(&g_tiletexs[TILE_SNOW], "textures/terrain/default/snow.jpg", false);
	QueueTexture(&g_tiletexs[TILE_ROCK], "textures/terrain/default/rock.jpg", false);

	//QueueModel(&themodel, "models/battlecomp/battlecomp.ms3d", Vec3f(0.1f,0.1f,0.1f) * 100 / 64, Vec3f(0,100,0));
}