

#ifndef BUILD_H
#define BUILD_H

#include "../math/vec3f.h"
#include "../math/vec2i.h"

class Building;
class BuildingT;
class Matrix;

void DrawSBuild();
void UpdateSBuild();
bool CheckCanPlace(int type, Vec2i pos);
void DrawBReason(Matrix* mvp, float width, float height, bool persp);

#endif