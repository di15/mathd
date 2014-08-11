#ifndef AI_H
#define AI_H

class Player;
class Building;

void UpdateAI();
void UpdateAI(Player* p);
void AdjPr(Player* p);
void AdjPr(Building* b);

#endif
