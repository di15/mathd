


#include "net.h"
#include "sendpackets.h"
#include "../sim/player.h"
#include "packets.h"
#include "../sim/simdef.h"
#include "lockstep.h"

void SendAll(char* data, int size, bool reliable, IPaddress* exception)
{
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
	{
		if(ci->ctype != CONN_CLIENT)
			continue;

		if(exception && memcmp(&ci->addr, exception, sizeof(IPaddress)) == 0)
			continue;

		//InfoMessage("sa", "sa");

		SendData(data, size, &ci->addr, reliable, &*ci, &g_sock, true);
	}
}

void SendData(char* data, int size, IPaddress * paddr, bool reliable, NetConn* nc, UDPsocket* sock, bool bindaddr)
{
	UDPpacket *out = SDLNet_AllocPacket(65535);

	g_lastS = GetTickCount64();

#if 0
	if(((PacketHeader*)data)->type == PACKET_NETTURN)
	{
		char msg[128];
		sprintf(msg, "send netturn for netfr%u load=%u", ((NetTurnPacket*)data)->fornetfr, (unsigned int)((NetTurnPacket*)data)->loadsz);
		InfoMessage("s", msg);
		//g_log<<msg<<std::endl;
		//FILE* fp = fopen("sntp.txt", "wb");
		//fwrite(msg, strlen(msg)+1, 1, fp);
		//fclose(fp);
	}
#endif

	if(reliable)
	{
		//InfoMessage("sa", "s2");
		((PacketHeader*)data)->ack = nc->sendack;
		OldPacket p;
		p.buffer = new char[ size ];
		p.len = size;
		memcpy(p.buffer, data, size);
		memcpy((void*)&p.addr, (void*)paddr, sizeof(IPaddress));
		p.last = GetTickCount64();
		p.first = p.last;
		g_sent.push_back(p);
		nc->sendack = NextAck(nc->sendack);
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
			char msg[1280];
			sprintf(msg, "SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
			ErrorMessage("Error", msg);
			//printf("SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
			//exit(7);
		}
	}

#if 0
	g_log<<"send t"<<((PacketHeader*)data)->type<<std::endl;
	g_log.flush();

			char msg[128];
			sprintf(msg, "send ack%u t%d", (unsigned int)((PacketHeader*)out->data)->ack, ((PacketHeader*)out->data)->type);
			InfoMessage("send", msg);
#endif

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
			//i->freemem();
			//i = g_sent.erase(i);

#if 0
			g_log<<"expire at "<<DateTime()<<" dt="<<expire<<"-"<<p->first<<"="<<(expire-p->first)<<"(>"<<RESEND_EXPIRE<<") left = "<<g_sent.size()<<endl;
			g_log.flush();
#endif

			//continue;
		}

		SendData(p->buffer, p->len, &p->addr, false, Match(&p->addr), &g_sock, true);

		p->last = now;
#ifdef _IOS
		NSLog(@"Resent at %lld", now);
#endif

#if 0
		g_log<<"resent at "<<DateTime()<<" left = "<<g_sent.size()<<endl;
		g_log.flush();
#endif

		i++;
	}
}

void Acknowledge(unsigned short ack, NetConn* nc, IPaddress* addr, UDPsocket* sock, bool bindaddr, char* buffer, int bytes)
{
#if 0	//actually, don't mess with RUDP protocol; build it on top of packets
	//if it's a NetTurnPacket, we have to do something special...
	if(((PacketHeader*)buffer)->type == PACKET_NETTURN)
	{
		NetTurnPacket* ntp = (NetTurnPacket*)buffer;
		
		//if it's not for the next turn, drop the ack so the server waits for us to complete up to the necessary turn
		if(ntp->fornetfr != (g_netframe / NETTURN + 1) * NETTURN)
		{
			char msg[128];
			sprintf(msg, "faulty turn fr%u", ntp->fornetfr);
			InfoMessage("Er", msg);
			return;
		}
	}
#endif

	AcknowledgmentPacket p;
	p.header.type = PACKET_ACKNOWLEDGMENT;
	p.header.ack = ack;

	SendData((char*)&p, sizeof(AcknowledgmentPacket), addr, false, nc, sock, bindaddr);
}




