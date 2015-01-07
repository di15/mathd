

#ifndef LOCKSTEP_H
#define LOCKSTEP_H

#include "../platform.h"
#include "net.h"
#include "packets.h"

#define NETTURN		(200/SIM_FRAME_RATE)	//200 ms delay between net turns

extern std::list<PacketHeader*> g_nextcmdq;
extern std::list<PacketHeader*> g_localcmd;
extern bool g_canturn;

void Cl_StepTurn();
void Sv_StepTurn();
void SP_StepTurn();
void FreeCmds(std::list<PacketHeader*>* q);
void AppendCmds(std::list<PacketHeader*>* q, NetTurnPacket* ntp);
void AppendCmd(std::list<PacketHeader*>* q, PacketHeader* p, short sz);
bool CanTurn();
void UpdTurn();
void LockCmd(PacketHeader* p, short sz);

#endif