#ifndef ASTARPATH_H
#define ASTARPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../platform.h"

class PathJob;
class PathNode;
class Unit;
class Building;

void AStarPath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype, int cdtype,
               std::list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb,
               int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
               int maxsearch,
			   int nminx, int nminy, int nmaxx, int nmaxy, bool bounded, bool capend);

void Expand_A(PathJob* pj, PathNode* node);

#endif
