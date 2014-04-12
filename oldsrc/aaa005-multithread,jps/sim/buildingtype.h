

#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "resources.h"

class BuildingT
{
public:
	// width in tiles
	int widthx;
	int widthz;

	int model;
	int cmodel;

	char name[64];

	int foundation;

	int input[RESOURCES];
	int output[RESOURCES];

	int conmat[RESOURCES];
	
	int reqdeposit;

	BuildingT();
};

#define FOUNDATION_LAND			0
#define FOUNDATION_COASTAL		1
#define FOUNDATION_SEA			2

#define BUILDING_NONE			-1
#define BUILDING_APARTMENT		0
#define BUILDING_HOUSE			1
#define BUILDING_SHOPMALL		2
#define BUILDING_FARM			3
#define BUILDING_GASSTATION		4
#define BUILDING_BANK			5
#define BUILDING_CARDEALERSHIP	6
#define BUILDING_CARFACTORY		7
#define BUILDING_MILITARYBASE	8
#define BUILDING_TRUCKFACTORY	9
#define BUILDING_CONCGARBTRFAC	10
#define BUILDING_TANKERTRFAC	11
#define BUILDING_CURREXCENTER	12
#define BUILDING_COALMINE		13
#define BUILDING_HARBOUR		14
#define BUILDING_LUMBERMILL		15
#define BUILDING_FOREQFACTORY	16
#define BUILDING_CEMENTPLANT	17
#define BUILDING_CHEMPLANT		18
#define BUILDING_COALPOW		19
#define BUILDING_ELECPLANT		20
#define BUILDING_IRONMINE		21
#define BUILDING_URANMINE		22
#define BUILDING_NUCPOW			23
#define BUILDING_OILREFINERY	24
#define BUILDING_OILWELL		25
#define BUILDING_RESEARCHFAC	26
#define BUILDING_STEELMILL		27
#define BUILDING_STONEQUARRY	28
#define BUILDING_URANENRICHPL	29
#define BUILDING_WATERTOWER		30
#define BUILDING_WATERPUMPSTN	31
#define BUILDING_OFFSHOREOILRIG	32

#define BUILDING_TYPES			33

#define BUILDING_ROAD			(BUILDING_TYPES+1)
#define BUILDING_POWERL			(BUILDING_TYPES+2)
#define BUILDING_CRUDEPIPE		(BUILDING_TYPES+3)
#define BUILDING_WATERPIPE		(BUILDING_TYPES+4)

#define TOTAL_BUILDABLES		(BUILDING_TYPES+5)

extern BuildingT g_buildingT[BUILDING_TYPES];

void DefineBuildingType(int type, const char* name, Vec2i size, const char* modelrelative, Vec3f scale, Vec3f translate, const char* cmodelrelative,  Vec3f cscale, Vec3f ctranslate, int foundation, int reqdeposit);
void BuildingConMat(int type, int res, int amt);
void BuildingInput(int type, int res, int amt);
void BuildingOutput(int type, int res, int amt);