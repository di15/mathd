#ifndef ISOMATH_H
#define ISOMATH_H

#include "vec3i.h"
#include "vec2i.h"

//#define TILE_PIXEL_WIDTH	256
#define TILE_PIXEL_WIDTH	128
#define TILE_PIXEL_RISE		(TILE_PIXEL_WIDTH / 8)

Vec2i CartToIso(Vec3i cmpos);
int CartYToIso(int cmy);

#endif
