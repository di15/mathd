

#ifndef SELECTION_H
#define SELECTION_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec3f.h"

class Selection
{
public:
	list<int> units;
	list<int> buildings;
	list<Vec2i> roads;
	list<Vec2i> powerlines;
	list<Vec2i> crudepipes;

	void clear();
};

extern Selection g_selection;
extern unsigned int g_circle;

Selection DoSelection(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir);
void DrawSelectionCircles(Matrix* projection, Matrix* modelmat, Matrix* viewmat);
void DrawMarquee();

#endif