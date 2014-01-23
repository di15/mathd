

#include "../render/heightmap.h"
#include "../texture.h"
#include "../utils.h"
#include "../math/vec3f.h"
#include "../window.h"
#include "../math/camera.h"
#include "../render/territory.h"

void LoadJPGMap(const char* relative)
{
	g_hmap.destroy();

	LoadedTex *pImage = NULL;

	char full[1024];
	FullPath(relative, full);

	pImage = LoadJPG(full);

	if(!pImage)
		return;

	g_hmap.allocate(pImage->sizeX-1, pImage->sizeY-1);

	for(int x=0; x<pImage->sizeX; x++)
	{
		for(int z=0; z<pImage->sizeY; z++)
		{
			float r = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 0 ];
			float g = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 1 ];
			float b = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 2 ];

			// Apply exponential scale to height data.
			float y = (r+g+b)/3.0f*TILE_Y_SCALE/255.0f - TILE_Y_SCALE/2.0f;
			y = y / fabs(y) * pow(fabs(y), TILE_Y_POWER) * TILE_Y_AFTERPOW;

			g_hmap.setheight(x, z, y);
		}
	}

	g_hmap.remesh();

	DivideTerritory();

	Vec3f center = Vec3f( g_hmap.m_widthx * TILE_SIZE/2.0f, g_hmap.getheight(g_hmap.m_widthx/2, g_hmap.m_widthz/2), g_hmap.m_widthz * TILE_SIZE/2.0f );
	Vec3f delta = center - g_camera.m_view;
	g_camera.move(delta);
	Vec3f viewvec = g_camera.m_view - g_camera.m_pos;
	viewvec = Normalize(viewvec) * max(g_hmap.m_widthx, g_hmap.m_widthz) * TILE_SIZE;
	g_camera.m_pos = g_camera.m_view - viewvec;
	g_zoom = INI_ZOOM;

	if(pImage)
	{
		if (pImage->data)							// If there is texture data
		{
			free(pImage->data);						// Free the texture data, we don't need it anymore
		}

		free(pImage);								// Free the image structure

		g_log<<relative<<"\n\r";
		g_log.flush();
	}
	
	g_camera.position(
		-1000.0f/3, 1000.0f/3 + 5000, -1000.0f/3, 
		0, 5000, 0, 
		0, 1, 0);
	
	g_camera.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	g_camera.move( Vec3f(g_hmap.m_widthx*TILE_SIZE/2, 1000, g_hmap.m_widthz*TILE_SIZE/2) );
}