
#ifndef GGUI_H
#define GGUI_H

#include "../../common/math/vec3f.h"

class View;

extern Vec3f g_vdrag[2];

#if 0
extern bool g_canselect;
#endif

#define LEFT_PANEL_WIDTH	200

extern int g_build;
extern char g_lastsave[MAX_PATH+1];

void Click_NewGame();
void Click_OpenEditor();
void FillGUI();
void Click_LoadMapButton();
void Click_SaveMapButton();
void Click_QSaveMapButton();

#endif	//GGUI_H
