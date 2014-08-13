#ifndef AI_H
#define AI_H

class Player;
class Building;

void UpdateAI();
void UpdateAI(Player* p);
bool AdjPr(Player* p);
bool AdjPr(Building* b);

#endif
