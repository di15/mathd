

#ifndef CLIENT_H
#define CLIENT_H

#include "../gui/richtext.h"
#include "../sim/player.h"
#include "netconn.h"

//a client is like a player, but concerns networking.
//a client must be a human player.
//a client controls a player slot.
class Client
{
public:
	bool on;
	int player;
	RichText name;
	//unsigned char color;
	NetConn* nc;
};

#define CLIENTS	PLAYERS

extern Client g_client[CLIENTS];
extern int g_localC;
extern RichText g_name;

void ResetClients();
bool AddClient(NetConn* nc, RichText name, int* retci);

#endif