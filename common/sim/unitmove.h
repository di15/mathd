#ifndef UNITMOVE_H
#define UNITMOVE_H

class Unit;

void MoveUnit(Unit* u);
bool UnitCollides(Unit* u, Vec2i cmpos, int utype);
bool CheckIfArrived(Unit* u);
void OnArrived(Unit* u);

#endif
