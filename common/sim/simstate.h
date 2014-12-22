

#ifndef SIMSTATE_H
#define SIMSTATE_H

#include "unit.h"

//Everything inside this class must:
//1. Change completely deterministically
//2. Be synchronized between clients

class SimState
{
public:
	unsigned int simframe;
	Unit unit[UNITS];
	//TO DO
};

#endif