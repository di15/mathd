#ifndef AI_H
#define AI_H

class Player;
class Building;

void UpdAI();
void UpdAI(Player* p);
bool AdjPr(Player* p);
bool AdjPr(Building* b);

#endif
