

#include "simflow.h"

unsigned int g_simframe = 0;
unsigned int g_netframe = 0;	//net frames follow sim frames except when there's a pause, simframes will stop but netframes will continue
unsigned char g_speed = SPEED_PLAY;
bool g_gameover = false;