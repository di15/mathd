

#ifndef GMAIN_H
#define GMAIN_H

#include "../common/window.h"

#define VERSION				1.0f
#define CONFIGFILE			"config.ini"
#define TITLE				"Corporation-States"
#define SERVER_ADDR			"corpstates.com"
#define CYCLE_FRAMES		(FRAME_RATE*60)

//#define TRUCK_DEBUG
//#define TRUCK_DEBUG2
//#define PATH_DEBUG

//#define LOCAL_TRANSX	//define this if you want only player-specific transaction hovering notifications to appear

enum APPMODE{LOGO, INTRO, LOADING, RELOADING, MENU, PLAY, PAUSE, EDITOR};
extern APPMODE g_mode;
extern int g_reStage;
extern bool g_mouseout;

void WriteConfig();

#endif
