

#ifndef DEMAND_H
#define DEMAND_H

#include "../math/vec2i.h"

class Demand
{
	unsigned char type;
	Vec2i cmpos;
	bool isalt;
};

#define DEMAND_FORRES		0
#define DEMAND_FORUNIT		1

#endif