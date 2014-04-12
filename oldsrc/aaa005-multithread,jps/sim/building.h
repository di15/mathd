

#ifndef BUILDING_H
#define BUILDING_H

#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "../platform.h"
#include "resources.h"

class Building
{
public:
	bool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;
	
	Vec2i tilepos;	//position in tiles
	Vec3f drawpos;	//drawing position in centimeters

	bool finished;

	int pownetw;
	int crpipenetw;
	list<int> roadnetw;

	int stocked[RESOURCES];
	int inuse[RESOURCES];

	void destroy();
	void fillcollider();
	void freecollider();
	Building();
	~Building();
};

#define BUILDINGS	256

extern Building g_building[BUILDINGS];

void FreeBuildings();
bool PlaceBuilding(int type, Vec2i pos, bool finished, int stateowner, int corpowner, int unitowner);
void DrawBuildings();
bool CheckCanPlace(int type, Vec2i pos);

#endif