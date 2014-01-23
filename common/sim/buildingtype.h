

#include "../math/vec3f.h"
#include "../math/vec2i.h"

class BuildingT
{
public:
	int model;
	int widthx;
	int widthz;
	
	BuildingT();
};

#define BUILDING_NONE			-1
#define BUILDING_APARTMENT		0
#define BUILDING_CEMENTPLANT	1
#define BUILDING_CHEMPLANT		2
#define BUILDING_COALPOW		3
#define BUILDING_ELECPLANT		4
#define BUILDING_FACTORY		5
#define BUILDING_FARM			6
#define BUILDING_GASSTATION		7
#define BUILDING_MINE			8
#define BUILDING_NUCPOW			9
#define BUILDING_OILREFINERY	10
#define BUILDING_OILWELL		11
#define BUILDING_RESEARCHFAC	12
#define BUILDING_SHOPMALL		13
#define BUILDING_STEELMILL		14
#define BUILDING_STONEQUARRY	15
#define BUILDING_URANENRICHPL	16

#define BUILDING_TYPES			17

#define BUILDING_ROAD			(BUILDING_TYPES+1)
#define BUILDING_POWERL			(BUILDING_TYPES+2)
#define BUILDING_OILPIPEL		(BUILDING_TYPES+3)

extern BuildingT g_buildingT[BUILDING_TYPES];

void DefineBuildingType(int type, const char* modelrelative, Vec3f scale, Vec3f translate);