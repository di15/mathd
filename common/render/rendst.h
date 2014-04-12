

#ifndef RENDST_H
#define RENDST_H

#include "../sim/unit.h"
#include "../sim/road.h"
#include "../sim/crudepipeline.h"
#include "../sim/powerline.h"
#include "../sim/building.h"

extern Unit::RenderState g_urendst[2][UNITS];
extern Building::RenderState g_blrendst[2][BUILDINGS];
extern PowlTile::RenderState** g_powlrendst[2];
extern CrPipeTile::RenderState** g_crpiperendst[2];
extern RoadTile::RenderState** g_roadrendst[2];

#endif