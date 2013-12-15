
#include "main.h"
#include "order.h"
#include "unit.h"
#include "selection.h"
#include "shader.h"
#include "sound.h"
#include "player.h"
#include "chat.h"

vector<COrder> g_order;

void DrawOrders()
{
	glBindTexture(GL_TEXTURE_2D, g_circle);
	//glBegin(GL_QUADS);
	
	COrder* o;
	Vec3f p;
	float r;
	float a;

	Shader* s = &g_shader[BILLBOARD];

	for(int i=0; i<g_order.size(); i++)
	{
		o = &g_order[i];
		p = o->pos;
		r = o->radius;
		a = 1.0f - (float)(GetTickCount() - o->tick)/(float)ORDER_EXPIRE;

		/*
		glColor4f(1, 1, 1, a);
	
		glTexCoord2f(0, 0);		glVertex3f(p.x - r, p.y + 1, p.z - r);
		glTexCoord2f(0, 1);		glVertex3f(p.x - r, p.y + 1, p.z + r);
		glTexCoord2f(1, 1);		glVertex3f(p.x + r, p.y + 1, p.z + r);
		glTexCoord2f(1, 0);		glVertex3f(p.x + r, p.y + 1, p.z - r);
		*/
		
		glUniform4f(s->m_slot[SLOT::COLOR], 1, 1, 1, a);

        float vertices[] =
        {
            //posx, posy posz   texx, texy
            p.x + r, p.y + 1, p.z - r,          1, 0,
            p.x + r, p.y + 1, p.z + r,          1, 1,
            p.x - r, p.y + 1, p.z + r,          0, 1,
            
            p.x - r, p.y + 1, p.z + r,          0, 1,
            p.x - r, p.y + 1, p.z - r,          0, 0,
            p.x + r, p.y + 1, p.z - r,          1, 0
        };
		
		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		
		glVertexAttribPointer(s->m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexAttribPointer(s->m_slot[SLOT::TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

        glDrawArrays(GL_TRIANGLES, 0, 6);

		if(GetTickCount() - o->tick > ORDER_EXPIRE)
		{
			g_order.erase(g_order.begin() + i);
			i--;
		}
	}

	//glEnd();
	//glColor4f(1, 1, 1, 1);
	glUniform4f(s->m_slot[SLOT::COLOR], 1, 1, 1, 1);
}

void Order()
{
	vector<int> selection = g_selection;
	int selecttype = g_selectType;

	g_selection.clear();

	SelectOne();

	vector<int> targets = g_selection;
	int targettype = g_selectType;

	g_selection = selection;
	g_selectType = selecttype;

	//char msg[128];
	//sprintf(msg, "s.size()=%d, stype=%d", (int)g_selection.size(), g_selectType);
	//Chat(msg);

	if(targets.size() <= 0 || (targettype != SELECT_UNIT && targettype != SELECT_BUILDING))
	{
		Vec3f center(0, 0, 0);
		Vec3f min = Vec3f(g_hmap.m_widthX*TILE_SIZE, 0, g_hmap.m_widthZ*TILE_SIZE);
		Vec3f max = Vec3f(0, 0, 0);
		int i;
		CUnit* u;
		CUnitType* t;
		Vec3f order = g_vMouse;
		order.x = Clip(order.x, 0, g_hmap.m_widthX*TILE_SIZE);
		order.z = Clip(order.z, 0, g_hmap.m_widthZ*TILE_SIZE);
		Vec3f p;
		float radius = 0;

		for(int j=0; j<g_selection.size(); j++)
		{
			i = g_selection[j];
			u = &g_unit[i];

			if(u->hp <= 0.0f)
			{
				g_selection.erase( g_selection.begin() + j );
				j--;
				continue;
			}

			p = u->camera.Position();
			u->target = -1;
			u->underOrder = true;

			if(p.x < min.x)
				min.x = p.x;
			if(p.z < min.z)
				min.z = p.z;
			if(p.x > max.x)
				max.x = p.x;
			if(p.z > max.z)
				max.z = p.z;
		
			//center = center + p;
			center = center + p / (float)g_selection.size();
		}

		//center = center / (float)g_selection.size();
		//Vec3f half = (min + max) / 2.0f;

		// All units to one goal
		//if(fabs(center.x - order.x) < half.x && fabs(center.z - order.z) < half.z)
		if(order.x <= max.x && order.x >= min.x && order.z <= max.z && order.z >= min.z)
		{	
			for(int j=0; j<g_selection.size(); j++)
			{
				i = g_selection[j];
				u = &g_unit[i];
				u->goal = order;
				u->underOrder = true;
				t = &g_unitType[u->type];
				if(t->radius > radius)
					radius = t->radius;
				u->goal.x = Clip(u->goal.x, 0 + t->radius, g_hmap.m_widthX*TILE_SIZE - t->radius);
				u->goal.z = Clip(u->goal.z, 0 + t->radius, g_hmap.m_widthZ*TILE_SIZE - t->radius);
				u->nopath = false;
			}

			g_order.push_back(COrder(order, GetTickCount(), radius));
		}
		// Formation goal
		else
		{
			Vec3f offset;

			for(int j=0; j<g_selection.size(); j++)
			{
				i = g_selection[j];
				u = &g_unit[i];
				p = u->camera.Position();
				offset = p - center;
				u->goal = order + offset;
				t = &g_unitType[u->type];
				radius = t->radius;
				u->goal.x = Clip(u->goal.x, 0 + radius, g_hmap.m_widthX*TILE_SIZE - radius);
				u->goal.z = Clip(u->goal.z, 0 + radius, g_hmap.m_widthZ*TILE_SIZE - radius);
				u->nopath = false;
				g_order.push_back(COrder(u->goal, GetTickCount(), radius));
			}
		}
	
		AckSnd();
	}
	else if(targets.size() > 0 && targettype == SELECT_UNIT)
	{
		int targi = targets[0];
		CUnit* targu = &g_unit[targi];
		Vec3f p = targu->camera.Position();

		if(targu->owner == g_localP)
			return;

		MakeWar(g_localP, targu->owner);

		for(int j=0; j<g_selection.size(); j++)
		{
			int i = g_selection[j];
			CUnit* u = &g_unit[i];
			u->goal = p;
			u->underOrder = true;
			u->targetU = true;
			u->target = targi;
		}
		
		AckSnd();
	}
	else if(targets.size() > 0 && targettype == SELECT_BUILDING)
	{
		int targi = targets[0];
		CBuilding* targb = &g_building[targi];
		Vec3f p = targb->pos;

		if(targb->owner == g_localP)
			return;

		MakeWar(g_localP, targb->owner);

		for(int j=0; j<g_selection.size(); j++)
		{
			int i = g_selection[j];
			CUnit* u = &g_unit[i];
			u->goal = p;
			u->underOrder = true;
			u->targetU = false;
			u->target = targi;
		}
		
		AckSnd();
	}
}