
#include "chat.h"
#include "gui.h"
#include "../../game/gmain.h"

void Chat(const char* chat)
{
#if 0
	g_GUI.getview("chat")->getwidget("chat0", TEXT)->text = g_GUI.getview("chat")->getwidget("chat1", TEXT)->text;
	g_GUI.getview("chat")->getwidget("chat1", TEXT)->text = g_GUI.getview("chat")->getwidget("chat2", TEXT)->text;
	g_GUI.getview("chat")->getwidget("chat2", TEXT)->text = g_GUI.getview("chat")->getwidget("chat3", TEXT)->text;
	g_GUI.getview("chat")->getwidget("chat3", TEXT)->text = g_GUI.getview("chat")->getwidget("chat4", TEXT)->text;
	g_GUI.getview("chat")->getwidget("chat4", TEXT)->text = g_GUI.getview("chat")->getwidget("chat5", TEXT)->text;
	g_GUI.getview("chat")->getwidget("chat5", TEXT)->text = chat;
#endif
}