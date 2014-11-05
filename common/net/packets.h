

#ifndef PACKETS_H
#define PACKETS_H

#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../sim/player.h"
#include "../math/vec2i.h"
#include "../math/camera.h"

class OldPacket
{
public:
	char* buffer;
	int len;
	long long last;
	long long first;
#ifdef MATCHMAKER
	IPaddress addr;
#endif

	void freemem()
	{
		if(buffer != NULL)
			delete [] buffer;
		buffer = NULL;
	}

	OldPacket()
	{
		buffer = NULL;
	}
	~OldPacket()
	{
		freemem();
	}

	OldPacket(const OldPacket& original)
	{
		*this = original;
	}

	OldPacket& operator=(const OldPacket &original)
	{
		if(original.buffer && original.len > 0)
		{
			len = original.len;
			buffer = new char[len];
			memcpy((void*)buffer, (void*)original.buffer, len);
			last = original.last;
			first = original.first;
#ifdef MATCHMAKER
			addr = original.addr;
			//memcpy((void*)&addr, (void*)&original.addr, sizeof(struct sockaddr_in));
#endif
		}
		else
		{
			buffer = NULL;
			len = 0;
		}

		return *this;
	}
};

#define	PACKET_NULL						0
#define PACKET_DISCONNECT				1
#define PACKET_CONNECT					2
#define	PACKET_ACKNOWLEDGMENT			3

struct PacketHeader
{
	int type;
	unsigned short ack;
};

struct DisconnectPacket
{
	PacketHeader header;
};

struct ConnectPacket
{
	PacketHeader header;
};

struct AcknowledgmentPacket
{
	PacketHeader header;
};

#endif



