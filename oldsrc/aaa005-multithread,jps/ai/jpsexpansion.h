
#ifndef JPSEXPANSION_H
#define JPSEXPANSION_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../sys/workthread.h"
#include "../platform.h"

class Unit;
class Building;
class PathNode;
class PathJob;

extern Vec2i straightoffsets[4];
extern Vec2i diagonaloffsets[4];

void IdentifySuccessors(PathJob* pj, PathNode* node);

#endif