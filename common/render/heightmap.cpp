
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
#include "../ai/collidertile.h"
#include "foliage.h"
#include "../save/savemap.h"
#include "../texture.h"
#include "compilebl.h"
#include "water.h"
#include "../sim/player.h"
#include "../debug.h"

unsigned int g_tiletexs[TILE_TYPES];
Vec2i g_mapview[2];
Heightmap g_hmap;
Heightmap g_hmap2;
Heightmap g_hmap4;
Heightmap g_hmap8;
unsigned int g_rimtexs[TEX_TYPES];

void AllocGrid(int wx, int wz)
{
	g_log<<"allocating class arrays "<<wx<<","<<wz<<endl;
	g_log.flush();

	g_road = new RoadTile [ (wx * wz) ];
	g_roadplan = new RoadTile [ (wx * wz) ];
	g_powl = new PowlTile [ (wx * wz) ];
	g_powlplan = new PowlTile [ (wx * wz) ];
	g_crpipe = new CrPipeTile[ (wx * wz) ];
	g_crpipeplan = new CrPipeTile[ (wx * wz) ];

	if(!g_road) OutOfMem(__FILE__, __LINE__);
	if(!g_roadplan) OutOfMem(__FILE__, __LINE__);
	if(!g_powl) OutOfMem(__FILE__, __LINE__);
	if(!g_powlplan) OutOfMem(__FILE__, __LINE__);
	if(!g_crpipe) OutOfMem(__FILE__, __LINE__);
	if(!g_crpipeplan) OutOfMem(__FILE__, __LINE__);
}

/*
allocate room for a number of tiles, not height points.
*/
void Heightmap::allocate(int wx, int wz)
{
	destroy();

	// Vertices aren't shared between tiles or triangles.
	int numverts = wx * wz * 3 * 2;

	m_widthx = wx;
	m_widthz = wz;

	m_heightpoints = new float [ (wx+1) * (wz+1) ];
	m_drawvertices = new Vec3f [ numverts ];
	m_collverts = new Vec3f [ numverts ];
	m_normals = new Vec3f [ numverts ];
	m_texcoords0 = new Vec2f [ numverts ];
	m_countryowner = new int [ wx * wz ];
	m_triconfig = new bool [ wx * wz ];
	m_tridivider = new Plane3f [ wx * wz ];

	if(!m_heightpoints) OutOfMem(__FILE__, __LINE__);
	if(!m_drawvertices) OutOfMem(__FILE__, __LINE__);
	if(!m_collverts) OutOfMem(__FILE__, __LINE__);
	if(!m_normals) OutOfMem(__FILE__, __LINE__);
	if(!m_texcoords0) OutOfMem(__FILE__, __LINE__);
	if(!m_countryowner) OutOfMem(__FILE__, __LINE__);
	if(!m_triconfig) OutOfMem(__FILE__, __LINE__);
	if(!m_tridivider) OutOfMem(__FILE__, __LINE__);

	//g_log<<"setting heights to 0"<<endl;
	//g_log.flush();

	// Set to initial height.
	for(int x=0; x<=wx; x++)
		for(int z=0; z<=wz; z++)
			//m_heightpoints[ z*(wx+1) + x ] = rand()%1000;
			m_heightpoints[ z*(wx+1) + x ] = 0;

	// Set to no country owner.
	for(int x=0; x<wx; x++)
		for(int z=0; z<wz; z++)
		{
			m_countryowner[ z*wx + x ] = -1;
		}

	remesh(1);
	//retexture();
	
	AllocWater(wx, wz);
}

void FreeGrid()
{
	if(g_road)
	{
		delete [] g_road;
		g_road = NULL;
	}

	if(g_roadplan)
	{
		delete [] g_roadplan;
		g_roadplan = NULL;
	}

	if(g_powl)
	{
		delete [] g_powl;
		g_powl = NULL;
	}

	if(g_crpipeplan)
	{
		delete [] g_crpipeplan;
		g_crpipeplan = NULL;
	}

	if(g_crpipe)
	{
		delete [] g_crpipe;
		g_crpipe = NULL;
	}
}

void Heightmap::destroy()
{
	if(m_widthx <= 0 || m_widthz <= 0)
		return;

	g_log<<"deleting [] g_open"<<endl;
	g_log.flush();
/*
	delete [] g_open;

	g_log<<"deleting [] g_road"<<endl;
	g_log.flush();
	*/

	delete [] m_heightpoints;
	delete [] m_drawvertices;
	delete [] m_collverts;
	delete [] m_normals;
	delete [] m_texcoords0;
	delete [] m_countryowner;

	if(m_fulltex.data)
		free(m_fulltex.data);

	m_widthx = 0;
	m_widthz = 0;

	FreeWater();
	
	FreePathGrid();
}

void Heightmap::changeheight(int x, int z, float change)
{
	m_heightpoints[ (z)*(m_widthx+1) + x ] += change;
}

void Heightmap::setheight(int x, int z, float height)
{
	m_heightpoints[ (z)*(m_widthx+1) + x ] = height;
}

inline float Heightmap::getheight(int tx, int tz)
{
	return m_heightpoints[ (tz)*(m_widthx+1) + tx ];
}

float Heightmap::accheight(int x, int z)
{
	int tx = x / TILE_SIZE;
	int tz = z / TILE_SIZE;

#if 0
	if(tx >= g_hmap.m_widthx)
		tx = g_hmap.m_widthx-1;

	if(tz >= g_hmap.m_widthz)
		tz = g_hmap.m_widthz-1;
#endif

	int tileindex = tz*m_widthx + tx;
	int tileindex6v = tileindex * 6;

	Vec3f point = Vec3f(x, 0, z);

	Vec3f tri[3];

	if(PointBehindPlane(point, m_tridivider[tileindex]))
	{
		tri[0] = m_collverts[ tileindex6v + 0 ];
		tri[1] = m_collverts[ tileindex6v + 1 ];
		tri[2] = m_collverts[ tileindex6v + 2 ];
	}
	else
	{
		tri[0] = m_collverts[ tileindex6v + 3 ];
		tri[1] = m_collverts[ tileindex6v + 4 ];
		tri[2] = m_collverts[ tileindex6v + 5 ];
	}

	Plane3f plane;

	Vec3f trinorm = Normal(tri);

#if 0
	g_log<<"trinorm = "<<trinorm.x<<","<<trinorm.y<<","<<trinorm.z<<endl;
#endif

	MakePlane(&plane.m_normal, &plane.m_d, tri[0], trinorm);

#if 0
	g_log<<"tri0 = "<<tri[0].x<<","<<tri[0].y<<","<<tri[0].z<<endl;
	g_log<<"tri1 = "<<tri[1].x<<","<<tri[1].y<<","<<tri[1].z<<endl;
	g_log<<"tri2 = "<<tri[2].x<<","<<tri[2].y<<","<<tri[2].z<<endl;
#endif

	float y = - ( x*plane.m_normal.x + z*plane.m_normal.z + plane.m_d ) / plane.m_normal.y;

#if 0
	g_log<<"- ( "<<x<<"*"<<plane.m_normal.x<<" + "<<z<<"*"<<plane.m_normal.z<<" + "<<plane.m_d<<" ) / "<<plane.m_normal.y<<" = "<<endl;

	g_log<<"y = "<<y<<endl;
#endif

	if(RoadAt(tx, tz)->on)
		y += TILE_SIZE/30;

	return y;
}

float Heightmap::accheight2(int x, int z)
{
	int tx = x / TILE_SIZE;
	int tz = z / TILE_SIZE;

	if(tx < 0)
		tx = 0;

	if(tz < 0)
		tz = 0;

	if(tx >= g_hmap.m_widthx)
		tx = g_hmap.m_widthx-1;

	if(tz >= g_hmap.m_widthz)
		tz = g_hmap.m_widthz-1;

	int tileindex = tz*m_widthx + tx;
	int tileindex6v = tileindex * 6;

	Vec3f point = Vec3f(x, 0, z);

	Vec3f tri[3];

	if(PointBehindPlane(point, m_tridivider[tileindex]))
	{
		tri[0] = m_collverts[ tileindex6v + 0 ];
		tri[1] = m_collverts[ tileindex6v + 1 ];
		tri[2] = m_collverts[ tileindex6v + 2 ];
	}
	else
	{
		tri[0] = m_collverts[ tileindex6v + 3 ];
		tri[1] = m_collverts[ tileindex6v + 4 ];
		tri[2] = m_collverts[ tileindex6v + 5 ];
	}

	Plane3f plane;

	Vec3f trinorm = Normal(tri);

	MakePlane(&plane.m_normal, &plane.m_d, tri[0], trinorm);

	float y = - ( x*plane.m_normal.x + z*plane.m_normal.z + plane.m_d ) / plane.m_normal.y;

	return y;
}


int &Heightmap::getcountry(int x, int z)
{
	return m_countryowner[ m_widthx*z + x ];
}

Vec3f *Heightmap::getdrawtileverts(int x, int z)
{
	return &m_drawvertices[ (z * m_widthx + x) * 6 ];
}

Vec3f *Heightmap::getcolltileverts(int x, int z)
{
	return &m_collverts[ (z * m_widthx + x) * 6 ];
}

Vec3f Heightmap::getnormal(int x, int z)
{
	return m_normals[ (z * m_widthx + x) * 6 ];
}

void Heightmap::hidetile(int x, int z)
{
	Vec3f* tileverts = getdrawtileverts(x, z);

	for(int i=0; i<6; i++)
		tileverts[i] = Vec3f(0,0,0);
}

void Heightmap::unhidetile(int x, int z)
{
	Vec3f* tileverts = getdrawtileverts(x, z);
	Vec3f* origtileverts = getcolltileverts(x, z);

	for(int i=0; i<6; i++)
		tileverts[i] = origtileverts[i];
}

/*
Regenerate the mesh vertices (m_drawvertices) and normals (m_normals) from the height points.
Texture coordinates (m_texcoords0) will also be generated.
*/
void Heightmap::remesh(float tilescale)
{
	m_tilescale = tilescale;

	/*
	These are the vertices of the tile.

	(0,0)      (1,0)
	c or 2     d or 3
	------------
	|          |
	|          |
	|          |
	|__________|
	b or 1     a or 0
	(0,1)      (1,1)
	*/

	Vec3f a, b, c, d;	// tile corner vertices
	Vec3f norm0, norm1;	// the two tile triangle normals
	Vec3f tri0[3];	//  first tile triangle
	Vec3f tri1[3];	// second tile triangle
	float heights[4];
	float aD, bD, cD, dD;

	// The numbers here indicate whether the vertex in
	// question is involved, counting from the top,left (0,0)
	// corner going clockwise.
	float diag1010;	// height difference along diagonal a-to-c
	float diag0101;	// height difference along diagonal b-to-d

	/*
	We will blend the normals of all the vertices with 4 neighbouring
	tiles, so that the terrain doesn't look blocky.

	Each tile has 6 vertices for two triangles.
	
	We need pointers for this because we construct the normals as we go along, 
	and only blend them in the end.
	*/
	vector<Vec3f**> *addedvertnormals = new vector<Vec3f**>[ m_widthx * m_widthz * 6 ];

	if(!addedvertnormals) OutOfMem(__FILE__, __LINE__);

	/*
	Because triangles will alternate, we need to keep an ordered list
	for the tile corner vertex normals for each tile, for 
	figuring out the corner vertex normals of neighbouring tiles.
	*/
	struct TileNormals
	{
		/*
		Depending on the triangle configuration, 
		there might be two normals at a corner,
		one for either triangle.
		*/
		Vec3f *normal_a_rightmost;
		Vec3f *normal_a_bottommost;
		Vec3f *normal_b_leftmost;
		Vec3f *normal_b_bottommost;
		Vec3f *normal_c_leftmost;
		Vec3f *normal_c_topmost;
		Vec3f *normal_d_topmost;
		Vec3f *normal_d_rightmost;
	};

	
	/*
	(0,0)      (1,0)
	c or 2     d or 3
	------------
	|          |
	|          |
	|          |
	|__________|
	b or 1     a or 0
	(0,1)      (1,1)
	*/

	TileNormals *tilenormals = new TileNormals[ m_widthx * m_widthz ];

	if(!tilenormals) OutOfMem(__FILE__, __LINE__);

	for(int x=0; x<m_widthx; x++)
		for(int z=0; z<m_widthz; z++)
		{
			heights[0] = getheight(x+1, z+1);
			heights[1] = getheight(x, z+1);
			heights[2] = getheight(x, z);
			heights[3] = getheight(x+1, z);

			/*
			These are the vertices of the tile.

			(0,0)      (1,0)
			c or 2     d or 3
			------------
			|          |
			|          |
			|          |
			|__________|
			b or 1     a or 0
			(0,1)      (1,1)

			We need to decide which way the diagonal (shared edge of the
			triangles) will be, because it doesn't look nice where the 
			land meets the water if we don't alternate them according
			to which diagonal has the greater height difference. And also,
			it doesn't look good in general anywhere where there's
			mountains or ridges.

			What we're going to do here is give smoothness to changing
			slopes, instead of hard corners. 

			Imagine if there's a really strong difference in height 
			along the diagonal c-to-a. 

			If we break the tile into triangles like this, 

			(0,0)      (1,0)
			c or 2     d or 3
			------------
			| \        |
			|    \     |
			|      \   |
			|________\_|
			b or 1     a or 0
			(0,1)      (1,1)

			Then there will be a sharp corner at vertex a.

			However, if we cut the tile into triangles like this,

			
			(0,0)      (1,0)
			c or 2     d or 3
			------------
			|        / |
			|      /   |
			|    /     |
			|_/________|
			b or 1     a or 0
			(0,1)      (1,1)

			then this will be a bevelled corner. 

			Combined with other tiles this will look like:

			          |
			          |
			          |
			         /
			       /
			_____/

			Otherwise, if we didn't check which way
			to cut the tile into triangles, it would look
			like a sharp corner:

			          |
			          |
			          |
			__________|

			*/

			a = Vec3f( (x+1)*TILE_SIZE*tilescale, heights[0], (z+1)*TILE_SIZE*tilescale );
			b = Vec3f( (x)*TILE_SIZE*tilescale, heights[1], (z+1)*TILE_SIZE*tilescale );
			c = Vec3f( (x)*TILE_SIZE*tilescale, heights[2], (z)*TILE_SIZE*tilescale );
			d = Vec3f( (x+1)*TILE_SIZE*tilescale, heights[3], (z)*TILE_SIZE*tilescale );

			/*
			Get the difference between each tile vertex
			and the average of the two neighbouring 
			(diagonal) vertices.
			*/

			aD = fabsf( heights[0] - (heights[1]+heights[3])/2.0f );
			bD = fabsf( heights[1] - (heights[2]+heights[0])/2.0f );
			cD = fabsf( heights[2] - (heights[3]+heights[1])/2.0f );
			dD = fabsf( heights[3] - (heights[0]+heights[2])/2.0f );

			/*
			For either of the two possible diagonals,
			get the maximum difference, to see
			which diagonal is steeper.
			*/

			diag1010 = max( aD, cD );
			diag0101 = max( bD, dD );

			/*
			If diagonal a-to-c has a greater
			height difference, 

			(0,0)      (1,0)
			c or 2     d or 3
			------------
			|          |
			|          |
			|          |
			|__________|
			b or 1     a or 0
			(0,1)      (1,1)

			triangle 1 will be made of vertices
			a, b, and d, and triangle 2 will be
			made of vertices b, c, d.
			*/

			int tileindex6v = (z * m_widthx + x) * 3 * 2;
			int tileindex = (z * m_widthx + x);

			if(diag1010 > diag0101)
			{
				m_triconfig[ tileindex ] = 0;

				Vec3f divnorm = Cross(Normalize(b-d), Vec3f(0,1,0));
				MakePlane(&m_tridivider[ tileindex ].m_normal, &m_tridivider[ tileindex ].m_d, Vec3f(((float)x+0.5f)*TILE_SIZE, 0, ((float)z+0.5f)*TILE_SIZE), divnorm);

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|        / |
				|      /   |
				|    /     |
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)
				*/

				m_drawvertices[ tileindex6v + 0 ] = a;
				m_drawvertices[ tileindex6v + 1 ] = b;
				m_drawvertices[ tileindex6v + 2 ] = d;
				m_drawvertices[ tileindex6v + 3 ] = b;
				m_drawvertices[ tileindex6v + 4 ] = c;
				m_drawvertices[ tileindex6v + 5 ] = d;


				// Need triangles to figure out 
				// the tile normals.
				tri0[0] = a;
				tri0[1] = b;
				tri0[2] = d;
				tri1[0] = b;
				tri1[1] = c;
				tri1[2] = d;

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d

				Now we have to remember that the triangle indices don't correspond to the vertex numbers;
				there's 6 triangle vertices (2 repeated) and only 4 unique tile corner vertices.
				*/
				
				tilenormals[ tileindex ].normal_a_rightmost = &m_normals[ tileindex6v + 0 ];
				tilenormals[ tileindex ].normal_a_bottommost = &m_normals[ tileindex6v + 0 ];
				tilenormals[ tileindex ].normal_b_leftmost = &m_normals[ tileindex6v + 3 ];
				tilenormals[ tileindex ].normal_b_bottommost = &m_normals[ tileindex6v + 1 ];
				tilenormals[ tileindex ].normal_c_leftmost = &m_normals[ tileindex6v + 4 ];
				tilenormals[ tileindex ].normal_c_topmost = &m_normals[ tileindex6v + 4 ];
				tilenormals[ tileindex ].normal_d_topmost = &m_normals[ tileindex6v + 5 ];
				tilenormals[ tileindex ].normal_d_rightmost = &m_normals[ tileindex6v + 2 ];

				// Add the normals for this tile itself
				addedvertnormals[ tileindex6v + 0 ].push_back( &tilenormals[ tileindex ].normal_a_rightmost );
				addedvertnormals[ tileindex6v + 1 ].push_back( &tilenormals[ tileindex ].normal_b_bottommost );
				addedvertnormals[ tileindex6v + 1 ].push_back( &tilenormals[ tileindex ].normal_b_leftmost );
				addedvertnormals[ tileindex6v + 2 ].push_back( &tilenormals[ tileindex ].normal_d_rightmost );
				addedvertnormals[ tileindex6v + 2 ].push_back( &tilenormals[ tileindex ].normal_d_topmost );
				addedvertnormals[ tileindex6v + 3 ].push_back( &tilenormals[ tileindex ].normal_b_leftmost );
				addedvertnormals[ tileindex6v + 3 ].push_back( &tilenormals[ tileindex ].normal_b_bottommost );
				addedvertnormals[ tileindex6v + 4 ].push_back( &tilenormals[ tileindex ].normal_c_topmost );
				addedvertnormals[ tileindex6v + 5 ].push_back( &tilenormals[ tileindex ].normal_d_topmost );
				addedvertnormals[ tileindex6v + 5 ].push_back( &tilenormals[ tileindex ].normal_d_rightmost );

				//If there's a tile in the x-1 direction, add its normal to corners c(2) and b(1).
				// c(2) is the vertex index 4 of the two triangle vertices.
				// b(1) is the vertex index 1 and 3 of the two triangle vertices.
				if(x > 0)
				{
					int nearbytileindex = (z * m_widthx + (x-1));
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 4 of the triangles (corner c(2)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_d_rightmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_d_topmost );

					// vertex 1 of the triangles (corner b(1)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_a_bottommost );
					
					// vertex 3 of the triangles (corner b(1)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_a_bottommost );
				}

				
				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d
				*/

				//If there's a tile in the x+1 direction, add its normal to corners d(3) and a(0).
				// d(3) is the vertex index 2 and 5 of the two triangles vertices.
				// a(0) is the vertex index 0 of the two triangles.
				if(x < m_widthx-1)
				{
					int nearbytileindex = (z * m_widthx + (x+1));
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 2 of the triangles (corner d(3)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_c_topmost );

					// vertex 5 of the triangles (corner d(3)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_c_topmost );
					
					// vertex 0 of the triangles (corner a(0)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_b_bottommost );
				}

				//If there's a tile in the z-1 direction, add its normal to corners c(2) and d(3). 
				// c(2) is the vertex index 4 of the two triangle vertices.
				// d(3) is the vertex index 2 and 5 of the two triangle vertices.
				if(z > 0)
				{
					int nearbytileindex = ((z-1) * m_widthx + x);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 4 of the triangles (corner c(2)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_b_bottommost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_b_leftmost );

					// vertex 2 of the triangles (corner d(3)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_a_bottommost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_a_rightmost );
					
					// vertex 5 of the triangles (corner d(3)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_a_bottommost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_a_rightmost );
				}
				
				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d
				*/

				//If there's a tile in the z+1 direction, add its normal to corners b(1) and a(0).
				// b(1) is the vertex index 1 and 3 of the two triangles vertices.
				// a(0) is the vertex index 0 of the two triangles.
				if(z < m_widthz-1)
				{
					int nearbytileindex = ((z+1) * m_widthx + x);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 1 of the triangles (corner b(1)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_c_topmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_c_leftmost );

					// vertex 3 of the triangles (corner b(1)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_c_topmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_c_leftmost );
					
					// vertex 0 of the triangles (corner a(0)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_d_rightmost );
				}

				// We've added the normals of the horizontally and vertically neighbouring tiles.
				// Now we'll add the normals of diagonal tiles.

				//If there's a tile in the x-1,z-1 direction, add its normal to corner c(2).
				// c(2) is the vertex index 4 of the two triangles.
				if(x-1 >= 0 && z-1 >= 0)
				{
					int nearbytileindex = ((z-1) * m_widthx + x-1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 4 of the triangles (corner c(2)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_a_bottommost );
				}
				
				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d
				*/
				
				//If there's a tile in the x+1,z-1 direction, add its normal to corner d(3).
				// d(3) is the vertex index 2 and 5 of the two triangles.
				if(x+1 < m_widthx && z-1 >= 0)
				{
					int nearbytileindex = ((z-1) * m_widthx + x+1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 2,5 of the triangles (corner d(3)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_b_bottommost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_b_bottommost );
				}

				//If there's a tile in the x+1,z+1 direction, add its normal to corner a(0).
				// a(0) is the vertex index 0 of the two triangles.
				if(x+1 < m_widthx && z+1 < m_widthz)
				{
					int nearbytileindex = ((z+1) * m_widthx + x+1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 0 of the triangles (corner a(0)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_c_topmost );
				}
				
				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				| tri1   / |
				|      /   |
				|    / tri0|
				|_/________|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,d
				tri 1 = b,c,d
				*/
				
				//If there's a tile in the x-1,z+1 direction, add its normal to corner b(1).
				// b(1) is the vertex index 1,3 of the two triangles.
				if(x-1 >= 0 && z+1 < m_widthz)
				{
					int nearbytileindex = ((z+1) * m_widthx + x-1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 1 of the triangles (corner b(1)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_d_rightmost );
					
					// vertex 3 of the triangles (corner b(1)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_d_rightmost );
				}
			}
			
			/*
			Otherwise, if diagonal d-to-b has a 
			greater height difference, 

			(0,0)      (1,0)
			c or 2     d or 3
			------------
			|  \       |
			|    \     |
			|      \   |
			|________\_|
			b or 1     a or 0
			(0,1)      (1,1)

			triangle 1 will be made of vertices
			a, b, and c, and triangle 2 will be
			made of vertices d, a, c.
			*/

			else
			{
				m_triconfig[ tileindex ] = 1;
				
				Vec3f divnorm = Cross(Normalize(c-a), Vec3f(0,1,0));
				MakePlane(&m_tridivider[ tileindex ].m_normal, &m_tridivider[ tileindex ].m_d, Vec3f(((float)x+0.5f)*TILE_SIZE, 0, ((float)z+0.5f)*TILE_SIZE), divnorm);

				m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 0 ] = a;
				m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 1 ] = b;
				m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 2 ] = c;
				m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 3 ] = d;
				m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 4 ] = a;
				m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 5 ] = c;

				tri0[0] = a;
				tri0[1] = b;
				tri0[2] = c;
				tri1[0] = d;
				tri1[1] = a;
				tri1[2] = c;

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/

				tilenormals[ tileindex ].normal_a_rightmost = &m_normals[ tileindex6v + 4 ];
				tilenormals[ tileindex ].normal_a_bottommost = &m_normals[ tileindex6v + 0 ];
				tilenormals[ tileindex ].normal_b_leftmost = &m_normals[ tileindex6v + 1 ];
				tilenormals[ tileindex ].normal_b_bottommost = &m_normals[ tileindex6v + 1 ];
				tilenormals[ tileindex ].normal_c_leftmost = &m_normals[ tileindex6v + 2 ];
				tilenormals[ tileindex ].normal_c_topmost = &m_normals[ tileindex6v + 5 ];
				tilenormals[ tileindex ].normal_d_topmost = &m_normals[ tileindex6v + 3 ];
				tilenormals[ tileindex ].normal_d_rightmost = &m_normals[ tileindex6v + 3 ];

				// Add the normals for this tile itself
				addedvertnormals[ tileindex6v + 0 ].push_back( &tilenormals[ tileindex ].normal_a_bottommost );
				addedvertnormals[ tileindex6v + 0 ].push_back( &tilenormals[ tileindex ].normal_a_rightmost );
				addedvertnormals[ tileindex6v + 1 ].push_back( &tilenormals[ tileindex ].normal_b_leftmost );
				addedvertnormals[ tileindex6v + 2 ].push_back( &tilenormals[ tileindex ].normal_c_leftmost );
				addedvertnormals[ tileindex6v + 2 ].push_back( &tilenormals[ tileindex ].normal_c_topmost );
				addedvertnormals[ tileindex6v + 3 ].push_back( &tilenormals[ tileindex ].normal_d_rightmost );
				addedvertnormals[ tileindex6v + 4 ].push_back( &tilenormals[ tileindex ].normal_a_rightmost );
				addedvertnormals[ tileindex6v + 4 ].push_back( &tilenormals[ tileindex ].normal_a_bottommost );
				addedvertnormals[ tileindex6v + 5 ].push_back( &tilenormals[ tileindex ].normal_c_topmost );
				addedvertnormals[ tileindex6v + 5 ].push_back( &tilenormals[ tileindex ].normal_c_leftmost );

				//If there's a tile in the x-1 direction, add its normal to corners c(2) and b(1).
				// c(2) is the vertex index 2 and 5 of the two triangle vertices.
				// b(1) is the vertex index 1 of the two triangle vertices.
				if(x > 0)
				{
					int nearbytileindex = (z * m_widthx + (x-1));
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 2 of the triangles (corner c(2)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_d_rightmost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_d_topmost );

					// vertex 5 of the triangles (corner c(2)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_d_rightmost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_d_topmost );
					
					// vertex 1 of the triangles (corner b(1)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_a_bottommost );
				}
				
				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/

				//If there's a tile in the x+1 direction, add its normal to corners d(3) and a(0).
				// d(3) is the vertex index 3 of the two triangles vertices.
				// a(0) is the vertex index 0 and 4 of the two triangles.
				if(x < m_widthx-1)
				{
					int nearbytileindex = (z * m_widthx + (x+1));
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 3 of the triangles (corner d(3)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_c_topmost );

					// vertex 0 of the triangles (corner a(0)) is corner b(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_b_bottommost );
					
					// vertex 4 of the triangles (corner a(0)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_b_bottommost );
				}

				//If there's a tile in the z-1 direction, add its normal to corners c(2) and d(3). 
				// c(2) is the vertex index 2 and 5 of the two triangle vertices.
				// d(3) is the vertex index 3 of the two triangle vertices.
				if(z > 0)
				{
					int nearbytileindex = ((z-1) * m_widthx + x);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 2 of the triangles (corner c(2)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_b_bottommost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_b_leftmost );

					// vertex 5 of the triangles (corner c(2)) is corner b(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_b_bottommost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_b_leftmost );
					
					// vertex 3 of the triangles (corner d(3)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_a_bottommost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_a_rightmost );
				}
				
				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/
				
				//If there's a tile in the z+1 direction, add its normal to corners b(1) and a(0).
				// b(1) is the vertex index 1 of the two triangles vertices.
				// a(0) is the vertex index 0 and 4 of the two triangles.
				if(z < m_widthz-1)
				{
					int nearbytileindex = ((z+1) * m_widthx + x);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];

					// vertex 1 of the triangles (corner b(1)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_c_topmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_c_leftmost );

					// vertex 0 of the triangles (corner a(1)) is corner d(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_d_rightmost );
					
					// vertex 4 of the triangles (corner a(0)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_d_rightmost );
				}

				// We've added the normals of the horizontally and vertically neighbouring tiles.
				// Now we'll add the normals of diagonal tiles.
				
				//If there's a tile in the x-1,z-1 direction, add its normal to corner c(2).
				// c(2) is the vertex index 2,5 of the two triangles.
				if(x-1 >= 0 && z-1 >= 0)
				{
					int nearbytileindex = ((z-1) * m_widthx + x-1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 2 of the triangles (corner c(2)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 2 ].push_back( &nearbytilenormals->normal_a_bottommost );

					// vertex 5 of the triangles (corner c(2)) is corner a(0) of the neighbouring tile
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_a_rightmost );
					addedvertnormals[ tileindex6v + 5 ].push_back( &nearbytilenormals->normal_a_bottommost );
				}
				
				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/

				//If there's a tile in the x+1,z-1 direction, add its normal to corner d(3).
				// d(3) is the vertex index 3 of the two triangles.
				if(x+1 < m_widthx && z-1 >= 0)
				{
					int nearbytileindex = ((z-1) * m_widthx + x+1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 3 of the triangles (corner d(3)) is corner b(1) of the neighbouring tile
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_b_leftmost );
					addedvertnormals[ tileindex6v + 3 ].push_back( &nearbytilenormals->normal_b_bottommost );
				}

				//If there's a tile in the x+1,z+1 direction, add its normal to corner a(0).
				// a(0) is the vertex index 0,4 of the two triangles.
				if(x+1 < m_widthx && z+1 < m_widthz)
				{
					int nearbytileindex = ((z+1) * m_widthx + x+1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 0,4 of the triangles (corner a(0)) is corner c(2) of the neighbouring tile
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 0 ].push_back( &nearbytilenormals->normal_c_topmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_c_leftmost );
					addedvertnormals[ tileindex6v + 4 ].push_back( &nearbytilenormals->normal_c_topmost );
				}

				/*
				(0,0)      (1,0)
				c or 2     d or 3
				------------
				|  \ tri 1 |
				|    \     |
				|tri 0 \   |
				|________\_|
				b or 1     a or 0
				(0,1)      (1,1)

				tri 0 = a,b,c
				tri 1 = d,a,c
				*/
				
				//If there's a tile in the x-1,z+1 direction, add its normal to corner b(1).
				// b(1) is the vertex index 1 of the two triangles.
				if(x-1 >= 0 && z+1 < m_widthz)
				{
					int nearbytileindex = ((z+1) * m_widthx + x-1);
					TileNormals* nearbytilenormals = &tilenormals[ nearbytileindex ];
					
					// vertex 1 of the triangles (corner b(1)) is corner d(3) of the neighbouring tile
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_d_topmost );
					addedvertnormals[ tileindex6v + 1 ].push_back( &nearbytilenormals->normal_d_rightmost );
				}
			}

			// Generate the texture coordinates based on world position of vertices.
			for(int i=0; i<6; i++)
			{
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + i ].x = m_drawvertices[ (z * m_widthx + x) * 3 * 2 + i ].x / TILE_SIZE;
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + i ].y = m_drawvertices[ (z * m_widthx + x) * 3 * 2 + i ].z / TILE_SIZE;
			}

			// Triangle normals.
			norm0 = Normal(tri0);
			norm1 = Normal(tri1);

			// These are temporary normals before we blend them between tiles.
			m_normals[ (z * m_widthx + x) * 3 * 2 + 0 ] = norm0;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 1 ] = norm0;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 2 ] = norm0;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 3 ] = norm1;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 4 ] = norm1;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 5 ] = norm1;
		}

	Vec3f* tempnormals = new Vec3f[ m_widthx * m_widthz * 3 * 2 ];

	if(!tempnormals) OutOfMem(__FILE__, __LINE__);

	// Average the added up normals and store them in tempnormals.
	for(int x=0; x<m_widthx; x++)
		for(int z=0; z<m_widthz; z++)
			for(int trivert = 0; trivert < 6; trivert++)
			{
				int tileindex6v = (z * m_widthx + x) * 3 * 2 + trivert;
				int tileindex = (z * m_widthx + x);
				vector<Vec3f**> vertexnormals = addedvertnormals[ tileindex6v ];

				// Before we add up the added normals, we make sure we don't repeat
				// any normals, since for diagonals we add the two normals for 
				// the two possible triangles. Now that we have the triangle
				// configuration, we know which corners have one triangle and
				// which have two.
				// Remove repeating added normals.
#if 1
				for(auto normiter = vertexnormals.begin(); normiter != vertexnormals.end(); normiter++)
				{
					auto normiter2 = normiter+1;
					while(normiter2 != vertexnormals.end())
					{
						// If both pointers are pointing to the same normal vector, remove the second copy.
						if(**normiter2 == **normiter)
						{
							normiter2 = vertexnormals.erase( normiter2 );
							continue;
						}

						normiter2++;
					}
				}
#endif

				Vec3f finalnormal(0,0,0);

				// Average all the added normals for each tile vertex

				for(int i=0; i<vertexnormals.size(); i++)
					finalnormal = finalnormal + **(vertexnormals[i]);

				if(vertexnormals.size() <= 0)
					continue;

				tempnormals[ tileindex6v ] = Normalize( finalnormal / (float)vertexnormals.size() );
			}
		
	// Transfer the normals from tempnormals to m_normals.
	for(int x=0; x<m_widthx; x++)
		for(int z=0; z<m_widthz; z++)
			for(int trivert = 0; trivert < 6; trivert ++)
			{
				int tileindex6v = (z * m_widthx + x) * 3 * 2 + trivert;
				m_normals[ tileindex6v ] = tempnormals[ tileindex6v ];
				m_collverts[ tileindex6v ] = m_drawvertices[ tileindex6v ];
			}

	delete [] tempnormals;
	delete [] addedvertnormals;
	delete [] tilenormals;

	// Now to remesh the rim (underground)
	m_rimva.alloc( 3 * 2 * (m_widthx + m_widthz) * 2 + 6 );
	float lowy = ConvertHeight(0);

	// North side
	for(int x=0; x<m_widthx; x++)
	{
		int index = 3 * 2 * x + 0;

		m_rimva.vertices[index + 0] = Vec3f((x+1)*TILE_SIZE, getheight(x+1, 0), 0);
		m_rimva.vertices[index + 1] = Vec3f((x+0)*TILE_SIZE, getheight(x+0, 0), 0);
		m_rimva.vertices[index + 2] = Vec3f((x+0)*TILE_SIZE, lowy, 0);
		
		m_rimva.vertices[index + 3] = Vec3f((x+1)*TILE_SIZE, getheight(x+1, 0), 0);
		m_rimva.vertices[index + 4] = Vec3f((x+0)*TILE_SIZE, lowy, 0);
		m_rimva.vertices[index + 5] = Vec3f((x+1)*TILE_SIZE, lowy, 0);
		
		m_rimva.vertices[index + 0].y = max(m_rimva.vertices[index + 0].y, WATER_LEVEL);
		m_rimva.vertices[index + 1].y = max(m_rimva.vertices[index + 1].y, WATER_LEVEL);
		m_rimva.vertices[index + 3].y = max(m_rimva.vertices[index + 3].y, WATER_LEVEL);

		for(int i=0; i<6; i++)
		{
			m_rimva.normals[index + i] = Vec3f(0,0,-1);
			m_rimva.texcoords[index + i] = Vec2f(-m_rimva.vertices[index + i].x/TILE_SIZE/10, m_rimva.vertices[index + i].y/TILE_SIZE/10);
		}
	}
	
	// South side
	for(int x=0; x<m_widthx; x++)
	{
		int index = 3 * 2 * x + 3*2*m_widthx + 0;

		m_rimva.vertices[index + 0] = Vec3f((x+0)*TILE_SIZE, getheight(x+0, m_widthz), (m_widthz)*TILE_SIZE);
		m_rimva.vertices[index + 1] = Vec3f((x+1)*TILE_SIZE, getheight(x+1, m_widthz), (m_widthz)*TILE_SIZE);
		m_rimva.vertices[index + 2] = Vec3f((x+1)*TILE_SIZE, lowy, (m_widthz)*TILE_SIZE);
		
		m_rimva.vertices[index + 3] = Vec3f((x+0)*TILE_SIZE, getheight(x+0, m_widthz), (m_widthz)*TILE_SIZE);
		m_rimva.vertices[index + 4] = Vec3f((x+1)*TILE_SIZE, lowy, (m_widthz)*TILE_SIZE);
		m_rimva.vertices[index + 5] = Vec3f((x+0)*TILE_SIZE, lowy, (m_widthz)*TILE_SIZE);
		
		m_rimva.vertices[index + 0].y = max(m_rimva.vertices[index + 0].y, WATER_LEVEL);
		m_rimva.vertices[index + 1].y = max(m_rimva.vertices[index + 1].y, WATER_LEVEL);
		m_rimva.vertices[index + 3].y = max(m_rimva.vertices[index + 3].y, WATER_LEVEL);

		for(int i=0; i<6; i++)
		{
			m_rimva.normals[index + i] = Vec3f(0,0,1);
			m_rimva.texcoords[index + i] = Vec2f(m_rimva.vertices[index + i].x/TILE_SIZE/10, m_rimva.vertices[index + i].y/TILE_SIZE/10);
		}
	}
	
	// West side
	for(int z=0; z<m_widthz; z++)
	{
		int index = 3 * 2 * z + 3*2*m_widthx*2 + 0;

		m_rimva.vertices[index + 0] = Vec3f(0, getheight(0, z+0), (z+0)*TILE_SIZE);
		m_rimva.vertices[index + 1] = Vec3f(0, getheight(0, z+1), (z+1)*TILE_SIZE);
		m_rimva.vertices[index + 2] = Vec3f(0, lowy, (z+1)*TILE_SIZE);
		
		m_rimva.vertices[index + 3] = Vec3f(0, getheight(0, z+0), (z+0)*TILE_SIZE);
		m_rimva.vertices[index + 4] = Vec3f(0, lowy, (z+1)*TILE_SIZE);
		m_rimva.vertices[index + 5] = Vec3f(0, lowy, (z+0)*TILE_SIZE);
		
		m_rimva.vertices[index + 0].y = max(m_rimva.vertices[index + 0].y, WATER_LEVEL);
		m_rimva.vertices[index + 1].y = max(m_rimva.vertices[index + 1].y, WATER_LEVEL);
		m_rimva.vertices[index + 3].y = max(m_rimva.vertices[index + 3].y, WATER_LEVEL);

		for(int i=0; i<6; i++)
		{
			m_rimva.normals[index + i] = Vec3f(-1,0,0);
			m_rimva.texcoords[index + i] = Vec2f(m_rimva.vertices[index + i].z/TILE_SIZE/10, m_rimva.vertices[index + i].y/TILE_SIZE/10);
		}
	}
	
	// East side
	for(int z=0; z<m_widthz; z++)
	{
		int index = 3 * 2 * z + 3*2*m_widthx*2 + 3*2*m_widthz;

		m_rimva.vertices[index + 0] = Vec3f((m_widthx)*TILE_SIZE, getheight(m_widthx, z+1), (z+1)*TILE_SIZE);
		m_rimva.vertices[index + 1] = Vec3f((m_widthx)*TILE_SIZE, getheight(m_widthx, z+0), (z+0)*TILE_SIZE);
		m_rimva.vertices[index + 2] = Vec3f((m_widthx)*TILE_SIZE, lowy, (z+0)*TILE_SIZE);
		
		m_rimva.vertices[index + 3] = Vec3f((m_widthx)*TILE_SIZE, getheight(m_widthx, z+1), (z+1)*TILE_SIZE);
		m_rimva.vertices[index + 4] = Vec3f((m_widthx)*TILE_SIZE, lowy, (z+0)*TILE_SIZE);
		m_rimva.vertices[index + 5] = Vec3f((m_widthx)*TILE_SIZE, lowy, (z+1)*TILE_SIZE);
		
		m_rimva.vertices[index + 0].y = max(m_rimva.vertices[index + 0].y, WATER_LEVEL);
		m_rimva.vertices[index + 1].y = max(m_rimva.vertices[index + 1].y, WATER_LEVEL);
		m_rimva.vertices[index + 3].y = max(m_rimva.vertices[index + 3].y, WATER_LEVEL);

		for(int i=0; i<6; i++)
		{
			m_rimva.normals[index + i] = Vec3f(1,0,0);
			m_rimva.texcoords[index + i] = Vec2f(-m_rimva.vertices[index + i].z/TILE_SIZE/10, m_rimva.vertices[index + i].y/TILE_SIZE/10);
		}
	}

	// Bottom side
	{
		int index = 3*2*m_widthx*2 + 3*2*m_widthz*2;

		m_rimva.vertices[index + 0] = Vec3f((m_widthx)*TILE_SIZE, lowy, (m_widthz)*TILE_SIZE);
		m_rimva.vertices[index + 1] = Vec3f((m_widthx)*TILE_SIZE, lowy, (0)*TILE_SIZE);
		m_rimva.vertices[index + 2] = Vec3f((0)*TILE_SIZE, lowy, (0)*TILE_SIZE);
		
		m_rimva.vertices[index + 3] = Vec3f((m_widthx)*TILE_SIZE, lowy, (m_widthz)*TILE_SIZE);
		m_rimva.vertices[index + 4] = Vec3f((0)*TILE_SIZE, lowy, (0)*TILE_SIZE);
		m_rimva.vertices[index + 5] = Vec3f((0)*TILE_SIZE, lowy, (m_widthz)*TILE_SIZE);

		for(int i=0; i<6; i++)
		{
			m_rimva.normals[index + i] = Vec3f(0,-1,0);
			m_rimva.texcoords[index + i] = Vec2f(-m_rimva.vertices[index + i].x/TILE_SIZE/10, m_rimva.vertices[index + i].z/TILE_SIZE/10);
		}
	}
}

void Heightmap::draw()
{
	if(m_widthx <= 0 || m_widthz <= 0)
		return;
	//return;
	Shader* s = &g_shader[g_curS];
	
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_SAND] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_SANDTEX], 0);
	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_GRASS] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_GRASSTEX], 1);
	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_SNOW] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_SNOWTEX], 2);
	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_ROCK] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_ROCKTEX], 3);
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_CRACKEDROCK] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_CRACKEDROCKTEX], 4);
	glActiveTextureARB(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_CRACKEDROCK_NORM] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_CRACKEDROCKNORMTEX], 5);
	glActiveTextureARB(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_ROCK_NORM] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_ROCKNORMTEX], 6);
	
	//float yscale = TILE_Y_SCALE / 2000.0f;
	glUniform1f(s->m_slot[SSLOT_SANDONLYMAXY], ELEV_SANDONLYMAXY);
	glUniform1f(s->m_slot[SSLOT_SANDGRASSMAXY], ELEV_SANDGRASSMAXY);
	glUniform1f(s->m_slot[SSLOT_GRASSONLYMAXY], ELEV_GRASSONLYMAXY);
	glUniform1f(s->m_slot[SSLOT_GRASSROCKMAXY], ELEV_GRASSROCKMAXY);
	glUniform1f(s->m_slot[SSLOT_MAPMINZ], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXZ], m_widthz*TILE_SIZE*m_tilescale);
	glUniform1f(s->m_slot[SSLOT_MAPMINX], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXX], m_widthx*TILE_SIZE*m_tilescale);
	glUniform1f(s->m_slot[SSLOT_MAPMINY], ConvertHeight(0));
	glUniform1f(s->m_slot[SSLOT_MAPMAXY], ConvertHeight(255));

	/*
	for(int x=0; x<m_widthx; x++)
		for(int z=0; z<m_widthz; z++)
		{
			glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthx + x) * 3 * 2 + 0 ]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		*/
#if 1
	// Draw all tiles
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, m_drawvertices);
	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, m_texcoords0);
	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, m_normals);
	glDrawArrays(GL_TRIANGLES, 0, (m_widthx) * (m_widthz) * 3 * 2);

#else
	int tilescale = m_tilescale;

	// Draw only visible tiles in strips
	for(int z=g_mapview[0].y/tilescale; z<=g_mapview[1].y/tilescale; z++)
	{
		int starti = m_widthx*z + g_mapview[0].x/tilescale;
		int spanx = g_mapview[1].x/tilescale - g_mapview[0].x/tilescale;
		int spanz = g_mapview[1].y/tilescale - g_mapview[0].y/tilescale;
		//int stridei = m_widthx - spanx;
		int stridei = 0;
	
		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, m_drawvertices);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, m_texcoords0);
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, m_normals);
		glDrawArrays(GL_TRIANGLES, starti * 2 * 3, spanx * 3 * 2);
	}
#endif
}

void Heightmap::draw2()
{
	if(m_widthx <= 0 || m_widthz <= 0)
		return;

	Shader* s = &g_shader[g_curS];
	
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_SAND] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_SANDTEX], 0);

	CheckGLError(__FILE__, __LINE__);

	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_GRASS] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_GRASSTEX], 1);

	CheckGLError(__FILE__, __LINE__);

	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_SNOW] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_SNOWTEX], 2);

	CheckGLError(__FILE__, __LINE__);

	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_ROCK] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_ROCKTEX], 3);

	CheckGLError(__FILE__, __LINE__);

	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_CRACKEDROCK] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_CRACKEDROCKTEX], 4);

	CheckGLError(__FILE__, __LINE__);

	glActiveTextureARB(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_CRACKEDROCK_NORM] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_CRACKEDROCKNORMTEX], 5);

	CheckGLError(__FILE__, __LINE__);

	glActiveTextureARB(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[TILE_ROCK_NORM] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_ROCKNORMTEX], 6);

	CheckGLError(__FILE__, __LINE__);

	
	//float yscale = TILE_Y_SCALE / 2000.0f;
	glUniform1f(s->m_slot[SSLOT_SANDONLYMAXY], ELEV_SANDONLYMAXY);
	glUniform1f(s->m_slot[SSLOT_SANDGRASSMAXY], ELEV_SANDGRASSMAXY);
	glUniform1f(s->m_slot[SSLOT_GRASSONLYMAXY], ELEV_GRASSONLYMAXY);
	glUniform1f(s->m_slot[SSLOT_GRASSROCKMAXY], ELEV_GRASSROCKMAXY);
	glUniform1f(s->m_slot[SSLOT_MAPMINZ], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXZ], m_widthz*TILE_SIZE*m_tilescale);
	glUniform1f(s->m_slot[SSLOT_MAPMINX], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXX], m_widthx*TILE_SIZE*m_tilescale);
	glUniform1f(s->m_slot[SSLOT_MAPMINY], ConvertHeight(0));
	glUniform1f(s->m_slot[SSLOT_MAPMAXY], ConvertHeight(255));

	CheckGLError(__FILE__, __LINE__);


	/*
	for(int x=0; x<m_widthx; x++)
		for(int z=0; z<m_widthz; z++)
		{
			glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthx + x) * 3 * 2 + 0 ]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		*/

#if 1
	// Draw all tiles
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, m_collverts);

	CheckGLError(__FILE__, __LINE__);

	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, m_texcoords0);

	CheckGLError(__FILE__, __LINE__);

	if(s->m_slot[SSLOT_NORMAL] != -1)
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, m_normals);

	CheckGLError(__FILE__, __LINE__);

	glDrawArrays(GL_TRIANGLES, 0, (m_widthx) * (m_widthz) * 3 * 2);

	CheckGLError(__FILE__, __LINE__);


#else
	int tilescale = m_tilescale;

	// Draw only visible tiles in strips
	for(int z=g_mapview[0].y/tilescale; z<=g_mapview[1].y/tilescale; z++)
	{
		int starti = m_widthx*z + g_mapview[0].x/tilescale;
		int spanx = g_mapview[1].x/tilescale - g_mapview[0].x/tilescale;
		int spanz = g_mapview[1].y/tilescale - g_mapview[0].y/tilescale;
		//int stridei = m_widthx - spanx;
		int stridei = 0;
	
		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, m_drawvertices);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, m_texcoords0);
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, m_normals);
		glDrawArrays(GL_TRIANGLES, starti * 2 * 3, spanx * 3 * 2);
	}
#endif
}

void Heightmap::drawrim()
{
	if(m_widthx <= 0 || m_widthz <= 0)
		return;
	//return;
	Shader* s = &g_shader[g_curS];
	
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_rimtexs[TEX_DIFF] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_TEXTURE0], 0);
	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_rimtexs[TEX_SPEC] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_SPECULARMAP], 1);
	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_rimtexs[TEX_NORM] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_NORMALMAP], 2);
	glActiveTextureARB(GL_TEXTURE3);
	
	//float yscale = TILE_Y_SCALE / 2000.0f;
	glUniform1f(s->m_slot[SSLOT_SANDONLYMAXY], ELEV_SANDONLYMAXY);
	glUniform1f(s->m_slot[SSLOT_SANDGRASSMAXY], ELEV_SANDGRASSMAXY);
	glUniform1f(s->m_slot[SSLOT_GRASSONLYMAXY], ELEV_GRASSONLYMAXY);
	glUniform1f(s->m_slot[SSLOT_GRASSROCKMAXY], ELEV_GRASSROCKMAXY);
	glUniform1f(s->m_slot[SSLOT_MAPMINZ], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXZ], m_widthz*TILE_SIZE*m_tilescale);
	glUniform1f(s->m_slot[SSLOT_MAPMINX], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXX], m_widthx*TILE_SIZE*m_tilescale);
	glUniform1f(s->m_slot[SSLOT_MAPMINY], ConvertHeight(0));
	glUniform1f(s->m_slot[SSLOT_MAPMAXY], ConvertHeight(255));

	/*
	for(int x=0; x<m_widthx; x++)
		for(int z=0; z<m_widthz; z++)
		{
			glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthx + x) * 3 * 2 + 0 ]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		*/
#if 1
	// Draw all tiles
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, m_rimva.vertices);
	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, m_rimva.texcoords);
	if(s->m_slot[SSLOT_NORMAL] != -1)
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, m_rimva.normals);
	glDrawArrays(GL_TRIANGLES, 0, m_rimva.numverts);

#else
	int tilescale = m_tilescale;

	// Draw only visible tiles in strips
	for(int z=g_mapview[0].y/tilescale; z<=g_mapview[1].y/tilescale; z++)
	{
		int starti = m_widthx*z + g_mapview[0].x/tilescale;
		int spanx = g_mapview[1].x/tilescale - g_mapview[0].x/tilescale;
		int spanz = g_mapview[1].y/tilescale - g_mapview[0].y/tilescale;
		//int stridei = m_widthx - spanx;
		int stridei = 0;
	
		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, m_drawvertices);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, m_texcoords0);
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, m_normals);
		glDrawArrays(GL_TRIANGLES, starti * 2 * 3, spanx * 3 * 2);
	}
#endif
}