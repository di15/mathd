

#include "../platform.h"
#include "netconn.h"

UDPsocket g_sock;
UDPsocket g_svsock2;	//for server list
std::list<NetConn> g_conn;
std::list<OldPacket> g_sent;
std::list<OldPacket> g_recv;

NetConn* g_svconn = NULL;
NetConn* g_mcconn = NULL;	//matchmaker

NetConn* Match(IPaddress* addr)
{
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(memcmp((void*)&ci->addr, (void*)addr, sizeof(IPaddress)) == 0)
			return &*ci;

	return NULL;
}
