
#ifndef READPACKETS_H
#define READPACKETS_H

#include "../platform.h"
#include "packets.h"

class NetConn;

void TranslatePacket(char* buffer, int bytes, bool checkprev, UDPsocket* sock, IPaddress* from);
void PacketSwitch(int type, char* buffer, int bytes, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);
void ReadPlaceBlPacket(PlaceBlPacket* pbp, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);
void ReadAcknowledgmentPacket(AcknowledgmentPacket* ap, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);
void ReadDisconnectPacket(DisconnectPacket* dp, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);
void ReadConnectPacket(ConnectPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock, bool bindaddr);

#endif	//READPACKETS_H