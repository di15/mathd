


#ifndef DEBUG_H
#define DEBUG_H

#include "platform.h"
#include "gui/richtext.h"

void LogRich(const RichText* rt);
void CheckNum(const char* num);
void LastNum(const char* l);
void CheckGLError(const char* file, int line);


enum PROFILE{FRAME, EVENT, UPDATE, DRAW, UPDATEUNITS, UPDATEBUILDINGS, UPDUONCHECK, UPDUNITAI, MOVEUNIT, ANIMUNIT, DRAWBUILDINGS, DRAWUNITS, DRAWRIM, DRAWWATER, DRAWCRPIPES, DRAWPOWLS, DRAWFOLIAGE, SORTPARTICLES, DRAWPARTICLES, DRAWMAP, SHADOWS, DRAWSKY, DRAWPOWERLINES, DRAWROADS, DRAWMODEL1, DRAWMODEL2, DRAWMODEL3, PROFILES};

class Profile
{
public:
	char name[64];
	double averagems;
	int lastframe;
	int frames;
	//double framems;
	long long starttick;
	//double timescountedperframe;
	//double lastframeaverage;
	int lastframeelapsed;
	int inside;

	Profile()
	{
		averagems = 0.0;
		lastframeelapsed = 0;
		lastframe = 0;
		frames = 0;
		inside = -1;
	}
};

extern Profile g_profile[PROFILES];

void StartTimer(int id);
void StopTimer(int id);
void WriteProfiles(int in, int layer);
void InitProfiles();

#endif	//DEBUG_H