

#ifndef BRUSH_H
#define BRUSH_H

#include "vec3f.h"
#include "plane.h"
#include "polygon.h"
#include "triangle.h"
#include "brushside.h"

#define STOREY_HEIGHT	250.0f //20.0f

class Brush
{
public:
	int m_nsides;
	BrushSide* m_sides;
	int m_texture;	//used to determine brush attributes
	bool m_broken;
	
	Brush(const Brush& original);
	Brush();
	~Brush();
	Vec3f traceray(Vec3f line[]);
};

class EdBrush
{
public:
	int m_nsides;
	EdBrushSide* m_sides;
	int m_nsharedv;
	Vec3f* m_sharedv;	//shared vertices array
	int m_texture;	//used to determine brush attributes
	
	EdBrush& operator=(const EdBrush& original);
	EdBrush(const EdBrush& original);
	EdBrush();
	~EdBrush();
	void add(EdBrushSide b);
	void removeside(int i);
	void collapse();
	void remaptex();
	Vec3f traceray(Vec3f line[]);
	void prunev(bool* invalidv);
	void moveto(Vec3f newp);
	void move(Vec3f move);
};

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float radius, const float height);
void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float hwx, const float hwz, const float height);
bool LineInterHull(const Vec3f* line, const Vec3f* norms, const float* ds, const int numplanes);

#endif