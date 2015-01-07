#include "gmain.h"
#include "../engine/app.h"

#ifdef PLATFORM_WIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
	Init();
	MakeWindow(TITLE);
	FillGUI();
	SDL_ShowCursor(false);
	Queue();
	EventLoop();
	Deinit();
	SDL_ShowCursor(true);
	return 0;
}
