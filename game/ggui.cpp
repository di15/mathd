





#include "gmain.h"
#include "../common/gui/gui.h"
#include "keymap.h"
#include "../common/render/heightmap.h"
#include "../common/math/camera.h"
#include "../common/render/shadow.h"
#include "../common/render/screenshot.h"

//bool g_canselect = true;

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

void Resize_NewGameLink(Widget* thisw)
{
}

void Resize_LoadingStatus(Widget* thisw)
{
	thisw->m_pos[0] = g_width/2;
	thisw->m_pos[1] = g_height/2;
	thisw->m_tpos[0] = g_width/2;
	thisw->m_tpos[1] = g_height/2;
}

void LoadJPGMap(const char* relative)
{
	g_hmap.destroy();

	LoadedTex *pImage = NULL;

	char full[1024];
	FullPath(relative, full);

	pImage = LoadJPG(full);

	if(!pImage)
		return;

	g_hmap.allocate(pImage->sizeX-1, pImage->sizeY-1);

	for(int x=0; x<pImage->sizeX; x++)
	{
		for(int z=0; z<pImage->sizeY; z++)
		{
			float r = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 0 ];
			float g = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 1 ];
			float b = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 2 ];

			float y = (r+g+b)/3.0f*TILE_Y_SCALE/255.0f - TILE_Y_SCALE/2.0f;
			y = y / fabs(y) * pow(fabs(y), TILE_Y_POWER) * TILE_Y_AFTERPOW;

			g_hmap.setheight(x, z, y);
		}
	}

	g_hmap.remesh();

	Vec3f center = Vec3f( g_hmap.m_widthx * TILE_SIZE/2.0f, g_hmap.getheight(g_hmap.m_widthx/2, g_hmap.m_widthz/2), g_hmap.m_widthz * TILE_SIZE/2.0f );
	Vec3f delta = center - g_camera.m_view;
	g_camera.move(delta);
	Vec3f viewvec = g_camera.m_view - g_camera.m_pos;
	viewvec = Normalize(viewvec) * max(g_hmap.m_widthx, g_hmap.m_widthz) * TILE_SIZE;
	g_camera.m_pos = g_camera.m_view - viewvec;
	g_zoom = INI_ZOOM;

	if(pImage)
	{
		if (pImage->data)							// If there is texture data
		{
			free(pImage->data);						// Free the texture data, we don't need it anymore
		}

		free(pImage);								// Free the image structure

		g_log<<relative<<"\n\r";
		g_log.flush();
	}
}

void Click_NewGame()
{
	g_hmap.allocate(128, 128);

	LoadJPGMap("heightmaps/heightmap0c.jpg");

	g_camera.position(
		-1000.0f/3, 1000.0f/3 + 5000, -1000.0f/3, 
		0, 5000, 0, 
		0, 1, 0);
	
	g_camera.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	g_camera.move( Vec3f(g_hmap.m_widthx*TILE_SIZE/2, 1000, g_hmap.m_widthz*TILE_SIZE/2) );

	g_mode = PLAY;

	OpenSoleView("play gui");
}

void Resize_ResNamesTextBlock(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_ResAmtsTextBlock(Widget* thisw)
{
	thisw->m_pos[0] = 300;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_ResDeltasTextBlock(Widget* thisw)
{
	thisw->m_pos[0] = 500;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

#define CORNER_PANEL_SIZE	200

void Resize_BottRightPanel(Widget* thisw)
{
	thisw->m_pos[0] = g_width - CORNER_PANEL_SIZE;
	thisw->m_pos[1] = g_height - CORNER_PANEL_SIZE;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void FillGUI()
{
	DrawSceneFunc = DrawScene;
	DrawSceneDepthFunc = DrawSceneDepth;

	AssignKey(VK_F1, SaveScreenshot, NULL);

	// Loading View

	View* loadingview = AddView("loading");

	loadingview->widget.push_back(new Text(NULL, "status", RichText("Loading..."), MAINFONT8, Resize_LoadingStatus));

	// Main View

	View* mainmenuview = AddView("mainmenu");

	mainmenuview->widget.push_back(new Link(NULL, "new game link", RichText("New Game"), MAINFONT8, Resize_NewGameLink, Click_NewGame));

	OpenSoleView("loading");

	// Play GUI

	View* playguiview = AddView("play gui");

	/*
	DefineIcon(ICON_DOLLAR, "gui/icons/dollars.png", ":funds:");
	DefineIcon(ICON_LABOUR, "gui/icons/labour.png", ":labour:");
	DefineIcon(ICON_HOUSING, "gui/icons/housing.png", ":housing:");
	DefineIcon(ICON_FARMPRODUCT, "gui/icons/farmproducts.png", ":farmprod:");
	DefineIcon(ICON_FOOD, "gui/icons/food.png", ":food:");
	DefineIcon(ICON_CHEMICALS, "gui/icons/chemicals.png", ":chemicals:");
	DefineIcon(ICON_ELECTRONICS, "gui/icons/electronics.png", ":electronics:");
	DefineIcon(ICON_RESEARCH, "gui/icons/research.png", ":research:");
	DefineIcon(ICON_PRODUCTION, "gui/icons/production.png", ":production:");
	DefineIcon(ICON_IRONORE, "gui/icons/ironore.png", ":ironore:");
	DefineIcon(ICON_URANIUMORE, "gui/icons/uraniumore.png", ":uraniumore:");
	DefineIcon(ICON_STEEL, "gui/icons/steel.png", ":steel:");
	DefineIcon(ICON_CRUDEOIL, "gui/icons/crudeoil.png", ":crudeoil:");
	DefineIcon(ICON_WSFUEL, "gui/icons/fuelwholesale.png", ":wsfuel:");
	DefineIcon(ICON_STONE, "gui/icons/stone.png", ":stone:");
	DefineIcon(ICON_CEMENT, "gui/icons/cement.png", ":cement:");
	DefineIcon(ICON_ENERGY, "gui/icons/energy.png", ":energy:");
	DefineIcon(ICON_ENRICHEDURAN, "gui/icons/uranium.png", ":enricheduran:");
	DefineIcon(ICON_COAL, "gui/icons/coal.png", ":coal:");
	DefineIcon(ICON_TIME, "gui/icons/time.png", ":time:");
	DefineIcon(ICON_RETFUEL, "gui/icons/fuelretail.png", ":retfuel:");
	*/

	RichText resnamestext = ParseTags(RichText(
		"RESOURCE \n"\
		":funds:Funds: \n"\
		":housing:Occupancy: \n"\
		":farmprod:Farm Products: \n"\
		":food:Food: \n"
		":chemicals:Chemicals: \n"\
		":electronics:Electronics: \n"
		":research:Research: \n"\
		":production:Production: \n"\
		":ironore:Iron Ore: \n"\
		":uraniumore:Uranium Ore: \n"\
		":steel:Steel: \n"\
		":crudeoil:Crude Oil: \n"\
		":wsfuel:Wholesale Fuel: \n"\
		":retfuel:Retail Fuel: \n"\
		":stone:Stone: \n"\
		":cement:Cement: \n"\
		":energy:Energy: \n"\
		":enricheduran:Enriched Uranium: \n"\
		":coal:Coal: \n"), 
		NULL);

	
	RichText resamtstext = ParseTags(RichText(
		"AMOUNT \n"\
		"100 \n"\
		"100/231 \n"\
		"100 \n"\
		"100 \n"
		"100 \n"\
		"100 \n"
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100/146 \n"\
		"100 \n"\
		"100 \n"), 
		NULL);
	
	RichText resdeltastext = ParseTags(RichText(
		"DELTA \n"\
		"+11/:time: \n"\
		" \n"\
		"+8/:time:  \n"\
		"+8/:time: \n"
		"+8/:time: \n"\
		"+8/:time: \n"
		"+8/:time: \n"\
		"+8/:time: \n"\
		"+8/:time: \n"\
		"+8/:time: \n"\
		"+8/:time: \n"\
		"+8/:time: \n"\
		"+8/:time: \n"\
		"+8/:time: \n"\
		"+8/:time: \n"\
		"+8/:time: \n"\
		" \n"\
		"-8/:time: \n"\
		"+12/:time: \n"), 
		NULL);
	
	playguiview->widget.push_back(new TextBlock(NULL, "resnames", resnamestext, MAINFONT32, Resize_ResNamesTextBlock));
	playguiview->widget.push_back(new TextBlock(NULL, "resamts", resamtstext, MAINFONT32, Resize_ResAmtsTextBlock));
	playguiview->widget.push_back(new TextBlock(NULL, "resdeltas", resdeltastext, MAINFONT32, Resize_ResDeltasTextBlock));

	playguiview->widget.push_back(new Image(NULL, "gui/filled.jpg", Resize_BottRightPanel)); 
}