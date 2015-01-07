#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2f.h"
#include "../math/vec2uc.h"
#include "../texture.h"
#include "vertexarray.h"
#include "tile.h"

#define TILE_SIZE			1000

//#define MAX_MAP			128
#define MAX_MAP				64

class Vec2f;
class Vec3f;
class Shader;
class Matrix;
class Plane3f;

/*
Number of tiles, not heightpoints/corners.
Number of height points/corners is +1.
*/
extern Vec2uc g_mapsz;
extern unsigned char* g_hmap;

#endif
