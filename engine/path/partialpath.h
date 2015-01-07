#ifndef PARTIALPATH_H
#define PARTIALPATH_H

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

void PartialPath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype, int cdtype,
                 std::list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb,
                 int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
                 int maxsearch, bool capend, bool allowpart);

void Expand_QP(PathJob* pj, PathNode* node);

#endif
