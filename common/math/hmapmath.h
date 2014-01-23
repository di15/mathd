
#ifndef HMAPMATH_H
#define HMAPMATH_H

#include "../math/vec3f.h"

class Heightmap;

float Bilerp(Heightmap* hmap, float x, float z);
bool GetMapIntersection2(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection);
bool GetMapIntersection(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection);
bool FastMapIntersect(Heightmap* hmap, Vec3f line[2], Vec3f* intersection);

#endif