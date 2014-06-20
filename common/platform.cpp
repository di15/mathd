
#include "platform.h"

#ifdef PLATFORM_WIN
HINSTANCE g_hInstance = NULL;
#endif

#ifdef PLATFORM_LINUX
Display *g_display;
Window g_window;
#endif

SDL_Window *g_window;
SDL_Renderer* g_renderer;
SDL_GLContext g_glcontext;