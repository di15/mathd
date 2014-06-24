
#include "player.h"
#include "resources.h"
#include "../render/model.h"
#include "../render/heightmap.h"
#include "../sim/build.h"
#include "../sim/building.h"
#include "../gui/cursor.h"

Player g_player[PLAYERS];
int g_localP = 0;
int g_curP = g_localP;
int g_playerm;

Player::Player()
{
	Zero(local);
	Zero(global);

	global[RES_MINERALS] = 10000;
	//global[RES_LABOUR] = 10000;

	on = false;
	zoom = INI_ZOOM;
	width = INI_WIDTH;
	height = INI_HEIGHT;
	bpp = INI_BPP;
	keyintercepted = false;
	mouseout = false;
	moved = false;
	canplace = false;
	bpcol = -1;
	build = BUILDING_NONE;
	bptype = -1;
	bpyaw = 0;
	mouseoveraction = false;
	curst = CU_DEFAULT;
	kbfocus = 0;
}

Player::~Player()
{
}

void DefP(int ID, float red, float green, float blue, float alpha, RichText name)
{
	Player* py = &g_player[ID];
	py->colorcode[0] = red;
	py->colorcode[1] = green;
	py->colorcode[2] = blue;
	py->colorcode[3] = alpha;
	py->name = name;
}

void DrawPy()
{
	Model* m = &g_model[g_playerm];

	m->draw(0, Vec3f(TILE_SIZE*10, TILE_SIZE*5, TILE_SIZE*10), 30);
}
