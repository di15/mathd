

#ifndef TRUCK_H
#define TRUCK_H

class Unit;

void UpdTruck(Unit* u);

#define TRSND_NEWJOB	0
#define TRSND_DONEJOB	1
#define TRSND_WORK		2
#define TR_SOUNDS		3

extern short g_trsnd[TR_SOUNDS];

#endif