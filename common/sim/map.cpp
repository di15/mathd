
#include "main.h"
#include "image.h"
#include "map.h"
#include "shader.h"
#include "3dmath.h"
#include "model.h"
#include "unit.h"
#include "selection.h"
#include "gui.h"
#include "menu.h"
#include "tileset.h"
#include "shadow.h"
#include "chat.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "pathfinding.h"
#include "ai.h"


void LoadMap()
{
	g_hmap.allocate(32, 32);
	//QueueModel(&g_hmap, "maps\\000\\map.ms3d", Vec3f(1, 1, 1), Vec3f(0, 0, 0));
}

void InitSurroundings()
{
	//QueueModel(&g_surroundings, "models\\surroundings\\surroundings.ms3d", Vec3f(1,1,1), Vec3f(0,0,0));
}

void DrawSurroundings()
{
	//g_model[g_surroundings].Draw(0, Vec3f(MAP_SIZE*TILE_SIZE/2, 0, MAP_SIZE*TILE_SIZE/2), 0);
}

void LoadTerrainTextures()
{
	/*
	CreateTexture(g_terrain, "enviro\\terrain.jpg");
	CreateTexture(g_detail, "enviro\\detail.jpg");
	CreateTexture(g_underground, "enviro\\underground.jpg");
	CreateTexture(g_detail2, "enviro\\detail2.jpg");
	CreateTexture(g_sky, "enviro\\sky.jpg");
	CreateTexture(g_circle, "gui\\circle.png");
	*/
	//QueueTexture(&g_terrain, "enviro\\terrain.jpg");
	//QueueTexture(&g_detail, "enviro\\detail.jpg");
	//QueueTexture(&g_underground, "enviro\\underground.jpg");
	//QueueTexture(&g_detail2, "enviro\\detail2.jpg");
	//QueueTexture(&g_sky, "enviro\\sky.jpg");
	QueueTexture(&g_circle, "gui\\circle.png", false);
	QueueTexture(&g_water, "enviro\\water.png", true);
}

void RandomMap()
{
	for(int i=0; i<10; i++)
	{
		Vec3f pos;
		pos.x = MIN_RADIUS + (g_hmap.m_widthX*TILE_SIZE-MIN_RADIUS*2)*(float)(rand()%10000)/10000.0f;
		pos.y = 0;
		pos.z = MIN_RADIUS + (g_hmap.m_widthZ*TILE_SIZE-MIN_RADIUS*2)*(float)(rand()%10000)/10000.0f;

		int u = PlaceUnit(LABOURER, -1, pos);
		g_unit[u].ResetLabourer();
	}
}