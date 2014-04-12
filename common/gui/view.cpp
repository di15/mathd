

#include "gui.h"
#include "../texture.h"

void View::draw()
{
	//if(_stricmp(name.c_str(), "brush edit") == 0)
	//	g_log<<"draw brush edit"<<endl;

	//g_log<<name<<endl;
	//g_log.flush();

	for(auto i=widget.begin(); i!=widget.end(); i++)
	{
	//g_log<<(*i)->m_name<<endl;
	//g_log.flush();

		(*i)->draw();
	}
}

void View::draw2()
{
	//g_log<<name<<"2"<<endl;
	//g_log.flush();

	for(auto i=widget.rbegin(); i!=widget.rend(); i++)
	{
	//g_log<<(*i)->m_name<<"2"<<endl;
	//g_log.flush();
		(*i)->draw2();
	}
}