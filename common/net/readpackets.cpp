

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

#if 0

#ifdef MATCHMAKER
void ParseRecieved(unsigned int first, unsigned int last, struct sockaddr_in addr, Client* c)
#else
void ParseRecieved(unsigned int first, unsigned int last)
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

#ifdef MATCHMAKER
			if(memcmp((void*)&p->addr, (void*)&addr, sizeof(struct sockaddr_in)) != 0)
				continue;

			PacketSwitch(header->type, p->buffer, p->len, addr, c);
#else
			PacketSwitch(header->type, p->buffer, p->len);
#endif

			p->freemem();
			i = g_recv.erase(i);
			current = NextAck(current);
			break;
		}
	} while(current != afterlast);
}

#ifdef MATCHMAKER
bool Recieved(unsigned int first, unsigned int last, struct sockaddr_in addr)
#else
bool Recieved(unsigned int first, unsigned int last)
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

#ifdef MATCHMAKER
			if(memcmp((void*)&p->addr, (void*)&addr, sizeof(struct sockaddr_in)) != 0)
				continue;
#endif

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
void AddRecieved(char* buffer, int len, struct sockaddr_in addr)
#else
void AddRecieved(char* buffer, int len)
#endif
{
	OldPacket p;
	p.buffer = new char[ len ];
	p.len = len;
	memcpy((void*)p.buffer, (void*)buffer, len);
#ifdef MATCHMAKER
	memcpy((void*)&p.addr, (void*)&addr, sizeof(struct sockaddr_in));
#endif
	g_recv.push_back(p);
}

#ifdef MATCHMAKER
void TranslatePacket(char* buffer, int bytes, struct sockaddr_in from, bool checkprev)
#else
void TranslatePacket(char* buffer, int bytes, bool checkprev)
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

	//g_log<<"ack"<<header->ack<<" type"<<header->type<<std::endl;
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
		if(PastAck(header->ack, g_recvack) || Recieved(header->ack, header->ack))
#endif
		{
#ifdef MATCHMAKER
			Acknowledge(header->ack, from);
#else
			Acknowledge(header->ack);
#endif
			//g_log<<"ack "<<header->ack<<endl;
			return;
		}

#ifdef MATCHMAKER
		unsigned int next = NextAck(c->m_recvack);
#else
		unsigned int next = NextAck(g_recvack);
#endif

		if(header->ack == next) {}  // Translate packet
		else  // More than +1 after recvack?
		{
			unsigned int last = PrevAck(header->ack);
#ifdef MATCHMAKER
			if(Recieved(next, last, from))
				ParseRecieved(next, last, from, c);  // Translate in order
#else
			if(Recieved(next, last))
				ParseRecieved(next, last);  // Translate in order
#endif
			else
			{
#ifdef MATCHMAKER
				AddRecieved(buffer, bytes, from);
#else
				AddRecieved(buffer, bytes);
#endif
				return;
			}
		}
	}

#ifdef MATCHMAKER
	PacketSwitch(header->type, buffer, bytes, from, c);
#else
	PacketSwitch(header->type, buffer, bytes);
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
		g_recvack = header->ack;
		Acknowledge(header->ack);
#endif
	}
}

#ifdef MATCHMAKER
void PacketSwitch(int type, char* buffer, int bytes, struct sockaddr_in from, Client* c)
#else
void PacketSwitch(int type, char* buffer, int bytes)
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
		ReadAcknowledgmentPacket((AcknowledgmentPacket*)buffer);
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
void ReadAcknowledgmentPacket(AcknowledgmentPacket* ap)
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
		if(header->ack == ap->header.ack)
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

void ReadDisconnectPacket(DisconnectPacket* dp)
{
	
}

void ReadConnectPacket(ConnectPacket* dp)
{
	
}

#endif	//0