

#include "../render/model.h"
#include "buildingtype.h"

BuildingT g_buildingT[BUILDING_TYPES];

BuildingT::BuildingT()
{
}

void DefineBuildingType(int type, const char* name, Vec2i size, const char* modelrelative, Vec3f scale, Vec3f translate, const char* cmodelrelative,  Vec3f cscale, Vec3f ctranslate, int foundation, int reqdeposit)
{
	BuildingT* t = &g_buildingT[type];
	t->widthx = size.x;
	t->widthz = size.y;
	sprintf(t->name, name);
	QueueModel(&t->model, modelrelative, scale, translate);
	QueueModel(&t->cmodel, cmodelrelative, cscale, ctranslate);
	t->foundation = foundation;

	Zero(t->input);
	Zero(t->output);
	Zero(t->conmat);

	t->reqdeposit = reqdeposit;
}

void BuildingConMat(int type, int res, int amt)
{
	BuildingT* t = &g_buildingT[type];
	t->conmat[res] = amt; 
}

void BuildingInput(int type, int res, int amt)
{
	BuildingT* t = &g_buildingT[type];
	t->input[res] = amt; 
}

void BuildingOutput(int type, int res, int amt)
{
	BuildingT* t = &g_buildingT[type];
	t->output[res] = amt; 
}