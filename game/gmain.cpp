

#include "gmain.h"
#include "../common/gui/gui.h"
#include "keymap.h"
#include "../common/render/shader.h"
#include "gres.h"
#include "../common/gui/font.h"
#include "../common/render/texture.h"
#include "../common/render/model.h"
#include "../common/math/frustum.h"
#include "../common/render/billboard.h"
#include "../common/render/skybox.h"
#include "ggui.h"
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

ofstream g_log;
bool g_quit = false;
APPMODE g_mode = LOGO;

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

void CalculateFrameRate()
{
	static double framesPerSecond   = 0.0f;		// This will store our fps
    static double lastTime			= 0.0f;		// This will hold the time from the last frame

	static double frameTime = 0.0f;				// This stores the last frame's time

	// Get the current time in seconds
    double currentTime = timeGetTime() * 0.001f;				


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// We added a small value to the frame interval to account for some video
	// cards (Radeon's) with fast computers falling through the floor without it.

	// Here we store the elapsed time between the current and last frame,
	// then keep the current frame in our static variable for the next frame.
 	g_FrameInterval = currentTime - frameTime + 0.005f;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


	frameTime = currentTime;

	// Increase the frame counter
    ++framesPerSecond;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
    if( currentTime - lastTime > 1.0f )
	{
		char msg[128];
		sprintf(msg, "FPS: %f, %fs", (float)framesPerSecond, (float)(currentTime - lastTime)/(float)framesPerSecond);
		g_GUI.getview("chat")->getwidget("fps", TEXT)->text = msg;

		// Here we set the lastTime to the currentTime
	    lastTime = currentTime;
		
		// Copy the frames per second into a string to display in the window title bar
		//sprintf(strFrameRate, "Current Frames Per Second: %d", int(framesPerSecond));

		// Set the window title bar to our string
		//SetWindowText(g_hWnd, strFrameRate);

		// Reset the frames per second
        framesPerSecond = 0;
    }
}

void AfterDraw(Matrix projection, Matrix viewmat, Matrix modelmat)
{
	//g_log<<"sizeof(g_scenery) = "<<sizeof(g_scenery)<<endl;

	Matrix projmodlview = projection;
	projmodlview.postMultiply(viewmat);
	projmodlview.postMultiply(modelmat);
	
	DrawSelection(projection, viewmat, modelmat);
	
	Use(COLOR3D);
    glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
	glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION]);
	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	//DrawGrid();
	//DrawUnitSquares();
	//DrawPaths();
	//DrawVelocities();

	Use(BILLBOARD);
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
	
	Use(ORTHO);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::HEIGHT], (float)g_height);
    glUniform4f(g_shader[SHADER::ORTHO].m_slot[SLOT::COLOR], 1, 1, 1, 1);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::POSITION]);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::TEXCOORD0]);
	
	DrawUnitStatus(projmodlview);
	DrawBStatus(projmodlview);
	DrawTransactions(projmodlview);
	
	Use(COLOR2D);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::HEIGHT], (float)g_height);
    glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 1, 1, 1, STATUS_ALPHA);
    glEnableVertexAttribArray(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION]);

	DrawUnitStats(projmodlview);
	DrawBStats(projmodlview);
}

void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();	

	if(g_mode == PLAY || g_mode == EDITOR)
	{
		
		//TurnOffShader();
		//g_camera.Look();
		
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
		g_shader[SHADER::MAP].Use();
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
			g_shader[SHADER::COLOR3D].Use();
			glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
			glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
			glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
			glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 0, 1, 0, 1);
			glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION]);
			glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL]);
			DrawTileSq();
		}

		g_shader[SHADER::MODEL].Use();
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
	}

	g_GUI.frame();
	g_GUI.draw();

	SwapBuffers(g_hDC);
}

void UpdateLoading()
{
	static int stage = 0;

	switch(stage)
	{
	case 0: Status("Loading textures...", true); stage++; break;
	case 1: LoadTiles(); LoadMap(); LoadTerrainTextures(); LoadHoverTex(); LoadSkyBox("defsky"); Status("Loading particles...", true); stage++; break;
	case 2: LoadParticles(); Status("Loading projectiles...", true); stage++; break;
	case 3: LoadProjectiles(); Status("Loading unit sprites...", true); stage++; break;
	case 4: LoadUnitSprites(); Status("Loading sounds...", true); stage++; break;
	case 5: LoadSounds(); Status("Loading building sprites...", true); stage++; break;
	case 6: BSprites(); Status("Loading models...", true); stage++; break;
	case 7: if(Load1Model()) stage++; break;
	case 8:
		if(Load1Texture())
		{
			g_mode = MENU;
			RedoGUI();
			g_hmap.retexture();
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
		if(Load1Texture())
		{
			g_mode = MENU;
			RedoGUI();
			g_hmap.retexture();
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
		g_GUI.getview("logo")->widget[0].rgba[3] = a;
	}
	else if(stage < 120)
	{
		float a = 1.0f - (float)(stage-60) / 60.0f;
		g_GUI.getview("logo")->widget[0].rgba[3] = a;
	}
	else
		SkipLogo();

	stage++;
}

void UpdateIntro()
{
	static int stage = 0;

	if(stage < 60)
	{
		float a = (float)stage / 60.0f;
		g_GUI.getview("intro")->widget[0].rgba[3] = a;
	}
	else if(stage < 120)
	{
		float a = 1.0f - (float)(stage-60) / 60.0f;
		g_GUI.getview("intro")->widget[0].rgba[3] = a;
	}
	else
	{
		g_mode = MENU;
		RedoGUI();
	}

	stage++;
}

void UpdateGameState()
{
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
}

void UpdateEditor()
{
	CalculateFrameRate();
	Scroll();
	UpdateFPS();
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
	ifstream config("config.ini");
	
	//getline(config, g_username);
	//getline(config, g_servername);

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
}

void WriteConfig()
{
	ofstream config;
	config.open("config.ini", ios_base::out);
	
	//config<<g_username<<endl;
	//config<<g_servername<<endl;
	
	int fulls;
	if(g_fullscreen)
		fulls = 1;
	else
		fulls = 0;

	config<<fulls<<endl;
	config<<g_selectedRes.width<<" "<<g_selectedRes.height<<endl;
	config<<g_bpp;
}

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
	g_log.open("log.txt", ios_base::out);
	g_log<<"Version "<<VERSION<<endl<<endl;
	g_log.flush();

	srand(GetTickCount());
	
	LoadConfig();
	//EnumerateMaps();
	EnumerateDisplay();
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
}

void Deinit()
{
	g_log<<"Freeing map...";
	g_log.flush();
	g_hmap.free();
	g_log<<"Map freed."<<endl;
	g_log.flush();
	FreeScript();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	MSG msg;
	g_hInstance = hInstance;

	Init();
	
	if(!MakeWindow())
		return 0;

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

	DestroyWindow();
	Deinit();

	return msg.wParam;
}


