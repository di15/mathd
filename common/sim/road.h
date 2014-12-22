#ifndef ROAD_H
#define ROAD_H

#include "connectable.h"
#include "resources.h"
#include "../render/vertexarray.h"
#include "../render/model.h"
#include "../math/vec3i.h"
#include "conduit.h"

class RoadTile : public CdTile
{
public:
	unsigned char cdtype();
	void fillcollider();
	void freecollider();
};

#define ROAD_MAX_FOREW_INCLINE		(TILE_SIZE)
#define ROAD_MAX_SIDEW_INCLINE		(TILE_SIZE/2)

#endif

