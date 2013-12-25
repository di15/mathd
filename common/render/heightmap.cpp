

#include "heightmap.h"
#include "../math/polygon.h"
#include "water.h"

CTileType g_tiletype[TILETYPES];
unsigned int g_terrain;
unsigned int g_detail;
unsigned int g_underground;
unsigned int g_detail2;
unsigned int g_sky;
unsigned int g_circle;
Vec3f g_vMouse;
Vec3f g_vMouseStart;
Vec3f g_vTile;
Vec3f g_vStart;
int g_surroundings;
unsigned int g_water;
unsigned int g_transparency;
//int g_hmap;
Heightmap g_hmap;

bool GetMapIntersection2(Vec3f vLine[], Vec3f* vIntersection)
{
	Vec3f vQuad[4];
	
	vQuad[0] = Vec3f(-10*g_hmap.m_widthX*TILE_SIZE, WATER_HEIGHT*2.0f, -10*g_hmap.m_widthZ*TILE_SIZE);
	vQuad[1] = Vec3f(10*g_hmap.m_widthX*TILE_SIZE, WATER_HEIGHT*2.0f, -10*g_hmap.m_widthZ*TILE_SIZE);
	vQuad[2] = Vec3f(10*g_hmap.m_widthX*TILE_SIZE, WATER_HEIGHT*2.0f, 10*g_hmap.m_widthZ*TILE_SIZE);
	vQuad[3] = Vec3f(-10*g_hmap.m_widthX*TILE_SIZE, WATER_HEIGHT*2.0f, 10*g_hmap.m_widthZ*TILE_SIZE);
	
	if(IntersectedPolygon(vQuad, vLine, 4, vIntersection))
		return true;

	return false;
}

bool GetMapIntersection(Vec3f* vLine, Vec3f* vIntersection)
{
	Vec3f vTri[3];

	Vec3f* v = g_hmap.m_vertices;
	int wx = g_hmap.m_widthx;
	int wz = g_hmap.m_widthz;

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

bool GetMapIntersectionD(Vec3f vLine[], Vec3f* vIntersection)
{
	/*
	Vec3f vQuad[4];
	
	vQuad[0] = Vec3f(-10*MAP_SIZE*TILE_SIZE, 0, -10*MAP_SIZE*TILE_SIZE);
	vQuad[1] = Vec3f(10*MAP_SIZE*TILE_SIZE, 0, 0);
	vQuad[2] = Vec3f(10*MAP_SIZE*TILE_SIZE, 0, 10*MAP_SIZE*TILE_SIZE);
	vQuad[3] = Vec3f(0, 0, 10*MAP_SIZE*TILE_SIZE);
	
	if(IntersectedPolygon(vQuad, vLine, 4, vIntersection))
		return true;*/

	Vec3f vTri[3];

	Vec3f* v = g_hmap.m_vertices;
	int wx = g_hmap.m_widthX;
	int wz = g_hmap.m_widthZ;

	int z;
	for(int x=0; x<wx; x++)
		for(z=0; z<wz; z++)
		{
			vTri[0] = v[ (z * wx + x) * 3 * 2 + 0 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 2 ];

			if(IntersectedPolygon(vTri, vLine, 3, vIntersection))
			{
				int dx = (vLine[0].x/TILE_SIZE)-x;
				int dz = (vLine[0].z/TILE_SIZE)-z;
				char msg[128];
				sprintf(msg, "gmid (%d,%d)", dx, dz);
				Chat(msg);
				return true;
			}
			
			vTri[0] = v[ (z * wx + x) * 3 * 2 + 3 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 5 ];

			if(IntersectedPolygon(vTri, vLine, 3, vIntersection))
			{
				int dx = (vLine[0].x/TILE_SIZE)-x;
				int dz = (vLine[0].z/TILE_SIZE)-z;
				char msg[128];
				sprintf(msg, "gmid (%d,%d)", dx, dz);
				Chat(msg);
				return true;
			}
		}

	return false;
}

void QueueTile(unsigned int* tex, char* path, const char* file)
{
	char full[128];
	sprintf(full, "%s%s", path, file);
	FindTextureExtension(full);
	QueueTexture(tex, full, true);
}

void CTileType::reloadTextures()
{
	char path[64];
	sprintf(path, "tiles\\%s\\", m_name);

	if(!stricmp(m_name, "none"))
	{
		QueueTile(&m_diffuse[TILE_0001], path, "0000");
		QueueTile(&m_diffuse[TILE_0010], path, "0000");
		QueueTile(&m_diffuse[TILE_0011], path, "0000");
		QueueTile(&m_diffuse[TILE_0100], path, "0000");
		QueueTile(&m_diffuse[TILE_0101], path, "0000");
		QueueTile(&m_diffuse[TILE_0110], path, "0000");
		QueueTile(&m_diffuse[TILE_0111], path, "0000");
		QueueTile(&m_diffuse[TILE_1000], path, "0000");
		QueueTile(&m_diffuse[TILE_1001], path, "0000");
		QueueTile(&m_diffuse[TILE_1010], path, "0000");
		QueueTile(&m_diffuse[TILE_1011], path, "0000");
		QueueTile(&m_diffuse[TILE_1100], path, "0000");
		QueueTile(&m_diffuse[TILE_1101], path, "0000");
		QueueTile(&m_diffuse[TILE_1110], path, "0000");
		QueueTile(&m_diffuse[TILE_1111], path, "0000");
		QueueTile(&g_transparency, path, "0000");
	}
	else
	{
		QueueTile(&m_diffuse[TILE_0001], path, "0001");
		QueueTile(&m_diffuse[TILE_0010], path, "0010");
		QueueTile(&m_diffuse[TILE_0011], path, "0011");
		QueueTile(&m_diffuse[TILE_0100], path, "0100");
		QueueTile(&m_diffuse[TILE_0101], path, "0101");
		QueueTile(&m_diffuse[TILE_0110], path, "0110");
		QueueTile(&m_diffuse[TILE_0111], path, "0111");
		QueueTile(&m_diffuse[TILE_1000], path, "1000");
		QueueTile(&m_diffuse[TILE_1001], path, "1001");
		QueueTile(&m_diffuse[TILE_1010], path, "1010");
		QueueTile(&m_diffuse[TILE_1011], path, "1011");
		QueueTile(&m_diffuse[TILE_1100], path, "1100");
		QueueTile(&m_diffuse[TILE_1101], path, "1101");
		QueueTile(&m_diffuse[TILE_1110], path, "1110");
		QueueTile(&m_diffuse[TILE_1111], path, "1111");
		/*
		QueueTile(&m_specular[TILE_0001], path, "spec0001");
		QueueTile(&m_specular[TILE_0010], path, "spec0010");
		QueueTile(&m_specular[TILE_0011], path, "spec0011");
		QueueTile(&m_specular[TILE_0100], path, "spec0100");
		QueueTile(&m_specular[TILE_0101], path, "spec0101");
		QueueTile(&m_specular[TILE_0110], path, "spec0110");
		QueueTile(&m_specular[TILE_0111], path, "spec0111");
		QueueTile(&m_specular[TILE_1000], path, "spec1000");
		QueueTile(&m_specular[TILE_1001], path, "spec1001");
		QueueTile(&m_specular[TILE_1010], path, "spec1010");
		QueueTile(&m_specular[TILE_1011], path, "spec1011");
		QueueTile(&m_specular[TILE_1100], path, "spec1100");
		QueueTile(&m_specular[TILE_1101], path, "spec1101");
		QueueTile(&m_specular[TILE_1110], path, "spec1110");
		QueueTile(&m_specular[TILE_1111], path, "spec1111");
		QueueTile(&m_normal[TILE_0001], path, "norm0001");
		QueueTile(&m_normal[TILE_0010], path, "norm0010");
		QueueTile(&m_normal[TILE_0011], path, "norm0011");
		QueueTile(&m_normal[TILE_0100], path, "norm0100");
		QueueTile(&m_normal[TILE_0101], path, "norm0101");
		QueueTile(&m_normal[TILE_0110], path, "norm0110");
		QueueTile(&m_normal[TILE_0111], path, "norm0111");
		QueueTile(&m_normal[TILE_1000], path, "norm1000");
		QueueTile(&m_normal[TILE_1001], path, "norm1001");
		QueueTile(&m_normal[TILE_1010], path, "norm1010");
		QueueTile(&m_normal[TILE_1011], path, "norm1011");
		QueueTile(&m_normal[TILE_1100], path, "norm1100");
		QueueTile(&m_normal[TILE_1101], path, "norm1101");
		QueueTile(&m_normal[TILE_1110], path, "norm1110");
		QueueTile(&m_normal[TILE_1111], path, "norm1111");*/
	}
}

void Heightmap::allocate(int wx, int wz)
{
	int numverts = wx * wz * 3 * 2;

	m_widthX = wx;
	m_widthZ = wz;

	/*
	float* heightpoints;
	Vec3f* vertices;
	Vec2f* texcoords0;
	Vec3f* normals;
	int* tiletype;
	unsigned int* tex;
	*/
	
	g_pathfindszx = (int)(wx*TILE_SIZE/(MIN_RADIUS*2.0f));
	g_pathfindszz = (int)(wz*TILE_SIZE/(MIN_RADIUS*2.0f));
	g_open = new unsigned int [ g_pathfindszx * g_pathfindszz ];

	for(int i=0; i<(g_pathfindszx-1)*(g_pathfindszz-1); i++)
		g_open[i] = g_curpath;
	g_log<<"g_open = new bool [ "<<g_pathfindszx<<" * "<<g_pathfindszz<<" = "<<(g_pathfindszx * g_pathfindszz)<<" ]"<<endl;
	g_log.flush();

	g_log<<"allocating class arrays "<<wx<<","<<wz<<endl;
	g_log.flush();

	g_road = new CRoad [ (wx * wz) ];
	g_roadPlan = new CRoad [ (wx * wz) ];
	g_powerline = new CPowerline [ (wx * wz) ];
	g_powerlinePlan = new CPowerline [ (wx * wz) ];
	g_pipeline = new CPipeline[ (wx * wz) ];
	g_pipelinePlan = new CPipeline[ (wx * wz) ];

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

	g_log<<"allocating map primitive arrays"<<endl;
	g_log.flush();

	m_heightpoints = new float [ (wx+1) * (wz+1) ];
	m_vertices = new Vec3f [ numverts ];
	m_normals = new Vec3f [ numverts ];
	m_texcoords0 = new Vec2f [ numverts ];
	m_tiletype = new int [ (wx+1) * (wz+1) ];
	m_diffuse = new unsigned int [ wx * wz * 4 ];
	m_specular = new unsigned int [ wx * wz * 4 ];
	m_normalmap = new unsigned int [ wx * wz * 4 ];

	g_log<<"setting heights to 0"<<endl;
	g_log.flush();

	for(int x=0; x<=wx; x++)
		for(int z=0; z<=wz; z++)
			m_heightpoints[ z*(wx+1) + x ] = 0;
	
	
	g_log<<"setting tile types to grass"<<endl;
	g_log.flush();

	//int dirtts = FindTileset("dirt");
	int grassts = FindTileset("grass");

	for(int x=0; x<=wx; x++)
		for(int z=0; z<=wz; z++)
		{
			//m_tiletype[ (z+1) * (wx+1) + (x+1) ] = grassts;
			//m_tiletype[ (z+1) * (wx+1) + (x) ] = grassts;
			m_tiletype[ (z) * (wx+1) + (x) ] = grassts;
			//m_tiletype[ (z) * (wx+1) + (x+1) ] = grassts;
		}
		
	g_log<<"remeshing"<<endl;
	g_log.flush();

	remesh();
	retexture();
	FillColliderCells();
}

void Heightmap::free()
{
	if(m_widthX <= 0 || m_widthZ <= 0)
		return;

	g_log<<"deleting [] g_open"<<endl;
	g_log.flush();

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
	
	g_log<<"deleting [] m_heightpoints"<<endl;
	g_log.flush();
	
	delete [] m_heightpoints;
	delete [] m_vertices;
	delete [] m_normals;
	delete [] m_tiletype;
	delete [] m_diffuse;
	delete [] m_specular;
	delete [] m_normalmap;
	delete [] m_texcoords0;
	
	g_log<<"setting wx,z to 0"<<endl;
	g_log.flush();

	m_widthX = 0;
	m_widthZ = 0;
}

void Heightmap::settile(int x, int z, int type)
{
	m_tiletype[ (z) * (m_widthX+1) + (x) ] = type;
}

void Heightmap::changeheight(int x, int z, float change)
{
	m_heightpoints[ (z)*(m_widthX+1) + x ] += change;
}

void Heightmap::setheight(int x, int z, float height)
{
	m_heightpoints[ (z)*(m_widthX+1) + x ] = height;
}

inline float Heightmap::getheight(int x, int z)
{
	return m_heightpoints[ (z)*(m_widthX+1) + x ];
}

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

	for(int x=0; x<m_widthX; x++)
		for(z=0; z<m_widthZ; z++)
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
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 0 ] = a;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 1 ] = b;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 2 ] = d;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 3 ] = b;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 4 ] = c;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 5 ] = d;

				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 0 ] = Vec2f(0, 0);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 1 ] = Vec2f(1, 0);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 2 ] = Vec2f(0, 1);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 3 ] = Vec2f(1, 0);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 4 ] = Vec2f(1, 1);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 5 ] = Vec2f(0, 1);

				tri0[0] = a;
				tri0[1] = b;
				tri0[2] = d;
				tri1[0] = b;
				tri1[1] = c;
				tri1[2] = d;
			}
			else
			{
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 0 ] = a;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 1 ] = b;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 2 ] = c;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 3 ] = d;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 4 ] = a;
				m_vertices[ (z * m_widthX + x) * 3 * 2 + 5 ] = c;
			
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 0 ] = Vec2f(0, 0);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 1 ] = Vec2f(1, 0);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 2 ] = Vec2f(1, 1);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 3 ] = Vec2f(0, 1);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 4 ] = Vec2f(0, 0);
				m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 5 ] = Vec2f(1, 1);

				tri0[0] = a;
				tri0[1] = b;
				tri0[2] = c;
				tri1[0] = d;
				tri1[1] = a;
				tri1[2] = c;
			}
			
			norm0 = Normal(tri0);
			norm1 = Normal(tri1);
				
			m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ] = norm0;
			m_normals[ (z * m_widthX + x) * 3 * 2 + 1 ] = norm0;
			m_normals[ (z * m_widthX + x) * 3 * 2 + 2 ] = norm0;
			m_normals[ (z * m_widthX + x) * 3 * 2 + 3 ] = norm1;
			m_normals[ (z * m_widthX + x) * 3 * 2 + 4 ] = norm1;
			m_normals[ (z * m_widthX + x) * 3 * 2 + 5 ] = norm1;
		}
}

void Heightmap::retexture()
{
	int tileset_none = FindTileset("none");
	//int tileset_dirt = FindTileset("dirt");
	
	unsigned int none_1111_dif = g_tiletype[tileset_none].m_diffuse[TILE_1111];
	//unsigned int dirt_1111 = g_tiletype[tileset_dirt].m_diffuse[TILE_1111];

	int tiletypes[4];

	int z;
	int i, j, k;
	bool used;

	bool a, b, c, d;
	int connectivity;
	unsigned int diff;

	for(int x=0; x<m_widthX; x++)
		for(z=0; z<m_widthZ; z++)
		{
			tiletypes[0] = m_tiletype[ (z+1) * (m_widthX+1) + (x+1) ];
			tiletypes[1] = m_tiletype[ (z+1) * (m_widthX+1) + (x) ];
			tiletypes[2] = m_tiletype[ (z) * (m_widthX+1) + (x) ];
			tiletypes[3] = m_tiletype[ (z) * (m_widthX+1) + (x+1) ];
			//m_diffuse[ (z * m_widthX + x) * 4 + 0 ] = dirt_1111;
			//m_diffuse[ (z * m_widthX + x) * 4 + 1 ] = dirt_1111;
			//m_diffuse[ (z * m_widthX + x) * 4 + 2 ] = dirt_1111;
			//m_diffuse[ (z * m_widthX + x) * 4 + 3 ] = dirt_1111;

			vector<int> usedtypes;
			int least = TILETYPES+1;

			for(i=0; i<4; i++)
			{
				least = TILETYPES+1;
				for(j=0; j<4; j++)
				{
					used = false;
					for(k=0; k<usedtypes.size(); k++)
					{
						if(usedtypes[k] == tiletypes[j])
						{
							used = true;
							break;
						}
					}
					if(used)
						continue;

					if(tiletypes[j] < least)
						least = tiletypes[j];
				}

				if(least == TILETYPES+1)
					diff = none_1111_dif;
				else
				{
					usedtypes.push_back(least);

					if(i == 0)
						connectivity = TILE_1111;
					else
					{
						a = (tiletypes[0] == least);
						b = (tiletypes[1] == least);
						c = (tiletypes[2] == least);
						d = (tiletypes[3] == least);
						connectivity = TileConn(a, b, c, d);
					}

					diff = g_tiletype[least].m_diffuse[connectivity];

					//g_log<<"diff = "<<g_tiletype[least].m_name<<endl;
					//g_log.flush();
				}

				m_diffuse[ (z * m_widthX + x) * 4 + i ] = diff;
			}
		}
}

void Heightmap::draw()
{
	Shader* s = &g_shader[g_curS];

	int z;
	for(int x=0; x<m_widthX; x++)
		for(z=0; z<m_widthZ; z++)
		{
			glActiveTextureARB(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, g_depth);
			glBindTexture(GL_TEXTURE_2D, m_diffuse[ (z * m_widthX + x) * 4 + 0 ]);
			//glUniform1iARB(s->m_slot[SLOT::TEXTURE0], 0);
			
			glActiveTextureARB(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, g_depth);
			glBindTexture(GL_TEXTURE_2D, m_diffuse[ (z * m_widthX + x) * 4 + 1 ]);
			//glUniform1iARB(s->m_slot[SLOT::TEXTURE1], 1);
			
			glActiveTextureARB(GL_TEXTURE2);
			//glBindTexture(GL_TEXTURE_2D, g_depth);
			glBindTexture(GL_TEXTURE_2D, m_diffuse[ (z * m_widthX + x) * 4 + 2 ]);
			//glUniform1iARB(s->m_slot[SLOT::TEXTURE2], 2);
			
			glActiveTextureARB(GL_TEXTURE3);
			//glBindTexture(GL_TEXTURE_2D, g_depth);
			glBindTexture(GL_TEXTURE_2D, m_diffuse[ (z * m_widthX + x) * 4 + 3 ]);
			//glUniform1iARB(s->m_slot[SLOT::TEXTURE3], 3);
    
			glVertexAttribPointer(s->m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &m_vertices[ (z * m_widthX + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SLOT::TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &m_texcoords0[ (z * m_widthX + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
}

int TileConn(bool a, bool b, bool c, bool d)
{
	if(!a && !b && !c && d)
		return TILE_0001;
	else if(!a && !b && c && !d)
		return TILE_0010;
	else if(!a && !b && c && d)
		return TILE_0011;
	else if(!a && b && !c && !d)
		return TILE_0100;
	else if(!a && b && !c && d)
		return TILE_0101;
	else if(!a && b && c && !d)
		return TILE_0110;
	else if(!a && b && c && d)
		return TILE_0111;
	else if(a && !b && !c && !d)
		return TILE_1000;
	else if(a && !b && !c && d)
		return TILE_1001;
	else if(a && !b && c && !d)
		return TILE_1010;
	else if(a && !b && c && d)
		return TILE_1011;
	else if(a && b && !c && !d)
		return TILE_1100;
	else if(a && b && !c && d)
		return TILE_1101;
	else if(a && b && c && !d)
		return TILE_1110;
	else if(a && b && c && d)
		return TILE_1111;

	return TILE_1111;
}

int FindTileset(const char* name)
{
	for(int i=0; i<TILETYPES; i++)
		if(!stricmp(g_tiletype[i].m_name, name))
			return i;

	return -1;
}

int NewTileset()
{
	for(int i=0; i<TILETYPES; i++)
	{
		if(!g_tiletype[i].on)
			return i;
	}

	return -1;
}

void LoadTileset(char* name)
{
	int i = NewTileset();
	if(i < 0)
		return;
	
	CTileType* t = &g_tiletype[i];

	t->on = true;
	strcpy(t->m_name, name);
	t->reloadTextures();

	/*
	char full[128];
	sprintf(full, "tiles\\%s\\1111", name);
	FindTextureExtension(full);

	//Button("tileset buttons", full, &Click_TileSet, &Over_TileSet, &CloseOverTS, i);
	*/
}

void LoadNoneTileset()
{
	int i = NewTileset();
	if(i < 0)
		return;

	CTileType* t = &g_tiletype[i];

	t->on = true;
	strcpy(t->m_name, "none");
	t->reloadTextures();
	
	/*
	char full[128];
	sprintf(full, "tiles\\%s\\0000", "none");
	FindTextureExtension(full);

	Button("tileset buttons", full, &Click_TileSet, &Over_TileSet, &CloseOverTS, i);*/
}

void LoadTiles()
{
	LoadNoneTileset();

	WIN32_FIND_DATA ffd;
	string tilesPath = ExePath() + "\\tiles\\*";
	HANDLE hFind = FindFirstFileEx(tilesPath.c_str(), FindExInfoStandard, &ffd, FindExSearchLimitToDirectories, NULL, 0);

	if(INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			if(!stricmp(ffd.cFileName, ".") || !stricmp(ffd.cFileName, "..") || !stricmp(ffd.cFileName, "none"))
				continue;

			LoadTileset(ffd.cFileName);
		} while(FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	else
	{
		// Failure
	}
}

void StartEndCorners(int& x1, int& x2, int& z1, int& z2)
{
	x1 = Clipi( (g_vMouseStart.x + TILE_SIZE/2)/TILE_SIZE, 0, g_hmap.m_widthX);
	z1 = Clipi( (g_vMouseStart.z + TILE_SIZE/2)/TILE_SIZE, 0, g_hmap.m_widthZ);
	x2 = Clipi( (g_vMouse.x + TILE_SIZE/2)/TILE_SIZE, 0, g_hmap.m_widthX);
	z2 = Clipi( (g_vMouse.z + TILE_SIZE/2)/TILE_SIZE, 0, g_hmap.m_widthZ);
	/*
	x1 = Clip( (g_vStart.x + TILE_SIZE/2)/TILE_SIZE, 0, g_hmap.m_widthX);
	z1 = Clip( (g_vStart.z + TILE_SIZE/2)/TILE_SIZE, 0, g_hmap.m_widthZ);
	x2 = Clip( (g_vTile.x + TILE_SIZE/2)/TILE_SIZE, 0, g_hmap.m_widthX);
	z2 = Clip( (g_vTile.z + TILE_SIZE/2)/TILE_SIZE, 0, g_hmap.m_widthZ);
	/*
	x1 = Clip( (g_vStart.x)/TILE_SIZE, 0, g_hmap.m_widthX);
	z1 = Clip( (g_vStart.z)/TILE_SIZE, 0, g_hmap.m_widthZ);
	x2 = Clip( (g_vTile.x)/TILE_SIZE, 0, g_hmap.m_widthX);
	z2 = Clip( (g_vTile.z)/TILE_SIZE, 0, g_hmap.m_widthZ);*/
}

void MinMaxCorners(int& minx, int& maxx, int& minz, int& maxz)
{
	int x1, x2, z1, z2;
	StartEndCorners(x1, x2, z1, z2);

	minx = min(x1, x2);
	minz = min(z1, z2);
	maxx = max(x1, x2);
	maxz = max(z1, z2);
}

float Bilerp(float x, float y)
{
	x /= (float)TILE_SIZE;
	y /= (float)TILE_SIZE;

	int x1 = (int)(x);
	int x2 = x1 + 1;
	
	int y1 = (int)(y);
	int y2 = y1 + 1;

	float hR1 = g_hmap.getheight(x1,y1)*(x2-x)/(x2-x1) + g_hmap.getheight(x2,y1)*(x-x1)/(x2-x1);
	float hR2 = g_hmap.getheight(x1,y2)*(x2-x)/(x2-x1) + g_hmap.getheight(x2,y2)*(x-x1)/(x2-x1);

	return hR1*(y2-y)/(y2-y1) + hR2*(y-y1)/(y2-y1);
}

float MapMeshHeight(float x, float z)
{
	Vec3f line[2];
	line[0] = Vec3f(x, 999.0f, z);
	line[1] = Vec3f(x, -999.0f, z);

	Vec3f inter;
	Vec3f vTri[3];

	Vec3f* v = g_hmap.m_vertices;
	int wx = g_hmap.m_widthX;
	int wz = g_hmap.m_widthZ;

	//bool res = GetMapIntersectionD(line, &inter);
	
	int tx = x / TILE_SIZE;
	int tz = z / TILE_SIZE;

	vTri[0] = v[ (tz * wx + tx) * 3 * 2 + 0 ];
	vTri[1] = v[ (tz * wx + tx) * 3 * 2 + 1 ];
	vTri[2] = v[ (tz * wx + tx) * 3 * 2 + 2 ];

	if(IntersectedPolygon(vTri, line, 3, &inter))
	{
		Chat("mh succ");
		return inter.y;
	}
			
	vTri[0] = v[ (tz * wx + tx) * 3 * 2 + 3 ];
	vTri[1] = v[ (tz * wx + tx) * 3 * 2 + 4 ];
	vTri[2] = v[ (tz * wx + tx) * 3 * 2 + 5 ];

	if(IntersectedPolygon(vTri, line, 3, &inter))
	{
		Chat("mh succ");
		return inter.y;
	}

	int tx2, tz2;
	//if((int)(x)%TILE_SIZE < TILE_SIZE/2)
	{
		tx2 = tx-1;
		tz2 = tz;

		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 0 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 1 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 2 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
			
		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 3 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 4 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 5 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
	}
	//else
	{
		tx2 = tx+1;
		tz2 = tz;

		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 0 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 1 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 2 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
			
		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 3 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 4 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 5 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
	}

	//if((int)(z)%TILE_SIZE < TILE_SIZE/2)
	{
		tx2 = tx;
		tz2 = tz-1;

		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 0 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 1 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 2 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
			
		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 3 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 4 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 5 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
	}
	//else
	{
		tx2 = tx;
		tz2 = tz+1;

		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 0 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 1 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 2 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
			
		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 3 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 4 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 5 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
	}

	{
		tx2 = tx-1;
		tz2 = tz-1;

		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 0 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 1 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 2 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
			
		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 3 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 4 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 5 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
	}
	{
		tx2 = tx-1;
		tz2 = tz+1;

		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 0 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 1 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 2 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
			
		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 3 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 4 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 5 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
	}
	{
		tx2 = tx+1;
		tz2 = tz-1;

		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 0 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 1 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 2 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
			
		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 3 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 4 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 5 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
	}
	{
		tx2 = tx+1;
		tz2 = tz+1;

		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 0 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 1 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 2 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
			
		vTri[0] = v[ (tz2 * wx + tx2) * 3 * 2 + 3 ];
		vTri[1] = v[ (tz2 * wx + tx2) * 3 * 2 + 4 ];
		vTri[2] = v[ (tz2 * wx + tx2) * 3 * 2 + 5 ];

		if(IntersectedPolygon(vTri, line, 3, &inter))
			return inter.y;
	}

	//if(!res)
	{
		char msg[128];
		sprintf(msg, "mmh fail (%d,%d)", tx, tz);
		Chat(msg);
	}

	return inter.y;
}

// Select corner
void DrawTileSq()
{
	int minx, maxx, minz, maxz;
	MinMaxCorners(minx, maxx, minz, maxz);

	//char msg[128];
	//sprintf(msg, "(%d,%d) => (%d,%d)", minx, minz, maxx, maxz);
	//Chat(msg);
    
	/*
	if(minx == maxx)
		maxx = Clip(maxx+1, 0, g_hmap.m_widthX-1);
	if(minz == maxz)
		maxz = Clip(maxz+1, 0, g_hmap.m_widthZ-1);
		*/

	const float heightoffset = 0.1f;
	const float linehalfwidth = 0.5f;
	const float lineoffset = 2.0f;
	float height;

	Vec3f a, b, c, d;
	int x;
	int z=minz;
	for(x=minx; x<=maxx; x++)
	{
		height = g_hmap.getheight(x, z) + heightoffset;
		a = Vec3f((x)*TILE_SIZE + lineoffset, height, (z)*TILE_SIZE - lineoffset + linehalfwidth);
		b = Vec3f((x)*TILE_SIZE, height, (z)*TILE_SIZE - lineoffset + linehalfwidth);
		c = Vec3f((x)*TILE_SIZE, height, (z)*TILE_SIZE - lineoffset - linehalfwidth);
		d = Vec3f((x)*TILE_SIZE + lineoffset, height, (z)*TILE_SIZE - lineoffset - linehalfwidth);

		float vertices[] =
		{
            a.x, a.y, a.z,
            b.x, b.y, b.z,
            c.x, c.y, c.z,
            
            d.x, d.y, d.z,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
		};

		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
		//glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	
	z=maxz;
	for(x=minx; x<=maxx; x++)
	{
		height = g_hmap.getheight(x, z) + heightoffset;
		a = Vec3f((x)*TILE_SIZE + lineoffset, height, (z)*TILE_SIZE + lineoffset + linehalfwidth);
		b = Vec3f((x)*TILE_SIZE, height, (z)*TILE_SIZE + lineoffset + linehalfwidth);
		c = Vec3f((x)*TILE_SIZE, height, (z)*TILE_SIZE + lineoffset - linehalfwidth);
		d = Vec3f((x)*TILE_SIZE + lineoffset, height, (z)*TILE_SIZE + lineoffset - linehalfwidth);

		float vertices[] =
		{
            a.x, a.y, a.z,
            b.x, b.y, b.z,
            c.x, c.y, c.z,
            
            d.x, d.y, d.z,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
		};

		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
		//glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	x=minx;
	for(z=minz; z<=maxz; z++)
	{
		height = g_hmap.getheight(x, z) + heightoffset;
		a = Vec3f((x)*TILE_SIZE - lineoffset + linehalfwidth, height, (z)*TILE_SIZE + lineoffset);
		b = Vec3f((x)*TILE_SIZE - lineoffset - linehalfwidth, height, (z)*TILE_SIZE + lineoffset);
		c = Vec3f((x)*TILE_SIZE - lineoffset - linehalfwidth, height, (z)*TILE_SIZE);
		d = Vec3f((x)*TILE_SIZE - lineoffset + linehalfwidth, height, (z)*TILE_SIZE);

		float vertices[] =
		{
            a.x, a.y, a.z,
            b.x, b.y, b.z,
            c.x, c.y, c.z,
            
            d.x, d.y, d.z,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
		};

		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
		//glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	
	x=maxx;
	for(z=minz; z<=maxz; z++)
	{
		height = g_hmap.getheight(x, z) + heightoffset;
		a = Vec3f((x)*TILE_SIZE + lineoffset + linehalfwidth, height, (z)*TILE_SIZE + lineoffset);
		b = Vec3f((x)*TILE_SIZE + lineoffset - linehalfwidth, height, (z)*TILE_SIZE + lineoffset);
		c = Vec3f((x)*TILE_SIZE + lineoffset - linehalfwidth, height, (z)*TILE_SIZE);
		d = Vec3f((x)*TILE_SIZE + lineoffset + linehalfwidth, height, (z)*TILE_SIZE);

		float vertices[] =
		{
            a.x, a.y, a.z,
            b.x, b.y, b.z,
            c.x, c.y, c.z,
            
            d.x, d.y, d.z,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
		};

		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
		//glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

// Select tile
void DrawTileSq2()
{
	int x1 = Clip(g_vStart.x/TILE_SIZE, 0, g_hmap.m_widthX-1);
	int z1 = Clip(g_vStart.z/TILE_SIZE, 0, g_hmap.m_widthZ-1);

	int x2 = Clip(g_vTile.x/TILE_SIZE, 0, g_hmap.m_widthX-1);
	int z2 = Clip(g_vTile.z/TILE_SIZE, 0, g_hmap.m_widthZ-1);

	int minx = min(x1, x2);
	int minz = min(z1, z2);
	int maxx = max(x1, x2);
	int maxz = max(z1, z2);
    
	/*
	if(minx == maxx)
		maxx = Clip(maxx+1, 0, g_hmap.m_widthX-1);
	if(minz == maxz)
		maxz = Clip(maxz+1, 0, g_hmap.m_widthZ-1);
		*/

	const float heightoffset = 0.1f;
	const float linehalfwidth = 0.5f;

	Vec3f a, b, c, d;
	int x;
	int z=minz;
	for(x=minx; x<=maxx; x++)
	{
		a = Vec3f((x+1)*TILE_SIZE, g_hmap.getheight(x+1, z) + heightoffset, (z)*TILE_SIZE + linehalfwidth);
		b = Vec3f((x)*TILE_SIZE, g_hmap.getheight(x, z) + heightoffset, (z)*TILE_SIZE + linehalfwidth);
		c = Vec3f((x)*TILE_SIZE, g_hmap.getheight(x, z) + heightoffset, (z)*TILE_SIZE - linehalfwidth);
		d = Vec3f((x+1)*TILE_SIZE, g_hmap.getheight(x+1, z) + heightoffset, (z)*TILE_SIZE - linehalfwidth);

		float vertices[] =
		{
            a.x, a.y, a.z,
            b.x, b.y, b.z,
            c.x, c.y, c.z,
            
            d.x, d.y, d.z,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
		};

		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
		//glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	
	z=maxz+1;
	for(x=minx; x<=maxx; x++)
	{
		a = Vec3f((x+1)*TILE_SIZE, g_hmap.getheight(x+1, z+1) + heightoffset, (z)*TILE_SIZE + linehalfwidth);
		b = Vec3f((x)*TILE_SIZE, g_hmap.getheight(x, z+1) + heightoffset, (z)*TILE_SIZE + linehalfwidth);
		c = Vec3f((x)*TILE_SIZE, g_hmap.getheight(x, z+1) + heightoffset, (z)*TILE_SIZE - linehalfwidth);
		d = Vec3f((x+1)*TILE_SIZE, g_hmap.getheight(x+1, z+1) + heightoffset, (z)*TILE_SIZE - linehalfwidth);

		float vertices[] =
		{
            a.x, a.y, a.z,
            b.x, b.y, b.z,
            c.x, c.y, c.z,
            
            d.x, d.y, d.z,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
		};

		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
		//glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	x=minx;
	for(z=minz; z<=maxz; z++)
	{
		a = Vec3f((x)*TILE_SIZE + linehalfwidth, g_hmap.getheight(x, z+1) + heightoffset, (z+1)*TILE_SIZE);
		b = Vec3f((x)*TILE_SIZE - linehalfwidth, g_hmap.getheight(x, z+1) + heightoffset, (z+1)*TILE_SIZE);
		c = Vec3f((x)*TILE_SIZE - linehalfwidth, g_hmap.getheight(x, z) + heightoffset, (z)*TILE_SIZE);
		d = Vec3f((x)*TILE_SIZE + linehalfwidth, g_hmap.getheight(x, z) + heightoffset, (z)*TILE_SIZE);

		float vertices[] =
		{
            a.x, a.y, a.z,
            b.x, b.y, b.z,
            c.x, c.y, c.z,
            
            d.x, d.y, d.z,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
		};

		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
		//glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	
	x=maxx+1;
	for(z=minz; z<=maxz; z++)
	{
		a = Vec3f((x)*TILE_SIZE + linehalfwidth, g_hmap.getheight(x+1, z+1) + heightoffset, (z+1)*TILE_SIZE);
		b = Vec3f((x)*TILE_SIZE - linehalfwidth, g_hmap.getheight(x+1, z+1) + heightoffset, (z+1)*TILE_SIZE);
		c = Vec3f((x)*TILE_SIZE - linehalfwidth, g_hmap.getheight(x+1, z) + heightoffset, (z)*TILE_SIZE);
		d = Vec3f((x)*TILE_SIZE + linehalfwidth, g_hmap.getheight(x+1, z) + heightoffset, (z)*TILE_SIZE);

		float vertices[] =
		{
            a.x, a.y, a.z,
            b.x, b.y, b.z,
            c.x, c.y, c.z,
            
            d.x, d.y, d.z,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
		};

		glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
		//glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthX + x) * 3 * 2 + 0 ]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

void TerrainTexCoord(float x, float z)
{
	// Terrain texture
	float u = x / (float)(g_hmap.m_widthX*TILE_SIZE);
	float v = z / (float)(g_hmap.m_widthZ*TILE_SIZE);

	// Detail texture
	float u2 = x / (float)TILE_SIZE;
	float v2 = z / (float)TILE_SIZE;
	
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u2, v2);
}

void UndergroundTexCoord(float x, float z)
{
	// Underground texture
	float u = x;
	float v = z;

	// Detail texture
	float u2 = x * g_hmap.m_widthX;
	float v2 = z * g_hmap.m_widthZ;
	
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u2, v2);
}