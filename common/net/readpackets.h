
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

class NetConn;

#ifdef MATCHMAKER
class Client;
void TranslatePacket(char* buffer, int bytes, struct sockaddr_in from, bool checkprev, NetConn* nc, UDPsocket* sock);
void PacketSwitch(int type, char* buffer, int bytes, struct sockaddr_in from, Client* c);
void ReadLoginPacket(LoginPacket* p, struct sockaddr_in from, Client* c);
void ReadRegistrationPacket(RegistrationPacket* p, struct sockaddr_in from, Client* c);
void ReadAcknowledgmentPacket(AcknowledgmentPacket* ap, struct sockaddr_in from, Client* c);
#else	//MATCHMAKER
void TranslatePacket(char* buffer, int bytes, bool checkprev, NetConn* nc, UDPsocket* sock, IPaddress* from);
void PacketSwitch(int type, char* buffer, int bytes, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);
void ReadAcknowledgmentPacket(AcknowledgmentPacket* ap, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);
void ReadDisconnectPacket(DisconnectPacket* dp, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);
void ReadConnectPacket(ConnectPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);
#endif	//MATCHMAKER

#endif	//READPACKETS_H