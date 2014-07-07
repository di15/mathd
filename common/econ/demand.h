
#ifndef DEMAND_H
#define DEMAND_H

#include "../math/vec2i.h"
#include "../platform.h"
#include "../sim/resources.h"
#include "../sim/unittype.h"
#include "../sim/buildingtype.h"
#include "../sim/build.h"

extern int g_rdem[RESOURCES];
extern int g_udem[UNIT_TYPES];
extern int g_bdem[BUILDING_TYPES];

void CalcDem();

#endif
