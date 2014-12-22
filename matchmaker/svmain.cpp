

#define MATCHMAKER

#include "svmain.h"
#include "../common/platform.h"
#include "../common/utils.h"
#include "../common/window.h"
#include "../common/net/net.h"
#include "../common/net/netconn.h"
#include "../common/net/readpackets.h"
#include "../common/net/sendpackets.h"
#include "../common/net/packets.h"

void CheckStop()
{
	FILE* fp = fopen("stop", "r");

	if(!fp)
		return;

	fclose(fp);
	unlink("stop");

	g_log<<"Stopping on command."<<std::endl;
	g_log.flush();
	g_quit = true;
}

int main(int argc, char* argv[])
{
	if(SDL_Init(0) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		std::cout<<msg<<std::endl;
	}

	if(SDLNet_Init() == -1) 
	{
		char msg[1280];
		sprintf(msg, "SDLNet_Init: %s\n", SDLNet_GetError());
		std::cout<<msg<<std::endl;
	}

	OpenLog("log.txt", VERSION);
	
	g_log<<"NETCONN_TIMEOUT = "<<NETCONN_TIMEOUT<<std::endl;
	g_log.flush();
	
	g_netmode = NETM_HOST;

	if(g_sock)
	{
		SDLNet_UDP_Close(g_sock);
		g_sock = NULL;
	}

	if(!(g_sock = SDLNet_UDP_Open(PORT)))
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		//ErrorMessage("Error", msg);
		g_log<<msg<<std::endl;
		return 1;
	}
	
	while(!g_quit)
	{
		UpdNet();
		
		if(UpdNextFrame(MM_FRAME_RATE))
		{
			CheckStop();
			//CheckClients();
			//Update();
		}
		else
			Sleep(1);
	}
	
	SDLNet_Quit();
	SDL_Quit();
	
	return 0;
}