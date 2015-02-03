#include "../appmain.h"
#include "../../engine/gui/gui.h"
#include "../../engine/gui/widgets/windoww.h"
#include "../../engine/input/keymap.h"
#include "../../engine/render/heightmap.h"
#include "../../engine/render/transaction.h"
#include "../../engine/math/camera.h"
#include "../../engine/render/screenshot.h"
#include "../../engine/save/savemap.h"
#include "appgui.h"
#include "playgui.h"
#include "../../engine/gui/icon.h"
#include "../../engine/gui/widgets/spez/resticker.h"
#include "../../engine/gui/widgets/spez/botpan.h"
#include "../../engine/gui/widgets/spez/blpreview.h"
#include "../../engine/gui/widgets/spez/blview.h"
#include "../../engine/gui/widgets/spez/cstrview.h"
#include "../../engine/gui/widgets/spez/svlist.h"
#include "../../engine/gui/widgets/spez/saveview.h"
#include "../../engine/gui/widgets/spez/loadview.h"
#include "../../engine/gui/widgets/spez/truckmgr.h"
#include "../../engine/sim/bltype.h"
#include "../../engine/sim/building.h"
#include "../../engine/sim/road.h"
#include "../../engine/sim/powl.h"
#include "../../engine/sim/crpipe.h"
#include "../../engine/sim/player.h"
#include "../../engine/sim/conduit.h"
#include "../../engine/sim/simflow.h"
#include "../../engine/sys/window.h"
#include "chattext.h"

void Resize_ResTicker(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_font[MAINFONT].gheight+5;
	thisw->m_tpos[0] = 0;
	thisw->m_tpos[1] = 0;
}

void UpdResTicker()
{
	Player* py;
	GUI* gui;
	ViewLayer* playview;
	ResTicker* restickerw;
	Widget* restickertw;
	RichText restext;
	UStr ustr;
	Resource* r;
	RichPart richpart;
	static float tickerpos = 0;
	char local[16];
	char global[16];
	char cstr1[128];
	char cstr2[40];
	char total[16];
	RichText subrestext;
	RichText subrestext2;
	int len;
	int endx;

	py = &g_player[g_localP];
	gui = &g_gui;
	playview = (ViewLayer*)gui->get("play");
	restickerw = (ResTicker*)playview->get("res ticker");
	restickertw = &restickerw->restext;
	
	RichText_Free(&restickertw->m_text);
	RichText_Init(&restext);

	for(int i=0; i<RESOURCES; i++)
	{
		if(i == RES_LABOUR)
			continue;

		r = &g_resource[i];

		if(r->capacity && py->local[i] <= 0)
			continue;
		else if(!r->capacity && py->local[i] + py->global[i] <= 0)
			continue;

		RichPart_Init_Icon(&richpart, r->icon);
		List_PushBack(&restext.m_part, &richpart, sizeof(RichPart));

		sprintf(cstr1, " %s: ", r->name);

		if(r->capacity)
		{
			IForm(py->local[i], local);
			IForm(py->global[i], global);
			sprintf(cstr2, "%s/%s", local, global);
		}
		else
		{
			IForm(py->local[i] + py->global[i], total);
			sprintf(cstr2, "%s", total);
		}

		strcat(cstr1, cstr2);
		UStr_Init_Str(&ustr, cstr1);
		RichPart_Init_UStr(&richpart, &ustr);
		List_PushBack(&restext.m_part, &richpart, sizeof(RichPart));
		UStr_Free(&ustr);

		UStr_Init_Str(&ustr, "    ");
		RichPart_Init_UStr(&richpart, &ustr);
		List_PushBack(&restext.m_part, &richpart, sizeof(RichPart));
		UStr_Free(&ustr);
	}

	len = restext.texlen();

	tickerpos += 0.5f * g_drawfrinterval * 20;

	if((int)tickerpos > len)
		tickerpos = 0;

	endx = EndX(&restext, restext.rawlen(), MAINFONT, 0, 0);

	if(endx > g_width)
	{
		RichText_Init(&restickertw->m_text);
		RichText_SubStr(&restext, (int)tickerpos, len-(int)tickerpos, &subrestext);
		RichText_SubStr(&restext, 0, (int)tickerpos, &subrestext2);
		RichText_Append_RichText(&restickertw->m_text, &subrestext);
		RichText_Append_RichText(&restickertw->m_text, &subrestext2);
		RichText_Free(&subrestext);
		RichText_Free(&subrestext2);
	}
	else
	{
		RichText_Copy(&restickertw->m_text, &restext);
	}

	RichText_Free(&restext);
}

void Resize_BottomPanel(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32);
	thisw->m_pos[2] = (float)g_width;
	thisw->m_pos[3] = (float)g_height;
}

void Out_BuildButton()
{
	GUI* gui = &g_gui;
	Widget_Close(gui, "bl preview");
}

void Click_BuildButton(int bwhat)
{
	g_build = bwhat;
}

void Over_BuildButton(int bwhat)
{
	GUI* gui;
	BlPreview* bp;
	Text* tl;
	const char* bname;
	RichText* cb;	//conmat block
	RichText* ib;	//inputs block
	RichText* ob;	//outputs block
	RichText* db;	//description block
	TextBlock* cbw;
	TextBlock* ibw;
	TextBlock* obw;
	TextBlock* dbw;
	BlType* bt;
	int ns;	//number of stocked good types
	int ri;	//res index
	RichPart richpart;
	Resource* r;
	char num[32];
	int ni;	//number of input good types
	int no;	//number of out

	gui = &g_gui;

	if(Widget_Get((Widget*)gui, "cstr view")->m_Opened)
		return;

	if(Widget_Get((Widget*)gui, "bl view")->m_Opened)
		return;

	g_bptype = bwhat;

	Widget_Open((Widget*)gui, "bl preview");
	bp = (BlPreview*)Widget_Get((Widget*)gui, "bl preview");
	tl = (Text*)Widget_Get((Widget*)gui, "title");

	cbw = (TextBlock*)Widget_Get((Widget*)bp, "conmat block");
	ibw = (TextBlock*)Widget_Get((Widget*)bp, "input block");
	obw = (TextBlock*)Widget_Get((Widget*)bp, "output block");
	dbw = (TextBlock*)Widget_Get((Widget*)bp, "desc block");
	
	cb = &cbw->m_text;
	ib = &ibw->m_text;
	ob = &obw->m_text;
	db = &dbw->m_text;
	
	RichText_Free(cb);
	RichText_Init(cb);
	RichText_QAppend_Str(cb, "CONSTRUCTION REQUISITES:");

	if(bwhat < 0)
		;
	else if(bwhat < BL_TYPES)
	{
		//Set inputs
		RichText_Free(ib);
		RichText_Init(ib);
		RichText_QAppend_Str(ib, "INPUTS:");
		
		//Set outputs
		RichText_Free(ob);
		RichText_Init(ob);
		RichText_QAppend_Str(ob, "OUTPUTS:");

		bt = &g_bltype[bwhat];
		bname = bt->name;

		//Set desc
		RichText_Free(db);
		RichText_Init(db);
		RichText_QAppend_Str(db, bt->desc);

		ns = 0;
		for(ri=0; ri<RESOURCES; ri++)
		{
			if(bt->conmat[ri] <= 0)
				continue;

			ns ++;
			r = &g_resource[ri];

			RichText_QAppend_Str(cb, "\n");
			RichText_QAppend_Str(cb, r->name);
			RichText_QAppend_Str(cb, " ");
			
			RichPart_Init_Icon(&richpart, r->icon);
			List_PushBack(&cb->m_part, sizeof(RichPart), &richpart);

			RichText_QAppend_Str(cb, ": ");

			sprintf(num, "%d", bt->conmat[ri]);
			RichText_QAppend_Str(cb, num);
		}

		if(ns <= 0)
			RichText_QAppend_Str(cb, "\nNone");

		ni = 0;
		for(ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			ni ++;
			r = &g_resource[ri];
			
			RichText_QAppend_Str(ib, "\n");
			RichText_QAppend_Str(ib, r->name);
			RichText_QAppend_Str(ib, " ");
			
			RichPart_Init_Icon(&richpart, r->icon);
			List_PushBack(&ib->m_part, sizeof(RichPart), &richpart);

			RichText_QAppend_Str(ib, ": ");

			sprintf(num, "%d", bt->input[ri]);
			RichText_QAppend_Str(ib, num);
		}

		if(ni <= 0)
			RichText_QAppend_Str(ib, "\nNone");

		no = 0;
		for(ri=0; ri<RESOURCES; ri++)
		{
			if(bt->output[ri] <= 0)
				continue;

			no ++;
			r = &g_resource[ri];
			
			RichText_QAppend_Str(ob, "\n");
			RichText_QAppend_Str(ob, r->name);
			RichText_QAppend_Str(ob, " ");
			
			RichPart_Init_Icon(&richpart, r->icon);
			List_PushBack(&ob->m_part, sizeof(RichPart), &richpart);
			
			RichText_QAppend_Str(ob, ": ");

			sprintf(num, "%d", bt->output[ri]);
			RichText_QAppend_Str(ob, num);
		}

		if(no <= 0)
			RichText_QAppend_Str(ob, "\nNone");
	}
	else if(bwhat < BL_TYPES + CONDUIT_TYPES)
	{
		CdType* ct = &g_cdtype[bwhat - BL_TYPES];
		bname = ct->name;

		//Set desc
		RichText_Free(db);
		RichText_Init(db);
		RichText_QAppend_Str(db, ct->desc);

		ns = 0;
		for(ri=0; ri<RESOURCES; ri++)
		{
			if(ct->conmat[ri] <= 0)
				continue;

			ns ++;
			r = &g_resource[ri];

			RichText_QAppend_Str(cb, "\n");
			RichText_QAppend_Str(cb, r->name);
			RichText_QAppend_Str(cb, " ");
			
			RichPart_Init_Icon(&richpart, r->icon);
			List_PushBack(&cb->m_part, sizeof(RichPart), &richpart);
			
			RichText_QAppend_Str(cb, ": ");

			sprintf(num, "%d", ct->conmat[ri]);
			RichText_QAppend_Str(cb, num);
		}

		if(ns <= 0)
			RichText_QAppend_Str(cb, "\nNone");
	}
	
	RichText_Free(&tl->m_text);
	RichText_Init(&tl->m_text);
	RichText_QAppend_Str(&tl->m_text, bname);
}

void Click_NextBuildButton(int nextpage)
{
	GUI* gui;
	ViewLayer* playview;
	BotPan* bp;
	int i;

	gui = &g_gui;
	playview = (ViewLayer*)Widget_Get((Widget*)gui, "play");
	bp = (BotPan*)Widget_Get((Widget*)playview, "bottom panel");

	for(i=0; i<9; i++)
		bp->bottomright_button_on[i] = FALSE;

	if(nextpage == 1)
	{
		//TODO set buttons
	}
	else if(nextpage == 2)
	{
	}
	else if(nextpage == 3)
	{
	}

	Widget_Reframe((Widget*)bp);
}

void Resize_BuildPreview(Widget* thisw)
{
	thisw->m_pos[0] = (float)(g_width - 400 - 64);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 60 - 320);
	thisw->m_pos[2] = (float)(g_width - 64);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 75);
}

void Resize_ConstructionView(Widget* thisw)
{
	thisw->m_pos[0] = (float)(g_width/2 - 250);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 300);
	thisw->m_pos[2] = (float)(g_width/2 + 100);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32 - 100);
}

void Resize_Message(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = (float)(g_width/2 - 200);
	thisw->m_pos[1] = (float)(g_height/2 - 100);
	thisw->m_pos[2] = (float)(g_width/2 + 200);
	thisw->m_pos[3] = (float)(g_height/2 + 100);
}

void Resize_MessageContinue(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = (float)(g_width/2 - 80);
	thisw->m_pos[1] = (float)(g_height/2 + 70);
	thisw->m_pos[2] = (float)(g_width/2 + 80);
	thisw->m_pos[3] = (float)(g_height/2 + 90);
}

void Click_MessageContinue()
{
	GUI* gui = &g_gui;
	Widget_Close((Widget*)gui, "message view");
}

void ShowMessage(const RichText* msg)
{
	GUI* gui;
	ViewLayer* msgview;
	TextBlock* msgblock;

	gui = &g_gui;
	msgview = (ViewLayer*)Widget_Get((Widget*)gui, "message view");
	msgblock = (TextBlock*)Widget_Get((Widget*)msgview, "message");
	RichText_Free(&msgblock->m_text);
	RichText_Copy(&msgblock->m_text, msg);
	Widget_Open((Widget*)gui, "message view");
}

void Resize_Window(Widget* thisw)
{
	thisw->m_pos[0] = (float)(g_width/2 - 200);
	thisw->m_pos[1] = (float)(g_height/2 - 200);
	thisw->m_pos[2] = (float)(g_width/2 + 200);
	thisw->m_pos[3] = (float)(g_height/2 + 200);
}

void Resize_DebugLines(Widget* thisw)
{
	thisw->m_pos[0] = (float)(0);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Transx(Widget* thisw)
{
	const int i = 1;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Save(Widget* thisw)
{
	const int i = 2;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_QSave(Widget* thisw)
{
	const int i = 3;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Load(Widget* thisw)
{
	const int i = 4;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Pause(Widget* thisw)
{
	const int i = 5;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Play(Widget* thisw)
{
	const int i = 6;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Fast(Widget* thisw)
{
	const int i = 7;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Click_Pause()
{
	g_speed = SPEED_PAUSE;
}

void Click_Play()
{
	g_speed = SPEED_PLAY;
}

void Click_Fast()
{
	g_speed = SPEED_FAST;
}

void Click_DebugLines()
{
	g_debuglines = !g_debuglines;
}

void Click_Transx()
{
	g_drawtransx = !g_drawtransx;
}

void Click_Save()
{
	GUI* gui;
	gui = &g_gui;
	Widget_Close((Widget*)gui, "load");
	Widget_Open((Widget*)gui, "save");
	Widget_SaveView_Regen((SaveView*)Widget_Get((Widget*)gui, "save"));
}

void Click_QSave()
{
	GUI* gui;

	if(!g_lastsave[0])
	{
		Click_Save();
		return;
	}

	SaveMap(g_lastsave);
	
	gui = &g_gui;
	Widget_Close((Widget*)gui, "save");
	Widget_Close((Widget*)gui, "load");
}

void Click_Load()
{
	GUI* gui;
	gui = &g_gui;
	Widget_Close((Widget*)gui, "save");
	Widget_Open((Widget*)gui, "load");
	Widget_LoadView_Regen((LoadView*)Widget_Get((Widget*)gui, "load"));
}

void Click_QuitToMenu()
{
	GUI* gui;

	EndSess();
	FreeMap();
	gui = &g_gui;
	Widget_CloseAll((Widget*)gui);
	Widget_Open((Widget*)gui, "main");
	g_mode = APPMODE_MENU;
}

void FillPlay()
{
	GUI* gui;
	ViewLayer* playview;
	ResTicker* resticker;
	BotPan* botpan;
	CstrView* cstrview;
	BlPreview* blpreview;
	BlView* blview;
	TruckMgr* truckmgr;
	SaveView* saveview;
	LoadView* loadview;
	Image* image;
	Link* link;
	ViewLayer* ingame;
	RichText blank;
	Button* debugbutton;
	Button* transxbutton;
	Button* savebutton;
	Button* qsavebutton;
	Button* loadbutton;
	Button* pausebutton;
	Button* playbutton;
	Button* fastbutton;
	RichText debugtip;
	RichText transxtip;
	RichText savetip;
	RichText qsavetip;
	RichText loadtip;
	RichText pausetip;
	RichText playtip;
	RichText fasttip;
	ViewLayer* ingame;
	Image* whitebg;
	Link* quitmenu;
	Link* cancel;
	RichText quitmenulabel;
	RichText cancellabel;
	ViewLayer* msgviewl;
	TextBlock* msgtextbl;
	TouchListen* msgtouchl;
	Button* msgbutton;
	RichText continuelabel;

	gui = &g_gui;

	//Play view-layer
	playview = (ViewLayer*)malloc(sizeof(ViewLayer));
	Widget_ViewLayer_Init(vl, (Widget*)gui, "play");
	Widget_Add((Widget*)gui, vl);
	
	//Res. ticker and bottom panel
	resticker = (ResTicker*)malloc(sizeof(ResTicker));
	botpan = (BotPan*)malloc(sizeof(BotPan));
	Widget_ResTicker_Init(resticker, (Widget*)playview, "res ticker", Resize_ResTicker);
	Widget_BotPan_Init(botpan, (Widget*)playview, "bottom panel", Resize_BottomPanel);
	Widget_Add((Widget*)gui, (Widget*)resticker);
	Widget_Add((Widget*)gui, (Widget*)botpan);

	//Play view buttons panel
	RichText_Init(&blank);
	debugbutton = (Button*)malloc(sizeof(Button));
	transxbutton = (Button*)malloc(sizeof(Button));
	savebutton = (Button*)malloc(sizeof(Button));
	qsavebutton = (Button*)malloc(sizeof(Button));
	loadbutton = (Button*)malloc(sizeof(Button));
	pausebutton = (Button*)malloc(sizeof(Button));
	playbutton = (Button*)malloc(sizeof(Button));
	fastbutton = (Button*)malloc(sizeof(Button));
	RichText_Init_Str(&debugtip, "Show debug info");
	RichText_Init_Str(&transxtip, "Show transactions");
	RichText_Init_Str(&savetip, "Save game");
	RichText_Init_Str(&qsavetip, "Quick save");
	RichText_Init_Str(&loadtip, "Load game");
	RichText_Init_Str(&pausetip, "Pause");
	RichText_Init_Str(&playtip, "Play");
	RichText_Init_Str(&fasttip, "Fast Forward");
	Widget_Button_Init(debugbutton, (Widget*)playview, "debug lines", "gui/debuglines.png", &blank, &debugtip, MAINFONT, BUST_LINEBASED, Resize_DebugLines, Click_DebugLines, NULL, NULL, NULL, NULL, -1);
	Widget_Button_Init(transxbutton, (Widget*)playview, "show transactions", "gui/transx.png", &blank, &transxtip, MAINFONT, BUST_LINEBASED, Resize_Transx, Click_Transx, NULL, NULL, NULL, NULL, -1);
	Widget_Button_Init(savebutton, (Widget*)playview, "save", "gui/edsave.png", &blank, &savetip, MAINFONT, BUST_LINEBASED, Resize_Save, Click_Save, NULL, NULL, NULL, NULL, -1);
	Widget_Button_Init(qsavebutton, (Widget*)playview, "qsave", "gui/qsave.png", &blank, &qsavetip, MAINFONT, BUST_LINEBASED, Resize_QSave, Click_QSave, NULL, NULL, NULL, NULL, -1);
	Widget_Button_Init(loadbutton, (Widget*)playview, "load", "gui/edload.png", &blank, &loadtip, MAINFONT, BUST_LINEBASED, Resize_Load, Click_Load, NULL, NULL, NULL, NULL, -1);
	Widget_Button_Init(pausebutton, (Widget*)playview, "pause", "gui/pause.png", &blank, &pausetip, MAINFONT, BUST_LINEBASED, Resize_Pause, Click_Pause, NULL, NULL, NULL, NULL, -1);
	Widget_Button_Init(playbutton, (Widget*)playview, "play", "gui/play.png", &blank, &playtip, MAINFONT, BUST_LINEBASED, Resize_Play, Click_Play, NULL, NULL, NULL, NULL, -1);
	Widget_Button_Init(fastbutton, (Widget*)playview, "fast", "gui/fastforward.png", &blank, &fasttip, MAINFONT, BUST_LINEBASED, Resize_Fast, Click_Fast, NULL, NULL, NULL, NULL, -1);
	Widget_Add((Widget*)playview, (Widget*)debugbutton);
	Widget_Add((Widget*)playview, (Widget*)transxbutton);
	Widget_Add((Widget*)playview, (Widget*)savebutton);
	Widget_Add((Widget*)playview, (Widget*)qsavebutton);
	Widget_Add((Widget*)playview, (Widget*)loadbutton);
	Widget_Add((Widget*)playview, (Widget*)pausebutton);
	Widget_Add((Widget*)playview, (Widget*)playbutton);
	Widget_Add((Widget*)playview, (Widget*)fastbutton);
	RichText_Free(&debugtip);
	RichText_Free(&transxtip);
	RichText_Free(&savetip);
	RichText_Free(&qsavetip);
	RichText_Free(&loadtip);
	RichText_Free(&pausetip);
	RichText_Free(&playtip);
	RichText_Free(&fasttip);

	AddChat(playview);

	//TODO preload all the build button images

	cstrview = (CstrView*)malloc(sizeof(CstrView));
	blpreview = (BlPreview*)malloc(sizeof(BlPreview));
	blview = (BlView*)malloc(sizeof(BlView));
	truckmgr = (TruckMgr*)malloc(sizeof(TruckMgr));
	saveview = (SaveView*)malloc(sizeof(SaveView));
	loadview = (LoadView*)malloc(sizeof(LoadView));
	Widget_CstrView_Init(cstrview, (Widget*)gui, "cstr view", Resize_ConstructionView));
	Widget_BlPreview_Init(blpreview, (Widget*)gui, "bl preview", Resize_BuildPreview));
	Widget_BlView_Init(blview, (Widget*)gui, "bl view", Resize_BuildPreview));
	Widget_TruckMgr_Init(truckmgr, (Widget*)gui, "truck mgr", Resize_BuildPreview));
	Widget_SaveView_Init(saveview, (Widget*)gui, "save", Resize_BuildPreview));
	Widget_LoadView_Init(loadview, (Widget*)gui, "load", Resize_BuildPreview));
	Widget_Add((Widget*)gui, (Widget*)cstrview);
	Widget_Add((Widget*)gui, (Widget*)blpreview);
	Widget_Add((Widget*)gui, (Widget*)blview);
	Widget_Add((Widget*)gui, (Widget*)truckmgr);
	Widget_Add((Widget*)gui, (Widget*)saveview);
	Widget_Add((Widget*)gui, (Widget*)loadview);

	//In-game menu
	ingame = (ViewLayer*)malloc(sizeof(ViewLayer));
	whitebg = (Image*)malloc(sizeof(Image));
	quitmenu = (Link*)malloc(sizeof(Link));
	cancel = (Link*)malloc(sizeof(Link));
	RichText_Init_Str(&quitmenulabel, "Quit to Menu");
	RichText_Init_Str(&cancellabel, "Cancel");
	Widget_ViewLayer_Init(ingame, (Widget*)gui, "ingame");
	Widget_Image_Init(whitebg, (Widget*)ingame, "gui/backg/white.jpg", true, Resize_Fullscreen, 0, 0, 0, 0.5f);
	Widget_Link_Init(quitmenu, (Widget*)ingame, "0", &quitmenulabel, MAINFONT, Resize_MenuItem, Click_QuitToMenu);
	Widget_Link_Init(cancel, (Widget*)ingame, "1", &cancellabel, MAINFONT, Resize_MenuItem, Escape);
	Widget_Add((Widget*)gui, (Widget*)ingame);
	Widget_Add((Widget*)ingame, (Widget*)whitebg);
	Widget_Add((Widget*)ingame, (Widget*)quitmenu);
	Widget_Add((Widget*)ingame, (Widget*)cancel);
	RichText_Free(&quitmenulabel);
	RichText_Free(&cancellabel);

	//Message view
	msgviewl = (ViewLayer*)malloc(sizeof(ViewLayer));
	whitebg = (Image*)malloc(sizeof(Image));
	msgtextbl = (TextBlock*)malloc(sizeof(TextBlock));
	msgtouchl = (TouchListen*)malloc(sizeof(TouchListen));
	msgbutton = (Button*)malloc(sizeof(Button));
	RichText_Init_Str(&continuelabel, "Continue");
	Widget_ViewLayer_Init(msgviewl, (Widget*)gui, "message view");
	Widget_Image_Init(whitebg, (Widget*)msgviewl, "gui/backg/white.jpg", true, Resize_Message, 1.0f, 1.0f, 1.0f, 1.0f);
	Widget_TextBlock_Init(msgtextbl, (Widget*)msgviewl, "message", &blank, MAINFONT, Resize_Message);
	Widget_TouchListen_Init(msgtouchl, (Widget*)msgviewl, Resize_Fullscreen, NULL, NULL, NULL, -1);
	Widget_Button_Init(msgbutton, (Widget*)msgviewl, "continue button", "gui/transp.png", &continuelabel, &blank, MAINFONT, BUST_LEFTIMAGE, Resize_MessageContinue, Click_MessageContinue, NULL, NULL, NULL, NULL, -1);
	Widget_Add((Widget*)gui, (Widget*)msgviewl);
	Widget_Add((Widget*)msgviewl, (Widget*)whitebg);
	Widget_Add((Widget*)msgviewl, (Widget*)msgtextbl);
	Widget_Add((Widget*)msgviewl, (Widget*)msgtouchl);
	Widget_Add((Widget*)msgviewl, (Widget*)msgbutton);
	RichText_Free(&continuelabel);
}
