
#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
    #define PLATFORM_WIN
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC
        #define PLATFORM_MAC
    #elif TARGET_OS_IPHONE
        #define PLATFORM_IOS
        #define PLATFORM_IPHONE
    #elif TARGET_OS_IPAD
        #define PLATFORM_IOS
        #define PLATFORM_IPAD
    #endif
#elif defined( __GNUC__ )
    #define PLATFORM_LINUX
#elif defined( __linux__ )
    #define PLATFORM_LINUX
#elif defined ( __linux )
    #define PLATFORM_LINUX
#endif

#pragma warning(disable: 4996)
#pragma warning(disable: 4018)
#pragma warning(disable: 4160)
#pragma warning(disable: 4244)
#pragma warning(disable: 4800)
#pragma warning(disable: 4305)

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#ifdef PLATFORM_WIN
#include <winsock2.h>	// winsock2 needs to be included before windows.h
#include <windows.h>
#include <mmsystem.h>
#endif

#ifdef PLATFORM_WIN
#include <commdlg.h>
#endif

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <fstream>
#include <iostream>
#include <math.h>

#ifdef PLATFORM_WIN
#include <jpeglib.h>
#include <png.h>
#include <zip.h>
#endif

//#define NO_SDL_GLEXT

#include <GL/glew.h>

//#define GL_GLEXT_PROTOTYPES

#if 1
#ifdef PLATFORM_LINUX
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

#ifdef PLATFORM_MAC
#include <GL/xglew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

#ifdef PLATFORM_WIN
#include <GL/wglew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#endif

#ifdef PLATFORM_WIN
#include <gl/glaux.h>
#endif

#ifdef PLATFORM_WIN
#pragma comment(lib, "x86/SDL2.lib")
#pragma comment(lib, "x86/SDL2main.lib")
//#pragma comment(lib, "SDL.lib")
//#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "libpng15.lib")
#pragma comment(lib, "zlibstatic.lib")
#pragma comment(lib, "zipstatic.lib")
#endif

using namespace std;

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef PLATFORM_WIN
#define SOCKET int
typedef unsigned char byte;
typedef unsigned int UINT;
#define _isnan isnan
#define stricmp strcasecmp
#endif

/*
#ifndef _isnan
int _isnan(double x) { return x != x; }
#endif
*/

#ifdef PLATFORM_WIN
extern HINSTANCE g_hInstance;
#endif

#ifdef PLATFORM_LINUX
extern Display *g_display;
extern Window g_window;
#endif

extern SDL_Window *g_window;
extern SDL_Renderer* g_renderer;
extern SDL_GLContext g_glcontext;

#endif
