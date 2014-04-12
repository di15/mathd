

#ifndef PARTIALPATH_H
#define PARTIALPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../sys/workthread.h"
#include "../platform.h"

class Unit;
class Building;

bool PartialPath(int wthread, int utype, int umode, int cmstartx, int cmstartz, int target, int targetx, int targetz, 
			  list<Vec2i> &path, Vec2i &subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb, 
			  int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminz, int cmgoalmaxx, int cmgoalmaxz,
			  int maxsearch);

#endif