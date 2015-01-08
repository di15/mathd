

#include "app.h"

int g_mode = APPMODE_LOADING;
int g_restage = 0;

void SkipLogo()
{
	GUI* gui;

	g_mode = APPMODE_LOADING;
	gui = &g_gui;
	Widget_CloseAll((Widget*)gui);
	Widget_Open((Widget*)gui, "load");
}

void UpdLogo()
{
	GUI* gui;
	Widget* viewlayer;
	Image* image;
	static int stage = 0;

	gui = &g_gui;
	viewlayer = Widget_Get(gui, "logo");
	image = (Image*)Widget_Get(viewlayer, "logo");

	if(stage < 60)
		image->m_rgba[3] = (float)stage / 60.0f;
	else if(stage < 120)
		image->m_rgba[3] = 1.0f - (float)(stage-60) / 60.0f;
	else
		SkipLogo();

	stage++;
}

void UpdLoad()
{
	static int stage = 0;
	GUI* gui;
	
	gui = &g_gui;

	switch(stage)
	{
	case 0:
		if(!Load1Model()) stage++;
		break;
	case 1:
		if(!Load1Texture())
		{
			g_mode = APPMODE_MENU;
			Widget_CloseAll((Widget*)gui);
			Widget_Open((Widget*)gui, "main");
		}
		break;
	}
}

void UpdReload()
{
	switch(g_restage)
	{
	case 0:
		if(!Load1Texture())
		{
			g_mode = APPMODE_MENU;
		}
		break;
	}
}

void UpdSim()
{
	if(!CanTurn())
		return;

	if(g_speed == SPEED_PAUSE)
	{
		UpdTurn();
		g_netframe ++;
		return;
	}
	
	UpdTurn();
	UpdAI();
	ManageTrips();
	StartTimer(TIMER_UPDATEUNITS);
	UpdUnits();
	StopTimer(TIMER_UPDATEUNITS);
	StartTimer(TIMER_UPDATEBUILDINGS);
	UpdBls();
	StopTimer(TIMER_UPDATEBUILDINGS);
	Tally();

	//must do this after UpdTurn
	g_simframe ++;
	g_netframe ++;
}

void UpdEd()
{
}

void Update()
{
	if(g_sock)
		UpdNet();

	if(g_mode == APPMODE_LOGO)
		UpdLogo();
	//else if(g_mode == APPMODE_INTRO)
	//	UpdIntro();
	else if(g_mode == APPMODE_LOADING)
		UpdLoad();
	else if(g_mode == APPMODE_RELOADING)
		UpdReload();
	else if(g_mode == APPMODE_PLAY)
		UpdSim();
	else if(g_mode == APPMODE_EDITOR)
		UpdEd();
}

void DrawScene()
{
}

void Draw()
{
	GUI* gui;
	char fpsstr[256];
	RichText fpsrstr;
	RichPart fpsrp;
	UStr fpsus;
	
	gui = &g_gui;

	StartTimer(TIMER_DRAWSETUP);
	CHECKGLERROR();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECKGLERROR();
	StopTimer(TIMER_DRAWSETUP);

	if(g_mode == APPMODE_PLAY || g_mode == APPMODE_EDITOR)
		DrawScene();
	CHECKGLERROR();

	StartTimer(TIMER_DRAWGUI);
	Widget_FrameUpd((Widget*)gui);
	CHECKGLERROR();
	Widget_Draw((Widget*)gui);
	StopTimer(TIMER_DRAWGUI);
	
	CHECKGLERROR();
	GUI_Ortho(g_width, g_height, 1, 1, 1, 1);
	CHECKGLERROR();
	glDisable(GL_DEPTH_TEST);
	CHECKGLERROR();

	if(g_debuglines)
	{
		UStr_Init(&fpsus);
		RichPart_Init(&fpsrp);
		RichText_Init(&fpsrstr);

		sprintf(fpsstr, "drw:%lf (%lf s/frame), upd:%lf (%lf s/frame), simfr:%s", g_instantdrawfps, 1.0/g_instantdrawfps, g_instantupdfps, 1.0/g_instantupdfps, IForm(g_simframe).c_str());
		UStr_Append_Str(&fpsus, fpsstr);
		RichPart_Append_UStr(&fpsrp, &fpsus);
		RichText_Append_RichPart(&fpsrt, &fpsrp);
		//fpsrstr = ParseTags(fpsrstr, NULL);

		DrawShadowedText(MAINFONT8, 0, g_height-MINIMAP_SIZE-32, &fpsrstr);
		CHECKGLERROR();
		
		UStr_Free(&fpsus);
		RichPart_Free(&fpsrp);
		RichText_Free(&fpsrstr);
	}

	glEnable(GL_DEPTH_TEST);
	Shader_End();
	CHECKGLERROR();

	SDL_GL_SwapWindow(g_window);
}

bool OverMinimap()
{
	return FALSE;
}

void Scroll()
{
	Bool moved;
	Camera* c;

	if(g_mouseout)
		return;

	moved = FALSE;

	if((g_keys[SDL_SCANCODE_UP] || g_keys[SDL_SCANCODE_W]) || (g_mouse.y <= SCROLL_BORDER))
	{
		g_vel.x += CAMERA_SPEED / g_zoom * g_drawfrinterval);
		moved = TRUE;
	}

	if((g_keys[SDL_SCANCODE_DOWN] || g_keys[SDL_SCANCODE_S]) || (g_mouse.y >= g_height-SCROLL_BORDER))
	{
		g_vel.x += -CAMERA_SPEED / g_zoom * g_drawfrinterval);
		moved = TRUE;
	}

	if((g_keys[SDL_SCANCODE_LEFT] || g_keys[SDL_SCANCODE_A]) || (g_mouse.x <= SCROLL_BORDER))
	{
		g_vel.y += -CAMERA_SPEED / g_zoom * g_drawfrinterval);
		moved = TRUE;
	}

	if((g_keys[SDL_SCANCODE_RIGHT] || g_keys[SDL_SCANCODE_D]) || (g_mouse.x >= g_width-SCROLL_BORDER))
	{
		g_vel.y += CAMERA_SPEED / g_zoom * g_drawfrinterval);
		moved = TRUE;
	}
}

void LoadConfig()
{
	char cfgfull[MAX_PATH+1];
	FILE* fp;
	char keystr[128];
	char actstr[128];
	float valuef;
	int valuei;
	Bool valueb;

	FullPath(CONFIGFILE, cfgfull);
	fp = fopen(cfgfull, "r");

	if(!fp)
		return;

	while(!feof(fp))
	{
		strcpy(keystr, "");
		strcpy(actstr, "");

		if(fscanf(fp, "%s %s", keystr, actstr) != 2)
			continue;

		valuef = StrToFloat(actstr);
		valuei = StrToInt(actstr);
		valueb = valuei ? TRUE : FALSE;

		if(stricmp(keystr, "fullscreen") == 0)					g_fullscreen = valueb;
		else if(stricmp(keystr, "client_width") == 0)			g_width = g_resol.x = valuei;
		else if(stricmp(keystr, "client_height") == 0)			g_height = g_resol.y = valuei;
		else if(stricmp(keystr, "screen_bpp") == 0)				g_bpp = valuei;
	}

	fclose(fp);
}

int testfunc(ObjectScript::OS* os, int nparams, int closure_values, int need_ret_values, void * param)
{
	InfoMess("os", "test");
	return 1;
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void SignalCallback(int signum)
{
	//printf("Caught signal %d\n",signum);
	// Cleanup and close up stuff here

	// Terminate program
	//g_quit = TRUE;
	exit(0);	//force quit NOW
}

void Init()
{
	char msg[1280];
	SDL_version compile_version;
	SDL_version *link_version;
	int flags;
	int initted;

#ifdef PLATFORM_LINUX
	signal(SIGINT, SignalCallback);
#endif

	if(SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		ErrMess("Error", msg);
	}

	if(SDLNet_Init() == -1)
	{
		sprintf(msg, "SDLNet_Init: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
	}

	link_version = Mix_Linked_Version();
	
	SDL_MIXER_VERSION(&compile_version);

	printf("compiled with SDL_mixer version: %d.%d.%d\n", 
			compile_version.major,
			compile_version.minor,
			compile_version.patch);
	printf("running with SDL_mixer version: %d.%d.%d\n", 
			link_version->major,
			link_version->minor,
			link_version->patch);

	// load support for the OGG and MOD sample/music formats
	//flags=MIX_INIT_OGG|MIX_INIT_MOD|MIX_INIT_MP3;
	flags = MIX_INIT_OGG|MIX_INIT_MP3;
	initted = Mix_Init(flags);

	if((initted & flags) != flags)
	{
		sprintf(msg, "Mix_Init: Failed to init required ogg and mod support!\nMix_Init: %s", Mix_GetError());
		ErrMess("Error", msg);
		// handle error
	}

	// start SDL with audio support
	if(SDL_Init(SDL_INIT_AUDIO) == -1)
	{
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		ErrMess("Error", msg);
		// handle error
		//exit(1);
	}
	// open 44.1KHz, signed 16bit, system byte order,
	//      stereo audio, using 1024 byte chunks
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1)
	{
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		ErrMess("Error", msg);
		// handle error
		//exit(2);
	}

	Mix_AllocateChannels(SOUND_CHANNELS);

	OpenLog("log.txt", VERSION);

	srand((unsigned int)GetTickCount64());

	LoadConfig();

	//g_os = ObjectScript::OS::create();
	//g_os->pushCFunction(testfunc);
	//g_os->setGlobal("testfunc");
	//os->eval("testfunc();");
	//os->eval("function require(){ /* if(relative == \"called.os\") */ { testfunc(); } }");
	//char autoexecpath[MAX_PATH+1];
	//FullPath("scripts/autoexec.os", autoexecpath);
	//g_os->require(autoexecpath);
	//g_os->release();

	//EnumerateMaps();
	//EnumerateDisplay();
	MapKeys();

	InitProfiles();
}

void Deinit()
{
	const unsigned long long start = GetTickCount64();

	WriteProfiles(-1, 0);
	DestroyWindow(TITLE);

	//After quit, wait to send out quit packet to make sure host/clients recieve it.
	while (GetTickCount64() - start < QUIT_DELAY)
	{
		if(NetQuit())
			break;
		if(g_sock)
			UpdNet();
	}

	// Clean up	
	
	if(g_sock)
	{
		SDLNet_UDP_Close(g_sock);
		g_sock = NULL;
	}

	FreeSounds();

	Mix_CloseAudio();

	// force a quit
	//while(Mix_Init(0))
	//	Mix_Quit();
	Mix_Quit();

	SDLNet_Quit();
	SDL_Quit();
}

void EventLoop()
{
	GUI* gui;
	SDL_Event e;
	InEv ie;
	
	gui = &g_gui;

	while(!g_quit)
	{
		StartTimer(TIMER_FRAME);
		StartTimer(TIMER_EVENT);

		while(SDL_PollEvent(&e))
		{
			ie.intercepted = FALSE;
			ie.curst = CU_DEFAULT;

			switch(e.type)
			{
			case SDL_QUIT:
				g_quit = TRUE;
				break;

			case SDL_KEYDOWN:
				ie.type = INEV_KEYDOWN;
				ie.key = e.key.keysym.sym;
				ie.scancode = e.key.keysym.scancode;

				//Handle copy
				if( e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
				{
					ie.type = INEV_COPY;
				}
				//Handle paste
				if( e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
				{
					ie.type = INEV_PASTE;
				}
				//Select all
				if( e.key.keysym.sym == SDLK_a && SDL_GetModState() & KMOD_CTRL )
				{
					ie.type = INEV_SELALL;
				}

				Widget_InEv((Widget*)gui, &ie);

				if(!ie.intercepted)
					g_keys[e.key.keysym.scancode] = TRUE;

				g_keyintercepted = ie.intercepted;
				break;

			case SDL_KEYUP:
				ie.type = INEV_KEYUP;
				ie.key = e.key.keysym.sym;
				ie.scancode = e.key.keysym.scancode;
				
				Widget_InEv((Widget*)gui, &ie);

				if(!ie.intercepted)
					g_keys[e.key.keysym.scancode] = FALSE;
				break;

			case SDL_TEXTINPUT:
				//g_GUI.charin(e.text.text);	//UTF8
				ie.type = INEV_TEXTIN;
				strcpy(ie.text, e.text.text);
				Widget_InEv((Widget*)gui, &ie);
				break;

			case SDL_TEXTEDITING:
				//g_GUI.charin(e.text.text);	//UTF8
				ie.type = INEV_TEXTED;
				strcpy(ie.text, e.text.text);
				ie.cursor = e.edit.start;
				ie.sellen = e.edit.length;
				Widget_InEv((Widget*)gui, &ie);
				break;
	
			case SDL_MOUSEWHEEL:
				ie.type = INEV_MOUSEWHEEL;
				ie.amount = e.wheel.y;
				Widget_InEv((Widget*)gui, &ie);
				break;

			case SDL_MOUSEBUTTONDOWN:
				switch (e.button.button)
				{
				case SDL_BUTTON_LEFT:
					g_mousekeys[MOUSE_LEFT] = TRUE;
					g_moved = FALSE;

					ie.type = INEV_MOUSEDOWN;
					ie.key = MOUSE_LEFT;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;
					
					Widget_InEv((Widget*)gui, &ie);

					break;

				case SDL_BUTTON_RIGHT:
					g_mousekeys[MOUSE_RIGHT] = TRUE;

					ie.type = INEV_MOUSEDOWN;
					ie.key = MOUSE_RIGHT;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;
					
					Widget_InEv((Widget*)gui, &ie);
					break;

				case SDL_BUTTON_MIDDLE:
					g_mousekeys[MOUSE_MIDDLE] = TRUE;

					ie.type = INEV_MOUSEDOWN;
					ie.key = MOUSE_MIDDLE;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;

					Widget_InEv((Widget*)gui, &ie);
					break;
				}
				break;

			case SDL_MOUSEBUTTONUP:
				switch (e.button.button)
				{
				case SDL_BUTTON_LEFT:
					g_mousekeys[MOUSE_LEFT] = FALSE;

					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_LEFT;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;

					Widget_InEv((Widget*)gui, &ie);
					break;

				case SDL_BUTTON_RIGHT:
					g_mousekeys[MOUSE_RIGHT] = FALSE;

					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_RIGHT;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;

					Widget_InEv((Widget*)gui, &ie);
					break;

				case SDL_BUTTON_MIDDLE:
					g_mousekeys[MOUSE_MIDDLE] = FALSE;

					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_MIDDLE;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;

					Widget_InEv((Widget*)gui, &ie);
					break;
				}
				break;

			case SDL_MOUSEMOTION:
				if(g_mouseout)
				{
					//TrackMouse();
					g_mouseout = FALSE;
				}
				if(MousePosition())
				{
					g_moved = TRUE;

					ie.type = INEV_MOUSEMOVE;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;

					Widget_InEv((Widget*)gui, &ie);

					g_curst = ie.curst;
				}
				break;
			}
		}

		StopTimer(TIMER_EVENT);

		if(g_mode == APPMODE_LOADING || g_mode == APPMODE_RELOADING || DrawNextFrame(DRAW_FRAME_RATE) )
		{
			StartTimer(TIMER_DRAW);
			CalcDrawRate();
			CHECKGLERROR();
			Draw();
			CHECKGLERROR();
			if(g_mode == APPMODE_PLAY || g_mode == APPMODE_EDITOR)
			{
				Scroll();
				UpdResTicker();
			}
			StopTimer(TIMER_DRAW);
		}
		
		if(g_mode == APPMODE_LOADING || g_mode == APPMODE_RELOADING || UpdNextFrame(SIM_FRAME_RATE) )
		{
			StartTimer(TIMER_UPDATE);
			CalcUpdRate();
			Update();
			StopTimer(TIMER_UPDATE);
		}

		StopTimer(TIMER_FRAME);
	}
}