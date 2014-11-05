
#ifndef READPACKETS_H
#define READPACKETS_H

#include "../platform.h"

#ifdef MATCHMAKER
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "packets.h"

#ifdef MATCHMAKER
class Client;
void TranslatePacket(char* buffer, int bytes, struct sockaddr_in from, bool checkprev, NetConn* nc, UDPsocket* sock);
void PacketSwitch(int type, char* buffer, int bytes, struct sockaddr_in from, Client* c);
void ReadLoginPacket(LoginPacket* p, struct sockaddr_in from, Client* c);
void ReadRegistrationPacket(RegistrationPacket* p, struct sockaddr_in from, Client* c);
void ReadAcknowledgmentPacket(AcknowledgmentPacket* ap, struct sockaddr_in from, Client* c);
#else	//MATCHMAKER
void TranslatePacket(char* buffer, int bytes, bool checkprev, NetConn* nc, UDPsocket* sock);
void PacketSwitch(int type, char* buffer, int bytes);
void ReadAcknowledgmentPacket(AcknowledgmentPacket* ap);
void ReadDisconnectPacket(DisconnectPacket* dp);
void ReadConnectPacket(ConnectPacket* dp);
#endif	//MATCHMAKER

#endif	//READPACKETS_H