


#include "net.h"
#include "sendpackets.h"
#include "../sim/player.h"
#include "packets.h"

#ifdef MATCHMAKER
void SendData(char* data, int size, IPaddress * paddr, bool reliable, NetConn* nc, UDPsocket* sock, bool bindaddr, Client* c)
#else
void SendData(char* data, int size, IPaddress * paddr, bool reliable, NetConn* nc, UDPsocket* sock, bool bindaddr)
#endif
{
	UDPpacket *out = SDLNet_AllocPacket(65535);

#ifndef MATCHMAKER
	g_lastS = GetTickCount64();
#endif

	if(reliable)
	{
#ifdef MATCHMAKER
		((PacketHeader*)data)->ack = c->m_sendack;
#else
		((PacketHeader*)data)->ack = nc->sendack;
#endif
		OldPacket p;
		p.buffer = new char[ size ];
		p.len = size;
		memcpy(p.buffer, data, size);
#ifdef MATCHMAKER
		memcpy((void*)&p.addr, (void*)paddr, sizeof(struct sockaddr_in));
#endif
		p.last = GetTickCount64();
		p.first = p.last;
		g_sent.push_back(p);
#ifdef MATCHMAKER
		c->m_sendack = NextAck(c->m_sendack);
#else
		nc->sendack = NextAck(nc->sendack);
#endif
	}

	memcpy(out->data, data, size);
	out->len = size;
	//out->data[size] = 0;

#ifdef MATCHMAKER
	sendto(g_socket, data, size, 0, (struct addr *)paddr, sizeof(struct sockaddr_in));
#elif defined( _IOS )
	// Address is NULL and the data is automatically sent to g_hostAddr by virtue of the fact that the socket is connected to that address
	ssize_t bytes = sendto(sock, data, size, 0, NULL, 0);

	int err;
	int sock;
	socklen_t addrLen;
	sock = CFSocketGetNative(g_cfSocket);

	if(bytes < 0)
		err = errno;
	else if(bytes == 0)
		err = EPIPE;
	else
		err = 0;

	if (err != 0)
		NetError([NSError errorWithDomain:NSPOSIXErrorDomain code:err userInfo:nil]);
#else

	//if(bindaddr)
	{
		SDLNet_UDP_Unbind(*sock, 0);
		if(SDLNet_UDP_Bind(*sock, 0, (const IPaddress*)paddr) == -1)
		{
			//printf("SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
			//exit(7);
		}
	}

	//g_log<<"send t"<<((PacketHeader*)data)->type<<std::endl;
	//g_log.flush();

	//sendto(g_socket, data, size, 0, (struct addr *)paddr, sizeof(struct sockaddr_in));
	SDLNet_UDP_Send(*sock, 0, out);
#endif
	SDLNet_FreePacket(out);
}

void ResendPackets()
{
	OldPacket* p;
	long long now = GetTickCount64();
	long long due = now - RESEND_DELAY;
	long long expire = now - RESEND_EXPIRE;

	auto i=g_sent.begin();
	while(i!=g_sent.end())
	{
		p = &*i;
		if(p->last > due)
			continue;

		if(p->first < expire)
		{
			i = g_sent.erase(i);

#ifdef MATCHMAKER
			g_log<<"expire at "<<DateTime()<<" dt="<<expire<<"-"<<p->first<<"="<<(expire-p->first)<<"(>"<<RESEND_EXPIRE<<") left = "<<g_sent.size()<<endl;
			g_log.flush();
#endif

			continue;
		}

#ifdef MATCHMAKER
		SendData(p->buffer, p->len, &p->addr, false, NULL);
#else
		SendData(p->buffer, p->len, &p->addr, false, Match(&p->addr), &g_svsock, true);
#endif
		p->last = now;
#ifdef _IOS
		NSLog(@"Resent at %lld", now);
#endif

#ifdef MATCHMAKER
		g_log<<"resent at "<<DateTime()<<" left = "<<g_sent.size()<<endl;
		g_log.flush();
#endif

		i++;
	}
}

#ifdef MATCHMAKER
void Acknowledge(unsigned short ack, NetConn* nc, IPaddress* addr, UDPsocket* sock, bool bindaddr)
#else
void Acknowledge(unsigned short ack, NetConn* nc, IPaddress* addr, UDPsocket* sock, bool bindaddr)
#endif
{
	AcknowledgmentPacket p;
	p.header.type = PACKET_ACKNOWLEDGMENT;
	p.header.ack = ack;

#ifdef MATCHMAKER
	SendData((char*)&p, sizeof(AcknowledgmentPacket), addr, false, nc, sock, bindaddr);
#else
	SendData((char*)&p, sizeof(AcknowledgmentPacket), addr, false, nc, sock, bindaddr);
#endif
}




