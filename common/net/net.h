

#ifndef NET_H
#define NET_H

#include "../platform.h"
#include "packets.h"

#define PORT		50400
//#define SV_ADDR		"corpstates.com"	//live server
//#define SV_ADDR		"54.221.229.124"	//corp1 aws
//#define SV_ADDR			"192.168.1.100"		//home local server ip
#define SV_ADDR			"192.168.1.103"		//home local server ip
//#define SV_ADDR			"174.6.61.178"		//home public server ip

unsigned short NextAck(unsigned short ack);
unsigned short PrevAck(unsigned short ack);
bool PastAck(unsigned short test, unsigned short current);

#ifndef MATCHMAKER
extern long long g_lastS;  //last sent
extern long long g_lastR;  //last recieved

extern int g_netmode;

#define NET_SINGLE			0	//single player
#define NET_HOST			1	//hosting
#define NET_CLIENT			2	//client
#endif

void InitNet();
void DeinitNet();
void NetIn();
void ClearPackets();

#endif

