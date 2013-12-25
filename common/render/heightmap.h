
#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include "../math/vec3f.h"
#include "../math/vec2f.h"

//#define MAP_SIZE		32
#define TILE_SIZE		16

#define TILE_0001	0
#define TILE_0010	1
#define TILE_0011	2
#define TILE_0100	3
#define TILE_0101	4
#define TILE_0110	5
#define TILE_0111	6
#define TILE_1000	7
#define TILE_1001	8
#define TILE_1010	9
#define TILE_1011	10
#define TILE_1100	11
#define TILE_1101	12
#define TILE_1110	13
#define TILE_1111	14
#define TILETYPE_IMAGES	15
#define TILE_0000	0

class CTileType
{
public:
	bool on;
	char m_name[32];
	unsigned int m_diffuse[TILETYPE_IMAGES];
	unsigned int m_specular[TILETYPE_IMAGES];
	unsigned int m_normal[TILETYPE_IMAGES];

	void reloadTextures();
};

#define TILETYPES	128
extern CTileType g_tiletype[TILETYPES];

class Shader;

class Heightmap
{
public:
	int m_widthX;
	int m_widthZ;

	float* m_heightpoints;
	Vec3f* m_vertices;
	Vec2f* m_texcoords0;
	Vec3f* m_normals;
	int* m_tiletype;
	unsigned int* m_diffuse;
	unsigned int* m_specular;
	unsigned int* m_normalmap;

	Heightmap()
	{
		m_widthX = 0;
		m_widthZ = 0;
		
		/*
		m_texcoords0[0] = Vec2f(0, 0);
		m_texcoords0[1] = Vec2f(1, 0);
		m_texcoords0[2] = Vec2f(1, 1);
		m_texcoords0[3] = Vec2f(0, 1);
		m_texcoords0[4] = Vec2f(0, 0);
		m_texcoords0[5] = Vec2f(1, 1);
		*/
		/*
		m_texcoords0[0] = Vec2f(0.01f, 0.01f);
		m_texcoords0[1] = Vec2f(1, 0.01f);
		m_texcoords0[2] = Vec2f(1, 1);
		m_texcoords0[3] = Vec2f(0.01f, 1);
		m_texcoords0[4] = Vec2f(0.01f, 0.01f);
		m_texcoords0[5] = Vec2f(1, 1);*/
	}

	void allocate(int wx, int wz);
	void remesh();
	void retexture();
	void draw();
	inline float getheight(int x, int z);
	void changeheight(int x, int z, float change);
	void setheight(int x, int z, float height);
	void settile(int x, int z, int type);

	void free();

	~Heightmap()
	{
		free();
	}
};

extern Heightmap g_hmap;

extern Vec3f g_vMouse;
extern Vec3f g_vMouseStart;
extern Vec3f g_vTile;
extern Vec3f g_vStart;
extern int g_surroundings;
extern unsigned int g_circle;
extern unsigned int g_sky;
extern unsigned int g_water;
extern unsigned int g_transparency;
//extern int g_hmap;

bool GetMapIntersection2(Vec3f vLine[], Vec3f* vIntersection);
bool GetMapIntersection(Vec3f vLine[], Vec3f* vIntersection);
bool GetMapIntersectionD(Vec3f vLine[], Vec3f* vIntersection);

int FindTileset(const char* name);
int TileConn(bool a, bool b, bool c, bool d);
void LoadTiles();
void LoadMap();
void MinMaxCorners(int& minx, int& maxx, int& minz, int& maxz);
void StartEndCorners(int& x1, int& x2, int& z1, int& z2);
float Bilerp(float x, float y);
float MapMeshHeight(float x, float z);
void DrawTileSq();
void InitSurroundings();
void DrawSurroundings();
void LoadTerrainTextures();

#endif