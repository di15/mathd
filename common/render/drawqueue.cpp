

#include "drawqueue.h"
#include "shader.h"
#include "tile.h"
#include "sprite.h"
#include "../texture.h"
#include "../platform.h"
#include "../utils.h"
#include "../math/isomath.h"
#include "../sim/map.h"
#include "../gui/gui.h"
#include "tile.h"
#include "../save/savemap.h"
#include "../sim/unit.h"
#include "../debug.h"
#include "heightmap.h"
#include "../sim/building.h"

void DrawQueue()
{
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthy; z++)
		{
#if 0
			Vec2i screenpos = CartToIso(Vec3i( x * TILE_SIZE, y * TILE_SIZE / 2, z * TILE_SIZE ));
			DrawImage( g_texture[texindex].texname, screenpos.x, screenpos.y, screenpos.x + TILE_PIXEL_WIDTH, screenpos.y + TILE_PIXEL_WIDTH/2);
#endif
#if 1
			Tile& tile = SurfTile(x, z);
			TlType* tiletype = &g_tiletype[tile.tiletype];
			//tiletype = &g_tiletype[TILE_0000];
			Vec3i cmpos( x * TILE_SIZE + TILE_SIZE/2, tile.tilepos.y * TILE_RISE, z * TILE_SIZE + TILE_SIZE/2 );
			//cmpos.y = 0;
			Vec2i screenpos = CartToIso(cmpos);
			//screenpos = CartToIso(Vec3i( x * TILE_SIZE, y * TILE_SIZE / 2, z * TILE_SIZE ));
			Sprite* sprite = &tiletype->sprite;
			Texture* tex = &g_texture[sprite->texindex];

			//g_log<<"xyz "<<cmpos.x<<","<<cmpos.y<<","<<cmpos.z<<" screen "<<screenpos.x<<","<<screenpos.y<<endl;

#if 0
			if(x == 0 && z == 0)
			{

				//g_log<<"00dr "<<(screenpos.x + sprite->offset[0])<<","<<(screenpos.y + sprite->offset[1])<<endl;
				g_log<<"tile 00 pos "<<screenpos.x<<","<<screenpos.y<<" hy "<<((int)Height(0,0)*TILE_SIZE/2)<<endl;
			}
#endif
#if 1
			DrawImage( tex->texname,
				screenpos.x + sprite->offset[0],
				screenpos.y + sprite->offset[1],
				screenpos.x + sprite->offset[2],
				screenpos.y + sprite->offset[3]);
#else
			DrawImage( tex->texname, screenpos.x, screenpos.y, screenpos.x + TILE_PIXEL_WIDTH, screenpos.y + TILE_PIXEL_WIDTH/2);
#endif
#endif
			CheckGLError();
		}

	DrawBls();
	DrawUnits();

#if 0
	Vec2i screensz;
	screensz.x = Max2Pow(g_width);
	screensz.y = Max2Pow(g_height);
	//DrawImage(g_screentex, 0, 0, screensz.x, screensz.y);
#endif
}
