#include "../texture.h"
#include "tile.h"
#include "../utils.h"
#include "../sim/map.h"
#include "../render/heightmap.h"

TlType g_tiletype[TILE2PERMS];

void DefTl(int tiletype, const char* texpath, Vec2i spriteoffset, Vec2i spritesz)
{
	TlType* t = &g_tiletype[tiletype];
	//QueueTexture(&t->sprite.texindex, texpath, true);
	CreateTexture(t->sprite.texindex, texpath, true, false);
#if 0
	t->sprite.offset[0] = spriteoffset.x;
	t->sprite.offset[1] = spriteoffset.y;
	t->sprite.offset[2] = t->sprite.offset[0] + spritesz.x;
	t->sprite.offset[3] = t->sprite.offset[1] + spritesz.y;
#endif

#if 1
	char infopath[MAX_PATH+1];
	strcpy(infopath, texpath);
	StripExtension(infopath);
	strcat(infopath, ".txt");

	std::ifstream infos(infopath);

	if(!infos)
		return;

	int centeroff[2];
	int imagesz[2];
	int clipsz[2];

	infos>>centeroff[0]>>centeroff[1];
	infos>>imagesz[0]>>imagesz[1];
	infos>>clipsz[0]>>clipsz[1];

	t->sprite.offset[0] = -centeroff[0];
	t->sprite.offset[1] = -centeroff[1];
	t->sprite.offset[2] = t->sprite.offset[0] + imagesz[0];
	t->sprite.offset[3] = t->sprite.offset[1] + imagesz[1];
#endif
}

Tile &SurfTile(int tx, int tz)
{
	return g_hmap.m_surftile[ tz * g_hmap.m_widthx + tx ];
}
