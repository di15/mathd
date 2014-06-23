

#ifndef DEMAND_H
#define DEMAND_H

#include "../math/vec2i.h"
#include "../platform.h"

class Demand
{
public:
	unsigned char type;
	unsigned char subtype;
	unsigned char subtype2;
	bool alt;
	Vec2i cmpos;
	int cmdist;
	int askprice;
	unsigned char askcurry;	//asking currency
};

#define DEMAND_FORRES		0
#define DEMAND_FORUNIT		1

#define HOUSING_NONSPECIFIC			0
#define HOUSING_PRIVATE				1
#define HOUSING_FORRENT				2

extern std::list<Demand> g_demand;

#endif
