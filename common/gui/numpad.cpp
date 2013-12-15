

#include "numpad.h"
#include "gui.h"
#include "menu.h"
#include "font.h"
#include "main.h"
#include "chat.h"

int g_numPadPow = 0;
float g_numPadVal = 0;
bool g_numPadDecimal = false;
char g_numPadAfter[8];
char g_numPadBefore[8];
void (*g_numPadAccept)() = NULL;
void (*g_numPadCancel)() = NULL;


void Click_NextNumPad()
{
	NextPage("numpad keys");
}

void Click_NumPadLeft()
{
	//Chat("numpad left");

	if(g_numPadPow > NUMPAD_FIGURES-2)
		return;

	g_numPadPow ++;
	NumPadUnderscore();
}

void Click_NumPadRight()
{
	//Chat("numpad right");

	if(!g_numPadDecimal && g_numPadPow <= 0)
		return;

	if(g_numPadDecimal && g_numPadPow <= -2)
		return;

	g_numPadPow --;
	NumPadUnderscore();
}

void Click_NumPadPlus()
{
	NumPadChange( pow(10, g_numPadPow) );
}

void Click_NumPadMinus()
{
	NumPadChange( -pow(10, g_numPadPow) );
}

void Click_NumPadAccept()
{
	if(g_numPadAccept != NULL)
		g_numPadAccept();
}

void Click_NumPadCancel()
{
	if(g_numPadCancel != NULL)
		g_numPadCancel();
}


void CloseNumPad()
{
	CloseView("numpad");
	CloseView("numpad keys");
}

void NumPadUnderscore()
{
	CView* v = g_GUI.getview("numpad");
	CWidget* number = v->getwidget("number", TEXT);
	CWidget* underscore = v->getwidget("underscore", TEXT);

	float x = number->pos[0];
	CFont* f = &g_font[number->font];

	for(int i=0; i<strlen(g_numPadBefore); i++)
		x += f->glyph[ g_numPadBefore[i] ].w;

	int end = NUMPAD_FIGURES - g_numPadPow - 1;

	if(g_numPadPow < 0)
		end += 1;

	const char* numberstr = number->text.c_str();

	for(int i=0; i<end; i++)
		x += f->glyph[ numberstr[i] ].w;

	underscore->pos[0] = x;
}

float NumPadMax()
{
	float maxval = 0;

	for(int i=0; i<NUMPAD_FIGURES; i++)
		maxval += 9.0f * pow(10, i);

	if(g_numPadDecimal)
		maxval += 0.99;

	return maxval;
}

void NumPadChange(float change)
{
	float newval = g_numPadVal + change;

	if(newval > NumPadMax())
		return;

	if(newval < 0.00f)
		return;

	g_numPadVal += change;

	CView* v = g_GUI.getview("numpad");
	CWidget* number = v->getwidget("number", TEXT);

	char numberstr[32];
	if(g_numPadDecimal)
		sprintf(numberstr, "%s%06.2f%s", g_numPadBefore, g_numPadVal, g_numPadAfter);
	else
		sprintf(numberstr, "%s%03.0f%s", g_numPadBefore, g_numPadVal, g_numPadAfter);

	number->text = numberstr;
	
	NumPadUnderscore();
}

void OpenNumPad(void (*accept)(), void (*cancel)(), float val, bool decimal, const char* beforestr, const char* afterstr)
{
	OpenAnotherView("numpad");
	OpenAnotherView("numpad keys");
	g_numPadAccept = accept;
	g_numPadCancel = cancel;
	g_numPadVal = 0;
	g_numPadDecimal = decimal;
	g_numPadPow = 0;
	strcpy(g_numPadBefore, beforestr);
	strcpy(g_numPadAfter, afterstr);

	NumPadChange(val);
}