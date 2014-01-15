

#include "gmain.h"
#include "../common/gui/gui.h"
#include "keymap.h"
#include "../common/render/shader.h"
#include "gres.h"
#include "../common/gui/font.h"
#include "../common/texture.h"
#include "../common/render/model.h"
#include "../common/math/frustum.h"
#include "ggui.h"
#include "../common/gui/gui.h"
#include "../common/debug.h"
#include "../common/render/heightmap.h"
#include "../common/math/camera.h"
#include "../common/render/shadow.h"
#include "../common/window.h"
#include "../common/utils.h"
#include "../common/sim/sim.h"

APPMODE g_mode = LOADING;
bool g_mouseout = false;
bool g_moved = false;
Heightmap g_hmap;

//static long long g_lasttime = GetTickCount();

void TrackMouse()
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVER_DEFAULT;	//100;
	tme.hwndTrack = g_hWnd;
	TrackMouseEvent(&tme);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_SCREENSAVE: // Screensaver trying to start?
				case SC_MONITORPOWER: // Monitor trying to enter powersave?
				return 0; // Prevent from happening
			}
			break;
		}
			
		case WM_CREATE:
		{
			if(g_fullscreen)
				CenterMouse();
		}break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_MOUSELEAVE:
		{
			//TrackMouse();
			g_mouseout = true;
		}break;

		case WM_MOUSEHOVER:
		{
			TrackMouse();
			g_mouseout = false;
		}break;

		case WM_LBUTTONDOWN:
		{
			g_mousekeys[0] = true;
			g_moved = false;
			g_GUI.lbuttondown();
		}break;
			
		case WM_LBUTTONUP:
		{
			g_mousekeys[0] = false;
			g_GUI.lbuttonup(g_moved);
		}break;
		
		case WM_RBUTTONDOWN:
		{
			g_mousekeys[2] = true;
			g_GUI.rbuttondown();
		}break;
			
		case WM_RBUTTONUP:
		{
			g_mousekeys[2] = false;
			g_GUI.rbuttonup(g_moved);
		}break;
		
		case WM_MBUTTONDOWN:
		{
			g_mousekeys[1] = true;
			g_GUI.mbuttondown();
		}break;
			
		case WM_MBUTTONUP:
		{
			g_mousekeys[1] = false;
			g_GUI.mbuttonup();
		}break;

		case WM_MOUSEMOVE:
		{
			if(g_mouseout)
			{
				TrackMouse();
				g_mouseout = false;
			}
			if(MousePosition())
			{
				g_moved = true;
				g_GUI.mousemove();
			}
		}break;

		case WM_MOUSEWHEEL:
		{
			g_GUI.mousewheel(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
		}break;

		case WM_KEYDOWN:
		{
			if(!g_GUI.keydown(wParam))
				g_keys[wParam] = true;
			return 0;
		}

		case WM_KEYUP:
		{
			if(!g_GUI.keyup(wParam))
				g_keys[wParam] = false;	
			return 0;
		}

		case WM_CHAR:
		{
			g_GUI.charin(wParam);
			return 0;
		}

		case WM_SIZE:
		{
			Resize(LOWORD(lParam), HIWORD(lParam)); 
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
	UseShadow(SHADER_MAPTILES, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 8);
	g_hmap.draw();

#if 0
	UseShadow(SHADER_WATER, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
	DrawWater();
#endif

#if 0
	UseShadow(SHADER_MODEL, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
	//DrawEdBuilding(&g_edbldg, g_showsky);
#if 0
	DrawBuildings();
	DrawFoliage();
#endif
	g_model[themodel].draw(0, Vec3f(0,0,0), 0);
#endif
}

void DrawSceneDepth()
{
	//g_model[themodel].draw(0, Vec3f(0,0,0), 0);
	
	g_hmap.draw();
#if 0
	DrawBuildings();
	DrawFoliage();
	//DrawEdBuilding(&g_edbldg, false);
#endif
}

void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if(g_mode == PLAY || g_mode == EDITOR)
	{
		float aspect = fabsf((float)g_width / (float)g_height);
		Matrix projection = BuildPerspProjMat(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		//Matrix projection = setorthographicmat(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);

		Vec3f focusvec = g_camera.m_view;
		Vec3f posvec = g_camera.zoompos();
		Vec3f upvec = g_camera.m_up;

		Matrix viewmat = gluLookAt3(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.setTranslation(translation);
		modelmat.setTranslation(translation);
		modelview.postMultiply(viewmat);

		Matrix mvpmat;
		mvpmat.set(projection.m_matrix);
		mvpmat.postMultiply(viewmat);

		//if(v->m_type == VIEWPORT_MAIN3D)
		{
			//RenderToShadowMap(projection, viewmat, modelmat, g_camera.m_view);
			//RenderToShadowMap(projection, viewmat, modelmat, Vec3f(0,0,0));
			Vec3f focus;
			Vec3f vLine[2];
			Vec3f ray = Normalize(g_camera.m_view - posvec);
			Vec3f onnear = posvec;	//OnNear(g_width/2, g_height/2);
			vLine[0] = onnear;
			vLine[1] = onnear + (ray * 100000.0f);
			//if(!GetMapIntersection(&g_hmap, vLine, &focus))
			if(!FastMapIntersect(&g_hmap, vLine, &focus))
				GetMapIntersection2(&g_hmap, vLine, &focus);
			RenderToShadowMap(projection, viewmat, modelmat, focus);
			RenderShadowedScene(projection, viewmat, modelmat, modelview);
		}
	}

	g_GUI.frameupd();
	g_GUI.draw();

	SwapBuffers(g_hDC);
}

void UpdateLoading()
{
	static int stage = 0;

	switch(stage)
	{
	case 0: if(!Load1Model()) stage++; break;
	case 1:
		if(!Load1Texture())
		{
			g_mode = MENU;
			g_mode = PLAY;
			Click_NewGame();
		}
		break;
	}
}

int g_reStage = 0;
void UpdateReloading()
{
	switch(g_reStage)
	{
	case 0:
		if(!Load1Texture())
		{
			g_mode = MENU;
		}
		break;
	}
}

void SkipLogo()
{
	g_mode = LOADING;
	OpenSoleView("loading");
}

void UpdateLogo()
{
	static int stage = 0;

	if(stage < 60)
	{
		float a = (float)stage / 60.0f;
		g_GUI.getview("logo")->getwidget("logo", WIDGET_IMAGE)->m_rgba[3] = a;
	}
	else if(stage < 120)
	{
		float a = 1.0f - (float)(stage-60) / 60.0f;
		g_GUI.getview("logo")->getwidget("logo", WIDGET_IMAGE)->m_rgba[3] = a;
	}
	else
		SkipLogo();

	stage++;
}

bool OverMinimap()
{
	return false;
}

void Scroll()
{
	if(g_mouseout)
		return;

	bool moved = false;

	if((!g_keyintercepted && (g_keys[VK_UP] || g_keys['W'])) || (g_mouse.y <= SCROLL_BORDER && !OverMinimap())) 
	{				
		g_camera.accelerate(CAMERA_SPEED / g_zoom);			
		moved = true;	
	}

	if((!g_keyintercepted && (g_keys[VK_DOWN] || g_keys['S'])) || (g_mouse.y >= g_height-SCROLL_BORDER && !OverMinimap())) 
	{			
		g_camera.accelerate(-CAMERA_SPEED / g_zoom);	
		moved = true;			
	}

	if((!g_keyintercepted && (g_keys[VK_LEFT] || g_keys['A'])) || (g_mouse.x <= SCROLL_BORDER && !OverMinimap())) 
	{			
		g_camera.accelstrafe(-CAMERA_SPEED / g_zoom);
		moved = true;
	}

	if((!g_keyintercepted && (g_keys[VK_RIGHT] || g_keys['D'])) || (g_mouse.x >= g_width-SCROLL_BORDER && !OverMinimap())) 
	{			
		g_camera.accelstrafe(CAMERA_SPEED / g_zoom);
		moved = true;
	}

	if(moved)
	{
		if(g_camera.m_pos.x < -g_hmap.m_widthx*TILE_SIZE/4)
		{
			float d = -g_hmap.m_widthx*TILE_SIZE/4 - g_camera.m_pos.x;
			g_camera.move(Vec3f(d, 0, 0));
		}
		else if(g_camera.m_pos.x > g_hmap.m_widthx*TILE_SIZE*5/4)
		{
			float d = g_camera.m_pos.x - g_hmap.m_widthx*TILE_SIZE*5/4;
			g_camera.move(Vec3f(-d, 0, 0));
		}

		if(g_camera.m_pos.z < -g_hmap.m_widthz*TILE_SIZE/4)
		{
			float d = -g_hmap.m_widthz*TILE_SIZE/4 - g_camera.m_pos.z;
			g_camera.move(Vec3f(0, 0, d));
		}
		else if(g_camera.m_pos.z > g_hmap.m_widthz*TILE_SIZE*5/4)
		{
			float d = g_camera.m_pos.z - g_hmap.m_widthz*TILE_SIZE*5/4;
			g_camera.move(Vec3f(0, 0, -d));
		}

#if 0
		UpdateMouse3D();

		if(g_mode == EDITOR && g_mousekeys[0])
		{
			EdApply();
		}

		if(!g_mousekeys[0])
		{
			g_vStart = g_vTile;
			g_vMouseStart = g_vMouse;
		}
#endif

		g_camera.frameupd();
		g_camera.friction2();
	}
}

void UpdateGameState()
{
#if 0
	CalculateFrameRate();
#endif
	Scroll();
#if 0
	LastNum("pre upd u");
	UpdateUnits();
	LastNum("pre upd b");
	UpdateBuildings();
	LastNum("post upd b");
	UpdateParticles();
	LastNum("up pl");
	UpdatePlayers();
	LastNum("up ai");
	UpdateAI();
	ResourceTicker();
	//UpdateTimes();
	UpdateFPS();
#endif
}

void UpdateEditor()
{
#if 0
	CalculateFrameRate();
	Scroll();
	UpdateFPS();
#endif
}

void Update()
{
	if(g_mode == LOGO)
		UpdateLogo();
	else if(g_mode == LOADING)
		UpdateLoading();
	//else if(g_mode == INTRO)
	//	UpdateIntro();
	else if(g_mode == PLAY)
		UpdateGameState();
	else if(g_mode == EDITOR)
		UpdateEditor();
	else if(g_mode == RELOADING)
		UpdateReloading();
}

void LoadConfig()
{
	char cfgfull[MAX_PATH+1];
	FullPath(CONFIGFILE, cfgfull);
	
	ifstream f(cfgfull);
	string line;
	char keystr[32];
	char actstr[32];

	while(!f.eof())
	{

#if 0
		key = -1;
		down = NULL;
		up = NULL;
#endif 
		strcpy(keystr, "");
		strcpy(actstr, "");

		getline(f, line);
		sscanf(line.c_str(), "%s %s", keystr, actstr);
		
		float valuef = StrToFloat(actstr);
		int valuei = StrToInt(actstr);
		bool valueb = (bool)valuei;
		
		if(stricmp(keystr, "fullscreen") == 0)					g_fullscreen = valueb;
		else if(stricmp(keystr, "client_width") == 0)			g_width = g_selectedRes.width = valuei;
		else if(stricmp(keystr, "client_height") == 0)			g_height = g_selectedRes.height = valuei;
		else if(stricmp(keystr, "screen_bpp") == 0)				g_bpp = valuei;
		//else if(stricmp(keystr, "render_pitch") == 0)			g_renderpitch = valuef;
		//else if(stricmp(keystr, "render_yaw") == 0)				g_renderyaw = valuef;
		//else if(stricmp(keystr, "1_tile_pixel_width") == 0)		g_1tilewidth = valuei;
		//else if(stricmp(keystr, "sun_x") == 0)					g_lightOff.x = valuef;
		//else if(stricmp(keystr, "sun_y") == 0)					g_lightOff.y = valuef;
		//else if(stricmp(keystr, "sun_z") == 0)					g_lightOff.z = valuef;
	}
}

#if 0
void WriteConfig()
{
	ofstream config;
	config.open(CONFIGFILE, ios_base::out);
	
	int fulls;
	if(g_fullscreen)
		fulls = 1;
	else
		fulls = 0;

	config<<fulls<<endl;
	config<<g_selectedRes.width<<" "<<g_selectedRes.height<<endl;
	config<<g_bpp;
}
#endif

/*
void EnumerateMaps()
{
	WIN32_FIND_DATA ffd;
	string mapPath = ExePath() + "\\maps\\*";
	HANDLE hFind = FindFirstFile(mapPath.c_str(), &ffd);

	if(INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			if(!strstr(ffd.cFileName, ".bsp"))
				continue;

			int pos = string( ffd.cFileName ).find_last_of( ".bsp" );
			string name = string( ffd.cFileName ).substr(0, pos-3);

			g_maps.push_back(name);
		} while(FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	else
	{
		// Failure
	}
}*/

void Init()
{
	// Might be launched from command prompt, need
	// to set working directory to exe's folder.
	char exepath[MAX_PATH+1];
	ExePath(exepath);
	SetCurrentDirectory(exepath);

	OpenLog("log.txt", VERSION);

	srand(GetTickCount());
	
	LoadConfig();
	//EnumerateMaps();
	EnumerateDisplay();
	MapKeys();
}

void Deinit()
{
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	MSG msg;
	g_hInstance = hInstance;

	Init();
	
	if(!MakeWindow(TEXT(TITLE), LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_TRIGEAR)), &WndProc))
		return 0;
	
	Queue();
	FillGUI();

	while(!g_quit)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				g_quit = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if ((g_mode == LOADING || g_mode == RELOADING) || AnimateNextFrame(FRAME_RATE))
			{
				Update();
				Draw();
			}
			else
				Sleep(1);
		}
	}
	
	DestroyWindow(TEXT(TITLE));
	Deinit();

	return msg.wParam;
}


