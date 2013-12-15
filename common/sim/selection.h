
#ifndef SELECTION_H
#define SELECTION_H

#include "3dmath.h"
#include <vector>
#include "unit.h"

using namespace std;

#define SELECT_UNIT			0
#define SELECT_BUILDING		1
#define SELECT_SCENERY		2
#define SELECT_ROAD			-2
#define SELECT_POWERLINE	-3
#define SELECT_PIPELINE		-4

extern vector<int> g_selection;
extern vector<POINT> g_selection2;
extern int g_selectType;
extern Vec3f g_normalLeft;
extern Vec3f g_normalTop;
extern Vec3f g_normalRight;
extern Vec3f g_normalBottom;
extern float g_distLeft;
extern float g_distTop;
extern float g_distRight;
extern float g_distBottom;
extern void (*SelectionFunc)();

bool PointInsidePlanes(Vec3f p);
void DrawSelection(Matrix projection, Matrix viewmat, Matrix modelmat);
void SelectOne();
void Selection();
void RecheckSelection();
bool UnitSelected(CUnit* u);

#endif