





#include "gmain.h"
//#include "3dmath.h"
//#include "gui.h"
//#include "image.h"
//#include "menu.h"
//#include "player.h"
//#include "projectile.h"
//#include "model.h"
//#include "save.h"
//#include "editor.h"
//#include "script.h"
#include "keymap.h"
//#include "inspect.h"
//#include "selection.h"

//bool g_canselect = true;

/*
void Click_OpenLogin()
{
	OpenSoleView("login");
}

void Click_Login()
{
	CSpawn spawn = g_spawn[0];
	int e;
	bool place = PlaceEntity(HUMAN1, g_localP, spawn.pos, spawn.angle, &e);
	g_camera = &g_entity[e].camera;
	if(!place)
		g_player[g_localP].crouched = true;
	g_mode = PLAY;
	RedoGUI();
}*/

#if 0
void Click_SaveSlot1()
{
	SaveSlot(1);
	Click_OpenSave();
}

void Click_SaveSlot2()
{
	SaveSlot(2);
	Click_OpenSave();
}

void Click_SaveSlot3()
{
	SaveSlot(3);
	Click_OpenSave();
}

void Click_LoadSlot1()
{
	if(!LoadSlot(1))
		return;

	g_mode = PLAY;
	OpenPlay();
}

void Click_LoadSlot2()
{
	if(!LoadSlot(2))
		return;

	g_mode = PLAY;
	OpenPlay();
}

void Click_LoadSlot3()
{
	if(!LoadSlot(3))
		return;

	g_mode = PLAY;
	OpenPlay();
}

void Click_BackToInGame()
{
	OpenSoleView("ingame");
}

void Click_OpenSave()
{
	CView* v = g_GUI.getview("save");
	
	Widget* slot1 = v->getlink(0);
	Widget* slot2 = v->getlink(1);
	Widget* slot3 = v->getlink(2);

	//FILE* fp = fopen("saves\\slot1.sav", "rb");
	//if(!fp)
	if(!LoadMap("saves\\slot1.sav", true))
		slot1->text = "   Slot 1";
	else
	{
		char name[32];
		//FILE* fp = fopen("saves\\slot1.sav", "rb");
		//fread(name, sizeof(char), 32, fp);
		char full[MAX_PATH+1];
		FullPath("saves\\slot1.sav", full);
		MapName(full, name);
		slot1->text = name;
		//fclose(fp);
		//fp = NULL;
	}

	//fp = fopen("saves\\slot2.sav", "rb");
	//if(!fp)
	if(!LoadMap("saves\\slot2.sav", true))
		slot2->text = "   Slot 2";
	else
	{
		char name[32];
		//FILE* fp = fopen("saves\\slot2.sav", "rb");
		//fread(name, sizeof(char), 32, fp);
		char full[MAX_PATH+1];
		FullPath("saves\\slot2.sav", full);
		MapName(full, name);
		slot2->text = name;
		//fclose(fp);
		//fp = NULL;
	}

	//fp = fopen("saves\\slot3.sav", "rb");
	//if(!fp)
	if(!LoadMap("saves\\slot3.sav", true))
		slot3->text = "   Slot 3";
	else
	{
		char name[32];
		//FILE* fp = fopen("saves\\slot3.sav", "rb");
		//fread(name, sizeof(char), 32, fp);
		char full[MAX_PATH+1];
		FullPath("saves\\slot3.sav", full);
		MapName(full, name);
		slot3->text = name;
		//fclose(fp);
		//fp = NULL;
	}

	OpenSoleView("save");
}

void Click_OpenLoad()
{
	g_gameover = false;
	CView* v = g_GUI.getview("load");
	
	Widget* slot1 = v->getlink(0);
	Widget* slot2 = v->getlink(1);
	Widget* slot3 = v->getlink(2);

	//FILE* fp = fopen("saves\\slot1.sav", "rb");
	//if(!fp)
	if(!LoadMap("saves\\slot1.sav", true))
		slot1->text = "   Slot 1";
	else
	{
		char name[32];
		//FILE* fp = fopen("saves\\slot1.sav", "rb");
		//fread(name, sizeof(char), 32, fp);
		char full[MAX_PATH+1];
		FullPath("saves\\slot1.sav", full);
		MapName(full, name);
		slot1->text = name;
		//fclose(fp);
		//fp = NULL;
	}

	//fp = fopen("saves\\slot2.sav", "rb");
	//if(!fp)
	if(!LoadMap("saves\\slot2.sav", true))
		slot2->text = "   Slot 2";
	else
	{
		char name[32];
		//FILE* fp = fopen("saves\\slot2.sav", "rb");
		//fread(name, sizeof(char), 32, fp);
		char full[MAX_PATH+1];
		FullPath("saves\\slot2.sav", full);
		MapName(full, name);
		slot2->text = name;
		//fclose(fp);
		//fp = NULL;
	}

	//fp = fopen("saves\\slot3.sav", "rb");
	//if(!fp)
	if(!LoadMap("saves\\slot3.sav", true))
		slot3->text = "   Slot 3";
	else
	{
		char name[32];
		//FILE* fp = fopen("saves\\slot3.sav", "rb");
		//fread(name, sizeof(char), 32, fp);
		char full[MAX_PATH+1];
		FullPath("saves\\slot3.sav", full);
		MapName(full, name);
		slot3->text = name;
		//fclose(fp);
		//fp = NULL;
	}

	OpenSoleView("load");
}

void Click_Play()
{
	g_build = NOTHING;
	g_canselect = true;
	OpenSoleView("play");
}

void Click_NewGame()
{
	/*
	g_camera.PositionCamera(MAP_SIZE*TILE_SIZE/2, 86, 0, 
							MAP_SIZE*TILE_SIZE/2, 85, 1, 
							0, 1, 0);
							*/
	
	g_camera.PositionCamera(g_hmap.m_widthX*TILE_SIZE/2, 86, g_hmap.m_widthZ*TILE_SIZE, 
							g_hmap.m_widthX*TILE_SIZE/2, 85, g_hmap.m_widthZ*TILE_SIZE-1, 
							0, 1, 0);

	/*
	g_camera.PositionCamera(MAP_SIZE*TILE_SIZE/2, TILE_SIZE*3.0f, MAP_SIZE*TILE_SIZE/2, 
							MAP_SIZE*TILE_SIZE/2, TILE_SIZE*3.0f-3*TILE_SIZE, MAP_SIZE*TILE_SIZE/2-2*TILE_SIZE, 
							0, 1, 0);
							*/

	g_selP = g_localP;
	g_canselect = true;
	g_mode = PLAY;
	g_gameover = false;
	RedoGUI();
	//RandomMap();
	FreeMap();

	char full[MAX_PATH+1];
	FullPath("levels\\level0", full);
	LoadMap(full);

	UpdateMouse3D();
}

void Click_LoadGame()
{
}

void Click_Editor()
{
	g_camera.PositionCamera(g_hmap.m_widthX*TILE_SIZE/2, 86, g_hmap.m_widthZ*TILE_SIZE, 
							g_hmap.m_widthX*TILE_SIZE/2, 85, g_hmap.m_widthZ*TILE_SIZE-1, 
							0, 1, 0);
	
	g_mode = EDITOR;
	g_build = NOTHING;
	OpenEditor();
	//RandomMap();
	FreeMap();
	UpdateMouse3D();
	Change_EdCat();
	ReGUIScript();
	FillColliderCells();
}

void Click_BackToGame()
{
	//CloseView("quit");
	g_mode = PLAY;
	OpenPlay();
}

void Click_Settings()
{
	OpenSoleView("settings");
}

void Click_Credits()
{
	OpenSoleView("credits");
}

void Click_Quit()
{
	PostQuitMessage(0);
}

void Click_BackToMain()
{
	OpenSoleView("main");
}

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

void Click_LastHint()
{
	OpenAnotherView("game message");
}

void Click_Exit()
{
	Escape();
}

void Click_OpenBuild()
{
	g_canselect = false;
	OpenAnotherView("build selector");
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

void Click_OutBInspect()
{
	CView* v = g_GUI.getview("binspect");
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

	CloseView("binspect");
}

void Click_OutManufUnits()
{
	CloseView("manufunits");
	FillBInspect(g_selection[0]);
	OpenAnotherView("binspect");
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

void OpenBSelect()
{

}
#endif

void FillGUI()
{

}