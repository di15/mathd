

#include "gmain.h"
#include "../common/gui/gui.h"
#include "keymap.h"
#include "../common/render/shader.h"
#include "gres.h"
#include "../common/gui/font.h"
#include "../common/texture.h"
#include "../common/render/model.h"
#include "../common/math/frustum.h"
#include "gui/ggui.h"
#include "../common/gui/gui.h"
#include "../common/debug.h"
#include "../common/render/heightmap.h"
#include "../common/math/camera.h"
#include "../common/render/shadow.h"
#include "../common/window.h"
#include "../common/utils.h"
#include "../common/sim/sim.h"
#include "../common/math/hmapmath.h"
#include "../common/render/border.h"
#include "../common/sim/unit.h"
#include "../common/sim/building.h"
#include "../common/sim/buildingtype.h"
#include "../common/render/foliage.h"
#include "../common/render/water.h"
#include "../common/sim/road.h"
#include "../common/sim/crudepipeline.h"
#include "../common/sim/powerline.h"
#include "../common/sim/deposit.h"
#include "../common/sim/selection.h"
#include "../common/sim/order.h"
#include "../common/render/transaction.h"
#include "../common/ai/collidertile.h"
#include "../common/ai/pathdebug.h"
#include "gui/playgui.h"
#include "../common/gui/widgets/spez/bottompanel.h"
#include "../common/texture.h"

APPMODE g_mode = LOADING;
bool g_mouseout = false;
bool g_moved = false;

double g_instantupdfps = 0;
double g_updfrinterval = 0;

//static long long g_lasttime = GetTickCount();

LoadedTex* blittex;
LoadedTex blitscreen;

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
			TrackMouse();
		}break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_MOUSELEAVE:
		{
			TrackMouse();
			g_mouseout = true;
		}break;

		case WM_MOUSEHOVER:
		{
			TrackMouse();
			g_mouseout = false;
		}break;

		case WM_LBUTTONDOWN:
		{
			g_mousekeys[MOUSEKEY_LEFT] = true;
			g_moved = false;
			g_GUI.lbuttondown();
		}break;
			
		case WM_LBUTTONUP:
		{
			g_mousekeys[MOUSEKEY_LEFT] = false;
			g_GUI.lbuttonup(g_moved);
		}break;
		
		case WM_RBUTTONDOWN:
		{
			g_mousekeys[MOUSEKEY_RIGHT] = true;
			g_GUI.rbuttondown();
		}break;
			
		case WM_RBUTTONUP:
		{
			g_mousekeys[MOUSEKEY_RIGHT] = false;
			g_GUI.rbuttonup(g_moved);
		}break;
		
		case WM_MBUTTONDOWN:
		{
			g_mousekeys[MOUSEKEY_MIDDLE] = true;
			g_GUI.mbuttondown();
		}break;
			
		case WM_MBUTTONUP:
		{
			g_mousekeys[MOUSEKEY_MIDDLE] = false;
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
			//g_mode = PLAY;
			Click_NewGame();
			//Click_OpenEditor();
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

void CalcUpdFrameRate()
{
	static unsigned int frametime = 0;				// This stores the last frame's time
	static int framecounter = 0;
	static unsigned int lasttime;

	// Get the current time in seconds
    unsigned int currtime = timeGetTime();				

	// We added a small value to the frame interval to account for some video
	// cards (Radeon's) with fast computers falling through the floor without it.

	// Here we store the elapsed time between the current and last frame,
	// then keep the current frame in our static variable for the next frame.
 	g_updfrinterval = (currtime - frametime) / 1000.0f;	// + 0.005f;
	
	//g_instantdrawfps = 1.0f / (g_currentTime - frameTime);
	//g_instantdrawfps = 1.0f / g_drawfrinterval;

	frametime = currtime;

	// Increase the frame counter
    ++framecounter;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
    if( currtime - lasttime > 1000 )
	{
		g_instantupdfps = framecounter;

		// Here we set the lastTime to the currentTime
	    lasttime = currtime;

		// Reset the frames per second
        framecounter = 0;
    }
}

bool UpdNextFrame(int desiredFrameRate)
{
	static long long lastTime = GetTickCount64();
	static long long elapsedTime = 0;

	long long currentTime = GetTickCount64(); // Get the time (milliseconds = seconds * .001)
	long long deltaTime = currentTime - lastTime; // Get the slice of time
	int desiredFPS = 1000 / (float)desiredFrameRate; // Store 1 / desiredFrameRate

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
	g_simframe ++;

	UpdateUnits();
}

void UpdateEditor()
{
#if 0
	UpdateFPS();
#endif
}

void Update()
{	
	if(g_mode == LOGO)
		UpdateLogo();
	//else if(g_mode == INTRO)
	//	UpdateIntro();
	else if(g_mode == LOADING)
		UpdateLoading();
	else if(g_mode == RELOADING)
		UpdateReloading();
	else if(g_mode == PLAY)
		UpdateGameState();
	else if(g_mode == EDITOR)
		UpdateEditor();
}

void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
#if 1
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postMultiply(viewmat);

	g_frustum.construct(projection.m_matrix, viewmat.m_matrix);

#if 1
	UseShadow(SHADER_MAPTILES, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 8);
	SelectHMZoom(g_zoom)->draw();
	//g_hmap.draw();
#endif

#if 1
	UseShadow(SHADER_BORDERS, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	//glActiveTextureARB(GL_TEXTURE8);
	//glBindTexture(GL_TEXTURE_2D, g_depth);
	//glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 8);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
	DrawBorders();
#endif

#if 0
	UseShadow(SHADER_WATER, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
	DrawWater();
#endif
	
	UseShadow(SHADER_WATER, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
	DrawWater3();

	UseShadow(SHADER_OWNED, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 5);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_OWNCOLOR], 1, 0, 0, 1);
	DrawBuildings();
	DrawRoads();
	DrawCrPipes();
	DrawPowls();
	DrawUnits();
	
#if 1
	UseShadow(SHADER_FOLIAGE, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 5);
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	DrawFoliage(g_camera.zoompos(), g_camera.m_up, g_camera.m_strafe);
#endif

#if 0
	UseShadow(SHADER_BILLBOARD, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
#if 0
	DrawUnits();
#endif
	//DrawFoliage();
#endif

	DrawSelectionCircles(&projection, &modelmat, &viewmat);
	DrawOrders(&projection, &modelmat, &viewmat);
		
#if 0
	UseShadow(SHADER_BILLBOARD, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
#endif

#if 0
	UseShadow(SHADER_COLOR3D, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	DrawGrid();
	DrawUnitSquares();
	DrawPaths();
	DrawSteps();
#endif

#if 0
	Ortho(g_width, g_height, 1, 1, 1, 1);
	glDisable(GL_DEPTH_TEST);
	DrawDeposits(projection, viewmat);
	DrawTransactions(mvpmat);
	glEnable(GL_DEPTH_TEST);
#endif
#endif

#if 0
	Ortho(g_width, g_height, 1, 1, 1, 1);
	glDisable(GL_DEPTH_TEST);
	FoliageT* t = &g_foliageT[FOLIAGE_TREE1];
	Model* m = &g_model[t->model];
	for(int i=0; i<30000; i++)
	{
		m->usetex();
		Texture* tex = &g_texture[m->m_diffusem];

		int x = rand()%g_width;
		int y = rand()%g_height;

		DrawImage(tex->texname, x, y, x+2, y+4, 0, 0, 1, 1);
	}
	glEnable(GL_DEPTH_TEST);
#endif
}

void DrawSceneDepth()
{
#if 1
	//if(rand()%2 == 1)
	SelectHMZoom(g_zoom)->draw2();
	//g_hmap.draw2();
	DrawBuildings();
	DrawRoads();
	DrawCrPipes();
	DrawPowls();
	DrawUnits();
#if 1
	DrawFoliage(g_lightPos, Vec3f(0,1,0), Cross(Vec3f(0,1,0), Normalize(g_lightEye - g_lightPos)));
#endif
#endif
}

void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 2
	if(g_mode == PLAY || g_mode == EDITOR)
	{

		float aspect = fabsf((float)g_width / (float)g_height);
		Matrix projection = BuildPerspProjMat(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE/g_zoom);
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
				//if(!GetMapIntersection(&g_hmap, vLine, &focus))
					GetMapIntersection2(&g_hmap, vLine, &focus);
			RenderToShadowMap(projection, viewmat, modelmat, focus, focus + g_lightOff / g_zoom, DrawSceneDepth);
			RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawScene);
		}
	}
#endif

#if 0
	g_log<<("before framupd")<<endl;
	g_log.flush();
#endif

	g_GUI.frameupd();
	
#if 0
	g_log<<("before gui dr")<<endl;
	g_log.flush();
#endif

	g_GUI.draw();

	DrawMarquee();

#if 0
	for(int i=0; i<30; i++)
	{
		int x = rand()%g_width;
		int y = rand()%g_height;

		Blit(blittex, &blitscreen, Vec2i(x,y));
	}

	glDrawPixels(blitscreen.sizeX, blitscreen.sizeY, GL_RGB, GL_BYTE, blitscreen.data);
#endif

	Ortho(g_width, g_height, 1, 1, 1, 1);
	glDisable(GL_DEPTH_TEST);

	char fpsstr[256];
	sprintf(fpsstr, "draw fps: %lf (%lf s/frame), upd fps: %lf (%lf s/frame), zoom: %f, simframe: %lld", g_instantdrawfps, 1.0/g_instantdrawfps, g_instantupdfps, 1.0/g_instantupdfps, g_zoom, g_simframe);
	RichText fpsrstr(fpsstr);
	DrawShadowedText(MAINFONT8, 0, g_height-MINIMAP_SIZE-32-10, &fpsrstr);
	glEnable(GL_DEPTH_TEST);
	
#if 0
	g_log<<("bef sw bf")<<endl;
	g_log.flush();
#endif

	SwapBuffers(g_hDC);
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

	if((!g_keyintercepted && (g_keys[VK_UP] || g_keys['W'])) || (g_mouse.y <= SCROLL_BORDER && !OverMinimap() && !g_mouseoveraction)) 
	{				
		g_camera.accelerate(CAMERA_SPEED / g_zoom * g_drawfrinterval);			
		moved = true;	
	}

	if((!g_keyintercepted && (g_keys[VK_DOWN] || g_keys['S'])) || (g_mouse.y >= g_height-SCROLL_BORDER && !OverMinimap() && !g_mouseoveraction)) 
	{			
		g_camera.accelerate(-CAMERA_SPEED / g_zoom * g_drawfrinterval);	
		moved = true;			
	}

	if((!g_keyintercepted && (g_keys[VK_LEFT] || g_keys['A'])) || (g_mouse.x <= SCROLL_BORDER && !OverMinimap() && !g_mouseoveraction)) 
	{			
		g_camera.accelstrafe(-CAMERA_SPEED / g_zoom * g_drawfrinterval);
		moved = true;
	}

	if((!g_keyintercepted && (g_keys[VK_RIGHT] || g_keys['D'])) || (g_mouse.x >= g_width-SCROLL_BORDER && !OverMinimap() && !g_mouseoveraction)) 
	{			
		g_camera.accelstrafe(CAMERA_SPEED / g_zoom * g_drawfrinterval);
		moved = true;
	}

#if 0
	if(moved)
#endif
	{
#if 0
		if(g_camera.zoompos().x < -g_hmap.m_widthx*TILE_SIZE)
		{
			float d = -g_hmap.m_widthx*TILE_SIZE - g_camera.zoompos().x;
			g_camera.move(Vec3f(d, 0, 0));
		}
		else if(g_camera.zoompos().x > g_hmap.m_widthx*TILE_SIZE)
		{
			float d = g_camera.zoompos().x - g_hmap.m_widthx*TILE_SIZE;
			g_camera.move(Vec3f(-d, 0, 0));
		}

		if(g_camera.zoompos().z < -g_hmap.m_widthz*TILE_SIZE)
		{
			float d = -g_hmap.m_widthz*TILE_SIZE - g_camera.zoompos().z;
			g_camera.move(Vec3f(0, 0, d));
		}
		else if(g_camera.zoompos().z > g_hmap.m_widthz*TILE_SIZE)
		{
			float d = g_camera.zoompos().z - g_hmap.m_widthz*TILE_SIZE;
			g_camera.move(Vec3f(0, 0, -d));
		}
#else
		
		if(g_camera.m_view.x < 0)
		{
			float d = 0 - g_camera.m_view.x;
			g_camera.move(Vec3f(d, 0, 0));
		}
		else if(g_camera.m_view.x > g_hmap.m_widthx*TILE_SIZE)
		{
			float d = g_camera.m_view.x - g_hmap.m_widthx*TILE_SIZE;
			g_camera.move(Vec3f(-d, 0, 0));
		}

		if(g_camera.m_view.z < 0)
		{
			float d = 0 - g_camera.m_view.z;
			g_camera.move(Vec3f(0, 0, d));
		}
		else if(g_camera.m_view.z > g_hmap.m_widthz*TILE_SIZE)
		{
			float d = g_camera.m_view.z - g_hmap.m_widthz*TILE_SIZE;
			g_camera.move(Vec3f(0, 0, -d));
		}
#endif

#if 0
		UpdateMouse3D();

		if(g_mode == EDITOR && g_mousekeys[MOUSEKEY_LEFT])
		{
			EdApply();
		}

		if(!g_mousekeys[MOUSEKEY_LEFT])
		{
			g_vStart = g_vTile;
			g_vMouseStart = g_vMouse;
		}
#endif
	}
	
	Vec3f line[2];
	line[0] = g_camera.zoompos();
	Camera oldcam = g_camera;
	g_camera.frameupd();
	line[1] = g_camera.zoompos();

	Vec3f ray = Normalize(line[1] - line[0]) * TILE_SIZE;
	//line[0] = line[0] - ray;
	line[1] = line[1] + ray;

	Vec3f clip;

#if 0
	if(GetMapIntersection(&g_hmap, line, &clip))
#else
	if(FastMapIntersect(&g_hmap, line, &clip))
	{
#endif
		g_camera = oldcam;
	}
	else
	{
		CalcMapView();
	}

	g_camera.friction2();
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

#if 0
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
}
#endif

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

#if 0
	g_log<<"sizeof(long long) = "<<sizeof(long long)<<endl;
	g_log.flush();
#endif
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
	
#if 0
	char blitfull[MAX_PATH+1];
	FullPath("models/spruce1/spruce1.png", blitfull);
	blittex = LoadPNG(blitfull);
	AllocTex(&blitscreen, g_width, g_height, 3);
#endif

	while(!g_quit)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				g_quit = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
#if 1
			if ((g_mode == LOADING || g_mode == RELOADING) || DrawNextFrame(DRAW_FRAME_RATE))
#endif
			{
				CalcDrawFrameRate();
				Draw();

				if(g_mode == PLAY || g_mode == EDITOR)
				{
					Scroll();
					UpdateResTicker();
				}
			}
			
			if((g_mode == LOADING || g_mode == RELOADING) || UpdNextFrame(SIM_FRAME_RATE))
			{
				CalcUpdFrameRate();
				Update();
			}
		}
	}
	
#if 0
	delete blittex;
#endif

	DestroyWindow(TEXT(TITLE));
	Deinit();

	return msg.wParam;
}


