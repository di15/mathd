

#ifndef RECONSTRUCTPATH_H
#define RECONSTRUCTPATH_H

#include "../platform.h"
#include "../math/vec2i.h"

class TryStep;

void ReconstructPath(list<Vec2i> &path, TryStep* bestS, Vec2i &subgoal, int cmgoalx, int cmgoalz);
void ReconstructPathJPS(list<Vec2i> &path, TryStep* bestS, Vec2i &subgoal, int cmgoalx, int cmgoalz);

#endif