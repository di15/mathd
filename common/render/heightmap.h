
#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2f.h"
#include "../texture.h"

#define TILE_SIZE			1000
#define TILE_Y_SCALE		20000.0f
#define TILE_Y_POWER		1.7f
#define TILE_Y_AFTERPOW		(TILE_Y_SCALE/pow(TILE_Y_SCALE/2.0f, TILE_Y_POWER))

#define TILE_Y_SCALE_2		(TILE_Y_SCALE / 2000.0f)
#define ELEV_SANDONLYMAXY	(10 * TILE_Y_SCALE_2)
#define ELEV_SANDGRASSMAXY	(100 * TILE_Y_SCALE_2)
#define ELEV_GRASSONLYMAXY	(600 * TILE_Y_SCALE_2)
#define ELEV_GRASSROCKMAXY	(990 * TILE_Y_SCALE_2)
#define MAPMINZ				(0)
#define MAPMAXZ				(m_widthz*TILE_SIZE)

#define MAX_MAP				256

class Vec2f;
class Vec3f;
class Shader;
class Matrix;
class Plane3f;

#define TILE_SAND				0
#define TILE_GRASS				1
#define TILE_SNOW				2
#define TILE_ROCK				3
#define TILE_ROCK_NORM			4
#define TILE_CRACKEDROCK		5
#define TILE_CRACKEDROCK_NORM	6
#define TILE_PRERENDER			7
#define TILE_TYPES				8

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
	Vec3f *m_drawvertices;
	Vec3f *m_collverts;
	Vec2f *m_texcoords0;
	Vec3f *m_normals;
	int *m_countryowner;
	bool *m_triconfig;
	Plane3f *m_tridivider;
	float m_tilescale;
	LoadedTex m_fulltex;

	Heightmap()
	{
		m_widthx = 0;
		m_widthz = 0;
		m_fulltex.data = NULL;
	}

	void allocate(int wx, int wz);
	void remesh(float tilescale);
	void draw();
	void draw2();
	void draw3();
	void draw4();
	void prerender();
	float getheight(int tx, int tz);
	float accheight(int x, int z);
	float accheight2(int x, int z);
	void changeheight(int x, int z, float change);
	void setheight(int x, int z, float height);
	void destroy();
	int &getcountry(int x, int z);
	Vec3f *getdrawtileverts(int x, int z);
	Vec3f *getcolltileverts(int x, int z);
	Vec3f getnormal(int x, int z);
	void hidetile(int x, int z);
	void unhidetile(int x, int z);

	~Heightmap()
	{
		destroy();
	}
};

extern Heightmap g_hmap;
extern Heightmap g_hmap2;
extern Heightmap g_hmap4;
extern Heightmap g_hmap8;

void AllocGrid(int wx, int wz);
void FreeGrid();

#endif