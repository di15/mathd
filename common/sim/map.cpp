#include "../window.h"
#include "map.h"
#include "../utils.h"
#include "../math/vec3i.h"

Vec2f g_scroll(0,0);
//unsigned char* g_hmap = NULL;
Tile* g_surftile = NULL;

void ScrollTo(int x, int z)
{
	g_scroll.x = x;
	g_scroll.y = z;
}
