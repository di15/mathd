

#include "readpackets.h"
#include "sendpackets.h"
#include "packets.h"
#include "net.h"
#include "netconn.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../utils.h"

/*
What this function does is take a range of packet ack's (acknowledgment number for reliable UDP transmission)
and executes that range of buffered received packets. This is needed because packets might arrive out of order,
be missing some in between, and I execute them only after a whole range up to the latest ack has been received.

The out-of-order packets are stored in the g_recv vector.

Notice that there is preprocessor check if we are compiling this for the master server MATCHMAKER (because I'm
making this for a persistent, online world) or client. If server, there's extra parameters to match the packets
to the right client; we're only interested in processing the packet range for a certain client.

Each packet goes to the PacketSwitch function, that is like a switch-table that executes the right
packet-execution function based on the packet type ID. The switch-table could probably be turned into
an array of function pointers to improve performance, probably only slightly.

The function takes a time of log(O) to execute, because it has to search through all the buffered packets
several times to execute them in the right order. And before that, there's a check to see if we even have
the whole range of packets from the last "recvack" before calling this function.

I keep a "sendack" and "recvack" for each client, for sent packets and received packets. I only update the
recvack up to the latest one once a continuous range has been received, with no missing packets. Recvack
is thus the last executed received packet.
*/


#ifdef MATCHMAKER
void ParseRecieved(unsigned int first, unsigned int last, NetConn* nc, struct sockaddr_in addr, Client* c)
#else
void ParseRecieved(unsigned int first, unsigned int last, NetConn* nc)
#endif
{
	OldPacket* p;
	PacketHeader* header;
	unsigned int current = first;
	unsigned int afterlast = NextAck(last);

	do
	{
		for(auto i=g_recv.begin(); i!=g_recv.end(); i++)
		{
			p = &*i;
			header = (PacketHeader*)&p->buffer;

			if(header->ack != current)
				continue;

			if(memcmp((void*)&p->addr, (void*)&nc->addr, sizeof(IPaddress)) != 0)
				continue;

#ifdef MATCHMAKER
			PacketSwitch(header->type, p->buffer, p->len, addr, c);
#else
			PacketSwitch(header->type, p->buffer, p->len, nc, &p->addr, &g_svsock, true);
#endif

			p->freemem();
			i = g_recv.erase(i);
			current = NextAck(current);
			break;
		}
	} while(current != afterlast);
}

#ifdef MATCHMAKER
bool Recieved(unsigned int first, unsigned int last, NetConn* nc, struct sockaddr_in addr)
#else
bool Recieved(unsigned int first, unsigned int last, NetConn* nc)
#endif
{
	OldPacket* p;
	PacketHeader* header;
	unsigned int current = first;
	unsigned int afterlast = NextAck(last);
	bool missed;

	do
	{
		missed = true;
		for(auto i=g_recv.begin(); i!=g_recv.end(); i++)
		{
			p = &*i;
			header = (PacketHeader*)&p->buffer;

			if(header->ack != current)
				continue;
			
			if(memcmp((void*)&p->addr, (void*)&nc->addr, sizeof(IPaddress)) != 0)
				continue;

			current = NextAck(current);
			missed = false;
			break;
		}

		if(missed)
			return false;
	} while(current != afterlast);

	return true;
}

#ifdef MATCHMAKER
void AddRecieved(char* buffer, int len, NetConn* nc, struct sockaddr_in addr)
#else
void AddRecieved(char* buffer, int len, NetConn* nc)
#endif
{
	OldPacket p;
	p.buffer = new char[ len ];
	p.len = len;
	memcpy((void*)p.buffer, (void*)buffer, len);	
	memcpy((void*)&p.addr, (void*)&nc->addr, sizeof(IPaddress));

	g_recv.push_back(p);
}

#ifdef MATCHMAKER
void TranslatePacket(char* buffer, int bytes, NetConn* nc, struct sockaddr_in from, bool checkprev, NetConn* nc, UDPsocket* sock)
#else
void TranslatePacket(char* buffer, int bytes, bool checkprev, NetConn* nc, UDPsocket* sock, IPaddress* from)
#endif
{
	PacketHeader* header = (PacketHeader*)buffer;

#ifdef MATCHMAKER

	PacketHeader* phtemp = (PacketHeader*)buffer;

	g_log<<"tp t"<<phtemp->type<<endl;
	g_log.flush();

	int client = MatchClient(from);
	Client* c = NULL;
	if(client >= 0)
	{
		c = &g_client[client];
		c->m_last = GetTickCount64();
	}
#else
	g_lastR = GetTickCount64();

	g_log<<"ack"<<header->ack<<" type"<<header->type<<std::endl;
	g_log.flush();
#endif

	bool bindaddr;

#ifdef MATCHMAKER
	bindaddr = true;
#else
	switch(header->type)
	{
	case PACKET_CONNECT:
	case PACKET_DISCONNECT:
		bindaddr = true;
		break;
	default:
		bindaddr = false;
		break;
	}
#endif

	switch(header->type)
	{
#ifdef MATCHMAKER
	case PACKET_ACKNOWLEDGMENT:
	case PACKET_REGISTRATION:
	case PACKET_LOGIN:
	{
		checkprev = false;
		break;
	}
#else
	case PACKET_ACKNOWLEDGMENT:
	case PACKET_CONNECT:
	case PACKET_DISCONNECT:
	{
		checkprev = false;
		break;
	}
#endif
	default:
		break;
	}

#ifndef MATCHMAKER
	//if(g_loadbytes > 0)
	{
		//char msg[128];
		//sprintf(msg, DateTime().c_str());
		//MessageBlock(msg, true);
	}
#endif

#ifdef MATCHMAKER
	if(checkprev && c != NULL)
#else
	if(checkprev)
#endif
	{
#ifdef MATCHMAKER
		if(PastAck(header->ack, c->m_recvack) || Recieved(header->ack, header->ack, from))
#else
		if(PastAck(header->ack, nc->recvack) || Recieved(header->ack, header->ack, nc))
#endif
		{
#ifdef MATCHMAKER
			Acknowledge(header->ack, nc, from);
#else
			Acknowledge(header->ack, nc, from, sock, bindaddr);
#endif
			//g_log<<"ack "<<header->ack<<endl;
			return;
		}

#ifdef MATCHMAKER
		unsigned int next = NextAck(c->m_recvack);
#else
		unsigned int next = NextAck(nc->recvack);
#endif

		if(header->ack == next) {}  // Translate packet
		else  // More than +1 after recvack?
		{
			unsigned int last = PrevAck(header->ack);
#ifdef MATCHMAKER
			if(Recieved(next, last, from))
				ParseRecieved(next, last, from, c);  // Translate in order
#else
			if(Recieved(next, last, nc))
				ParseRecieved(next, last, nc);  // Translate in order
#endif
			else
			{
#ifdef MATCHMAKER
				AddRecieved(buffer, bytes, nc, from);
#else
				AddRecieved(buffer, bytes, nc);
#endif
				return;
			}
		}
	}

#ifdef MATCHMAKER
	PacketSwitch(header->type, buffer, bytes, from, c);
#else
	PacketSwitch(header->type, buffer, bytes, nc, from, sock, bindaddr);
#endif

	if(header->type != PACKET_ACKNOWLEDGMENT)
	{
#ifdef MATCHMAKER
		if(c == NULL)
		{
			client = MatchClient(from);
			if(client >= 0)
				c = &g_client[client];
		}

		if(c != NULL)
			c->m_recvack = header->ack;
		Acknowledge(header->ack, from);
#else
		if(!nc)
		{
			NetConn newnc;
			newnc.addr = *from;
			newnc.connected = true;
			newnc.sendack = 0;
			g_conn.push_back(newnc);
			nc = &*g_conn.rbegin();
		}

		nc->recvack = header->ack;
		Acknowledge(header->ack, nc, from, sock, bindaddr);
#endif
	}
}

#ifdef MATCHMAKER
void PacketSwitch(int type, char* buffer, int bytes, struct sockaddr_in from, Client* c)
#else
void PacketSwitch(int type, char* buffer, int bytes, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr)
#endif
{
	switch(type)
	{
#ifdef MATCHMAKER
	case PACKET_LOGIN:
		ReadLoginPacket((LoginPacket*)buffer, from, c);
		break;
	case PACKET_REGISTRATION:
		ReadRegistrationPacket((RegistrationPacket*)buffer, from, c);
		break;
	case PACKET_ACKNOWLEDGMENT:
		ReadAcknowledgmentPacket((AcknowledgmentPacket*)buffer, from, c);
		break;
#else
	case PACKET_ACKNOWLEDGMENT:
		ReadAcknowledgmentPacket((AcknowledgmentPacket*)buffer, nc, from, sock, bindaddr);
		break;
	case PACKET_CONNECT:
		ReadConnectPacket((ConnectPacket*)buffer, nc, from, sock, bindaddr);
		break;
	case PACKET_DISCONNECT:
		ReadDisconnectPacket((DisconnectPacket*)buffer, nc, from, sock, bindaddr);
		break;
		/*
		case PACKET_CONNECTION_RESET:  ReadConnectionResetPacket((ConnectionResetPacket*)buffer);  break;
		case PACKET_DISCONNECT:        ReadDisconnectPacket((DisconnectPacket*)buffer);            break;
		*/
#endif
	default:
		break;
	}
}

#ifdef MATCHMAKER
void ReadAcknowledgmentPacket(AcknowledgmentPacket* ap, struct sockaddr_in from, Client* c)
#else
void ReadAcknowledgmentPacket(AcknowledgmentPacket* ap, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr)
#endif
{
	OldPacket* p;
	PacketHeader* header;

	for(auto i=g_sent.begin(); i!=g_sent.end(); i++)
	{
		p = &*i;
		header = (PacketHeader*)p->buffer;
#ifdef MATCHMAKER
		if(header->ack == ap->header.ack && memcmp((void*)&p->addr, (void*)&from, sizeof(struct sockaddr_in)) == 0)
#else
		if(header->ack == ap->header.ack && memcmp((void*)&p->addr, (void*)from, sizeof(IPaddress)) == 0)
#endif
		{
			p->freemem();
			i = g_sent.erase(i);
#ifdef MATCHMAKER
			g_log<<"left to ack "<<g_sent.size()<<endl;
			g_log.flush();
#endif
			return;
		}
	}
}

void ReadDisconnectPacket(DisconnectPacket* dp, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr)
{
	char msg[128];
	sprintf(msg, "dis %u:%u", from->host, (unsigned int)from->port);
	InfoMessage("d", msg);
}

void ReadConnectPacket(ConnectPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr)
{
	char msg[128];
	sprintf(msg, "con %u:%u", from->host, (unsigned int)from->port);
	InfoMessage("d", msg);
}
