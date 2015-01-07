

#ifndef JOB_H
#define JOB_H

#include "labourer.h"

class Unit;
class Building;

//job opportunity
class JobOpp
{
public:
	int jobutil;
	int jobtype;
	int target;
	int target2;
	//float bestDistWage = -1;
	//float distWage;
	//bool fullquota;
	int ctype;	//conduit type
	Vec2i goal;
	int targtype;
	Unit* ignoreu;
	Building* ignoreb;
};

bool FindJob(Unit* u);
void NewJob(int jobtype, int target, int target2, int cdtype);


#endif