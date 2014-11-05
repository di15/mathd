

#ifndef NETCONN_H
#define NETCONN_H

#include "net.h"

class OldPacket;

class NetConn
{
public:
	bool connected;
	unsigned short sendack;
	unsigned short recvack;
	UDPsocket socket;
	std::list<OldPacket> sent;
	std::list<OldPacket> recv;
	IPaddress sockaddr;

	NetConn()
	{
		connected = false;
		sendack = 0;
		recvack = 0;
	}
};

extern NetConn g_svconn;

#endif