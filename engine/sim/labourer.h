#ifndef LABOURER_H
#define LABOURER_H

#include "unit.h"

#define LABSND_WORK		0
#define LABSND_SHOP		1
#define LABSND_REST		2
#define LAB_SOUNDS		3

extern short g_labsnd[LAB_SOUNDS];


void UpdLab(Unit* u);
void UpdLab2(Unit* u);
void Evict(Unit* u);
void Disembark(Unit* op);

#endif
