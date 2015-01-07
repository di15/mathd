#ifndef TILE_H
#define TILE_H

#include "sprite.h"
#include "../phys/collidable.h"
#include "../math/vec2i.h"

class TlType
{
public:
	Sprite sprite;
};

#define TILE2PERM_0000		0
#define TILE2PERM_0001		1
#define TILE2PERM_0010		2
#define TILE2PERM_0011		3
#define TILE2PERM_0100		4
#define TILE2PERM_0101		5
#define TILE2PERM_0110		6
#define TILE2PERM_0111		7
#define TILE2PERM_1000		8
#define TILE2PERM_1001		9
#define TILE2PERM_1010		10
#define TILE2PERM_1011		11
#define TILE2PERM_1100		12
#define TILE2PERM_1101		13
#define TILE2PERM_1110		14
#define TILE2PERMS		15

extern TlType g_tiletype[TILE2PERMS];

class Tile : public Collidable
{
public:
	int tiletype;
	Vec3i tilepos;
};

void DefTl(int tiletype, const char* texpath, Vec2i spriteoffset, Vec2i spritesz);
Tile &SurfTile(int tx, int tz);

#endif
