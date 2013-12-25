

#include "main.h"

#define BUILDSTUFF_DELAY	(5*FRAME_RATE)
#define BUILDUNITS_DELAY	(5*FRAME_RATE)

#define SUPERIORITY_RATIO	1.1f	// AI will go to war if its calculated force is this much more powerful
#define MINIMUM_SUPERIORITY	((1000.0f * 1.0f / 100 + 100)*3)	//minimum force to go to war (3 battle comps)

#define MINIMUM_RESERVE	1.00f

class Vec3f;

void UpdateAI();
void AdjustPQuotas(int player);
bool PlaceRCentral(int player, int btype);
bool PlaceNearB(int player, int btype);
bool AIPlotRoad(int owner, int startBui, int endBui);
//bool AIPlotRoad(int owner, int startBstartx, int startBendx, int startBstartz, int startBendz, int endBstartx, int endBendx, int endBstartz, int endBendz, Vec3f pos, Vec3f goal);
bool AIPlotRoad(int owner, int startBstartx, int startBendx, int startBstartz, int startBendz,  Vec3f pos, int endBui);
void BuildPow(int player);
void BuildPipe(int player);