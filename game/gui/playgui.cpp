

#include "../gmain.h"
#include "../../common/gui/gui.h"
#include "../keymap.h"
#include "../../common/render/heightmap.h"
#include "../../common/math/camera.h"
#include "../../common/render/shadow.h"
#include "../../common/render/screenshot.h"
#include "../../common/save/savemap.h"
#include "ggui.h"
#include "playgui.h"
#include "../../common/gui/icon.h"
#include "../../common/gui/widgets/spez/resticker.h"
#include "../../common/gui/widgets/spez/bottompanel.h"
#include "gviewport.h"

void Resize_ResNamesTextBlock(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_ResAmtsTextBlock(Widget* thisw)
{
	thisw->m_pos[0] = 150;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_ResDeltasTextBlock(Widget* thisw)
{
	thisw->m_pos[0] = 250;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

#define CORNER_PANEL_SIZE	100

void Resize_BottRightPanel(Widget* thisw)
{
	thisw->m_pos[0] = g_width - CORNER_PANEL_SIZE;
	thisw->m_pos[1] = g_height - CORNER_PANEL_SIZE;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_BuildButton_TopLeft(Widget* thisw)
{
	thisw->m_pos[0] = g_width - CORNER_PANEL_SIZE + CORNER_PANEL_SIZE/3 * 0;
	thisw->m_pos[1] = g_height - CORNER_PANEL_SIZE + CORNER_PANEL_SIZE/3 * 0;
	thisw->m_pos[2] = thisw->m_pos[0] + CORNER_PANEL_SIZE/3 * 1;
	thisw->m_pos[3] = thisw->m_pos[1] + CORNER_PANEL_SIZE/3 * 1;
}

void Resize_BuildButton_TopMid(Widget* thisw)
{
	thisw->m_pos[0] = g_width - CORNER_PANEL_SIZE + CORNER_PANEL_SIZE/3 * 1;
	thisw->m_pos[1] = g_height - CORNER_PANEL_SIZE + CORNER_PANEL_SIZE/3 * 0;
	thisw->m_pos[2] = thisw->m_pos[0] + CORNER_PANEL_SIZE/3 * 1;
	thisw->m_pos[3] = thisw->m_pos[1] + CORNER_PANEL_SIZE/3 * 1;
}

void Resize_BuildButton_TopRight(Widget* thisw)
{
	thisw->m_pos[0] = g_width - CORNER_PANEL_SIZE + CORNER_PANEL_SIZE/3 * 2;
	thisw->m_pos[1] = g_height - CORNER_PANEL_SIZE + CORNER_PANEL_SIZE/3 * 0;
	thisw->m_pos[2] = thisw->m_pos[0] + CORNER_PANEL_SIZE/3 * 1;
	thisw->m_pos[3] = thisw->m_pos[1] + CORNER_PANEL_SIZE/3 * 1;
}

void Click_BuildApartment()
{
}

void Over_BuildApartment()
{
}

void Out_Build()
{
}

#define RBUTTON_WIDTH	500
#define RBUTTON_HEIGHT	100

void Resize_RightButton0(Widget* thisw)
{
	thisw->m_pos[0] = g_width - RBUTTON_WIDTH;
	thisw->m_pos[1] = RBUTTON_HEIGHT * 0;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = thisw->m_pos[1] + RBUTTON_HEIGHT;
	CenterLabel(thisw);
}

void Resize_RightButton1(Widget* thisw)
{
	thisw->m_pos[0] = g_width - RBUTTON_WIDTH;
	thisw->m_pos[1] = RBUTTON_HEIGHT * 1;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = thisw->m_pos[1] + RBUTTON_HEIGHT;
	CenterLabel(thisw);
}

void Resize_RightButton2(Widget* thisw)
{
	thisw->m_pos[0] = g_width - RBUTTON_WIDTH;
	thisw->m_pos[1] = RBUTTON_HEIGHT * 2;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = thisw->m_pos[1] + RBUTTON_HEIGHT;
	CenterLabel(thisw);
}

void Resize_RightButton3(Widget* thisw)
{
	thisw->m_pos[0] = g_width - RBUTTON_WIDTH;
	thisw->m_pos[1] = RBUTTON_HEIGHT * 3;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = thisw->m_pos[1] + RBUTTON_HEIGHT;
	CenterLabel(thisw);
}

void Resize_RightButton4(Widget* thisw)
{
	thisw->m_pos[0] = g_width - RBUTTON_WIDTH;
	thisw->m_pos[1] = RBUTTON_HEIGHT * 4;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = thisw->m_pos[1] + RBUTTON_HEIGHT;
	CenterLabel(thisw);
}

void Resize_RightButton5(Widget* thisw)
{
	thisw->m_pos[0] = g_width - RBUTTON_WIDTH;
	thisw->m_pos[1] = RBUTTON_HEIGHT * 5;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = thisw->m_pos[1] + RBUTTON_HEIGHT;
	CenterLabel(thisw);
}

#if 0
// Can only fit 6 rows of buttons (640 pixels)
void Resize_RightButton6(Widget* thisw)
{
	thisw->m_pos[0] = g_width - RBUTTON_WIDTH;
	thisw->m_pos[1] = RBUTTON_HEIGHT * 6;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = thisw->m_pos[1] + RBUTTON_HEIGHT;
	CenterLabel(thisw);
}
#endif

void CloseRightMenu()
{
	CloseView("play right opener");
	CloseView("play right main menu 1");
	CloseView("play right menu, save/load");
}

void Click_RightMenu_SaveLoadButton()
{
	CloseRightMenu();
	OpenAnotherView("play right menu, save/load");
}

void Click_RightMenu_Open()
{
	CloseRightMenu();
	OpenAnotherView("play right main menu 1");
}

void Click_RightMenu_SaveLoad_LoadButton()
{
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("saves\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("saves\\save", filepath);
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
	{
		strcpy(g_lastsave, filepath);
		Click_RightMenu_BackToOpener();
	}
}

void Click_RightMenu_SaveLoad_SaveButton()
{
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("saves\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	string filebase = "saves\\" + FileDateTime();
	FullPath(filebase.c_str(), filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	SaveMap(filepath);
	strcpy(g_lastsave, filepath);
	Click_RightMenu_BackToOpener();
}

void Click_RightMenu_SaveLoad_QSaveButton()
{
	if(g_lastsave[0] == '\0')
	{
		Click_RightMenu_SaveLoad_SaveButton();
		return;
	}
	
	SaveMap(g_lastsave);
	Click_RightMenu_BackToOpener();
}

void Click_RightMenu_BackToOpener()
{
	CloseRightMenu();
	OpenAnotherView("play right opener");
}

void Click_RightMenu_BackToMain()
{
	CloseRightMenu();
	OpenAnotherView("play right main menu 1");
}

#define RIGHTMENUFONT		MAINFONT16

void FillPlayRightMenu()
{
	View* playrightmenu = AddView("play right opener");

	playrightmenu->widget.push_back(new Button(NULL, "name", "gui/transp.png", RichText("Menu"), RichText(""), RIGHTMENUFONT, Resize_RightButton0, Click_RightMenu_Open, NULL, NULL));
	
	playrightmenu = AddView("play right main menu 1");

	playrightmenu->widget.push_back(new Button(NULL, "name", "gui/transp.png", RichText("Save/Load"), RichText(""), RIGHTMENUFONT, Resize_RightButton0, Click_RightMenu_SaveLoadButton, NULL, NULL));
	playrightmenu->widget.push_back(new Button(NULL, "name", "gui/transp.png", RichText("Back"), RichText(""), RIGHTMENUFONT, Resize_RightButton1, Click_RightMenu_BackToOpener, NULL, NULL));
	
	playrightmenu = AddView("play right menu, save/load");
	
	playrightmenu->widget.push_back(new Button(NULL, "name", "gui/transp.png", RichText("Load Session"), RichText(""), RIGHTMENUFONT, Resize_RightButton0, Click_RightMenu_SaveLoad_LoadButton, NULL, NULL));
	playrightmenu->widget.push_back(new Button(NULL, "name", "gui/transp.png", RichText("Save Session"), RichText(""), RIGHTMENUFONT, Resize_RightButton1, Click_RightMenu_SaveLoad_SaveButton, NULL, NULL));
	playrightmenu->widget.push_back(new Button(NULL, "name", "gui/transp.png", RichText("Quick Save"), RichText(""), RIGHTMENUFONT, Resize_RightButton2, Click_RightMenu_SaveLoad_QSaveButton, NULL, NULL));
	playrightmenu->widget.push_back(new Button(NULL, "name", "gui/transp.png", RichText("Back"), RichText(""), RIGHTMENUFONT, Resize_RightButton3, Click_RightMenu_BackToMain, NULL, NULL));
	//playrightmenu->widget.push_back(new Button(NULL, "name", "gui/transp.png", RichText("Electronics Manufacturing Plant"), RichText(""), RIGHTMENUFONT, Resize_RightButton4, Click_RightMenu_BackToOpener, NULL, NULL));
}

void Resize_ResTicker(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_font[MAINFONT16].gheight+5;
	thisw->m_tpos[0] = 0;
	thisw->m_tpos[1] = 0;
}

void UpdateResTicker()
{
	static float tickerpos = 0;

	View* playguiview = g_GUI.getview("play gui");
	ResTicker* restickerw = (ResTicker*)playguiview->getwidget("res ticker", WIDGET_RESTICKER);
	Widget* restickertw = &restickerw->restext;

#if 0
	RichText restext = RichText(RichTextP(RICHTEXT_ICON, ICON_DOLLARS)) + RichText(RichTextP(" Dollars: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_EUROS)) + RichText(RichTextP(" Euros: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_POUNDS)) + RichText(RichTextP(" Pounds: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_FRANCS)) + RichText(RichTextP(" Francs: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_RUPEES)) + RichText(RichTextP(" Rupees: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_YENS)) + RichText(RichTextP(" Yens: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_ROUBLES)) + RichText(RichTextP(" Roubles: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_HOUSING)) + RichText(RichTextP(" Housing: 100/120")) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_FARMPRODUCT)) + RichText(RichTextP(" Farm Products: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_FOOD)) + RichText(RichTextP(" Food: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_CHEMICALS)) + RichText(RichTextP(" Chemicals: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_ELECTRONICS)) + RichText(RichTextP(" Electronics: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_RESEARCH)) + RichText(RichTextP(" Research: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_PRODUCTION)) + RichText(RichTextP(" Production: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_IRONORE)) + RichText(RichTextP(" Iron Ore: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_URANIUMORE)) + RichText(RichTextP(" Uranium Ore: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_STEEL)) + RichText(RichTextP(" Steel: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_CRUDEOIL)) + RichText(RichTextP(" Crude Oil: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_WSFUEL)) + RichText(RichTextP(" Wholesale Fuel: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_RETFUEL)) + RichText(RichTextP(" Retail Fuel: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_STONE)) + RichText(RichTextP(" Stone: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_CEMENT)) + RichText(RichTextP(" Cement: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_ENERGY)) + RichText(RichTextP(" Energy: 100/120")) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_ENRICHEDURAN)) + RichText(RichTextP(" Enriched Uranium: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_COAL)) + RichText(RichTextP(" Coal: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_LOGS)) + RichText(RichTextP(" Logs: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_LUMBER)) + RichText(RichTextP(" Lumber: 100 +1/")) + RichText(RichTextP(RICHTEXT_ICON, ICON_TIME)) + RichText(RichTextP("    "))
		+ RichText(RichTextP(RICHTEXT_ICON, ICON_WATER)) + RichText(RichTextP(" Water: 100/120")) + RichText(RichTextP("    "));
#endif

	RichText restext;

	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_DOLLARS)); restext.m_part.push_back(RichTextP(" Dollars: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_EUROS)); restext.m_part.push_back(RichTextP(" Euros: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_POUNDS)); restext.m_part.push_back(RichTextP(" Pounds: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_FRANCS)); restext.m_part.push_back(RichTextP(" Francs: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_RUPEES)); restext.m_part.push_back(RichTextP(" Rupees: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_YENS)); restext.m_part.push_back(RichTextP(" Yens: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_ROUBLES)); restext.m_part.push_back(RichTextP(" Roubles: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_HOUSING)); restext.m_part.push_back(RichTextP(" Housing: 100/120")); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_FARMPRODUCT)); restext.m_part.push_back(RichTextP(" Farm Products: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_FOOD)); restext.m_part.push_back(RichTextP(" Food: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_CHEMICALS)); restext.m_part.push_back(RichTextP(" Chemicals: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_ELECTRONICS)); restext.m_part.push_back(RichTextP(" Electronics: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_RESEARCH)); restext.m_part.push_back(RichTextP(" Research: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_PRODUCTION)); restext.m_part.push_back(RichTextP(" Production: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_IRONORE)); restext.m_part.push_back(RichTextP(" Iron Ore: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_URANIUMORE)); restext.m_part.push_back(RichTextP(" Uranium Ore: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_STEEL)); restext.m_part.push_back(RichTextP(" Steel: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_CRUDEOIL)); restext.m_part.push_back(RichTextP(" Crude Oil: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_WSFUEL)); restext.m_part.push_back(RichTextP(" Wholesale Fuel: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_RETFUEL)); restext.m_part.push_back(RichTextP(" Retail Fuel: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_STONE)); restext.m_part.push_back(RichTextP(" Stone: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_CEMENT)); restext.m_part.push_back(RichTextP(" Cement: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_ENERGY)); restext.m_part.push_back(RichTextP(" Energy: 100/120")); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_ENRICHEDURAN)); restext.m_part.push_back(RichTextP(" Enriched Uranium: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_COAL)); restext.m_part.push_back(RichTextP(" Coal: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_LOGS)); restext.m_part.push_back(RichTextP(" Logs: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_LUMBER)); restext.m_part.push_back(RichTextP(" Lumber: 100 +1/")); restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_TIME)); restext.m_part.push_back(RichTextP("    "));
	restext.m_part.push_back(RichTextP(RICHTEXT_ICON, ICON_WATER)); restext.m_part.push_back(RichTextP(" Water: 100/120")); restext.m_part.push_back(RichTextP("    "));

#if 0
		":peso:Pesos: 100 +1/:time:    "\
		":euro:Euros: 100 +1/:time:    "\
		":pound:Pounds: 100 +1/:time:    "\
		":franc:Francs: 100 +1/:time:    "\
		":rupee:Rupees: 100 +1/:time:    "\
		":yen:Yens: 100 +1/:time:    "\
		":rouble:Roubles: 100 +1/:time:    "\
		":housing:Housing: 100/120    "\
		":farmprod:Farm Products: 100 +1/:time:    "\
		":food:Food: 100 +1/:time:    "\
		":chemicals:Chemicals: 100 +1/:time:    "\
		":electronics:Electronics: 100 +1/:time:    "\
		":research:Research: 100 +1/:time:    "\
		":production:Production: 100 +1/:time:    "\
		":ironore:Iron Ore: 100 +1/:time:    "\
		":uraniumore:Uranium Ore: 100 +1/:time:    "\
		":steel:Steel: 100 +1/:time:    "\
		":crudeoil:Crude Oil: 100 +1/:time:    "\
		":wsfuel:Wholesale Fuel: 100 +1/:time:    "\
		":retfuel:Retail Fuel: 100 +1/:time:    "\
		":stone:Stone: 100 +1/:time:    "\
		":cement:Cement: 100 +1/:time:    "\
		":energy:Energy: 100/120    "\
		":enricheduran:Enriched Uranium: 100 +1/:time:    "\
		":coal:Coal: 100 +1/:time:    "\
		":logs:Logs: 100 +1/:time:    "\
		":lumber:Lumber: 100 +1/:time:    "\
		":water:Water: 100/120    "), 
		NULL);
#endif

	int len = restext.texlen();

	tickerpos += 0.5f * g_drawfrinterval * 100;

	if((int)tickerpos > len)
		tickerpos = 0;

	RichText restext2 = restext.substr((int)tickerpos, len-(int)tickerpos) + restext.substr(0, (int)tickerpos);

	restickertw->m_text = restext2;
}

void Resize_BottomPanel(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = g_height - MINIMAP_SIZE - 32;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void FillPlayGUI()
{
	
	g_viewportT[VIEWPORT_MINIMAP] = ViewportT(Vec3f(0, 0, 0), Vec3f(0, 1, 0), "Minimap", true);
	g_viewportT[VIEWPORT_ENTVIEW] = ViewportT(Vec3f(0, MAX_DISTANCE/2, 0), Vec3f(0, 1, 0), "EntView", false);

	View* playguiview = AddView("play gui");
	
	//playguiview->widget.push_back(new Image(NULL, "gui/backg/white.png", Resize_ResTicker)); 
	//playguiview->widget.push_back(new Text(NULL, "res ticker", RichText(" "), MAINFONT16, Resize_ResTicker, true, 1, 1, 1, 1));
	playguiview->widget.push_back(new ResTicker(NULL, "res ticker", Resize_ResTicker));
	playguiview->widget.push_back(new BottomPanel(NULL, "bottom panel", Resize_BottomPanel));

#if 0
	FillPlayRightMenu();

	View* playguiview = AddView("play gui old");

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

#if 0
	RichText resnamestext = ParseTags(RichText(
		"RESOURCE \n"\
		":dollar:Dollars: \n"\
		":peso:Pesos: \n"\
		":euro:Euros: \n"\
		":pound:Pounds: \n"\
		":franc:Francs: \n"\
		":rupee:Rupees: \n"\
		":yen:Yens: \n"\
		":rouble:Roubles: \n"\
		":housing:Housing: \n"\
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
		":coal:Coal: \n"\
		":logs:Logs: \n"\
		":lumber:Lumber: \n"\
		":water:Water: \n"), 
		NULL);
	
	RichText resamtstext = ParseTags(RichText(
		"AMOUNT \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100/231 \n"\
		"100 \n"\
		"100 \n"
		"100 \n"\
		"100 \n"
		"100 \n"\
		"12/134 \n"\
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
		"100 \n"\
		"100 \n"\
		"100 \n"\
		"100 \n"), 
		NULL);
	
	RichText resdeltastext = ParseTags(RichText(
		"DELTA \n"\
		"+11/:time: \n"\
		"+11/:time: \n"\
		"+11/:time: \n"\
		"+11/:time: \n"\
		"+11/:time: \n"\
		"+11/:time: \n"\
		"+11/:time: \n"\
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
		"-8/:time: \n"\
		"-8/:time: \n"\
		"-8/:time: \n"\
		"+12/:time: \n"), 
		NULL);
	
	playguiview->widget.push_back(new TextBlock(NULL, "resnames", resnamestext, MAINFONT16, Resize_ResNamesTextBlock));
	playguiview->widget.push_back(new TextBlock(NULL, "resamts", resamtstext, MAINFONT16, Resize_ResAmtsTextBlock));
	playguiview->widget.push_back(new TextBlock(NULL, "resdeltas", resdeltastext, MAINFONT16, Resize_ResDeltasTextBlock));
#endif

#if 1
	playguiview->widget.push_back(new Image(NULL, "gui/filled.jpg", Resize_BottRightPanel)); 
	
	playguiview->widget.push_back(new Button(NULL, "models/apartment/texture.jpg", RichText("Apartment Building"), MAINFONT8, Resize_BuildButton_TopLeft, Click_BuildApartment, Over_BuildApartment, Out_Build));
	playguiview->widget.push_back(new Button(NULL, "models/cemplant/texture.jpg", RichText("Cement Plant"), MAINFONT8, Resize_BuildButton_TopMid, Click_BuildApartment, Over_BuildApartment, Out_Build));
	playguiview->widget.push_back(new Button(NULL, "models/chemplant/texture.jpg", RichText("Chemical Plant"), MAINFONT8, Resize_BuildButton_TopRight, Click_BuildApartment, Over_BuildApartment, Out_Build));
#endif
#endif
}