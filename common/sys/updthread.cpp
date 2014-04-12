

#include "workthread.h"
#include "updthread.h"
#include "../../game/gmain.h"
#include "../texture.h"
#include "../render/model.h"
#include "../../game/gui/ggui.h"
#include "../sim/sim.h"
#include "../sim/unit.h"
#include "../gui/gui.h"

double g_instantupdfps = 0;
double g_updfrinterval = 0;
HANDLE g_hupdthread;

void CalcUpdFrameRate()
{
	static double frametime = 0.0f;				// This stores the last frame's time
	static int framecounter = 0;
	static double lasttime;

	// Get the current time in seconds
    double currtime = timeGetTime() * 0.001f;				

	// We added a small value to the frame interval to account for some video
	// cards (Radeon's) with fast computers falling through the floor without it.

	// Here we store the elapsed time between the current and last frame,
	// then keep the current frame in our static variable for the next frame.
 	g_updfrinterval = currtime - frametime;	// + 0.005f;
	
	//g_instantdrawfps = 1.0f / (g_currentTime - frameTime);
	//g_instantdrawfps = 1.0f / g_drawfrinterval;

	frametime = currtime;

	// Increase the frame counter
    ++framecounter;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
    if( currtime - lasttime > 1.0f )
	{
		g_instantupdfps = framecounter;

		// Here we set the lastTime to the currentTime
	    lasttime = currtime;

		// Reset the frames per second
        framecounter = 0;
    }
}

bool UpdateNextFrame(int desiredFrameRate)
{
	static double lastTime = GetTickCount() * 0.001;
	static double elapsedTime = 0.0f;

	double currentTime = GetTickCount64() * 0.001; // Get the time (milliseconds = seconds * .001)
	double deltaTime = currentTime - lastTime; // Get the slice of time
	double desiredFPS = 1.0 / (double)desiredFrameRate; // Store 1 / desiredFrameRate

	elapsedTime += deltaTime; // Add to the elapsed time
	lastTime = currentTime; // Update lastTime

	// Check if the time since we last checked is greater than our desiredFPS
	if( elapsedTime > desiredFPS )
	{
		elapsedTime -= desiredFPS; // Adjust the elapsed time

		// Return true, to animate the next frame of animation
		return true;
	}

	// We don't animate right now.
	return false;
	/*
	long long currentTime = GetTickCount();
	float desiredFPMS = 1000.0f/(float)desiredFrameRate;
	int deltaTime = currentTime - g_lasttime;

	if(deltaTime >= desiredFPMS)
	{
		g_lasttime = currentTime;
		return true;
	}

	return false;*/
}

void UpdateGameState()
{
	MutexWaitInf(g_glovarmutex);
	g_simframe ++;
	MutexRelease(g_glovarmutex);
	
	UpdateUnits();
}

void UpdateEditor()
{
#if 0
	UpdateFPS();
#endif
}

void UpdateSim()
{
	MutexWaitInf(g_glovarmutex);
	APPMODE mode = g_mode;
	MutexRelease(g_glovarmutex);

	if(mode == PLAY)
		UpdateGameState();
	else if(mode == EDITOR)
		UpdateEditor();
}

DWORD WINAPI UpdMain(PVOID pparam)
{
	while(true)
	{
#if 1
		if(!UpdateNextFrame(UPD_FRAME_RATE))
		{
			Sleep(1);
			continue;
		}
#endif

		UpdateSim();

		MutexWait(g_glovarmutex);
		CalcUpdFrameRate();
		if(g_quit)
		{
			MutexRelease(g_glovarmutex);
			break;
		}
		MutexRelease(g_glovarmutex);
	}

	return 0;
}