#include "../appmain.h"
#include "../../engine/gui/gui.h"
#include "../keymap.h"
#include "../../engine/render/heightmap.h"
#include "../../engine/math/hmapmath.h"
#include "../../engine/math/camera.h"
#include "../../engine/render/shadow.h"
#include "../../engine/render/screenshot.h"
#include "../../engine/save/savemap.h"
#include "../../engine/sim/unit.h"
#include "../../engine/sim/utype.h"
#include "../../engine/sim/building.h"
#include "../../engine/sim/bltype.h"
#include "../../engine/sim/selection.h"
#include "../../engine/sim/road.h"
#include "../../engine/sim/powl.h"
#include "../../engine/sim/crpipe.h"
#include "../../engine/render/water.h"
#include "../../engine/sim/order.h"
#include "../../engine/path/pathdebug.h"
#include "../../engine/path/pathnode.h"
#include "../../engine/sim/build.h"
#include "../../engine/sim/player.h"
#include "../../engine/gui/widgets/windoww.h"
#include "../../engine/debug.h"
#include "../../engine/script/console.h"
#include "../../engine/sys/window.h"
#include "../../engine/net/lockstep.h"
#include "../../engine/net/sendpackets.h"
#include "../../engine/gui/widgets/spez/svlist.h"
#include "../../engine/gui/widgets/spez/newhost.h"
#include "../../engine/gui/widgets/spez/loadview.h"
#include "../../engine/gui/widgets/spez/lobby.h"

//not engine
#include "edgui.h"
#include "playgui.h"
#include "appgui.h"

//bool g_canselect = true;

char g_lastsave[MAX_PATH+1];

void Resize_Fullscreen(Widget* thisw)
{
	Player* py = &g_player[g_localP];

	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = (float)g_width-1;
	thisw->m_pos[3] = (float)g_height-1;
}

void Click_LoadMapButton()
{
}

void Click_SaveMapButton()
{
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

	Player* py = &g_player[g_localP];
	
	//thisw->m_pos[0] = g_width/2 - f->gheight*4;
	thisw->m_pos[0] = f->gheight*4;
	thisw->m_pos[1] = g_height/2 - f->gheight*4 + f->gheight*1.5f*row;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_LoadingStatus(Widget* thisw)
{
	Player* py = &g_player[g_localP];

	thisw->m_pos[0] = g_width/2 - 64;
	thisw->m_pos[1] = g_height/2;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
	thisw->m_tpos[0] = g_width/2;
	thisw->m_tpos[1] = g_height/2;
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
	GUI* gui = &g_gui;
	Widget_Open((Widget*)gui, "new host");
}

void Click_ListHosts()
{
	//g_mode = APPMODE_PLAY;

	//g_netmode = NETM_CLIENT;
	//g_needsvlist = true;
	//g_reqdsvlist = false;

	//Connect("localhost", PORT, false, true, false, false);
	//Connect(SV_ADDR, PORT, true, false, false, false);
	
	//BegSess();

	//g_canturn = true;	//temp, assuming no clients were handshook to server when netfr0 turn happened and server didn't have any cl's to send NetTurnPacket to

	GUI* gui;

	gui = &g_gui;
	
	Widget_Open((Widget*)gui, "sv list");
}

void Click_EndGame()
{
	EndSess();
}

void Click_NewGame()
{
	GUI* gui;

	g_mode = APPMODE_PLAY;
	gui = &g_gui;
	
	Widget_CloseAll((Widget*)gui);
	Widget_Open((Widget*)gui, "play");
	Widget_Open((Widget*)gui, "play right opener");

	g_lastsave[0] = '\0';
	BegSess();
	g_netmode = NETM_SINGLE;
}

void Click_OpenEd()
{
	GUI* gui;

	g_mode = APPMODE_EDITOR;
	gui = &g_gui;
	Widget_CloseAll((Widget*)gui);
	Widget_Open((Widget*)gui, "ed");
	g_lastsave[0] = '\0';
}

void Click_LoadGame()
{
	GUI* gui = &g_gui;

	Widget_Open((Widget*)gui, "load");
	Widget_LoadView_regen((LoadView*)gui->get("load"));
}

void Click_Options()
{
}

void Click_Quit()
{
	EndSess();
	FreeMap();
	g_quit = true;
}

void Resize_JoinCancel(Widget* thisw)
{
	thisw->m_pos[0] = g_width/2 - 75;
	thisw->m_pos[1] = g_height/2 + 100 - 30;
	thisw->m_pos[2] = g_width/2 + 75;
	thisw->m_pos[3] = g_height/2 + 100 - 0;
	CenterLabel(thisw);
}
	
void Click_JoinCancel()
{

}

void FillMenu()
{
	GUI* gui;
	ViewLayer* mainview;
	Image* mmbg;
	Link* newgamel;
	Link* loadgamel;
	Link* hostgamel;
	Link* joingamel;
	Link* optionsl;
	Link* quitl;
	RichText newgamert;
	RichText loadgamert;
	RichText hostgamert;
	RichText joingamert;
	RichText optionsrt;
	RichText quitrt;
	UStr newgameus;
	UStr loadgameus;
	UStr hostgameus;
	UStr joingameus;
	UStr optionsus;
	UStr quitus;

	gui = &g_gui;

	// Main ViewLayer

	mainview = (ViewLayer*)malloc(sizeof(ViewLayer));
	mmbg = (Image*)malloc(sizeof(Image));
	newgamel = (Link*)malloc(sizeof(Link));
	loadgamel = (Link*)malloc(sizeof(Link));
	hostgamel = (Link*)malloc(sizeof(Link));
	joingamel = (Link*)malloc(sizeof(Link));
	optionsl = (Link*)malloc(sizeof(Link));
	quitl = (Link*)malloc(sizeof(Link));
	
	RichText_Init_UStr(&newgamert, &newgameus);
	RichText_Init_UStr(&loadgamert, &loadgameus);
	RichText_Init_UStr(&hostgamert, &hostgameus);
	RichText_Init_UStr(&joingamert, &joingameus);
	RichText_Init_UStr(&optionsrt, &optionsus);
	RichText_Init_UStr(&quitrt, &quitus);

	Widget_ViewLayer_Init(mainview, (Widget*)gui, "main");
	Widget_Image_Init(mmbg, (Widget*)mainview, "gui/mmbg.jpg", true, Resize_Fullscreen);

	Widget_Add((Widget*)gui, (Widget*)mainview);

	mainview->add(new Image(mainview, "gui/mmbg.jpg", true, Resize_Fullscreen));
	
	mainview->add(new Link(mainview, "0", RichText("New Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_NewGame));
	mainview->add(new Link(mainview, "1", RichText("Load Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_LoadGame));
	mainview->add(new Link(mainview, "2", RichText("Host Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_HostGame));
	mainview->add(new Link(mainview, "3", RichText("Join Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_ListHosts));
	mainview->add(new Link(mainview, "4", RichText("Options"), FONT_EUROSTILE16, Resize_MenuItem, Click_Options));
	mainview->add(new Link(mainview, "5", RichText("Quit"), FONT_EUROSTILE16, Resize_MenuItem, Click_Quit));

	gui->add(new SvList(gui, "sv list", Resize_CenterWin));
	gui->add(new NewHost(gui, "new host", Resize_CenterWin2));
	gui->add(new Lobby(gui, "lobby"));
	
	gui->add(new ViewLayer(gui, "join"));
	ViewLayer* joinview = (ViewLayer*)gui->get("join");
	joinview->add(new Image(joinview, "gui/mmbg.jpg", true, Resize_Fullscreen));
	joinview->add(new Text(joinview, "status", RichText("Joining..."), MAINFONT16, Resize_LoadingStatus));
	joinview->add(new Button(joinview, "cancel", "gui/transp.png", RichText("Cancel"), RichText(), MAINFONT16, BUST_LINEBASED, Resize_JoinCancel, Click_JoinCancel, NULL, NULL, NULL, NULL, -1));
}

void StartRoadPlacement()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	g_vdrag[0] = Vec3f(-1,-1,-1);
	g_vdrag[1] = Vec3f(-1,-1,-1);

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
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
		Player* py = &g_player[g_localP];
		g_mousestart = g_mouse;
	}
}

void EdPlaceUnit()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
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

	//PlaceUnit(type, Vec3i(intersection.x, intersection.y, intersection.z), country, company, -1);
	PlaceUnit(type, Vec2i(intersection.x, intersection.z), country, NULL);
#if 0
	g_hmap.setheight( intersection.x / TILE_SIZE, intersection.z / TILE_SIZE, 0);
	g_hmap.remesh();
#endif
}

void EdPlaceBuilding()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
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
		PlaceBl(type, tilepos, true, -1, NULL);
#if 0
	g_hmap.setheight( intersection.x / TILE_SIZE, intersection.z / TILE_SIZE, 0);
	g_hmap.remesh();
#endif
}

void EdDeleteObject()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

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
		Player* py = &g_player[g_localP];
		Camera* c = &g_cam;
		GUI* gui = &g_gui;

		if(g_build == BL_NONE)
		{
			gui->close("cstr view");
			gui->close("bl view");
			gui->close("truck mgr");
			gui->close("save");
			gui->close("load");
			g_sel = DoSel(c->zoompos(), c->m_strafe, c->up2(), Normalize(c->m_view - c->zoompos()));
			AfterSel(&g_sel);
		}
		else if(g_build < BL_TYPES)
		{
			if(g_canplace)
			{
				//int bid;
				//PlaceBl(g_build, Vec2i(g_vdrag[0].x/TILE_SIZE, g_vdrag[0].z/TILE_SIZE), false, g_localP, &bid);

				//InfoMess("pl", "pl");

				//if(g_netmode != NETM_SINGLE)
				{
					PlaceBlPacket pbp;
					pbp.header.type = PACKET_PLACEBL;
					pbp.player = g_localP;
					pbp.btype = g_build;
					pbp.tpos = Vec2i(g_vdrag[0].x/TILE_SIZE, g_vdrag[0].z/TILE_SIZE);
					
					LockCmd((PacketHeader*)&pbp, sizeof(PlaceBlPacket));
				}
			}
			else
			{
			}

			g_build = -1;
		}
		else if(g_build >= BL_TYPES && g_build < BL_TYPES+CONDUIT_TYPES)
		{
			//g_log<<"place r"<<std::endl;
			PlaceCo(g_build - BL_TYPES);
			g_build = -1;
		}
	}
}

void RotateAbout()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	float dx = (float)( g_mouse.x - g_width/2 );
	float dy = (float)( g_mouse.y - g_height/2 );

	Camera oldcam = g_cam;
	Vec3f line[2];
	line[0] = c->zoompos();

	//c->rotateabout(c->m_view, dy / 100.0f, c->m_strafe.x, c->m_strafe.y, c->m_strafe.z);
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
		g_cam = oldcam;
	//else
	//	CalcMapView();

}

void UpdateRoadPlans()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_ROAD, 0, g_vdrag[0], g_vdrag[1]);
}

void UpdateCrPipePlans()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_CRPIPE, 0, g_vdrag[0], g_vdrag[1]);
}

void UpdatePowlPlans()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_POWL, 0, g_vdrag[0], g_vdrag[1]);
}

void MouseMove()
{
	if(g_mode == APPMODE_PLAY || g_mode == APPMODE_EDITOR)
	{
		Player* py = &g_player[g_localP];

		if(g_mousekeys[MOUSE_MIDDLE])
		{
			RotateAbout();
			CenterMouse();
		}

		UpdSBl();

		if(g_mousekeys[MOUSE_LEFT])
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
		Player* py = &g_player[g_localP];
		Camera* c = &g_cam;

		//if(!g_keyintercepted)
		{
			Order(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_view, Normalize(c->m_view - c->zoompos()), c->m_strafe, c->up2());
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
		GUI* gui = &g_gui;

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
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

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

	FillMenu();

	g_log<<"2.5"<<std::endl;
	g_log.flush();

	FillEd();

	g_log<<"2.6"<<std::endl;
	g_log.flush();

	FillPlay();

	g_log<<"2.7"<<std::endl;
	g_log.flush();

	FillConsole();
}
