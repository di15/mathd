#ifndef ANYPATH_H
#define ANYPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../platform.h"

class Unit;
class Building;
class PathJob;
class PathNode;

bool AnyPath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype, int cdtype,
                 Unit* thisu, Unit* ignoreu, Building* ignoreb,
                 int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
				 int nminx, int nminy, int nmaxx, int nmaxy);

void Expand_AP(PathJob* pj, PathNode* node);

#endif
