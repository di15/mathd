
#ifndef TRANSPORTATION_H
#define TRANSPORTATION_H

#include <vector>
#include "3dmath.h"

using namespace std;

#define TBID_DELAY	(3000*FRAME_RATE/1000)

class TransportJob
{
public:
	int targtype;
	int target;
	int target2;
	vector<Vec3f> path;
	float pathlen;
	int restype;
	float resamt;
	float fuelcost;
	float driverwagepaid;
	float netprofit;	// truck profit
	int transporter;
	float totalcosttoclient;	// transport fee + resource cost
	float clientcharge;	// transport fee
	int supplier;

	TransportJob()
	{
		pathlen = 0;
		totalcosttoclient = 0;
		clientcharge = 0;
		driverwagepaid = 0;
		netprofit = 0;
		fuelcost = 0;
		resamt = 0;
	}
};

class ResSource
{
public:
	int supplier;
	float amt;
};

extern vector<int> g_freetrucks;

void ManageTrips();

#endif