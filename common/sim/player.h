

#ifndef PLAYER_H
#define PLAYER_H

#include "country.h"
#include "company.h"

class Player
{
public:
	bool ai;
	signed char client;
};

#define PLAYERS (COUNTRIES+COMPANIES)

extern Player g_player[PLAYERS];

#endif