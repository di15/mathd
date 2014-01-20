
#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2f.h"

#define TILE_SIZE			1000
#define TILE_Y_SCALE		20000.0f
#define TILE_Y_POWER		1.7f
#define TILE_Y_AFTERPOW		(TILE_Y_SCALE/pow(TILE_Y_SCALE/2.0f, TILE_Y_POWER))

class Vec2f;
class Vec3f;
class Shader;
class Matrix;

#define TILE_SAND				0
#define TILE_GRASS				1
#define TILE_SNOW				2
#define TILE_ROCK				3
#define TILE_ROCK_NORM			4
#define TILE_CRACKEDROCK		5
#define TILE_CRACKEDROCK_NORM	6
#define TILE_TYPES				7


#ifndef _SERVER
extern unsigned int g_tiletexs[TILE_TYPES];
extern Vec2i g_mapview[2];
#endif

class Heightmap
{
public:
	int m_widthx;	//number of tiles, not heightpoints/corners
	int m_widthz;	//number of height points/corners is +1

	float* m_heightpoints;
	Vec3f* m_vertices;
	Vec2f* m_texcoords0;
	Vec3f* m_normals;

	Heightmap()
	{
		m_widthx = 0;
		m_widthz = 0;
	}

	void allocate(int wx, int wz);
#ifndef _SERVER
	void remesh();
	void draw();
#endif
	float getheight(int x, int z);
	void changeheight(int x, int z, float change);
	void setheight(int x, int z, float height);

	void destroy();

	~Heightmap()
	{
		destroy();
	}
};

float Bilerp(Heightmap* hmap, float x, float z);
bool GetMapIntersection2(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection);
bool GetMapIntersection(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection);
bool FastMapIntersect(Heightmap* hmap, Vec3f line[2], Vec3f* intersection);

#endif