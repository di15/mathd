
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

extern unsigned int g_tiletexs[TILE_TYPES];
extern Vec2i g_mapview[2];

class Heightmap
{
public:
	/*
	Number of tiles, not heightpoints/corners.
	Number of height points/corners is +1.
	*/
	int m_widthx;
	int m_widthz;

	float *m_heightpoints;
	Vec3f *m_vertices;
	Vec2f *m_texcoords0;
	Vec3f *m_normals;
	int *m_countryowner;

	Heightmap()
	{
		m_widthx = 0;
		m_widthz = 0;
	}

	void allocate(int wx, int wz);
	void remesh();
	void draw();
	float getheight(int x, int z);
	void changeheight(int x, int z, float change);
	void setheight(int x, int z, float height);
	void destroy();
	int &getcountry(int x, int z);

	~Heightmap()
	{
		destroy();
	}
};

extern Heightmap g_hmap;

#endif