


#ifndef DEBUG_H
#define DEBUG_H

#include <winsock2.h>	// winsock2 needs to be included before windows.h
#include <Windows.h>
#include <fstream>

using namespace std;


enum PROFILE{FRAME, UPDATE, DRAW, UPDATEUNITS, UPDATEBUILDINGS, DRAWBUILDINGS, DRAWUNITS, SORTPARTICLES, DRAWPARTICLES, DRAWMAP, SHADOWS, DRAWSKY, DRAWPOWERLINES, DRAWROADS, DRAWMODEL1, DRAWMODEL2, DRAWMODEL3, PROFILES};

class CProfile
{
public:
	char name[64];
	double averagems;
	long lastframe;
	double frames;
	//double framems;
	long starttick;
	//double timescountedperframe;
	//double lastframeaverage;
	double lastframetotal;
	int inside;

	CProfile()
	{
		lastframe = 0;
		frames = 0;
		inside = -1;
	}
};

extern CProfile g_profile[PROFILES];

void StartProfile(int id);
void EndProfile(int id);
void WriteProfiles(int in, int layer);
void InitProfiles();
void UpdateFPS();
void UDebug(int i);

void LastNum(const char* l);

#endif