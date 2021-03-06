
#ifndef APP_H
#define APP_H

#include "sys/window.h"

#define VERSION				3
#define CONFIGFILE			"config.ini"
#define TITLE				"Economic Prosperity"
#define SERVER_ADDR			"corpstates.com"

#define APPMODE_LOGO		0
#define APPMODE_INTRO		1
#define APPMODE_LOADING		2
#define APPMODE_RELOADING	3
#define APPMODE_MENU		4
#define APPMODE_PLAY		5
#define APPMODE_PAUSE		6
#define APPMODE_EDITOR		7
#define APPMODE_JOINING		8
extern int g_mode;
extern int g_restage;

#endif