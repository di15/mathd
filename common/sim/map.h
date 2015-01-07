#ifndef MAP_H
#define MAP_H

#include "../math/vec2i.h"
#include "../math/vec3i.h"
#include "../math/vec2f.h"
#include "../render/tile.h"

#define TILE_SIZE		(10*100)	//10 meters = 1,000 centimeters
//#define TILE_RISE		(TILE_SIZE/3)
#define TILE_DIAG		(sqrt(TILE_SIZE*TILE_SIZE*2))
//#define TILE_RISE		(tan(DEGTORAD(30))*TILE_DIAG/2)
#define TILE_RISE		(tan(DEGTORAD(30))*TILE_DIAG/4)

extern Vec2f g_scroll;

void ScrollTo(int x, int z);

//extern unsigned char* g_hmap;
extern Tile* g_surftile;

#endif
