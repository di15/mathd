

#ifndef BORDER_H
#define BORDER_H

#include "../math/vec3f.h"

class Borders
{
public:
	float *colorcodes;
	Vec3f *drawtris;

	Borders();
	~Borders();
	void destroy();
	void alloc(int widthx, int width);
};

extern Borders g_borders;

void DefineBorders(int minx, int minz, int maxx, int maxz);
void DrawBorders();

#endif