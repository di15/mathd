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

	std::list<Vec2i> path;
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
	//std::vector<TransportJob> bids;

	float frame[2];

	Vec2i subgoal;

	unsigned char mode;
	int pathdelay;
	long long lastpath;

	bool threadwait;

	std::list<Demand*> demands;

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

#define UMODE_NONE					0
#define UMODE_GOINGTOBLJOB			1
#define UMODE_BLJOB					2
#define UMODE_GOINGTOCSTJOB			3	//going to construction job
#define UMODE_CSTJOB				4
#define UMODE_GOINGTOCDJOB			5	//going to conduit job
#define UMODE_CDJOB					6
#define UMODE_GOINGTOSHOP			7
#define UMODE_SHOPPING				8
#define UMODE_GOINGTOREST			9
#define UMODE_RESTING				10
#define	UMODE_GOINGTOTRANSP			11	//going to transport job
#define UMODE_TRANSPDRV				12	//driving transport
#define UMODE_GOINGTOSUPPLIER		13	//transporter going to supplier
#define UMODE_GOINGTODEMANDERB		14	//transporter going to demander bl
#define UMODE_GOINGTOREFUEL			15
#define UMODE_REFUELING				16
#define UMODE_ATDEMANDERB			17
#define UMODE_ATSUPPLIER			18
#define UMODE_GOINGTODEMCD			19	//going to demander conduit
#define UMODE_ATDEMCD				20	//at demander conduit

void DrawUnits();
bool PlaceUnit(int type, Vec3i cmpos, int owner);
void FreeUnits();
void UpdUnits();

#endif
