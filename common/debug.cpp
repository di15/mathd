


#include "platform.h"
#include "debug.h"
#include "utils.h"
//#include "main.h"
//#include "gui.h"
//#include "unit.h"
//#include "pathfinding.h"
//#include "collision.h"
//#include "building.h"
#include "gui/gui.h"
#include "gui/widget.h"
#include "gui/widgets/spez/constructionview.h"
#include "sim/player.h"
#include "window.h"

void CheckNum(const char* num)
{
	g_log<<"Check #"<<num;
	g_log.flush();

	Player* py = &g_player[g_currP];
	GUI* gui = &py->gui;

	ViewLayer* v = (ViewLayer*)gui->get("construction view");
	ConstructionView* cv = (ConstructionView*)v->get("construction view");
	Text* tl = &cv->titletext;

	g_log<<": ";
	g_log.flush();

	g_log<<tl->m_text.rawstr()<<endl;
	g_log.flush();
}

void LastNum(const char* l)
{
	return;

#if 1
	char fullpath[MAX_PATH+1];
	FullPath("last.txt", fullpath);
	ofstream last;
	last.open(fullpath, ios_base::out);
	last<<l;
	last.flush();
#else
	g_log<<l<<endl;
	g_log.flush();
#endif
}

void CheckGLError(const char* file, int line)
{
	//char msg[2048];
	//sprintf(msg, "Failed to allocate memory in %s on line %d.", file, line);
	//ErrorMessage("Out of memory", msg);
	int error = glGetError();

	if(error == GL_NO_ERROR)
		return;

	g_log<<"GL Error #"<<error<<" in "<<file<<" on line "<<line<<" using shader #"<<g_curS<<endl;
}


void LogRich(const RichText* rt)
{
	g_log<<"RichText: "<<endl;

	for(auto rtiter = rt->m_part.begin(); rtiter != rt->m_part.end(); rtiter++)
	{
		if(rtiter->m_type == RICHTEXT_ICON)
		{
			g_log<<"[icon"<<rtiter->m_icon<<"]";
		}
		else if(rtiter->m_type == RICHTEXT_TEXT)
		{
			const UString* ustr = &rtiter->m_text;
			
			for(int i=0; i<ustr->m_length; i++)
			{
				g_log<<"[char#"<<ustr->m_data[i]<<"'"<<(char)ustr->m_data[i]<<"']";
			}
		}
	}

	g_log<<endl;
	g_log.flush();
}

#if 0
void UDebug(int i)
{
	return;

	Unit* u = &g_unit[i];
	UnitT* t = &g_unitType[u->type];

	g_log<<"UNIT DEBUG: "<<t->name<<" ("<<i<<")"<<endl;
	g_log<<"path size: "<<u->path.size()<<endl;
	
	if(u->collidesfast())
	{
		g_log<<"COLLIDES: type:"<<g_collidertype<<" ID:"<<g_lastcollider<<endl;

		if(g_collidertype == COLLIDER_BUILDING)
		{
			Building* b = &g_building[g_lastcollider];
			BuildingT* bt = &g_buildingT[b->type];

			g_log<<"COLLIDER B: "<<bt->name<<endl;

			if(u->confirmcollision(g_collidertype, g_lastcollider))
			{
				g_log<<"CONFIRMED COLLISION"<<endl;

				Vec3f p = u->camera.Position();
				Vec3f p2 = b->pos;

				float r = t->radius;
				float hwx = bt->widthX*TILE_SIZE/2.0f;
				float hwz = bt->widthZ*TILE_SIZE/2.0f;

				g_log<<"COLLISION DX,DZ: "<<(fabs(p2.x-p.x)-r-hwx)<<","<<(fabs(p2.z-p.z)-r-hwz)<<endl;
			}
		}
	}
}
#endif