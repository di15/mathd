#include "gui.h"
#include "../texture.h"
#include "../sim/player.h"

void ViewLayer::draw()
{
	if(!m_Opened)
		return;

	for(auto w=m_subwidg.begin(); w!=m_subwidg.end(); w++)
		(*w)->draw();

	subdraw();
}

void ViewLayer::drawover()
{
	if(!m_Opened)
		return;

	for(auto w=m_subwidg.begin(); w!=m_subwidg.end(); w++)
		(*w)->drawover();

	subdrawover();
}

void ViewLayer::inev(InEv* ie)
{
	if(!m_Opened)
		return;

	for(auto w=m_subwidg.rbegin(); w!=m_subwidg.rend(); w++)
		(*w)->inev(ie);

	subinev(ie);
}

void ViewLayer::reframe()
{
	Player* py = &g_player[g_localP];

	m_pos[0] = 0;
	m_pos[1] = 0;
	m_pos[2] = g_width-(float)1;
	m_pos[3] = g_height-(float)1;

	if(reframefunc)
		reframefunc(this);

	for(auto w=m_subwidg.begin(); w!=m_subwidg.end(); w++)
		(*w)->reframe();

	subreframe();
}
