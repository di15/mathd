
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

#ifndef _SERVER
unsigned int g_tiletexs[TILE_TYPES];
Vec2i g_mapview[2];
#endif

void Heightmap::allocate(int wx, int wz)
{
	int numverts = wx * wz * 3 * 2;

	m_widthx = wx;
	m_widthz = wz;

	/*
	float* heightpoints;
	Vec3f* vertices;
	Vec2f* texcoords0;
	Vec3f* normals;
	int* tiletype;
	unsigned int* tex;
	*/

	//g_pathfindszx = (int)(wx*TILE_SIZE/(MIN_RADIUS*2.0f));
	//g_pathfindszz = (int)(wz*TILE_SIZE/(MIN_RADIUS*2.0f));
	//g_open = new unsigned int [ g_pathfindszx * g_pathfindszz ];

	//for(int i=0; i<(g_pathfindszx-1)*(g_pathfindszz-1); i++)
	//	g_open[i] = g_curpath;
	//g_log<<"g_open = new bool [ "<<g_pathfindszx<<" * "<<g_pathfindszz<<" = "<<(g_pathfindszx * g_pathfindszz)<<" ]"<<endl;
	//g_log.flush();

	g_log<<"allocating class arrays "<<wx<<","<<wz<<endl;
	g_log.flush();

	//g_road = new CRoad [ (wx * wz) ];
	//g_roadPlan = new CRoad [ (wx * wz) ];
	//g_powerline = new CPowerline [ (wx * wz) ];
	//g_powerlinePlan = new CPowerline [ (wx * wz) ];
	//g_pipeline = new CPipeline[ (wx * wz) ];
	//g_pipelinePlan = new CPipeline[ (wx * wz) ];

	/*
	g_log<<"constructing class arrays"<<endl;
	g_log.flush();

	for(int i=0; i<wx*wz; i++)
	{
		g_road[i] = CRoad();
		g_roadPlan[i] = CRoad();
		g_powerline[i] = CPowerline();
		g_powerlinePlan[i] = CPowerline();
	}*/

	//g_road.resize( wx * wz );
	//g_roadPlan.resize( wx * wz );
	//g_powerline.resize( wx * wz );
	//g_powerlinePlan.resize( wx * wz );

	//g_log<<"allocating map primitive arrays"<<endl;
	//g_log.flush();

	m_heightpoints = new float [ (wx+1) * (wz+1) ];
#ifndef _SERVER
	m_vertices = new Vec3f [ numverts ];
	m_normals = new Vec3f [ numverts ];
	m_texcoords0 = new Vec2f [ numverts ];
#endif

	//g_log<<"setting heights to 0"<<endl;
	//g_log.flush();

	for(int x=0; x<=wx; x++)
		for(int z=0; z<=wz; z++)
			m_heightpoints[ z*(wx+1) + x ] = 0;

	//g_log<<"setting tile types to grass"<<endl;
	//g_log.flush();
	
	//QueueTexture(&g_tiletexs[TILE_SAND][TILE_TEX_GRADIENT], "textures/terrain/default/sand.gradient.jpg", false);
	//QueueTexture(&g_tiletexs[TILE_SAND][TILE_TEX_DETAIL], "textures/terrain/default/sand.detail.png", false);
	//QueueTexture(&g_tiletexs[TILE_GRASS][TILE_TEX_GRADIENT], "textures/terrain/default/grass.gradient.jpg", false);
	//QueueTexture(&g_tiletexs[TILE_GRASS][TILE_TEX_DETAIL], "textures/terrain/default/grass.detail.png", false);
	//QueueTexture(&g_tiletexs[TILE_DIRT][TILE_TEX_GRADIENT], "textures/terrain/default/dirt.gradient.jpg", false);
	//QueueTexture(&g_tiletexs[TILE_DIRT][TILE_TEX_DETAIL], "textures/terrain/default/dirt.detail.png", false);
	//QueueTexture(&g_tiletexs[TILE_ROCK][TILE_TEX_GRADIENT], "textures/terrain/default/rock.gradient.jpg", false);
	//QueueTexture(&g_tiletexs[TILE_ROCK][TILE_TEX_DETAIL], "textures/terrain/default/rock.detail.png", false);

#ifndef _SERVER
	remesh();
	//retexture();
#endif
	//FillColliderCells();
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

	if(g_road)
	{
		delete [] g_road;
		g_road = NULL;
	}

	g_log<<"deleting [] g_roadPlan"<<endl;
	g_log.flush();

	if(g_roadPlan)
	{
		delete [] g_roadPlan;
		g_roadPlan = NULL;
	}

	g_log<<"deleting [] g_powerline"<<endl;
	g_log.flush();

	if(g_powerline)
	{
		delete [] g_powerline;
		g_powerline = NULL;
	}

	g_log<<"deleting [] g_powerlinePlan"<<endl;
	g_log.flush();

	if(g_pipelinePlan)
	{
		delete [] g_pipelinePlan;
		g_pipelinePlan = NULL;
	}

	if(g_pipeline)
	{
		delete [] g_pipeline;
		g_pipeline = NULL;
	}
	*/
	//g_log<<"deleting [] m_heightpoints"<<endl;
	//g_log.flush();

	delete [] m_heightpoints;
#ifndef _SERVER
	delete [] m_vertices;
	delete [] m_normals;
	delete [] m_texcoords0;
#endif

	//g_log<<"setting wx,z to 0"<<endl;
	//g_log.flush();

	m_widthx = 0;
	m_widthz = 0;
}

void Heightmap::changeheight(int x, int z, float change)
{
	m_heightpoints[ (z)*(m_widthx+1) + x ] += change;
}

void Heightmap::setheight(int x, int z, float height)
{
	m_heightpoints[ (z)*(m_widthx+1) + x ] = height;
}

inline float Heightmap::getheight(int x, int z)
{
	return m_heightpoints[ (z)*(m_widthx+1) + x ];
}

#ifndef _SERVER

void Heightmap::remesh()
{
	Vec3f a, b, c, d;
	Vec3f norm0, norm1;
	Vec3f tri0[3];
	Vec3f tri1[3];
	int z;
	float heights[4];
	float aD, bD, cD, dD;
	float diag1010;
	float diag0101;

	for(int x=0; x<m_widthx; x++)
		for(z=0; z<m_widthz; z++)
		{
			heights[0] = getheight(x+1, z+1);
			heights[1] = getheight(x, z+1);
			heights[2] = getheight(x, z);
			heights[3] = getheight(x+1, z);

			a = Vec3f( (x+1)*TILE_SIZE, heights[0], (z+1)*TILE_SIZE );
			b = Vec3f( (x)*TILE_SIZE, heights[1], (z+1)*TILE_SIZE );
			c = Vec3f( (x)*TILE_SIZE, heights[2], (z)*TILE_SIZE );
			d = Vec3f( (x+1)*TILE_SIZE, heights[3], (z)*TILE_SIZE );

			aD = fabsf( heights[0] - (heights[1]+heights[3])/2.0f );
			bD = fabsf( heights[1] - (heights[2]+heights[0])/2.0f );
			cD = fabsf( heights[2] - (heights[3]+heights[1])/2.0f );
			dD = fabsf( heights[3] - (heights[0]+heights[2])/2.0f );
			diag1010 = max( aD, cD );
			diag0101 = max( bD, dD );

			if(diag1010 > diag0101)
			{
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 0 ] = a;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 1 ] = b;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 2 ] = d;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 3 ] = b;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 4 ] = c;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 5 ] = d;
				/*
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 0 ] = Vec2f(0, 0);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 1 ] = Vec2f(1, 0);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 2 ] = Vec2f(0, 1);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 3 ] = Vec2f(1, 0);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 4 ] = Vec2f(1, 1);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 5 ] = Vec2f(0, 1);
				*/
				tri0[0] = a;
				tri0[1] = b;
				tri0[2] = d;
				tri1[0] = b;
				tri1[1] = c;
				tri1[2] = d;
			}
			else
			{
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 0 ] = a;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 1 ] = b;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 2 ] = c;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 3 ] = d;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 4 ] = a;
				m_vertices[ (z * m_widthx + x) * 3 * 2 + 5 ] = c;
				/*
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 0 ] = Vec2f(0, 0);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 1 ] = Vec2f(1, 0);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 2 ] = Vec2f(1, 1);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 3 ] = Vec2f(0, 1);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 4 ] = Vec2f(0, 0);
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 5 ] = Vec2f(1, 1);
				*/
				tri0[0] = a;
				tri0[1] = b;
				tri0[2] = c;
				tri1[0] = d;
				tri1[1] = a;
				tri1[2] = c;
			}

			for(int i=0; i<6; i++)
			{
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + i ].x = m_vertices[ (z * m_widthx + x) * 3 * 2 + i ].x / TILE_SIZE;
				m_texcoords0[ (z * m_widthx + x) * 3 * 2 + i ].y = m_vertices[ (z * m_widthx + x) * 3 * 2 + i ].z / TILE_SIZE;
			}

			norm0 = Normal(tri0);
			norm1 = Normal(tri1);

			m_normals[ (z * m_widthx + x) * 3 * 2 + 0 ] = norm0;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 1 ] = norm0;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 2 ] = norm0;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 3 ] = norm1;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 4 ] = norm1;
			m_normals[ (z * m_widthx + x) * 3 * 2 + 5 ] = norm1;
		}
}

void Heightmap::draw()
{
	if(m_widthx <= 0 || m_widthz <= 0)
		return;

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
	
	float yscale = TILE_Y_SCALE / 2000.0f;
	glUniform1f(s->m_slot[SSLOT_SANDONLYMAXY], 10 * yscale);
	glUniform1f(s->m_slot[SSLOT_SANDGRASSMAXY], 100 * yscale);
	glUniform1f(s->m_slot[SSLOT_GRASSONLYMAXY], 600 * yscale);
	glUniform1f(s->m_slot[SSLOT_GRASSROCKMAXY], 990 * yscale);
	glUniform1f(s->m_slot[SSLOT_MAPMINZ], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXZ], m_widthz*TILE_SIZE);
	/*
	for(int x=0; x<m_widthx; x++)
		for(int z=0; z<m_widthz; z++)
		{
			glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &m_vertices[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthx + x) * 3 * 2 + 0 ]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		*/
	/*	
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, m_vertices);
	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, m_texcoords0);
	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, m_normals);
	glDrawArrays(GL_TRIANGLES, 0, (m_widthx) * (m_widthz) * 3 * 2);*/

	for(int z=g_mapview[0].y; z<=g_mapview[1].y; z++)
	{
		int starti = m_widthx*z + g_mapview[0].x;
		int spanx = g_mapview[1].x - g_mapview[0].x;
		int spanz = g_mapview[1].y - g_mapview[0].y;
		//int stridei = m_widthx - spanx;
		int stridei = 0;
	
		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, m_vertices);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, m_texcoords0);
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, m_normals);
		glDrawArrays(GL_TRIANGLES, starti * 2 * 3, spanx * 3 * 2);
	}

}

#endif	//#ifndef _SERVER

float Bilerp(Heightmap* hmap, float x, float z)
{
	x /= (float)TILE_SIZE;
	z /= (float)TILE_SIZE;

	int x1 = (int)(x);
	int x2 = x1 + 1;

	int z1 = (int)(z);
	int z2 = z1 + 1;

	float xdenom = x2-x1;
	float x2fac = (x2-x)/xdenom;
	float x1fac = (x-x1)/xdenom;

	float hR1 = hmap->getheight(x1,z1)*x2fac + hmap->getheight(x2,z1)*x1fac;
	float hR2 = hmap->getheight(x1,z2)*x2fac + hmap->getheight(x2,z2)*x1fac;

	float zdenom = z2-z1;

	return hR1*(z2-z)/zdenom + hR2*(z-z1)/zdenom;
}

bool GetMapIntersection2(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection)
{
	Vec3f vQuad[4];
	/*
	vQuad[0] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[1] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[2] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);
	vQuad[3] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);

	if(IntersectedPolygon(vQuad, vLine, 4, vIntersection))
		return true;
		*/

	return false;
}

bool GetMapIntersection(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection)
{
	Vec3f vTri[3];

	Vec3f* v = hmap->m_vertices;
	int wx = hmap->m_widthx;
	int wz = hmap->m_widthz;

	Vec3f tempint;
	bool intercepted = false;
	float closestint = 0;

	for(int x=0; x<wx; x++)
		for(int z=0; z<wz; z++)
		{
			/*
			vTri[0] = v[ (z * wx + x) * 3 * 2 + 0 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 2 ];

			if(IntersectedPolygon(vTri, vLine, 3, vIntersection))
				return true;

			vTri[0] = v[ (z * wx + x) * 3 * 2 + 3 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 5 ];

			if(IntersectedPolygon(vTri, vLine, 3, vIntersection))
				return true;
				*/
			
			vTri[0] = v[ (z * wx + x) * 3 * 2 + 0 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 2 ];

			if(IntersectedPolygon(vTri, vLine, 3, &tempint))
			{
				if(vIntersection)
				{
					float thisint = Magnitude2(vLine[0] - tempint);
					if(thisint < closestint || !intercepted)
					{
						*vIntersection = tempint;
						closestint = thisint;
					}
				}

				intercepted = true;
			}

			vTri[0] = v[ (z * wx + x) * 3 * 2 + 3 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 5 ];

			if(IntersectedPolygon(vTri, vLine, 3, &tempint))
			{
				if(vIntersection)
				{
					float thisint = Magnitude2(vLine[0] - tempint);
					if(thisint < closestint || !intercepted)
					{
						*vIntersection = tempint;
						closestint = thisint;
					}
				}

				intercepted = true;
			}
			

			/*
			vTri[0] = v[ (z * wx + x) * 3 * 2 + 2 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 0 ];

			if(IntersectedPolygon(vTri, vLine, 3, vIntersection))
				return true;

			vTri[0] = v[ (z * wx + x) * 3 * 2 + 5 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 3 ];

			if(IntersectedPolygon(vTri, vLine, 3, vIntersection))
				return true;*/
		}

	return intercepted;
	//return false;
}

/*
Given a point that is on a ray, move it until it is within a given heightmap's borders.
*/
void MoveIntoMap(Vec3f& point, Vec3f ray, Heightmap* hmap)
{
	// Already within map?
	if(point.x >= 1 
		&& point.x < hmap->m_widthx * TILE_SIZE - 1
		&& point.z >= 1
		&& point.z < hmap->m_widthz * TILE_SIZE - 1)
		return;

	// Get x distance off the map.

	float xdif = 0;

	if(point.x < 0)	// If start x is behind the map
		xdif = point.x - 5;	// Add padding to make sure we're within the map
	else if(point.x > hmap->m_widthx * TILE_SIZE)	// If start x is in front of the map
		xdif = hmap->m_widthx * TILE_SIZE - point.x + 5;	// Add padding to make sure we're within the map

	// Ray is of unit length, so this gives us how much we travel along the ray to get  x to within the map border.
	float x0moveratio = -xdif / ray.x;
	point = point + ray * x0moveratio;
	
	// Get z distance off the map.

	float zdif = 0;

	if(point.z < 0)	// If start z is behind the map
		zdif = point.z - 5;	// Add padding to make sure we're within the map
	else if(point.z > hmap->m_widthz * TILE_SIZE)	// If start z is in front of the map
		zdif = hmap->m_widthz * TILE_SIZE - point.z + 5;	// Add padding to make sure we're within the map
	
	// Ray is of unit length, so this gives us how much we travel along the ray to get z to within the map border.
	float z0moveratio = -zdif / ray.z;
	point = point + ray * z0moveratio;
}

bool TileIntersect(Heightmap* hmap, Vec3f* line, int x, int z, Vec3f* intersection)
{
	Vec3f tri[3];
	const int wx = hmap->m_widthx;
	const int wz = hmap->m_widthz;
	Vec3f* v = hmap->m_vertices;

	tri[0] = v[ (z * wx + x) * 3 * 2 + 0 ];
	tri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
	tri[2] = v[ (z * wx + x) * 3 * 2 + 2 ];

	if(IntersectedPolygon(tri, line, 3, intersection))
		return true;

	tri[0] = v[ (z * wx + x) * 3 * 2 + 3 ];
	tri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
	tri[2] = v[ (z * wx + x) * 3 * 2 + 5 ];

	if(IntersectedPolygon(tri, line, 3, intersection))
		return true;

	return false;
}

bool FastMapIntersect(Heightmap* hmap, Vec3f line[2], Vec3f* intersection)
{
	// If both start and end are on one side of the map, we can't get an intersection, so return false.

	if(line[0].x < 0 && line[1].x < 0)
		return false;

	if(line[0].x >= hmap->m_widthx * TILE_SIZE && line[1].x >= hmap->m_widthx * TILE_SIZE)
		return false;

	if(line[0].z < 0 && line[1].z < 0)
		return false;
	
	if(line[0].z >= hmap->m_widthz * TILE_SIZE && line[1].z >= hmap->m_widthz * TILE_SIZE)
		return false;
	
	Vec3f ray = Normalize( line[1] - line[0] );
	float lengthsqrd = Magnitude2( line[1] - line[0] );

	MoveIntoMap(line[0], ray, hmap);	// Move the start to within the map if it isn't already.
	MoveIntoMap(line[1], ray, hmap);	// Move the end to within the map if it isn't already.

	float lengthdone = 0;

	Vec3f currpoint = line[0];

	int currtilex = currpoint.x / TILE_SIZE;
	int currtilez = currpoint.z / TILE_SIZE;

	int nexttilex = currtilex;
	int nexttilez = currtilez;
	
	// The directions in which we will move to the next tile on the x and z axis
	int tiledx = 0;
	int tiledz = 0;

	if(ray.x > 0)
		tiledx = 1;
	else if(ray.x < 0)
		tiledx = -1;

	if(ray.z > 0)
		tiledz = 1;
	else if(ray.z < 0)
		tiledz = -1;

	// Move from tile to tile along the line until,
	// testing each tile's triangles for intersection.
	while(lengthdone*lengthdone < lengthsqrd)
	{
		float xdif = fabs( currtilex * TILE_SIZE - currpoint.x );
		float zdif = fabs( currtilez * TILE_SIZE - currpoint.z );

		float xmoveratio = xdif / ray.x + 1;	// Add padding to make sure we get into the next tile
		float zmoveratio = zdif / ray.z + 1;	// Add padding to make sure we get into the next tile

		float moveratio = 0;

		// Move the smallest distance to the next tile
		if(xmoveratio < zmoveratio)
		{
			moveratio = xmoveratio;
		}
		else
		{
			moveratio = zmoveratio;
		}

		currpoint = currpoint + ray * moveratio; 

		if(TileIntersect(hmap, line, currtilex, currtilez, intersection))
			return true;

		lengthdone += moveratio;

		// Move the smallest distance to the next tile
		if(xmoveratio < zmoveratio)
		{
			currtilex = nexttilex;
			nexttilex += tiledx;
		}
		else
		{
			currtilez = nexttilez;
			nexttilez += tiledz;
		}
	}

	return false;
}