



#include "gmain.h"
#include "keymap.h"
#include "../common/platform.h"
#include "../common/gui/gui.h"
#include "../common/math/camera.h"
#include "../common/math/vec3f.h"

#if 0
#include "3dmath.h"
#include "gui.h"
#include "menu.h"
#include "sound.h"
#include "map.h"
#include "building.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "unit.h"
#include "selection.h"
#include "order.h"
#include "waves.h"
#include "chat.h"
#include "tileset.h"
#include "editor.h"
#include "minimap.h"
#include "script.h"
#include "player.h"

void SkipIntro()
{
	if(g_mode != INTRO)
		return;

	g_mode = MENU;
	RedoGUI();
}

void Escape()
{
	SkipIntro();

	if(g_mode != PLAY && g_mode != EDITOR)
		return;

	//Click_Quit();

	bool play = (g_mode == PLAY);

	g_mode = MENU;
	OpenMenu();

	if(play)
		OpenSoleView("ingame");

	//OpenAnotherView("quit");
	//g_mode = PAUSE;
	//OpenSoleView("pause");
}

void MouseLeftButtonDown()
{
	if(g_mode == LOGO)
		SkipLogo();
	else if(g_mode == PLAY || g_mode == EDITOR)
	{
		if(g_gameover)
			return;

		if(OverMinimap())
		{
			g_minimapdrag = true;
			ScrollToMouse();
			return;
		}

		EdApply();

		if(g_build >= 0 && g_build < BUILDING_TYPES)
		{
		}
		else if(g_build == ROAD || g_build == POWERLINE || g_build == PIPELINE)
		{
			g_vStart = g_vTile;
			g_vMouseStart = g_vMouse;

			if(g_build == ROAD)
				UpdateRoadPlans();
			else if(g_build == POWERLINE)
				UpdatePowerlinePlans();
			else if(g_build == PIPELINE)
				UpdatePipelinePlans();
		}
		else
		{
			g_mousestart = g_mouse;
		}

		//CloseOverWin();
	}
	/*
	else if(g_mode == EDITOR)
	{
		//g_vStart = g_vTile;
		EdApply();

		CloseOverWin();
	}*/
}

void MouseLeftButtonUp()
{
	if(g_mode == PLAY || g_mode == EDITOR)
	{
		if(g_gameover)
			return;

		if(g_minimapdrag)
		{
			g_minimapdrag = false;
			return;
		}

		if(g_mode == EDITOR)
		{
			EdApplyUp();

			g_vStart = g_vTile;
			g_vMouseStart = g_vMouse;
		}

		if(g_build >= 0 && g_build < BUILDING_TYPES)
		{
			if(g_canPlace)
				PlaceB(g_selP, g_build, g_vTile);
			else
				Chat("Can't build there.");
			if(g_mode != EDITOR)
				g_canselect = true;
			//Start();
			if(g_canPlace)
				OnPlaceB(g_build);
			g_build = NOTHING;
		}
		else if(g_build == ROAD)
		{
			PlaceRoad();
			g_build = NOTHING;
			if(g_mode != EDITOR)
				g_canselect = true;
			OnPlaceB(ROAD);
			//Start();
		}
		else if(g_build == POWERLINE)
		{
			PlacePowerline();
			g_build = NOTHING;
			if(g_mode != EDITOR)
				g_canselect = true;
			OnPlaceB(POWERLINE);
			//Start();
		}
		else if(g_build == PIPELINE)
		{
			PlacePipeline();
			g_build = NOTHING;
			if(g_mode != EDITOR)
				g_canselect = true;
			OnPlaceB(PIPELINE);
		}
		else if(g_canselect)
		{
			Selection();
		}
	}
}

void MouseRightButtonUp()
{
	if(g_mode == PLAY)
	{
		if(g_gameover)
			return;

		if(g_selection.size() <= 0)
			return;

		int i = g_selection[0];
		CUnit* u = &g_unit[i];

		if(u->type == LABOURER || u->type == TRUCK)
			return;

		if(u->owner != g_localP)
			return;

		Order();
	}
}

void MouseWheel(int delta)
{
	if(g_mode == PLAY || g_mode == EDITOR)
	{
		g_camera.Rise(-delta*10);

		float y = g_camera.Position().y;
		if(y < 2)
			g_camera.Rise(-y+2);

		UpdateMouse3D();
	}
}

void MouseMove()
{
	if(g_mode == PLAY || g_mode == EDITOR)
	{
		if(g_mousekeys[1])
			g_camera.SetViewByMouse();

		if(g_mousekeys[0] && g_minimapdrag)
		{
			if(OverMinimap())
			{
				ScrollToMouse();
			}
		}

		UpdateMouse3D();

		if(g_mode == EDITOR)
		{
			if(!g_mousekeys[0])
			{
				g_vStart = g_vTile;
				g_vMouseStart = g_vMouse;
			}
			else
				EdApply();
		}
	}
}

#endif

void MouseMidButtonDown()
{
	if(g_mode == PLAY || g_mode == EDITOR)
	{
		if(g_mousekeys[1])
		{
			CenterMouse();
		}
	}
}

void MouseMidButtonUp()
{
}

void RotateAbout()
{
	float dx = g_mouse.x - g_width/2;
	float dy = g_mouse.y - g_height/2;

	g_camera.rotateabout(g_camera.m_view, dy / 100.0f, g_camera.m_strafe.x, g_camera.m_strafe.y, g_camera.m_strafe.z);
	g_camera.rotateabout(g_camera.m_view, dx / 100.0f, g_camera.m_up.x, g_camera.m_up.y, g_camera.m_up.z);
}

void MouseMove()
{
	if(g_mode == PLAY || g_mode == EDITOR)
	{
		if(g_mousekeys[1])
		{
			RotateAbout();
			CenterMouse();
		}
	}
}

void MouseWheel(int delta)
{        
	if(g_mode == PLAY || g_mode == EDITOR)
	{
		g_zoom *= 1.0f + (float)delta / 10.0f;
		Vec3f ray = g_camera.m_view - g_camera.m_pos;
		ray = ray * 1.0f/(1.0f + (float)delta / 10.0f);
		g_camera.m_pos = g_camera.m_view - ray;
	}
}

void MapKeys()
{	
#if 0
	AssignKey(VK_ESCAPE, &Escape, NULL);
	AssignLButton(&MouseLeftButtonDown, &MouseLeftButtonUp);
	AssignRButton(NULL, &MouseRightButtonUp);
#endif
	
	AssignMouseMove(&MouseMove);
	AssignMouseWheel(&MouseWheel);
	AssignMButton(MouseMidButtonDown, MouseMidButtonUp);

	/*
	int key;
	void (*down)();
	void (*up)();
	ifstream f("keymap.ini");
	string line;
	char keystr[32];
	char actstr[32];

	while(!f.eof())
	{
		key = -1;
		down = NULL;
		up = NULL;
		strcpy(keystr, "");
		strcpy(actstr, "");

		getline(f, line);
		sscanf(line.c_str(), "%s %s", keystr, actstr);

		if(stricmp(keystr, "VK_ESCAPE") == 0)			key = VK_ESCAPE;
		else if(stricmp(keystr, "VK_SHIFT") == 0)		key = VK_SHIFT;
		else if(stricmp(keystr, "VK_CONTROL") == 0)		key = VK_CONTROL;
		else if(stricmp(keystr, "VK_SPACE") == 0)		key = VK_SPACE;
		else if(stricmp(keystr, "MouseLButton") == 0)	key = -2;
		else if(stricmp(keystr, "F1") == 0)				key = VK_F1;
		else if(stricmp(keystr, "F2") == 0)				key = VK_F2;
		else if(stricmp(keystr, "F3") == 0)				key = VK_F3;
		else if(stricmp(keystr, "F4") == 0)				key = VK_F4;
		else if(stricmp(keystr, "F5") == 0)				key = VK_F5;
		else if(stricmp(keystr, "F6") == 0)				key = VK_F6;
		else if(stricmp(keystr, "F7") == 0)				key = VK_F7;
		else if(stricmp(keystr, "F8") == 0)				key = VK_F8;
		else if(stricmp(keystr, "F9") == 0)				key = VK_F9;
		else if(stricmp(keystr, "F10") == 0)			key = VK_F10;
		else if(stricmp(keystr, "F11") == 0)			key = VK_F11;
		else if(stricmp(keystr, "F12") == 0)			key = VK_F12;
		else if(stricmp(keystr, "'A'") == 0)			key = 'A';
		else if(stricmp(keystr, "'B'") == 0)			key = 'B';
		else if(stricmp(keystr, "'C'") == 0)			key = 'C';
		else if(stricmp(keystr, "'D'") == 0)			key = 'D';
		else if(stricmp(keystr, "'E'") == 0)			key = 'E';
		else if(stricmp(keystr, "'F'") == 0)			key = 'F';
		else if(stricmp(keystr, "'G'") == 0)			key = 'G';
		else if(stricmp(keystr, "'H'") == 0)			key = 'H';
		else if(stricmp(keystr, "'I'") == 0)			key = 'I';
		else if(stricmp(keystr, "'J'") == 0)			key = 'J';
		else if(stricmp(keystr, "'K'") == 0)			key = 'K';
		else if(stricmp(keystr, "'L'") == 0)			key = 'L';
		else if(stricmp(keystr, "'M'") == 0)			key = 'M';
		else if(stricmp(keystr, "'N'") == 0)			key = 'N';
		else if(stricmp(keystr, "'O'") == 0)			key = 'O';
		else if(stricmp(keystr, "'P'") == 0)			key = 'P';
		else if(stricmp(keystr, "'Q'") == 0)			key = 'Q';
		else if(stricmp(keystr, "'R'") == 0)			key = 'R';
		else if(stricmp(keystr, "'S'") == 0)			key = 'S';
		else if(stricmp(keystr, "'T'") == 0)			key = 'T';
		else if(stricmp(keystr, "'U'") == 0)			key = 'U';
		else if(stricmp(keystr, "'V'") == 0)			key = 'V';
		else if(stricmp(keystr, "'W'") == 0)			key = 'W';
		else if(stricmp(keystr, "'X'") == 0)			key = 'X';
		else if(stricmp(keystr, "'Y'") == 0)			key = 'Y';
		else if(stricmp(keystr, "'Z'") == 0)			key = 'Z';
		else if(stricmp(keystr, "'0'") == 0)			key = '0';
		else if(stricmp(keystr, "'1'") == 0)			key = '1';
		else if(stricmp(keystr, "'2'") == 0)			key = '2';
		else if(stricmp(keystr, "'3'") == 0)			key = '3';
		else if(stricmp(keystr, "'4'") == 0)			key = '4';
		else if(stricmp(keystr, "'5'") == 0)			key = '5';
		else if(stricmp(keystr, "'6'") == 0)			key = '6';
		else if(stricmp(keystr, "'7'") == 0)			key = '7';
		else if(stricmp(keystr, "'8'") == 0)			key = '8';
		else if(stricmp(keystr, "'9'") == 0)			key = '9';

		if(key == -1)
		{
			g_log<<"Unknown input: "<<keystr<<endl;
			continue;
		}
		
		if(stricmp(actstr, "Escape();") == 0)				{	down = &Escape;			up = NULL;			}
		else if(stricmp(actstr, "Forward();") == 0)			{	down = &Forward;		up = NULL;			}
		else if(stricmp(actstr, "Left();") == 0)			{	down = &Left;			up = NULL;			}
		else if(stricmp(actstr, "Right();") == 0)			{	down = &Right;			up = NULL;			}
		else if(stricmp(actstr, "Back();") == 0)			{	down = &Back;			up = NULL;			}

		if(down == NULL)		g_log<<"Unknown action: "<<actstr<<endl;
		else if(key == -2)		AssignLButton(down, up);
		else					AssignKey(key, down, up);
	}
	*/
}
