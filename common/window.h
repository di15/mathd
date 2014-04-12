

#ifndef WINDOW_H
#define WINDOW_H

#include "platform.h"
#include "math/3dmath.h"
#include "math/vec2i.h"

#define INI_WIDTH			800
#define INI_HEIGHT			600
#define INI_BPP				32
#define DRAW_FRAME_RATE			30
#define MIN_DISTANCE		1.0f
//#define MAX_DISTANCE		(100 * 1000 * 10.0f) //10km
//#define MAX_DISTANCE		(10 * 1000 * 10.0f) //10km
#define MAX_DISTANCE		(5 * 1000 * 10.0f) //10km
#define FIELD_OF_VIEW		45.0f
//#define PROJ_LEFT			(-16*4)
#define PROJ_RIGHT			600 //(500)	//(30*12.5f)
//#define PROJ_TOP			(16*4)
//#define PROJ_BOTTOM			(-16*4)
#define INI_ZOOM			0.025f
//#define MIN_ZOOM		0.1f
//#define MIN_ZOOM		0.025f
#define MIN_ZOOM		0.005f
//#define MAX_ZOOM		0.7f 
#define MAX_ZOOM		0.7f

extern double g_drawfrinterval;
extern int g_width;
extern int g_height;
extern int g_bpp;
extern bool g_quit;
extern bool g_active;
extern bool g_fullscreen;
extern Vec2i g_mouse;
extern Vec2i g_mousestart;
#if 0
extern double g_currentTime;
extern double g_lastTime;
extern double g_framesPerSecond;
#endif
extern double g_instantdrawfps;
extern float g_zoom;

struct Resolution
{
	int width;
	int height;
};

extern Resolution g_selectedRes;
extern vector<Resolution> g_resolution;
extern vector<int> g_bpps;

extern bool g_keyintercepted;
extern bool g_keys[256];
extern bool g_mousekeys[3];

#define MOUSEKEY_LEFT	0
#define MOUSEKEY_MIDDLE	1
#define MOUSEKEY_RIGHT	2

void AddRes(int w, int h);
void CalcDrawFrameRate();
bool DrawNextFrame(int desiredFrameRate);
void EnumerateDisplay();
void Resize(int width, int height);
void DestroyWindow(const char* title);
bool MakeWindow(const char* title, HICON icon, WNDPROC wndproc);
void TrackMouse();

#endif