#include "../platform.h"
#include "heightmap.h"

#include "../render/shader.h"
#include "../render/shadow.h"
#include "../texture.h"
#include "../gui/gui.h"
#include "../gui/icon.h"
#include "../math/vec4f.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../utils.h"
#include "../math/camera.h"
#include "../window.h"
#include "../math/polygon.h"
#include "../math/physics.h"
#include "../sim/road.h"
#include "../sim/powl.h"
#include "../sim/crpipe.h"
#include "../math/plane3f.h"
#include "../path/collidertile.h"
#include "foliage.h"
#include "../save/savemap.h"
#include "texture.h"
#include "water.h"
#include "../sim/player.h"
#include "../debug.h"

Vec2uc g_mapsz = Vec2uc(0,0);
unsigned char* g_hmap = NULL;

void AllocGrid(int wx, int wy)
{
	g_log<<"allocating class arrays "<<wx<<","<<wy<<std::endl;
	g_log.flush();

	if( !(g_cdtype[CONDUIT_ROAD].cdtiles[0] = new CdTile [ (wx * wy) ]) ) OutOfMem(__FILE__, __LINE__);
	if( !(g_cdtype[CONDUIT_ROAD].cdtiles[1] = new CdTile [ (wx * wy) ]) ) OutOfMem(__FILE__, __LINE__);
	if( !(g_cdtype[CONDUIT_POWL].cdtiles[0] = new CdTile [ (wx * wy) ]) ) OutOfMem(__FILE__, __LINE__);
	if( !(g_cdtype[CONDUIT_POWL].cdtiles[1] = new CdTile [ (wx * wy) ]) ) OutOfMem(__FILE__, __LINE__);
	if( !(g_cdtype[CONDUIT_CRPIPE].cdtiles[0] = new CdTile [ (wx * wy) ]) ) OutOfMem(__FILE__, __LINE__);
	if( !(g_cdtype[CONDUIT_CRPIPE].cdtiles[1] = new CdTile [ (wx * wy) ]) ) OutOfMem(__FILE__, __LINE__);
}

/*
allocate room for a number of tiles, not height points.
*/
void Heightmap::allocate(int wx, int wy)
{
	destroy();

	// Vertices aren't shared between tiles or triangles.
	int numverts = wx * wy * 3 * 2;

	m_widthx = wx;
	m_widthy = wy;

	m_heightpoints = new float [ (wx+1) * (wy+1) ];
	m_drawverts = new Vec3f [ numverts ];
	m_collverts = new Vec3f [ numverts ];
	m_normals = new Vec3f [ numverts ];
	m_texcoords0 = new Vec2f [ numverts ];
	//m_countryowner = new int [ wx * wy ];
	m_triconfig = new bool [ wx * wy ];
	m_tridivider = new Plane3f [ wx * wy ];

	if(!m_heightpoints) OutOfMem(__FILE__, __LINE__);
	if(!m_drawverts) OutOfMem(__FILE__, __LINE__);
	if(!m_collverts) OutOfMem(__FILE__, __LINE__);
	if(!m_normals) OutOfMem(__FILE__, __LINE__);
	if(!m_texcoords0) OutOfMem(__FILE__, __LINE__);
	//if(!m_countryowner) OutOfMem(__FILE__, __LINE__);
	if(!m_triconfig) OutOfMem(__FILE__, __LINE__);
	if(!m_tridivider) OutOfMem(__FILE__, __LINE__);

	//g_log<<"setting heights to 0"<<std::endl;
	//g_log.flush();

	// Set to initial height.
	for(int x=0; x<=wx; x++)
		for(int z=0; z<=wy; z++)
			//m_heightpoints[ z*(wx+1) + x ] = rand()%1000;
			m_heightpoints[ z*(wx+1) + x ] = 0;

	remesh(1);
	//retexture();

	AllocWater(wx, wy);
}

void FreeGrid()
{
	for(unsigned char ctype=0; ctype<CONDUIT_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];
		//nice
		CdTile*& actual = ct->cdtiles[(int)false];
		CdTile*& planned = ct->cdtiles[(int)true];

		if(planned)
		{
			delete [] planned;
			planned = NULL;
		}

		if(actual)
		{
			delete [] actual;
			actual = NULL;
		}
	}
}
