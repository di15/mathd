
#include "../math/vec3f.h"
#include "../render/heightmap.h"

void UpdateMouse3D()
{	
	/*
	Vec3f ray = OnNear(g_mouse.x, g_mouse.y);
	Vec3f line[2];
	line[0] = g_camera.Position();
	line[1] = g_camera.Position() + (ray * 100000.0f);
	GetMapIntersection(line, &g_vMouse);*/

	Vec3f ray = g_camera.View() - g_camera.Position();
	Vec3f onnear = OnNear(g_mouse.x, g_mouse.y);
	Vec3f line[2];
	line[0] = onnear;
	line[1] = onnear + (ray * 100000.0f);
	GetMapIntersection(line, &g_vMouse);
	
	g_vTile.x = Clip((int)(g_vMouse.x/TILE_SIZE)*TILE_SIZE, 0, (g_hmap.m_widthX-1)*TILE_SIZE);
	g_vTile.z = Clip((int)(g_vMouse.z/TILE_SIZE)*TILE_SIZE, 0, (g_hmap.m_widthZ-1)*TILE_SIZE);
	g_vTile.y = g_vMouse.y;

	if(g_build >= 0 && g_build < BUILDING_TYPES)
	{
		CBuildingType* t = &g_buildingType[g_build];

		if(t->widthX%2 == 1)
			g_vTile.x += 0.5f*TILE_SIZE;

		if(t->widthZ%2 == 1)
			g_vTile.z += 0.5f*TILE_SIZE;

		int startx = g_vTile.x/TILE_SIZE - t->widthX/2;
		int endx = startx + t->widthX - 1;
		int startz = g_vTile.z/TILE_SIZE - t->widthZ/2;
		int endz = startz + t->widthZ - 1;

		if(startx < 0)
			g_vTile.x += abs(startx)*TILE_SIZE;
		else if(endx >= g_hmap.m_widthX)
			g_vTile.x -= (g_hmap.m_widthX-endx+1)*TILE_SIZE;

		if(startz < 0)
			g_vTile.z += abs(startz)*TILE_SIZE;
		else if(endz >= g_hmap.m_widthZ)
			g_vTile.z -= (g_hmap.m_widthZ-endz+1)*TILE_SIZE;

		g_canPlace = CheckCanPlace(g_build, g_vTile);
	}
	else if(g_build == ROAD)
		UpdateRoadPlans();
	else if(g_build == POWERLINE)
		UpdatePowerlinePlans();
	else if(g_build == PIPELINE)
		UpdatePipelinePlans();
}