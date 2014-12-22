#ifndef PLAYER_H
#define PLAYER_H

#include "../platform.h"
#include "../net/netconn.h"
#include "resources.h"
#include "../gui/richtext.h"

#ifndef MATCHMAKER
#include "../gui/gui.h"
#include "../math/camera.h"
#include "selection.h"
#include "../../libs/objectscript/objectscript.h"
#endif

struct PlayerColor
{
	unsigned char color[3];
	char name[32];
};

#define PLAYER_COLORS	48

extern PlayerColor g_pycols[PLAYER_COLORS];

class Player
{
public:
	bool on;
	bool ai;

	int local[RESOURCES];	// used just for counting; cannot be used
	int global[RESOURCES];
	int resch[RESOURCES];	//resource changes/deltas
	int truckwage;	//truck driver wage per second
	int transpcost;	//transport cost per second

	float color[4];
	RichText name;
	int client;	//for server
	unsigned int curnetfr;

#ifndef MATCHMAKER
	Player();
	~Player();
#endif
};

//#define PLAYERS 32
#define PLAYERS ARRSZ(g_pycols)
//#define PLAYERS	6	//small number of AI players so it doesn't freeze (as much)

extern Player g_player[PLAYERS];
extern int g_localP;
extern int g_playerm;
extern bool g_diplomacy[PLAYERS][PLAYERS];

void DefP(int ID, float red, float green, float blue, float alpha, RichText name);
void DrawPy();
void Bankrupt(int player, const char* reason);

#endif
