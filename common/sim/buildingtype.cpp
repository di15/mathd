

#include "../render/model.h"
#include "buildingtype.h"

BuildingT g_buildingT[BUILDING_TYPES];

BuildingT::BuildingT()
{
}

void DefineBuildingType(int type, const char* modelrelative, Vec3f scale, Vec3f translate)
{
	BuildingT* pbtype = &g_buildingT[type];
	QueueModel(&pbtype->model, modelrelative, scale, translate);
}