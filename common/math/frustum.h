



#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "main.h"
#include "3dmath.h"

class CFrustum 
{
public:
	void CalculateFrustum(const float* proj, const float* modl);
	bool PointInFrustum(float x, float y, float z);
	bool SphereInFrustum(float x, float y, float z, float radius);
	bool CubeInFrustum(float x, float y, float z, float size);
	bool BoxInFrustum(float x, float y, float z, float sizeX, float sizeY, float sizeZ);

private:
	float m_Frustum[6][4];	// This holds the A B C and D values for each side of our frustum.
};

class CDebug 
{
public:
	void AddDebugLine(Vec3f vPoint1, Vec3f vPoint2);
	void AddDebugBox(Vec3f vCenter, float width, float height, float depth);
	void RenderDebugLines();		
	void Clear();					

private:
	vector<Vec3f> m_vLines;		
};

extern CFrustum g_frustum;

#endif
