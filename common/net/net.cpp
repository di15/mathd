
#include "../platform.h"
#include "net.h"
#include "netconn.h"
#include "readpackets.h"
#include "packets.h"
#include "../utils.h"

#ifndef MATCHMAKER
long long g_lastS;  //last sent
long long g_lastR;  //last recieved
int g_netmode = NET_SINGLE;
#endif

unsigned short NextAck(unsigned short ack)
{
	//if(ack == UINT_MAX)
	//	ack = 0;
	//else
	//	ack ++;

	ack++;

	return ack;
}

unsigned short PrevAck(unsigned short ack)
{
	//if(ack == 0)
	//	ack = UINT_MAX;
	//else
	//	ack --;

	ack--;

	return ack;
}

bool PastAck(unsigned short test, unsigned short current)
{
	return ((current >= test) && (current - test <= USHRT_MAX/2))
	       || ((test > current) && (test - current > USHRT_MAX/2));
}


#if 0

void InitNet()
{

#ifdef MATCHMAKER
	g_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if(g_socket < 0)
		g_error<<"Error creating socket errno="<<errno<<endl;

	// set SO_REUSEADDR on a socket to true (1):
	int optval = 1;
	setsockopt(g_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	g_sockaddr.sin_family = AF_INET;
	g_sockaddr.sin_addr.s_addr = INADDR_ANY;
	g_sockaddr.sin_port = htons(PORT);

	int length = sizeof(struct sockaddr_in);

	if(bind(g_socket, (struct addr *)&g_sockaddr, length) < 0)
		g_error<<"Error binding socket errno="<<errno<<endl;

	int flags = fcntl(g_socket, F_GETFL);
	flags |= O_NONBLOCK;

	fcntl(g_socket, F_SETFL, flags);

	//g_log<<"Net inited."<<endl;
#else
#ifdef _WIN

	WSAData data;
	WSAStartup(MAKEWORD(2, 2), &data);

	g_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in clientaddr;
	int socklen = sizeof(struct addr);

	memset((char*)&clientaddr, 0, sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = INADDR_ANY;
	clientaddr.sin_port = htons((u_short)PORT);

	struct hostent* hp = gethostbyname(SV_ADDR);
	g_sockaddr.sin_family = AF_INET;
	memcpy(&(g_sockaddr.sin_addr.s_addr), hp->h_addr, hp->h_length);
	//g_matchAd.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	g_sockaddr.sin_port = htons(PORT);

	/*
	g_svAd.sin_family = AF_INET;
	g_svAd.sin_addr.s_addr = htonl(INADDR_ANY);
	g_svAd.sin_port = htons(PORT);
	bind(g_socket, &g_svAd, sizeof(g_svAd));
	*/

	//g_socket = socket(PF_INET, SOCK_DGRAM, 0);

	if(g_socket < 0)
	{
		g_log<<"socket creation failed"<<endl;
		return;
	}

	if(bind(g_socket, (LPSOCKADDR)&clientaddr, socklen)<0)
	{
		g_log<<"bind failed"<<endl;
		return;
	}

	unsigned long int nonBlockingMode = 1;
	int result = ioctlsocket(g_socket, FIONBIO, &nonBlockingMode);

	if(result)
	{
		g_log<<"ioctlsocket failed"<<endl;
		return;
	}

#elif defined(_IOS)
	InitNetIOS();
#endif
#endif
}

void DeinitNet()
{
#ifdef MATCHMAKER
	close(g_socket);
#else
#ifdef _WIN
	WSACleanup();
#elif defined(_IOS)

#endif
#endif
}

#endif

//Net input
void NetIn()
{
	//struct sockaddr_in from;
	//socklen_t fromlen = sizeof(struct sockaddr_in);
	char buffer[1024];
	int bytes;

	UDPpacket *in;

	in = SDLNet_AllocPacket(65535);

	//IPaddress* addr = &ns->addr;

	//unsigned int svipaddr = SDL_SwapBE32(addr->host);
	//unsigned short svport = SDL_SwapBE16(addr->port);

	UDPsocket* sock = &g_svsock;

	do
	{
		in->data[0] = 0;
		//bytes = recvfrom(g_socket, buffer, 1024, 0, (struct addr *)&from, &fromlen);
		bytes = SDLNet_UDP_Recv(*sock, in);

		IPaddress ip;

		memcpy(&ip, &in->address, sizeof(IPaddress));
		//const char* host = SDLNet_ResolveIP(&ip);
		unsigned int ipaddr = SDL_SwapBE32(ip.host);
		unsigned short port = SDL_SwapBE16(ip.port);

		if(bytes > 0)
		{
#ifdef MATCHMAKER
			TranslatePacket(buffer, bytes, from, true);
#else
			//if(memcmp((void*)&from, (void*)&g_sockaddr, sizeof(struct sockaddr_in)) != 0)
			//if(ipaddr != svipaddr)
			//	continue;
			
			//g_log<<"r t"<<((PacketHeader*)in->data)->type<<std::endl;
			//g_log.flush();
			
			//for(int i=0; i<bytes; i++)
			//for(int i=0; i<4; i++)
			//{
			//	g_log<<"r"<<i<<": "<<(unsigned int)(in->data[i])<<std::endl;
			//	g_log.flush();
			//}

			//TranslatePacket(buffer, bytes, true);
			TranslatePacket((char*)in->data, bytes, true, Match(&ip), &g_svsock, &ip);
#endif
		}
	} while(bytes > 0);

	SDLNet_FreePacket(in);
}

#if 0

void ClearPackets()
{
	for(auto i=g_sent.begin(); i!=g_sent.end(); i++)
		i->freemem();

	for(auto i=g_recv.begin(); i!=g_recv.end(); i++)
		i->freemem();
}

#endif