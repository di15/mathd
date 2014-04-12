

#include "rendst.h"
#include "../platform.h"

Unit::RenderState g_urendst[2][UNITS];
Building::RenderState g_blrendst[2][BUILDINGS];
PowlTile::RenderState** g_powlrendst[2] = {NULL, NULL};
CrPipeTile::RenderState** g_crpiperendst[2] = {NULL, NULL};
RoadTile::RenderState** g_roadrendst[2] = {NULL, NULL};

