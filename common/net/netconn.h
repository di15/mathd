

#ifndef NETCONN_H
#define NETCONN_H

#include "../platform.h"
#include "net.h"
#include "packets.h"

extern UDPsocket g_svsock;
extern UDPsocket g_svsock2;	//for server list

extern std::list<OldPacket> g_sent;
extern std::list<OldPacket> g_recv;

class NetConn
{
public:
	unsigned short sendack;
	unsigned short recvack;
	bool connected;
	IPaddress addr;

	NetConn()
	{
		connected = false;
		sendack = 0;
		recvack = 0;
	}
};

extern std::list<NetConn> g_conn;

#endif