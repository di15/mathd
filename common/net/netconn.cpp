

#include "../platform.h"
#include "netconn.h"

UDPsocket g_svsock;
UDPsocket g_svsock2;	//for server list
std::list<NetConn> g_conn;
std::list<OldPacket> g_sent;
std::list<OldPacket> g_recv;