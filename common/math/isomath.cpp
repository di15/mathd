#include "isomath.h"
#include "../sim/map.h"
#include "../platform.h"
#include "3dmath.h"
#include "../utils.h"

//cartesian to isometric
//i.e., 3d to screen
Vec2i CartToIso(Vec3i cmpos)
{
	Vec2i screenpos(0,0);
	
	screenpos.x += cmpos.x * TILE_PIXEL_WIDTH / 2 / TILE_SIZE;
	screenpos.y += cmpos.x * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2;
	
	screenpos.x -= cmpos.z * TILE_PIXEL_WIDTH / 2 / TILE_SIZE;
	screenpos.y += cmpos.z * TILE_PIXEL_WIDTH / 2 / TILE_SIZE / 2;

	//screenpos.y -= cmpos.y * (TILE_PIXEL_WIDTH / 4) / (TILE_SIZE/2);
	screenpos.y -= CartYToIso(cmpos.y);

	return screenpos;
}

int CartYToIso(int cmy)
{
	//return cmy / tan(DEGTORAD(30));
	//return cmy * (TILE_PIXEL_WIDTH / 8) / (TILE_SIZE/2);
	//g_log<<"cmy "<<cmy<<endl;
	//g_log<<"result "<<(cmy * TILE_PIXEL_RISE / TILE_RISE)<<endl;
	return cmy * TILE_PIXEL_RISE / TILE_RISE;
}