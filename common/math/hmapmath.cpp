

#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/polygon.h"
#include "../math/3dmath.h"
#include "hmapmath.h"
#include "../render/water.h"
#include "../utils.h"
#include "../phys/collision.h"
#include "../window.h"
#include "../math/camera.h"


float Bilerp(Heightmap* hmap, float x, float z)
{
	x /= (float)TILE_SIZE;
	z /= (float)TILE_SIZE;

	int x1 = (int)(x);
	int x2 = x1 + 1;

	int z1 = (int)(z);
	int z2 = z1 + 1;

	float xdenom = x2-x1;
	float x2fac = (x2-x)/xdenom;
	float x1fac = (x-x1)/xdenom;

	float hR1 = hmap->getheight(x1,z1)*x2fac + hmap->getheight(x2,z1)*x1fac;
	float hR2 = hmap->getheight(x1,z2)*x2fac + hmap->getheight(x2,z2)*x1fac;

	float zdenom = z2-z1;

	return hR1*(z2-z)/zdenom + hR2*(z-z1)/zdenom;
}

bool GetMapIntersection2(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection)
{
	Vec3f vQuad[4];

#if 1
	vQuad[0] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[1] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[2] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);
	vQuad[3] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);
#else
	vQuad[3] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[2] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[1] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);
	vQuad[0] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);
#endif

	if(InterPoly(vQuad, vLine, 4, vIntersection))
		return true;
		
	vQuad[0] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[1] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, TILE_SIZE*500, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[2] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, TILE_SIZE*500, 10*hmap->m_widthz*TILE_SIZE);
	vQuad[3] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);

	if(InterPoly(vQuad, vLine, 4, vIntersection))
		return true;
	
	vQuad[0] = Vec3f(10*hmap->m_widthx*TILE_SIZE, TILE_SIZE*500, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[1] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[2] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);
	vQuad[3] = Vec3f(10*hmap->m_widthx*TILE_SIZE, TILE_SIZE*500, 10*hmap->m_widthz*TILE_SIZE);
	
	if(InterPoly(vQuad, vLine, 4, vIntersection))
		return true;

	vQuad[0] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[1] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, TILE_SIZE*500, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[2] = Vec3f(10*hmap->m_widthx*TILE_SIZE, TILE_SIZE*500, -10*hmap->m_widthz*TILE_SIZE);
	vQuad[3] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, -10*hmap->m_widthz*TILE_SIZE);

	if(InterPoly(vQuad, vLine, 4, vIntersection))
		return true;
	
	vQuad[0] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, TILE_SIZE*500, 10*hmap->m_widthz*TILE_SIZE);
	vQuad[1] = Vec3f(-10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);
	vQuad[2] = Vec3f(10*hmap->m_widthx*TILE_SIZE, WATER_LEVEL*2.0f, 10*hmap->m_widthz*TILE_SIZE);
	vQuad[3] = Vec3f(10*hmap->m_widthx*TILE_SIZE, TILE_SIZE*500, 10*hmap->m_widthz*TILE_SIZE);
	
	if(InterPoly(vQuad, vLine, 4, vIntersection))
		return true;

	return false;
}

bool GetMapIntersection(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection)
{
	Vec3f vTri[3];

	Vec3f* v = hmap->m_collverts;
	int wx = hmap->m_widthx;
	int wz = hmap->m_widthz;

	Vec3f tempint;
	bool intercepted = false;
	float closestint = 0;

	for(int x=0; x<wx; x++)
		for(int z=0; z<wz; z++)
		{
			/*
			vTri[0] = v[ (z * wx + x) * 3 * 2 + 0 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 2 ];

			if(InterPoly(vTri, vLine, 3, vIntersection))
				return true;

			vTri[0] = v[ (z * wx + x) * 3 * 2 + 3 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 5 ];

			if(InterPoly(vTri, vLine, 3, vIntersection))
				return true;
				*/
			
			vTri[0] = v[ (z * wx + x) * 3 * 2 + 0 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 2 ];

			if(InterPoly(vTri, vLine, 3, &tempint))
			{
				if(vIntersection)
				{
					float thisint = Magnitude2(vLine[0] - tempint);
					if(thisint < closestint || !intercepted)
					{
						*vIntersection = tempint;
						closestint = thisint;
					}
				}

				intercepted = true;
			}

			vTri[0] = v[ (z * wx + x) * 3 * 2 + 3 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 5 ];

			if(InterPoly(vTri, vLine, 3, &tempint))
			{
				if(vIntersection)
				{
					float thisint = Magnitude2(vLine[0] - tempint);
					if(thisint < closestint || !intercepted)
					{
						*vIntersection = tempint;
						closestint = thisint;
					}
				}

				intercepted = true;
			}
			

			/*
			vTri[0] = v[ (z * wx + x) * 3 * 2 + 2 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 0 ];

			if(InterPoly(vTri, vLine, 3, vIntersection))
				return true;

			vTri[0] = v[ (z * wx + x) * 3 * 2 + 5 ];
			vTri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
			vTri[2] = v[ (z * wx + x) * 3 * 2 + 3 ];

			if(InterPoly(vTri, vLine, 3, vIntersection))
				return true;*/
		}

	return intercepted;
	//return false;
}

/*
Given a point that is on a ray, move it until it is within a given heightmap's borders.
Returns success (true) or failure (false).
*/
bool MoveIntoMap(Vec3f& point, Vec3f ray, Heightmap* hmap)
{
	// Already within map?
	if(point.x >= 1 
		&& point.x < (hmap->m_widthx-1) * TILE_SIZE - 1
		&& point.z >= 1
		&& point.z < (hmap->m_widthz-1) * TILE_SIZE - 1)
		return true;

	// Get x distance off the map.

	float xdif = 0;

	if(point.x < 0)	// If start x is behind the map
		xdif = point.x - 5;	// Add padding to make sure we're within the map
	else if(point.x >= (hmap->m_widthx-1) * TILE_SIZE)	// If start x is in front of the map
		xdif = (hmap->m_widthx-1) * TILE_SIZE - point.x + 5;	// Add padding to make sure we're within the map

	// Ray is of unit length, so this gives us how much we travel along the ray to get  x to within the map border.
	float x0moveratio = -xdif / ray.x;
	point = point + ray * x0moveratio;
	
	// Get z distance off the map.

	float zdif = 0;

	if(point.z < 0)	// If start z is behind the map
		zdif = point.z - 5;	// Add padding to make sure we're within the map
	else if(point.z >= (hmap->m_widthz-1) * TILE_SIZE)	// If start z is in front of the map
		zdif = (hmap->m_widthz-1) * TILE_SIZE - point.z + 5;	// Add padding to make sure we're within the map
	
	// Ray is of unit length, so this gives us how much we travel along the ray to get z to within the map border.
	float z0moveratio = -zdif / ray.z;
	point = point + ray * z0moveratio;
	
	// If we still couldn't get the point within the map 
	// (maybe the ray is outside the map, beside a corner)
	// then return false.
	if(point.x < 1 
		|| point.x >= (hmap->m_widthx-1) * TILE_SIZE - 1
		|| point.z < 1
		|| point.z >= (hmap->m_widthz-1) * TILE_SIZE - 1)
		return false;

	return true;
}

bool TileIntersect(Heightmap* hmap, Vec3f* line, int x, int z, Vec3f* intersection)
{
	Vec3f tri[3];
	const int wx = hmap->m_widthx;
	const int wz = hmap->m_widthz;
	Vec3f* v = hmap->m_collverts;

	tri[0] = v[ (z * wx + x) * 3 * 2 + 0 ];
	tri[1] = v[ (z * wx + x) * 3 * 2 + 1 ];
	tri[2] = v[ (z * wx + x) * 3 * 2 + 2 ];

	if(InterPoly(tri, line, 3, intersection))
		return true;

	tri[0] = v[ (z * wx + x) * 3 * 2 + 3 ];
	tri[1] = v[ (z * wx + x) * 3 * 2 + 4 ];
	tri[2] = v[ (z * wx + x) * 3 * 2 + 5 ];

	if(InterPoly(tri, line, 3, intersection))
		return true;

	return false;
}

bool FastMapIntersectOld(Heightmap* hmap, Vec3f line[2], Vec3f* intersection)
{
	//return GetMapIntersection(hmap, line, intersection);

	// If both start and end are on one side of the map, we can't get an intersection, so return false.

	if(line[0].x < 0 && line[1].x < 0)
		return false;

	if(line[0].x >= hmap->m_widthx * TILE_SIZE && line[1].x >= hmap->m_widthx * TILE_SIZE)
		return false;

	if(line[0].z < 0 && line[1].z < 0)
		return false;
	
	if(line[0].z >= hmap->m_widthz * TILE_SIZE && line[1].z >= hmap->m_widthz * TILE_SIZE)
		return false;
	
	Vec3f ray = Normalize( line[1] - line[0] );
	float lengthsqrd = Magnitude2( line[1] - line[0] );	//length squared

	// Move the start to within the map if it isn't already.
	if(!MoveIntoMap(line[0], ray, hmap))
		return false;

	// Move the end to within the map if it isn't already.
	if(!MoveIntoMap(line[1], ray, hmap))
		return false;

	float lengthdone = 0;

	Vec3f currpoint = line[0];

	int currtilex = currpoint.x / TILE_SIZE;
	int currtilez = currpoint.z / TILE_SIZE;
	
	// The directions in which we will move to the next tile on the x and z axis
	int tiledx = 0;
	int tiledz = 0;

	if(ray.x > 0)
		tiledx = 1;
	else if(ray.x < 0)
		tiledx = -1;

	if(ray.z > 0)
		tiledz = 1;
	else if(ray.z < 0)
		tiledz = -1;

	if(tiledx == 0 && tiledz == 0)
		return false;
	
	int nexttilex = currtilex + tiledx;
	int nexttilez = currtilez + tiledz;

	// Move from tile to tile along the line until,
	// testing each tile's triangles for intersection.
	while(lengthdone*lengthdone < lengthsqrd)
	{
		float xdif = currtilex * TILE_SIZE - currpoint.x;
		float zdif = currtilez * TILE_SIZE - currpoint.z;
		
		float xmoveratio = xdif / ray.x;
		float zmoveratio = zdif / ray.z;

		// Advance to the next tile margin on the x axis
		while(xmoveratio < 0)
		{
			currtilex = nexttilex;
			nexttilex += tiledx;
			xdif = currtilex * TILE_SIZE - currpoint.x;
			xmoveratio = xdif / ray.x;
		}
		
		// Advance to the next tile margin on the zaxis
		while(zmoveratio < 0)
		{
			currtilez = nexttilez;
			nexttilez += tiledz;
			zdif = currtilez * TILE_SIZE - currpoint.z;
			zmoveratio = zdif / ray.z;
		}

		float moveratio = 0;

		// Move the smallest distance to the next tile
		if(xmoveratio < zmoveratio && xmoveratio > 0)
		{
			moveratio = xmoveratio;
		}
		else if(zmoveratio < xmoveratio && zmoveratio > 0)
		{
			moveratio = zmoveratio;
		}
		else if(xmoveratio == zmoveratio && xmoveratio > 0)
		{
			moveratio = xmoveratio;
		}
		else
		{
			return false;
		}

		//The currpoint's don't exactly follow the original ray/line,
		//so we have to check nearby tiles for intersections.
		int mintilex = currtilex;
		int mintilez = currtilez;
		int maxtilex = currtilex;
		int maxtilez = currtilez;

		//Are we close to the previous tile on the x axis?
		if( max(0, currpoint.x/TILE_SIZE-0.1f) < mintilex)
			mintilex--;

		//Are we close to the previous tile on the z axis?
		if( max(0, currpoint.z/TILE_SIZE-0.1f) < mintilez)
			mintilez--;

		//Are we close to the next tile on the x axis?
		if( min(hmap->m_widthx-1, currpoint.x/TILE_SIZE+0.1f) > maxtilex)
			maxtilex++;

		//Are we close to the next tile on the z axis?
		if( min(hmap->m_widthz-1, currpoint.z/TILE_SIZE+0.1f) > maxtilez)
			maxtilez++;

		for(int itertilex = mintilex; itertilex <= maxtilex; itertilex ++)
			for(int itertilez = mintilez; itertilez <= maxtilez; itertilez ++)
			{
				if(itertilex < 0 || itertilex >= hmap->m_widthx || itertilez < 0 || itertilez >= hmap->m_widthz)
					return false;

				if(TileIntersect(hmap, line, itertilex, itertilez, intersection))
					return true;
			}

		currpoint = currpoint + ray * moveratio; 

		lengthdone += moveratio;

		// Move the smallest distance to the next tile
		if(xmoveratio < zmoveratio && xmoveratio > 0)
		{
			currtilex = nexttilex;
			nexttilex += tiledx;
		}
		else if(zmoveratio < xmoveratio && zmoveratio > 0)
		{
			currtilez = nexttilez;
			nexttilez += tiledz;
		}
		else if(xmoveratio == zmoveratio)
		{
			currtilex = nexttilex;
			currtilez = nexttilez;
			nexttilex += tiledx;
			nexttilez += tiledz;
		}
	}

	return false;
}


bool FastMapIntersect(Heightmap* hmap, Vec3f* line, Vec3f* intersection)
{
	Vec3f ray = line[1] - line[0];
	Vec3f tilestart = line[0] / TILE_SIZE;
	Vec3f tileray = ray / TILE_SIZE;

	int len = Magnitude(tileray);

	for(int i=0; i<len; i++)
	{
		Vec3f pos = tilestart + tileray * (float)i / (float)len;
		int tx = pos.x;
		int tz = pos.z;

		if(tx < 0)
			continue;

		if(tz < 0)
			continue;

		if(tx >= hmap->m_widthx)
			continue;

		if(tz >= hmap->m_widthz)
			continue;
		
		if(TileIntersect(hmap, line, tx, tz, intersection))
			return true;
	}

	return false;
}

float Highest(int minx, int minz, int maxx, int maxz)
{
	float highest = 0;

	for(int x=minx; x<=maxx; x++)
		for(int z=minz; z<=maxz; z++)
		{
#if 0
			g_log<<"x,z"<<x<<","<<z<<endl;
			g_log.flush();
#endif

			if( g_hmap.getheight(x, z) > highest || (x==minx && z==minz) )
				highest = g_hmap.getheight(x, z);
		}

	return highest;
}

float Lowest(int minx, int minz, int maxx, int maxz)
{
	float lowest = 0;

	for(int x=minx; x<=maxx; x++)
		for(int z=minz; z<=maxz; z++)
		{
#if 0
			g_log<<"x,z"<<x<<","<<z<<endl;
			g_log.flush();
#endif

			if( g_hmap.getheight(x, z) < lowest || (x==minx && z==minz) )
				lowest = g_hmap.getheight(x, z);
		}

	if(lowest < WATER_LEVEL)
		lowest = WATER_LEVEL;

	return lowest;
}

bool TileUnclimablei(int tx, int tz)
{
	float h0 = g_hmap.getheight(tx, tz);
	float h1 = g_hmap.getheight(tx+1, tz);
	float h2 = g_hmap.getheight(tx, tz+1);
	float h3 = g_hmap.getheight(tx+1, tz+1);
    
	float minh = min(h0, min(h1, min(h2, h3)));
	float maxh = max(h0, max(h1, max(h2, h3)));
    
	if(fabs(maxh - minh) > MAX_CLIMB_INCLINE)
	{
#if 0
		g_log<<tx<<","<<tz<<" ("<<g_hmap.m_widthx<<","<<g_hmap.m_widthz<<" incline "<<fabs(maxh - minh)<<endl;
		g_log.flush();
#endif
#if 0
		g_collidertype = COLLIDER_TERRAIN;
#endif
		return true;
	}
    
	return false;
}

bool TileUnclimable(float px, float pz)
{
	int tx = px / TILE_SIZE;
	int tz = pz / TILE_SIZE;
    
	return TileUnclimablei(tx, tz);
}

bool AnyWateri(int tx, int tz)
{
	if(g_hmap.getheight(tx, tz) <= WATER_LEVEL)
		return true;
    
	if(g_hmap.getheight(tx+1, tz) <= WATER_LEVEL)
		return true;
    
	if(g_hmap.getheight(tx, tz+1) <= WATER_LEVEL)
		return true;
    
	if(g_hmap.getheight(tx+1, tz+1) <= WATER_LEVEL)
		return true;
    
	return false;
}

bool AnyLandi(int tx, int tz)
{
	if(g_hmap.getheight(tx, tz) > WATER_LEVEL)
		return true;
    
	if(g_hmap.getheight(tx+1, tz) > WATER_LEVEL)
		return true;
    
	if(g_hmap.getheight(tx, tz+1) > WATER_LEVEL)
		return true;
    
	if(g_hmap.getheight(tx+1, tz+1) > WATER_LEVEL)
		return true;
    
	return false;
}

bool AnyWater(int cmx, int cmz)
{
	int tx = cmx / TILE_SIZE;
	int tz = cmz / TILE_SIZE;
    
	return AnyWateri(tx, tz);
}

bool AtWater(int cmx, int cmz)
{
#if 0
	const float h = Bilerp(&g_hmap, cmx, cmz);
#else
	const float h = g_hmap.accheight(cmx, cmz);
#endif
    
	if(h <= WATER_LEVEL)
	{
#if 0
		g_collidertype = COLLIDER_TERRAIN;
#endif
		return true;
	}
    
	return false;
}

bool AtLand(int cmx, int cmz)
{
#if 0
	const float h = Bilerp(&g_hmap, cmx, cmz);
#else
	const float h = g_hmap.accheight(cmx, cmz);
#endif
    
	if(h > WATER_LEVEL)
	{
#if 0
		g_collidertype = COLLIDER_TERRAIN;
#endif
		return true;
	}
    
	return false;
}