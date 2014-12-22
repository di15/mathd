

#ifndef SIMFLOW_H
#define SIMFLOW_H

extern unsigned int g_simframe;
extern unsigned int g_netframe;	//net frames follow sim frames except when there's a pause, simframes will stop but netframes will continue

#define SPEED_PAUSE		1
#define SPEED_PLAY		2
#define SPEED_FAST		3

extern unsigned char g_speed;
extern bool g_gameover;

#endif