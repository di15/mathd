

#include "bgraph.h"
#include "main.h"
#include "gui.h"
#include "menu.h"
#include "inspect.h"
#include "chat.h"
#include "building.h"
#include "resource.h"
#include "shader.h"
#include "selection.h"
#include "font.h"

BGraph g_bgraph;

BGraph::BGraph()
{
	reset();
}

void BGraph::reset()
{
	periodicmarks.clear();
	bglines.clear();
	for(int i=0; i<RESOURCES; i++)
	{
		rescurve[i].clear();
		rescurve2[i].clear();
		resnamecolor[i].clear();
		resrelevant[i] = false;
		resrelevant2[i] = false;
	}
}

void BGraph::draw()
{/*
	float gray[] = {0.5f, 0.5f, 0.5f, 1.0f};
	for(int y=0; y<20; y++)
		DrawCenterShadText(MAINFONT8, g_width/2, g_height/2 + y*20, "oaslkasdka;sdkads", gray);

	Chat("call");*/

	UseS(COLOR2D);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::HEIGHT], (float)g_height);
    //glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 1, 1, 1, 1);
    glEnableVertexAttribArray(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION]);
	
    glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 0.8f, 0.8f, 0.8f, 1);
	for(int i=0; i<bglines.size(); i++)
	{
		glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, bglines[i].pos);
		glDrawArrays(GL_LINES, 0, 2);
	}

	for(int i=0; i<RESOURCES; i++)
	{
		if(!resrelevant[i] && !resrelevant2[i])
			continue;

		const CResource* r = &g_resource[i];
		const float* rgba = r->rgba;
		glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], rgba[0], rgba[1], rgba[2], rgba[3]);
		
		if(resrelevant[i])
		{
			glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &(rescurve[i][0]));
			glDrawArrays(GL_LINE_STRIP, 0, rescurve[i].size());
		}

		if(resrelevant2[i])
		{
			glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &(rescurve2[i][0]));
			glDrawArrays(GL_LINE_STRIP, 0, rescurve2[i].size());
		}
	}

	UseS(ORTHO);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::HEIGHT], (float)g_height);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::POSITION]);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::TEXCOORD0]);
	
	for(int i=0; i<periodicmarks.size(); i++)
	{
		periodicmarks[i].draw();
	}

	for(int i=0; i<RESOURCES; i++)
	{
		if(!resrelevant[i] && !resrelevant2[i])
			continue;

		resnamecolor[i].draw();
	}
}

void DrawBGraph()
{
	g_bgraph.draw();
}

void FillBGraph()
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType != SELECT_BUILDING)
		return;

	CBuilding* b = &g_building[g_selection[0]];

	const int frametop = g_height - 242 + 5;
	const int framebottom = g_height - 80;
	const int frameleft = 40;
	const int frameright = g_width - 40;
	const int framew = frameright - frameleft;
	const int frameh = framebottom - frametop;

	int maxperiodics = framew / 70;
	int skipperiods = max(1, b->cycleh.size() / maxperiodics);

	int endframes = b->cycleh.size() * CYCLE_FRAMES;
	int endseconds = endframes / FRAME_RATE;
	int allsec = endseconds;
	const int periods = b->cycleh.size();

	BGraph* graph = &g_bgraph;
	graph->reset();
	
	float gray[] = {0.5f, 0.5f, 0.5f, 1.0f};

	for(int i=0; i<periods; i+=skipperiods)
	{
		//Chat("skipped period");

		float horizratio = (float)(i*2+1) / (float)(periods*2+2);
		int mins = allsec / 60;
		int secs = allsec - 60 * mins;

		char periodstr[16];
		sprintf(periodstr, "-%d:%02d", mins, secs);
		Widget periodw;
		periodw.Text("name", periodstr, MAINFONT8, frameleft + horizratio * (float)framew - 15, framebottom, true, gray[0], gray[1], gray[2], gray[3]);
		graph->periodicmarks.push_back(periodw);

		allsec -= skipperiods*CYCLE_FRAMES/FRAME_RATE;
	}

	for(int i=0; i<periods; i++)
	{
		//Chat("line period");

		Line line;
		float horizratio = (float)(i*2+1) / (float)(periods*2+2);
		
		line.pos[0] = Vec3f(frameleft + horizratio * (float)framew, frametop, 0);
		line.pos[1] = Vec3f(frameleft + horizratio * (float)framew, framebottom, 0);

		graph->bglines.push_back(line);
	}

	float maxstock = 1;
	float maxcons = 1;
	for(int i=0; i<b->cycleh.size(); i++)
	{
		CCycleH* ch = &b->cycleh[i];
		for(int j=0; j<RESOURCES; j++)
		{
			if(ch->consumed[j] > 0.0f)
				graph->resrelevant2[j] = true;
				
			if(ch->produced[j] > 0.0f)
				graph->resrelevant[j] = true;

			if(ch->produced[j] > maxstock)
				maxstock = ch->produced[j];
			
			if(ch->consumed[j] > maxcons)
				maxcons = ch->consumed[j];
		}
	}

	float stockrange = maxstock + maxcons;

	int colw = 150;
	int rowh = g_font[MAINFONT8].gheight;
	int x = frameleft + 20;
	int y = frametop;

	for(int i=0; i<RESOURCES; i++)
	{
		if(!graph->resrelevant[i] && !graph->resrelevant2[i])
			continue;

		CResource* r = &g_resource[i];
		char resstr[64];
		sprintf(resstr, "%c %s", r->icon, r->name);
		const float* rgba = r->rgba;
		Widget* restext = &graph->resnamecolor[i];
		restext->Text(g_resource[i].name, resstr, MAINFONT8, x, y, true, rgba[0], rgba[1], rgba[2], rgba[3]);

		x += colw;

		if(x + colw >= frameright)
		{
			x = frameleft + 20;
			y += rowh;
		}
	}
	
	Widget prodtext;
	float vratio = (maxstock / (maxstock+maxcons));
	prodtext.Text("name", "Production and Earnings", MAINFONT8, frameleft + framew/2 - 75, frametop + (float)frameh * vratio - rowh, true, gray[0], gray[1], gray[2], gray[3]);
	graph->periodicmarks.push_back(prodtext);
	Widget constext;
	constext.Text("name", "Consumption and Expenses", MAINFONT8, frameleft + framew/2 - 75, frametop + (float)frameh * vratio, true, gray[0], gray[1], gray[2], gray[3]);
	graph->periodicmarks.push_back(constext);
	
	for(int i=0; i<maxstock; i+=10)
	{
		Line line;
		float vertratio = (1.0f - (float)(i)/maxstock) * maxstock / (maxstock+maxcons);
		
		line.pos[0] = Vec3f(frameleft, frametop + (float)frameh * vertratio, 0);
		line.pos[1] = Vec3f(frameright, frametop + (float)frameh * vertratio, 0);

		graph->bglines.push_back(line);

		char periodstr[16];
		sprintf(periodstr, "%d", i);
		Widget periodw;
		periodw.Text("name", periodstr, MAINFONT8, frameleft, frametop + (float)frameh * vertratio, true, gray[0], gray[1], gray[2], gray[3]);
		graph->periodicmarks.push_back(periodw);
	}
	
	//for(int i=5; i<maxcons; i+=5)
	for(int i=10; i<maxcons; i+=10)
	{
		Line line;
		float vertratio = (float)(i) / (maxstock+maxcons) + (maxstock / (maxstock+maxcons));
		
		line.pos[0] = Vec3f(frameleft, frametop + (float)frameh * vertratio, 0);
		line.pos[1] = Vec3f(frameright, frametop + (float)frameh * vertratio, 0);

		graph->bglines.push_back(line);

		char periodstr[16];
		sprintf(periodstr, "-%d", i);
		Widget periodw;
		periodw.Text("name", periodstr, MAINFONT8, frameleft, frametop + (float)frameh * vertratio, true, gray[0], gray[1], gray[2], gray[3]);
		graph->periodicmarks.push_back(periodw);
	}

	for(int i=0; i<b->cycleh.size(); i++)
	{
		float horizratio = (float)(i*2+1) / (float)(b->cycleh.size()*2+2);

		CCycleH* ch = &b->cycleh[i];
		for(int j=0; j<RESOURCES; j++)
		{
			if(graph->resrelevant[j])
			{
				//float vertratio = 1.0f - (float)(ch->produced[j]) / (float)(maxstock);
				float vertratio = (1.0f - (float)(ch->produced[j])/maxstock) * maxstock / (maxstock+maxcons);

				Vec3f point = Vec3f(frameleft + (float)framew * horizratio, frametop + (float)frameh * vertratio, 0);
				graph->rescurve[j].push_back(point);
			}
			
			if(graph->resrelevant2[j])
			{
				float vertratio2 = (float)(ch->consumed[j]) / (float)(maxcons) * (maxcons / (maxstock+maxcons)) + (maxstock / (maxstock+maxcons));

				Vec3f point2 = Vec3f(frameleft + (float)framew * horizratio, frametop + (float)frameh * vertratio2, 0);
				graph->rescurve2[j].push_back(point2);
			}
		}
	}
}