

#include "gmain.h"
#include "../common/gui/gui.h"
#include "keymap.h"
#include "../common/render/shader.h"
#include "gres.h"
#include "../common/gui/font.h"
#include "../common/texture.h"
#include "../common/render/model.h"
#include "../common/math/frustum.h"
#include "../common/render/billboard.h"
#include "../common/render/skybox.h"
#include "ggui.h"
#include "../common/gui/gui.h"
#include "../common/render/particle.h"
#include "../common/sim/building.h"
#include "../common/render/map.h"
#include "../common/render/shadow.h"
#include "../common/sim/road.h"
#include "../common/sim/powerline.h"
#include "../common/sim/zpipeline.h"
#include "../common/render/particle.h"
#include "../common/sim/unit.h"
#include "../common/sim/resource.h"
#include "../common/debug.h"
#include "../common/sim/selection.h"
#include "../common/sim/waves.h"
#include "../common/render/projectile.h"
#include "../common/sim/order.h"
#include "../common/gui/chat.h"
#include "../common/sim/editor.h"
#include "../common/render/water.h"
#include "../common/sim/transaction.h"
#include "../common/sim/player.h"
#include "../common/sound/sound.h"
#include "../common/render/minimap.h"
#include "../common/script/script.h"
#include "../common/ai/ai.h"
#include "../common/render/foliage.h"
#include "../common/window.h"
#include "../common/utils.h"

bool g_quit = false;
APPMODE g_mode = LOADING;

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
			g_GUI.rbuttonup();
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

void AfterDraw(Matrix projection, Matrix viewmat, Matrix modelmat)
{
	//g_log<<"sizeof(g_scenery) = "<<sizeof(g_scenery)<<endl;

#if 0
	Matrix projmodlview = projection;
	projmodlview.postMultiply(viewmat);
	projmodlview.postMultiply(modelmat);
	
	DrawSelection(projection, viewmat, modelmat);
	
	UseS(COLOR3D);
    glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
	glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION]);
	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	//DrawGrid();
	//DrawUnitSquares();
	//DrawPaths();
	//DrawVelocities();

	UseS(BILLBOARD);
    glUniformMatrix4fv(g_shader[SHADER::BILLBOARD].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::BILLBOARD].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::BILLBOARD].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
	glUniform4f(g_shader[SHADER::BILLBOARD].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	glEnableVertexAttribArray(g_shader[SHADER::BILLBOARD].m_slot[SLOT::POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER::BILLBOARD].m_slot[SLOT::TEXCOORD0]);
	//glEnableVertexAttribArray(g_shader[SHADER::BILLBOARD].m_slot[SLOT::NORMAL]);
    glActiveTextureARB(GL_TEXTURE0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	DrawOrders();
	DrawProjectiles();
	//SortParticles();
	//DrawParticles();
	//BeginVertexArrays();
	SortBillboards();
	DrawBillboards();
	//EndVertexArrays();
	
	UseS(ORTHO);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::HEIGHT], (float)g_height);
    glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::POSITION]);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::TEXCOORD0]);
	
	DrawUnitStatus(projmodlview);
	DrawBStatus(projmodlview);
	DrawTransactions(projmodlview);
	
	UseS(COLOR2D);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::HEIGHT], (float)g_height);
    glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 1, 1, 1, STATUS_ALPHA);
    glEnableVertexAttribArray(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION]);

	DrawUnitStats(projmodlview);
	DrawBStats(projmodlview);
#endif
}

void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();	

	if(g_mode == PLAY || g_mode == EDITOR)
	{
		
		//TurnOffShader();
		//g_camera.Look();
#if 0
		float aspect = fabsf((float)g_width / (float)g_height);
		//Matrix projection = BuildPerspProjMat(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		Matrix projection = setorthographicmat(-PROJ_RIGHT, PROJ_RIGHT, PROJ_RIGHT/aspect, -PROJ_RIGHT/aspect, MIN_DISTANCE, MAX_DISTANCE);
		
		Vec3f viewvec = g_camera.View();
        Vec3f posvec = g_camera.Position();
        Vec3f posvec2 = g_camera.LookPos();
        Vec3f upvec = g_camera.UpVector();

		//char msg[128];
		//sprintf(msg, "y = %f", posvec.y);
		//Chat(msg);
        
        Matrix viewmat = gluLookAt2(posvec2.x, posvec2.y, posvec2.z,
                                    viewvec.x, viewvec.y, viewvec.z,
                                    upvec.x, upvec.y, upvec.z);
		
		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.setTranslation(translation);
		modelmat.setTranslation(translation);
		modelview.postMultiply(viewmat);

		g_frustum.CalculateFrustum(projection.getMatrix(), modelview.getMatrix());

		/*
		g_shader[SHADER::MAP].UseS();
        glUniformMatrix4fv(g_shader[SHADER::MAP].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
        glUniformMatrix4fv(g_shader[SHADER::MAP].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
        glUniformMatrix4fv(g_shader[SHADER::MAP].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
        glUniform4f(g_shader[SHADER::MAP].m_slot[SLOT::COLOR], 1, 1, 1, 1);
		glEnableVertexAttribArray(g_shader[SHADER::MAP].m_slot[SLOT::POSITION]);
		glEnableVertexAttribArray(g_shader[SHADER::MAP].m_slot[SLOT::TEXCOORD0]);
		//glEnableVertexAttribArray(g_shader[SHADER::MAP].m_slot[SLOT::TEXCOORD1]);
		glEnableVertexAttribArray(g_shader[SHADER::MAP].m_slot[SLOT::NORMAL]);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		//DrawSkyBox(g_camera.LookPos());
		//DrawMap();
		g_hmap.draw(&g_shader[SHADER::MAP]);
		
		if(g_mode == EDITOR)
		{
			g_shader[SHADER::COLOR3D].UseS();
			glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
			glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
			glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
			glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 0, 1, 0, 1);
			glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION]);
			glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL]);
			DrawTileSq();
		}

		g_shader[SHADER::MODEL].UseS();
        glUniformMatrix4fv(g_shader[SHADER::MODEL].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
        glUniformMatrix4fv(g_shader[SHADER::MODEL].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
        glUniformMatrix4fv(g_shader[SHADER::MODEL].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
        glUniform4f(g_shader[SHADER::MODEL].m_slot[SLOT::COLOR], 1, 1, 1, 1);
		glEnableVertexAttribArray(g_shader[SHADER::MODEL].m_slot[SLOT::POSITION]);
		glEnableVertexAttribArray(g_shader[SHADER::MODEL].m_slot[SLOT::TEXCOORD0]);
		glEnableVertexAttribArray(g_shader[SHADER::MODEL].m_slot[SLOT::NORMAL]);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		//BeginVertexArrays();
		//DrawSurroundings();
		DrawWater(&g_shader[SHADER::MODEL]);
		DrawBuildings(&g_shader[SHADER::MODEL]);
		DrawPowerlines(&g_shader[SHADER::MODEL]);
		DrawRoads(&g_shader[SHADER::MODEL]);
		DrawSelB(&g_shader[SHADER::MODEL]);
		DrawUnits(&g_shader[SHADER::MODEL]);
		//EndVertexArrays();
		TurnOffShader();

		DrawUnitStatus();
		//DrawGrid();
		//DrawUnitSquares();
		//DrawPaths();
		DrawSelection();
		DrawOrders();
		DrawProjectiles();
		BeginVertexArrays();
		SortBillboards();
		DrawBillboards();
		EndVertexArrays();
		*/
		
				LastNum("pre rtsm");
		RenderToShadowMap(projection, viewmat, modelmat);
				LastNum("pre rss");
		RenderShadowedScene(projection, viewmat, modelmat, modelview);
				LastNum("pre ad");
		AfterDraw(projection, viewmat, modelmat);
				LastNum("post ad");

#endif
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
#if 0
	case 0: Status("Loading textures...", true); stage++; break;
	case 1: LoadTiles(); LoadMap(); LoadTerrainTextures(); LoadHoverTex(); LoadSkyBox("defsky"); Status("Loading particles...", true); stage++; break;
	case 2: LoadParticles(); Status("Loading projectiles...", true); stage++; break;
	case 3: LoadProjectiles(); Status("Loading unit sprites...", true); stage++; break;
	case 4: LoadUnitSprites(); Status("Loading sounds...", true); stage++; break;
	case 5: LoadSounds(); Status("Loading building sprites...", true); stage++; break;
	case 6: BSprites(); Status("Loading models...", true); stage++; break;
#endif
	case 0: if(!Load1Model()) stage++; break;
	case 1:
		if(!Load1Texture())
		{
			g_mode = MENU;
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

void UpdateGameState()
{
#if 0
	CalculateFrameRate();
	Scroll();
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

#if 0
	ifstream config(cfgfull);
	
	int fulls;
	config>>fulls;
	
	if(fulls)
		g_fullscreen = true;
	else
		g_fullscreen = false;

	config>>g_selectedRes.width>>g_selectedRes.height;
	config>>g_bpp;

	g_width = g_selectedRes.width;
	g_height = g_selectedRes.height;
#endif
	
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
#if 0
	MapKeys();
	InitPlayers();
	InitResources();
	InitScenery();
	InitBuildings();
	InitUnits();
	InitRoads();
	InitPowerlines();
	InitPipelines();
	InitProfiles();
#endif
}

void Deinit()
{
#if 0
	g_log<<"Freeing map...";
	g_log.flush();
	g_hmap.free();
	g_log<<"Map freed."<<endl;
	g_log.flush();
	FreeScript();
#endif
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	MSG msg;
	g_hInstance = hInstance;

	Init();
	
	if(!MakeWindow(TEXT(TITLE), LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_TRIGEAR)), &WndProc))
		return 0;
	
	//Queue();
	//FillGUI();

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
				LastNum("pre upd");
				Update();
				LastNum("pre draw");
				Draw();
				LastNum("post draw");
			}
			else
				Sleep(1);
		}
	}
	
	DestroyWindow(TEXT(TITLE));
	Deinit();

	return msg.wParam;
}


