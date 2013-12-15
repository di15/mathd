

#include "transaction.h"
#include "shader.h"
#include "main.h"
#include "font.h"
#include "resource.h"
#include "gui.h"

CTransaction g_transx[TRANSACTIONS];

void DrawTransactions(Matrix projmodlview)
{
	Vec3f* pos;
	Vec4f screenpos;
	CTransaction* t;
	string text;
	int size = g_font[MAINFONT8].gheight;
	int x1, y1;
	float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	for(int i=0; i<TRANSACTIONS; i++)
	{
		t = &g_transx[i];

		if(!t->on)
			continue;

		pos = &t->pos;

		screenpos.x = pos->x;
		screenpos.y = pos->y;
		screenpos.z = pos->z;
		screenpos.w = 1;

		screenpos.Transform(projmodlview);
		screenpos = screenpos / screenpos.w;
		screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
		screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;
		
		x1 = screenpos.x - t->halfwidth;
		y1 = screenpos.y;
		color[3] = t->life * STATUS_ALPHA;

		DrawCenterShadText(MAINFONT8, x1, y1, t->text, color);
		
		t->pos.y += TRANSACTION_RISE;
		t->life -= TRANSACTION_DECAY;

		if(t->life <= 0.0f)
			t->on = false;
	}
}

int NewTransx()
{
	for(int i=0; i<TRANSACTIONS; i++)
		if(!g_transx[i].on)
			return i;

	return -1;
}

void NewTransx(Vec3f pos, int res1, float res1d, int res2, float res2d)
{
	int i = NewTransx();
	if(i < 0)
		return;

	CTransaction* t = &g_transx[i];
	t->on = true;
	t->life = 1;
	t->text[0] = '\0';
	TransxAppend(t->text, res1, res1d);
	TransxAppend(t->text, res2, res2d);
	t->pos = pos + Vec3f(0, 5, 0);
}

void NewTransx(Vec3f pos, int res1, float res1d)
{
	int i = NewTransx();
	if(i < 0)
		return;

	CTransaction* t = &g_transx[i];
	t->on = true;
	t->life = 1;
	t->text[0] = '\0';
	TransxAppend(t->text, res1, res1d);
	t->pos = pos + Vec3f(0, 5, 0);
}

void NewTransx(Vec3f pos, const char* text)
{
	int i = NewTransx();
	if(i < 0)
		return;

	CTransaction* t = &g_transx[i];
	t->on = true;
	t->life = 1;
	strcpy(t->text, text);
	t->pos = pos + Vec3f(0, 5, 0);
}

void TransxAppend(char* str, int res1, float res1d)
{
	char append[16];

	if(res1 == RESOURCE::CURRENC)
	{
		if(res1d >= 0.0f)
			sprintf(append, "%c+%0.2f ", g_resource[res1].icon, res1d);
		else
			sprintf(append, "%c%0.2f ", g_resource[res1].icon, res1d);
	}
	else
	{
		if(res1d >= 0.0f)
			sprintf(append, "%c+%d ", g_resource[res1].icon, (int)res1d);
		else
			sprintf(append, "%c%d ", g_resource[res1].icon, (int)res1d);
	}

	strcat(str, append);
}