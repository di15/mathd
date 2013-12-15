


#ifndef BRUSHSIDE_H
#define BRUSHSIDE_H

#include "vec3f.h"
#include "plane.h"
#include "polygon.h"
#include "triangle.h"
#include "../draw/model.h"
#include "../draw/vertexarray.h"

class BrushSide
{
public:
	Plane m_plane;
	VertexArray m_drawva;
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	
	BrushSide(const BrushSide& original);
	BrushSide(){}
	virtual ~BrushSide(){}
	virtual void usetex();
};

//tex coord equation
class TexCEq
{
	float m_rot;	//degrees
	float m_scale[2];	//default 0.1, world to tex coordinates
	float m_offset[2];	//in world distances
};

class EdBrushSide : public BrushSide
{
public:
	int m_ntris;
	Triangle2* m_tris;
	Plane m_tceq[2];	//tex coord uv equations
	Polyg m_outline;
	int* m_vindices;	//indices into parent brush's shared vertex array; only stores unique vertices as defined by polygon outline
	Vec3f m_centroid;
	
	EdBrushSide(const EdBrushSide& original);
	EdBrushSide& operator=(const EdBrushSide &original);
	EdBrushSide();
	EdBrushSide(Vec3f normal, Vec3f point);
	~EdBrushSide();
	void makeva();
	//void usetex();
	void gentexeq();	//fit texture to face
	void fittex();
	void remaptex();
};

Vec3f PlaneCrossAxis(Vec3f normal);

#endif