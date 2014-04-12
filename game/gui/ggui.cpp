





#include "../gmain.h"
#include "../../common/gui/gui.h"
#include "../keymap.h"
#include "../../common/render/heightmap.h"
#include "../../common/math/hmapmath.h"
#include "../../common/math/camera.h"
#include "../../common/render/shadow.h"
#include "../../common/render/screenshot.h"
#include "../../common/save/savemap.h"
#include "editorgui.h"
#include "playgui.h"
#include "../../common/sim/unit.h"
#include "../../common/sim/unittype.h"
#include "../../common/sim/building.h"
#include "../../common/sim/buildingtype.h"
#include "../../common/sim/selection.h"
#include "../../common/sim/road.h"
#include "../../common/sim/powerline.h"
#include "../../common/sim/crudepipeline.h"
#include "../../common/render/water.h"
#include "../../common/sim/order.h"
#include "../../common/ai/pathdebug.h"

//bool g_canselect = true;

int g_build = BUILDING_NONE;
Vec3f g_vdrag[2];
char g_lastsave[MAX_PATH+1];


#if 0
void Change_Fullscreen()
{
	int selected = g_GUI.getview("settings")->getwidget("fullscreen", DROPDOWN)->selected;

	if(g_fullscreen == (bool)selected)
		return;

	DestroyWindow();
	g_fullscreen = selected;
	WriteConfig();
	MakeWindow();
	Reload();
	RedoGUI();
}

void Change_Resolution()
{
	int selected = g_GUI.getview("settings")->getwidget("resolution", DROPDOWN)->selected;

	if(g_selectedRes.width == g_resolution[selected].width && g_selectedRes.height == g_resolution[selected].height)
		return;

	g_selectedRes = g_resolution[selected];
	WriteConfig();

	if(g_fullscreen)
	{	
		DestroyWindow();
		MakeWindow();
		Reload();
		RedoGUI();
	}
	else
	{
		DWORD dwExStyle;
		DWORD dwStyle;
		RECT WindowRect;
		WindowRect.left=(long)0;
		WindowRect.right=(long)g_selectedRes.width;
		WindowRect.top=(long)0;
		WindowRect.bottom=(long)g_selectedRes.height;

		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
		//startx = CW_USEDEFAULT;
		//starty = CW_USEDEFAULT
		int startx = GetSystemMetrics(SM_CXSCREEN)/2 - g_selectedRes.width/2;
		int starty = GetSystemMetrics(SM_CYSCREEN)/2 - g_selectedRes.height/2;

		AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);
		MoveWindow(g_hWnd, startx, starty, WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, false);

		//int startx = GetSystemMetrics(SM_CXSCREEN)/2 - g_selectedRes.width/2;
		//int starty = GetSystemMetrics(SM_CYSCREEN)/2 - g_selectedRes.height/2;

		//MoveWindow(g_hWnd, startx, starty, g_selectedRes.width, g_selectedRes.height, false);
	}
}

void Change_BPP()
{
	int selected = g_GUI.getview("settings")->getwidget("bpp", DROPDOWN)->selected;

	if(g_bpp == g_bpps[selected])
		return;

	g_bpp = g_bpps[selected];
	WriteConfig();
	DestroyWindow();
	MakeWindow();
	Reload();
	RedoGUI();
}

void Click_BuildNum(int param)
{
	if(param == NOTHING && g_mode != EDITOR)
		g_canselect = true;
	
	//if(g_mode != EDITOR)
		CloseView("build selector");

	g_build = param;

	if(g_mode == EDITOR)
		g_edTool = EDTOOL::NOTOOL;

	//MessageBox(g_hWnd, "buildnum", "", NULL);
}

void Click_OutBuild()
{
	CView* v = g_GUI.getview("build selector");
	/*
	Widget* sw = &v->widget[0];
	sw->ldown = false;

	Widget* w;

	for(int i=0; i<sw->subwidg.size(); i++)
	{
		w = &sw->subwidg[i];
		w->ldown = false;
	}
	*/
	g_canselect = true;

	CloseView("build selector");
}

void Reload()
{
	g_mode = RELOADING;
	g_reStage = 0;
	g_lastLTex = -1;

	LoadTiles();
	LoadParticles();
	LoadProjectiles();
	LoadTerrainTextures();
	LoadHoverTex();
	LoadUnitSprites();
	BSprites();

	for(int i=0; i<MODELS; i++)
	{
		if(g_model[i].on)
			g_model[i].ReloadTexture();
	}
}
#endif

void Click_LoadMapButton()
{
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("map projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("map projects\\map project", filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	FreeMap();

	if(LoadMap(filepath))
		strcpy(g_lastsave, filepath);
}

void Click_SaveMapButton()
{
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps\\map", filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	SaveMap(filepath);
	strcpy(g_lastsave, filepath);
}

void Click_QSaveMapButton()
{
	if(g_lastsave[0] == '\0')
	{
		Click_SaveMapButton();
		return;
	}
	
	SaveMap(g_lastsave);
}

void Resize_NewGameLink(Widget* thisw)
{
}

void Resize_LoadingStatus(Widget* thisw)
{
	thisw->m_pos[0] = g_width/2;
	thisw->m_pos[1] = g_height/2;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
	thisw->m_tpos[0] = g_width/2;
	thisw->m_tpos[1] = g_height/2;
}


void Click_NewGame()
{
#if 0
	LoadJPGMap("heightmaps/heightmap0e2.jpg");
#elif 1
	LoadJPGMap("heightmaps/heightmap0c.jpg");
#else
	char fullpath[MAX_PATH+1];
	FullPath("maps/testmap", fullpath);
	//FullPath("maps/bigmap256", fullpath);
	LoadMap(fullpath);
#endif

	g_mode = PLAY;

	OpenSoleView("play gui");
	OpenAnotherView("play right opener");

	g_lastsave[0] = '\0';
}

void Click_OpenEditor()
{
	LoadJPGMap("heightmaps/heightmap0e2.jpg");
	
	g_mode = EDITOR;

	OpenSoleView("editor gui");

	g_lastsave[0] = '\0';
}

void FillMenuGUI()
{
	// Main View
	View* mainmenuview = AddView("mainmenu");

	mainmenuview->widget.push_back(new Link(NULL, "new game link", RichText("New Game"), MAINFONT8, Resize_NewGameLink, Click_NewGame));
}

void StartRoadPlacement()
{
	g_vdrag[0] = Vec3f(-1,-1,-1);
	g_vdrag[1] = Vec3f(-1,-1,-1);

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, g_camera.zoompos(), g_camera.m_strafe, g_camera.up2(), g_camera.m_view - g_camera.m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = g_camera.zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[0] = intersection;
	g_vdrag[1] = intersection;
}

void MouseLDown()
{
	if(g_mode == EDITOR)
	{
		int edtool = GetEdTool();

		if(edtool == EDTOOL_PLACEROADS ||
			edtool == EDTOOL_PLACECRUDEPIPES ||
			edtool == EDTOOL_PLACEPOWERLINES)
		{
			StartRoadPlacement();
		}
	}
	else if(g_mode == PLAY)
	{
		g_mousestart = g_mouse;
	}
}

void EdPlaceUnit()
{
	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, g_camera.zoompos(), g_camera.m_strafe, g_camera.up2(), g_camera.m_view - g_camera.m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = g_camera.zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	int type = GetPlaceUnitType();

	int country = GetPlaceUnitCountry();
	int company = GetPlaceUnitCompany();

	PlaceUnit(type, Vec3i(intersection.x, intersection.y, intersection.z), country, company, -1);
#if 0
	g_hmap.setheight( intersection.x / TILE_SIZE, intersection.z / TILE_SIZE, 0);
	g_hmap.remesh();
#endif
}

void EdPlaceBuilding()
{
	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, g_camera.zoompos(), g_camera.m_strafe, g_camera.up2(), g_camera.m_view - g_camera.m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = g_camera.zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	if(intersection.y < WATER_LEVEL && !GetMapIntersection2(&g_hmap, line, &intersection))
		return;


	int type = GetPlaceBuildingType();

	Vec2i tilepos (intersection.x/TILE_SIZE, intersection.z/TILE_SIZE);

	if(CheckCanPlace(type, tilepos))
		PlaceBuilding(type, tilepos, true, -1, -1, -1);
#if 0
	g_hmap.setheight( intersection.x / TILE_SIZE, intersection.z / TILE_SIZE, 0);
	g_hmap.remesh();
#endif
}

void EdDeleteObject()
{
	Selection sel = DoSelection(g_camera.zoompos(), g_camera.m_strafe, g_camera.up2(), g_camera.m_view - g_camera.m_pos);

	for(auto unititer = sel.units.begin(); unititer != sel.units.end(); unititer++)
	{
		g_unit[ *unititer ].destroy();
		return;
	}
}

void MouseLUp()
{
	if(g_mode == EDITOR)
	{
		int edtool = GetEdTool();

		if(edtool == EDTOOL_PLACEUNITS)
			EdPlaceUnit();
		else if(edtool == EDTOOL_PLACEBUILDINGS)
			EdPlaceBuilding();
		else if(edtool == EDTOOL_DELETEOBJECTS)
			EdDeleteObject();
		else if(edtool == EDTOOL_PLACEROADS)
			PlaceRoad();
		else if(edtool == EDTOOL_PLACECRUDEPIPES)
			PlaceCrPipe();
		else if(edtool == EDTOOL_PLACEPOWERLINES)
			PlacePowl();
	}
	else if(g_mode == PLAY)
	{
		g_selection = DoSelection(g_camera.zoompos(), g_camera.m_strafe, g_camera.up2(), Normalize(g_camera.m_view - g_camera.zoompos()));
	}
}

void RotateAbout()
{
	float dx = g_mouse.x - g_width/2;
	float dy = g_mouse.y - g_height/2;

	Camera oldcam = g_camera;
	Vec3f line[2];
	line[0] = g_camera.zoompos();

	g_camera.rotateabout(g_camera.m_view, dy / 100.0f, g_camera.m_strafe.x, g_camera.m_strafe.y, g_camera.m_strafe.z);
	g_camera.rotateabout(g_camera.m_view, dx / 100.0f, g_camera.m_up.x, g_camera.m_up.y, g_camera.m_up.z);
	
	line[1] = g_camera.zoompos();

	Vec3f ray = Normalize(line[1] - line[0]) * TILE_SIZE;

	//line[0] = line[0] - ray;
	line[1] = line[1] + ray;

	Vec3f clip;

#if 0
	if(GetMapIntersection(&g_hmap, line, &clip))
#else
	if(FastMapIntersect(&g_hmap, line, &clip))
#endif
		g_camera = oldcam;
	else
		CalcMapView();

}

void UpdateRoadPlans()
{
	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, g_camera.zoompos(), g_camera.m_strafe, g_camera.up2(), g_camera.m_view - g_camera.m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = g_camera.zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdateRoadPlans(0, g_vdrag[0], g_vdrag[1]);
}

void UpdateCrPipePlans()
{
	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, g_camera.zoompos(), g_camera.m_strafe, g_camera.up2(), g_camera.m_view - g_camera.m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = g_camera.zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdateCrPipePlans(0, g_vdrag[0], g_vdrag[1]);
}

void UpdatePowlPlans()
{
	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, g_camera.zoompos(), g_camera.m_strafe, g_camera.up2(), g_camera.m_view - g_camera.m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = g_camera.zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdatePowlPlans(0, g_vdrag[0], g_vdrag[1]);
}

void MouseMove()
{
	if(g_mode == PLAY || g_mode == EDITOR)
	{
		if(g_mousekeys[MOUSEKEY_MIDDLE])
		{
			RotateAbout();
			CenterMouse();
		}

		else if(g_mousekeys[MOUSEKEY_LEFT])
		{
			int edtool = GetEdTool();

			if(edtool == EDTOOL_PLACEROADS)
			{
				UpdateRoadPlans();
			}
			else if(edtool == EDTOOL_PLACECRUDEPIPES)
			{
				UpdateCrPipePlans();
			}
			else if(edtool == EDTOOL_PLACEPOWERLINES)
			{
				UpdatePowlPlans();
			}
		}
	}
}

void MouseRDown()
{
}

void MouseRUp()
{
	if(g_mode == PLAY)
	{
		if(!g_keyintercepted)
		{
			Order(g_mouse.x, g_mouse.y, g_width, g_height, g_camera.zoompos(), g_camera.m_view, Normalize(g_camera.m_view - g_camera.zoompos()), g_camera.m_strafe, g_camera.up2());
		}
	}
}

void FillGUI()
{
	//DrawSceneFunc = DrawScene;
	//DrawSceneDepthFunc = DrawSceneDepth;
	
	AssignKey(VK_F1, SaveScreenshot, NULL);
	AssignKey(VK_F2, LogPathDebug, NULL);
	AssignLButton(MouseLDown, MouseLUp);
	AssignRButton(MouseRDown, MouseRUp);
	AssignMouseMove(MouseMove);

	// Loading View

	View* loadingview = AddView("loading");

	loadingview->widget.push_back(new Text(NULL, "status", RichText("Loading..."), MAINFONT8, Resize_LoadingStatus));
	
	OpenSoleView("loading");

	FillMenuGUI();
	FillEditorGUI();
	FillPlayGUI();
}