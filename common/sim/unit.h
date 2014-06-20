
#ifndef UNIT_H
#define UNIT_H

#include "../math/camera.h"
#include "../platform.h"
#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "resources.h"

#define BODY_LOWER	0
#define BODY_UPPER	1

class Demand;

class Unit
{
public:
	bool on;
	int type;
	int owner;

	/*
	The draw (floating-point) position vectory is used for drawing.
	*/
	Vec3f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec2i cmpos;
	Vec3f facing;
	Vec2f rotation;

	list<Vec2i> path;
	Vec2i goal;

	int target;
	int target2;
	bool targetu;
	bool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	bool passive;
	Vec2i prevpos;
	int taskframe;
	bool pathblocked;
	int frameslookjobago;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//vector<TransportJob> bids;

	float frame[2];

	Vec2i subgoal;

	unsigned char mode;
	int pathdelay;
	long long lastpath;
	
	bool threadwait;

	list<Demand*> demands;

	// used for debugging - don't save to file
	bool collided;

	Unit();
	virtual ~Unit();
	void destroy();
	void fillcollider();
	void freecollider();
	void resetpath();
	bool hidden() const;
};

#define UNITS	(4096)

extern Unit g_unit[UNITS];

void DrawUnits();
bool PlaceUnit(int type, Vec3i cmpos, int owner);
void FreeUnits();
void UpdateUnits();

#endif