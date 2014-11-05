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
#include "../../common/sim/powl.h"
#include "../../common/sim/crpipe.h"
#include "../../common/render/water.h"
#include "../../common/sim/order.h"
#include "../../common/path/pathdebug.h"
#include "../../common/path/pathnode.h"
#include "../../common/sim/build.h"
#include "../../common/sim/player.h"
#include "../../common/gui/widgets/windoww.h"
#include "../../common/debug.h"
#include "../../common/script/console.h"
#include "../../common/window.h"

//bool g_canselect = true;

char g_lastsave[MAX_PATH+1];

void Resize_Fullscreen(Widget* thisw)
{
	Player* py = &g_player[g_curP];

	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = py->width-1;
	thisw->m_pos[3] = py->height-1;
}

void Click_LoadMapButton()
{
#ifdef PLATFORM_WIN
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("map projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("map projects/map project", filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize     = sizeof ( ofn );
	ofn.hwndOwner       = NULL;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile       = filepath;
	//ofn.lpstrFile[0]  = '\0';
	ofn.nMaxFile        = sizeof( filepath );
	//ofn.lpstrFilter   = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter     = "All\0*.*\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFileTitle  = NULL;
	ofn.nMaxFileTitle   = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags           = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	FreeMap();

	if(LoadMap(filepath))
		strcpy(g_lastsave, filepath);
#endif //PLATFORM_WIN
}

void Click_SaveMapButton()
{
#ifdef PLATFORM_WIN
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps/map", filepath);
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
#endif //PLATFORM_WIN
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

void Resize_MenuItem(Widget* thisw)
{
	int row;
	sscanf(thisw->m_name.c_str(), "%d", &row);
	Font* f = &g_font[thisw->m_font];

	Player* py = &g_player[g_curP];
	
	thisw->m_pos[0] = py->width/2 - f->gheight*4;
	thisw->m_pos[1] = py->height/2 - f->gheight*4 + f->gheight*1.5f*row;
	thisw->m_pos[2] = py->width;
	thisw->m_pos[3] = py->height;
}

void Resize_LoadingStatus(Widget* thisw)
{
	Player* py = &g_player[g_curP];

	thisw->m_pos[0] = py->width/2;
	thisw->m_pos[1] = py->height/2;
	thisw->m_pos[2] = py->width;
	thisw->m_pos[3] = py->height;
	thisw->m_tpos[0] = py->width/2;
	thisw->m_tpos[1] = py->height/2;
}

void Resize_WinText(Widget* thisw)
{
	Widget* parent = thisw->m_parent;

	thisw->m_pos[0] = parent->m_pos[0];
	thisw->m_pos[1] = parent->m_pos[1];
	thisw->m_pos[2] = thisw->m_pos[0] + 400;
	thisw->m_pos[3] = thisw->m_pos[1] + 2000;
}

void Click_HostGame()
{
	g_mode = APPMODE_PLAY;

	g_netmode = NET_HOST;

	if(!(g_svsock = SDLNet_UDP_Open(PORT)))
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		ErrorMessage("Error", msg);
		return;
	}

	//contact matcher TO DO
}

void Click_JoinGame()
{
	g_mode = APPMODE_PLAY;

	g_netmode = NET_HOST;

	IPaddress ip;

	if(SDLNet_ResolveHost(&ip, "localhost", PORT) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		ErrorMessage("Error", msg);
		return;
	}

	if(!(g_svsock = SDLNet_UDP_Open(0)))
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		ErrorMessage("Error", msg);
		return;
	}

	if(SDLNet_UDP_Bind(g_svsock, 0, &ip) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		ErrorMessage("Error", msg);
		return;
	}
}

void Click_NewGame()
{
	CheckGLError(__FILE__, __LINE__);
#if 1
	//LoadJPGMap("heightmaps/heightmap0e2s.jpg");
	//LoadJPGMap("heightmaps/heightmap0e2.jpg");
	LoadJPGMap("heightmaps/heightmap0e.jpg");
#elif 1
	LoadJPGMap("heightmaps/heightmap0c.jpg");
#else
	char fullpath[MAX_PATH+1];
	FullPath("maps/testmap", fullpath);
	//FullPath("maps/bigmap256", fullpath);
	LoadMap(fullpath);
#endif

	CheckGLError(__FILE__, __LINE__);

	for(int i=0; i<10; i++)
		for(int j=0; j<10; j++)
	//for(int i=0; i<15; i++)
	//	for(int j=0; j<1; j++)
	//for(int i=0; i<1; i++)
		//for(int j=0; j<1; j++)
		{

			//Vec3i cmpos((g_hmap.m_widthx+4)*TILE_SIZE/2 + (i+2)*PATHNODE_SIZE, 0, g_hmap.m_widthz*TILE_SIZE/2 + (j+2)*PATHNODE_SIZE);
			//cmpos.y = g_hmap.accheight(cmpos.x, cmpos.z);
			Vec2i cmpos((g_hmap.m_widthx+4)*TILE_SIZE/2 + (i+2)*PATHNODE_SIZE*4, g_hmap.m_widthz*TILE_SIZE/2 + (j+2)*PATHNODE_SIZE*4);

			//if(rand()%2 == 1)
			//	PlaceUnit(UNIT_ROBOSOLDIER, cmpos, 0);
			//else
			PlaceUnit(UNIT_LABOURER, cmpos, -1, NULL);
		}

	CheckGLError(__FILE__, __LINE__);

	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		p->on = true;
		p->ai = (i == g_localP) ? false : true;

		p->global[RES_DOLLARS] = 4000;
		p->global[RES_FARMPRODUCTS] = 4000;
		p->global[RES_RETFOOD] = 4000;
		p->global[RES_CEMENT] = 4000;
		p->global[RES_FUEL] = 4000;
		p->global[RES_URANIUM] = 4000;

#if 0
#define RES_DOLLARS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_PRODUCTION		5
#define RES_CEMENT		6
#define RES_CRUDEOIL		7
#define RES_FUEL			8
#define RES_FUEL			9
#define RES_ENERGY			10
#define RES_URANIUM			11
#define RESOURCES			12
#endif
	}

#if 0
	PlaceBl(BL_HARBOUR, Vec2i(g_hmap.m_widthx/2-1, g_hmap.m_widthz/2-3), true, 0);
	PlaceBl(BL_APARTMENT, Vec2i(g_hmap.m_widthx/2+2, g_hmap.m_widthz/2-2), true, 0);
	PlaceBl(BL_APARTMENT, Vec2i(g_hmap.m_widthx/2+4, g_hmap.m_widthz/2-3), true, 0);
	PlaceBl(BL_APARTMENT, Vec2i(g_hmap.m_widthx/2+6, g_hmap.m_widthz/2-3), true, 0);
	PlaceRoad(g_hmap.m_widthx/2+1, g_hmap.m_widthz/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+2, g_hmap.m_widthz/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+3, g_hmap.m_widthz/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+3, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+4, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+5, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+6, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthz/2-3, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthz/2-4, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthz/2-5, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthz/2-6, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthz/2-7, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+8, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+9, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+10, g_hmap.m_widthz/2-2, 1, false);
	PlaceBl(BL_FACTORY, Vec2i(g_hmap.m_widthx/2+9, g_hmap.m_widthz/2-3), true, 0);
	PlaceBl(BL_REFINERY, Vec2i(g_hmap.m_widthx/2+11, g_hmap.m_widthz/2-3), true, 0);
	PlaceBl(BL_NUCPOW, Vec2i(g_hmap.m_widthx/2+13, g_hmap.m_widthz/2-3), true, 0);
	PlaceRoad(g_hmap.m_widthx/2+11, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+12, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+13, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthz/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthz/2-3, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthz/2-4, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthz/2-5, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthz/2-6, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthz/2-7, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+15, g_hmap.m_widthz/2-2, 1, false);
	PlaceBl(BL_FARM, Vec2i(g_hmap.m_widthx/2+6, g_hmap.m_widthz/2-0), true, 0);
	PlaceRoad(g_hmap.m_widthx/2+10, g_hmap.m_widthz/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+10, g_hmap.m_widthz/2-0, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+10, g_hmap.m_widthz/2+1, 1, false);
	PlaceBl(BL_STORE, Vec2i(g_hmap.m_widthx/2+9, g_hmap.m_widthz/2-1), true, 0);
	PlaceBl(BL_OILWELL, Vec2i(g_hmap.m_widthx/2+9, g_hmap.m_widthz/2-0), true, 0);
	PlaceBl(BL_MINE, Vec2i(g_hmap.m_widthx/2+11, g_hmap.m_widthz/2-0), true, 0);
	PlaceBl(BL_MINE, Vec2i(g_hmap.m_widthx/2+12, g_hmap.m_widthz/2-0), true, 0);
	PlaceRoad(g_hmap.m_widthx/2+13, g_hmap.m_widthz/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+13, g_hmap.m_widthz/2-0, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+13, g_hmap.m_widthz/2+1, 1, false);
	CheckGLError(__FILE__, __LINE__);
	PlaceBl(BL_GASSTATION, Vec2i(g_hmap.m_widthx/2+14, g_hmap.m_widthz/2-1), true, 0);
	g_hmap.genvbo();
#endif

	CheckGLError(__FILE__, __LINE__);

	g_mode = APPMODE_PLAY;

	Player* py = &g_player[g_curP];
	GUI* gui = &py->gui;

	gui->closeall();
	gui->open("play gui");
	gui->open("play right opener");

	CheckGLError(__FILE__, __LINE__);

	gui->add(new WindowW(gui, "window", Resize_Window));
	//gui->open("window");

	WindowW* win = (WindowW*)gui->get("window");

	RichText bigtext;

	for(int i=0; i<2000; i++)
	{
		bigtext.m_part.push_back(RichPart((unsigned int)(rand()%3000+16000)));

		if(rand()%10 == 1)
			bigtext.m_part.push_back(RichPart((unsigned int)'\n'));
	}

	win->add(new TextBlock(win, "text block", bigtext, MAINFONT16, Resize_WinText));

	g_lastsave[0] = '\0';
}

void Click_OpenEditor()
{
	CheckGLError(__FILE__, __LINE__);
	LoadJPGMap("heightmaps/heightmap0e2.jpg");
	CheckGLError(__FILE__, __LINE__);

	g_mode = APPMODE_EDITOR;


	Player* py = &g_player[g_curP];
	GUI* gui = &py->gui;

	gui->closeall();
	gui->open("editor gui");

	g_lastsave[0] = '\0';
}

void FillMenuGUI()
{
	Player* py = &g_player[g_curP];
	GUI* gui = &py->gui;

	// Main ViewLayer
	gui->add(new ViewLayer(gui, "main"));
	ViewLayer* mainview = (ViewLayer*)gui->get("main");

	mainview->add(new Image(mainview, "gui/mmbg.jpg", true, Resize_Fullscreen));
	
	mainview->add(new Link(mainview, "0", RichText("New Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_NewGame));
	mainview->add(new Link(mainview, "1", RichText("Host Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_HostGame));
	mainview->add(new Link(mainview, "2", RichText("Join Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_JoinGame));
}

void StartRoadPlacement()
{
	Player* py = &g_player[g_curP];
	Camera* c = &py->camera;

	py->vdrag[0] = Vec3f(-1,-1,-1);
	py->vdrag[1] = Vec3f(-1,-1,-1);

	Vec3f ray = ScreenPerspRay(py->mouse.x, py->mouse.y, py->width, py->height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / py->zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	py->vdrag[0] = intersection;
	py->vdrag[1] = intersection;
}

void MouseLDown()
{
	if(g_mode == APPMODE_EDITOR)
	{
		int edtool = GetEdTool();

		if(edtool == EDTOOL_PLACEROADS ||
		                edtool == EDTOOL_PLACECRUDEPIPES ||
		                edtool == EDTOOL_PLACEPOWERLINES)
		{
			StartRoadPlacement();
		}
	}
	else if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_curP];
		py->mousestart = py->mouse;
	}
}

void EdPlaceUnit()
{
	Player* py = &g_player[g_curP];
	Camera* c = &py->camera;

	Vec3f ray = ScreenPerspRay(py->mouse.x, py->mouse.y, py->width, py->height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / py->zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	int type = GetPlaceUnitType();

	int country = GetPlaceUnitCountry();
	int company = GetPlaceUnitCompany();

	//PlaceUnit(type, Vec3i(intersection.x, intersection.y, intersection.z), country, company, -1);
	PlaceUnit(type, Vec2i(intersection.x, intersection.z), country, NULL);
#if 0
	g_hmap.setheight( intersection.x / TILE_SIZE, intersection.z / TILE_SIZE, 0);
	g_hmap.remesh();
#endif
}

void EdPlaceBuilding()
{
	Player* py = &g_player[g_curP];
	Camera* c = &py->camera;

	Vec3f ray = ScreenPerspRay(py->mouse.x, py->mouse.y, py->width, py->height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / py->zoom;

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
		PlaceBl(type, tilepos, true, -1, NULL);
#if 0
	g_hmap.setheight( intersection.x / TILE_SIZE, intersection.z / TILE_SIZE, 0);
	g_hmap.remesh();
#endif
}

void EdDeleteObject()
{
	Player* py = &g_player[g_curP];
	Camera* c = &py->camera;

	Selection sel = DoSel(c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos);

	for(auto unititer = sel.units.begin(); unititer != sel.units.end(); unititer++)
	{
		g_unit[ *unititer ].destroy();
		return;
	}
}

void MouseLUp()
{
	if(g_mode == APPMODE_EDITOR)
	{
		int edtool = GetEdTool();

		if(edtool == EDTOOL_PLACEUNITS)
			EdPlaceUnit();
		else if(edtool == EDTOOL_PLACEBUILDINGS)
			EdPlaceBuilding();
		else if(edtool == EDTOOL_DELETEOBJECTS)
			EdDeleteObject();
		else if(edtool == EDTOOL_PLACEROADS)
			PlaceCo(CONDUIT_ROAD);
		else if(edtool == EDTOOL_PLACECRUDEPIPES)
			PlaceCo(CONDUIT_CRPIPE);
		else if(edtool == EDTOOL_PLACEPOWERLINES)
			PlaceCo(CONDUIT_POWL);
	}
	else if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_curP];
		Camera* c = &py->camera;
		GUI* gui = &py->gui;

		if(py->build == BL_NONE)
		{
			gui->close("construction view");
			gui->close("building view");
			py->sel = DoSel(c->zoompos(), c->m_strafe, c->up2(), Normalize(c->m_view - c->zoompos()));
			AfterSel(&py->sel);
		}
		else if(py->build < BL_TYPES)
		{
			if(py->canplace)
			{
				PlaceBl(py->build, Vec2i(py->vdrag[0].x/TILE_SIZE, py->vdrag[0].z/TILE_SIZE), false, g_localP, NULL);
			}
			else
			{
			}

			py->build = -1;
		}
		else if(py->build >= BL_TYPES && py->build < BL_TYPES+CONDUIT_TYPES)
		{
			//g_log<<"place r"<<std::endl;
			PlaceCo(py->build - BL_TYPES);
			py->build = -1;
		}
	}
}

void RotateAbout()
{
	Player* py = &g_player[g_curP];
	Camera* c = &py->camera;

	float dx = py->mouse.x - py->width/2;
	float dy = py->mouse.y - py->height/2;

	Camera oldcam = py->camera;
	Vec3f line[2];
	line[0] = c->zoompos();

	c->rotateabout(c->m_view, dy / 100.0f, c->m_strafe.x, c->m_strafe.y, c->m_strafe.z);
	c->rotateabout(c->m_view, dx / 100.0f, c->m_up.x, c->m_up.y, c->m_up.z);

	line[1] = c->zoompos();

	Vec3f ray = Normalize(line[1] - line[0]) * TILE_SIZE;

	//line[0] = line[0] - ray;
	line[1] = line[1] + ray;

	Vec3f clip;

#if 0
	if(GetMapIntersection(&g_hmap, line, &clip))
#else
	if(FastMapIntersect(&g_hmap, line, &clip))
#endif
		py->camera = oldcam;
	//else
	//	CalcMapView();

}

void UpdateRoadPlans()
{
	Player* py = &g_player[g_curP];
	Camera* c = &py->camera;

	py->vdrag[1] = py->vdrag[0];

	Vec3f ray = ScreenPerspRay(py->mouse.x, py->mouse.y, py->width, py->height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / py->zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	py->vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_ROAD, 0, py->vdrag[0], py->vdrag[1]);
}

void UpdateCrPipePlans()
{
	Player* py = &g_player[g_curP];
	Camera* c = &py->camera;

	py->vdrag[1] = py->vdrag[0];

	Vec3f ray = ScreenPerspRay(py->mouse.x, py->mouse.y, py->width, py->height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / py->zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	py->vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_CRPIPE, 0, py->vdrag[0], py->vdrag[1]);
}

void UpdatePowlPlans()
{
	Player* py = &g_player[g_curP];
	Camera* c = &py->camera;

	py->vdrag[1] = py->vdrag[0];

	Vec3f ray = ScreenPerspRay(py->mouse.x, py->mouse.y, py->width, py->height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / py->zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	py->vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_POWL, 0, py->vdrag[0], py->vdrag[1]);
}

void MouseMove()
{
	if(g_mode == APPMODE_PLAY || g_mode == APPMODE_EDITOR)
	{
		Player* py = &g_player[g_curP];

		if(py->mousekeys[MOUSE_MIDDLE])
		{
			RotateAbout();
			CenterMouse();
		}

		UpdSBl();

		if(py->mousekeys[MOUSE_LEFT])
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
	if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_curP];
		Camera* c = &py->camera;

		if(!py->keyintercepted)
		{
			Order(py->mouse.x, py->mouse.y, py->width, py->height, c->zoompos(), c->m_view, Normalize(c->m_view - c->zoompos()), c->m_strafe, c->up2());
		}
	}
}

void FillGUI()
{
	//DrawSceneFunc = DrawScene;
	//DrawSceneDepthFunc = DrawSceneDepth;

	g_log<<"2.1"<<std::endl;
	g_log.flush();

	for(int i=0; i<PLAYERS; i++)
	{
		g_log<<"2.1.1"<<std::endl;
		g_log.flush();

		Player* py = &g_player[i];
		GUI* gui = &py->gui;

		g_log<<"2.1.2"<<std::endl;
		g_log.flush();

		gui->assignkey(SDL_SCANCODE_F1, SaveScreenshot, NULL);
		gui->assignkey(SDL_SCANCODE_F2, LogPathDebug, NULL);
		gui->assignlbutton(MouseLDown, MouseLUp);
		gui->assignrbutton(MouseRDown, MouseRUp);
		gui->assignmousemove(MouseMove);

		g_log<<"2.1.3"<<std::endl;
		g_log.flush();
	}

	g_log<<"2.2"<<std::endl;
	g_log.flush();

	// Loading ViewLayer
	Player* py = &g_player[g_curP];
	GUI* gui = &py->gui;

	g_log<<"2.2"<<std::endl;
	g_log.flush();

	gui->add(new ViewLayer(gui, "loading"));
	ViewLayer* loadview = (ViewLayer*)gui->get("loading");

	g_log<<"2.3"<<std::endl;
	g_log.flush();

	loadview->add(new Image(loadview, "gui/mmbg.jpg", true, Resize_Fullscreen));
	loadview->add(new Text(NULL, "status", RichText("Loading..."), MAINFONT8, Resize_LoadingStatus));

	gui->closeall();
	gui->open("loading");

	g_log<<"2.4"<<std::endl;
	g_log.flush();

	FillMenuGUI();

	g_log<<"2.5"<<std::endl;
	g_log.flush();

	FillEditorGUI();

	g_log<<"2.6"<<std::endl;
	g_log.flush();

	FillPlayGUI();

	g_log<<"2.7"<<std::endl;
	g_log.flush();

	FillConsole();
}
