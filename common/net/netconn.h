

#ifndef NETCONN_H
#define NETCONN_H

#include "../platform.h"
#include "net.h"
#include "packets.h"

extern UDPsocket g_sock;
extern UDPsocket g_svsock2;	//for server list

extern std::list<OldPacket> g_sent;
extern std::list<OldPacket> g_recv;

#define CONN_MATCHER		1
#define CONN_HOST			2
#define CONN_CLIENT			3

class NetConn
{
public:
	unsigned short sendack;
	unsigned short recvack;
	bool connected;
	IPaddress addr;
	unsigned char ctype;
	long long lastrecv;

	NetConn()
	{
		connected = false;
		sendack = 0;
		recvack = 0;
		ctype = 0;
		lastrecv = GetTickCount64();
	}
};

extern std::list<NetConn> g_conn;
extern NetConn* g_svconn;
extern NetConn* g_mcconn;	//matchmaker

NetConn* Match(IPaddress* addr);

#endif