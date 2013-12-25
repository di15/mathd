
#include "../window.h"
#include "../render/shader.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/player.h"
#include "../sim/road.h"
#include "../sim/powerline.h"
#include "../sim/zpipeline.h"
#include "heightmap.h"
#include "water.h"
#include "../gui/gui.h"
#include "minimap.h"
#include "../math/camera.h"
#include "../debug.h"

/*
#define MINIMAP_X1		(MMBAR_WIDTH*6.0f/128.0f)
#define MINIMAP_X2		(MMBAR_WIDTH*121.0f/128.0f)
#define MINIMAP_Y1		(MMBAR_HEIGHT*6.0f/128.0f)
#define MINIMAP_Y2		(MMBAR_HEIGHT*114.0f/128.0f)
*/

bool g_minimapdrag = false;

bool OverMinimap()
{
	if(g_mouse.x >= MINIMAP_X1 && g_mouse.y >= MINIMAP_Y1 && g_mouse.x <= MINIMAP_X2 && g_mouse.y <= MINIMAP_Y2)
		return true;

	return false;
}

void ScrollToMouse()
{
	Vec3f moveto;
	moveto.x = (TILE_SIZE*g_hmap.m_widthX)*(g_mouse.x - MINIMAP_X1)/(float)(MINIMAP_X2-MINIMAP_X1);
	moveto.z = (TILE_SIZE*g_hmap.m_widthZ)*(g_mouse.y - MINIMAP_Y1)/(float)(MINIMAP_Y2-MINIMAP_Y1);
	ScrollTo(moveto);
}

void ScrollTo(Vec3f view)
{
	Vec3f line[2];
	Vec3f ray = g_camera.View() - g_camera.Position();
	Vec3f onnear = OnNear(g_width/2, g_height/2);
	Vec3f lineTopLeft[2];
	line[0] = onnear;
	line[1] = onnear + (ray * 100000.0f);
	Vec3f intersec;
	if(!GetMapIntersection(line, &intersec))
		GetMapIntersection2(line, &intersec);

	Vec3f landdist = line[0] - intersec;
	line[0] = line[1] = view;
	line[0].y += 100000;
	line[1].y -= 100000;
	if(!GetMapIntersection(line, &intersec))
		GetMapIntersection2(line, &intersec);

	g_camera.MoveTo(intersec + landdist);
	
	/*
	Vec3f p = g_camera.Position();
	float dy = p.y - Bilerp(p.x, p.z);
	view.y = Bilerp(view.x, view.z) + dy;
	g_camera.MoveTo(view);?*/
}

void DrawMinimap()
{
	//return;

	Use(COLOR2D);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::COLOR2D].m_slot[SLOT::HEIGHT], (float)g_height);
    glEnableVertexAttribArray(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION]);

	float spanx = MINIMAP_X2 - MINIMAP_X1;
	float spany = MINIMAP_Y2 - MINIMAP_Y1;
	
	float tilespanx = spanx / (g_hmap.m_widthX+1.0f);
	float tilespany = spany / (g_hmap.m_widthZ+1.0f);

	for(int x=0; x<=g_hmap.m_widthX; x++)
		for(int z=0; z<=g_hmap.m_widthZ; z++)
		{
			float h = g_hmap.getheight(x, z);
			if(h < WATER_HEIGHT)
				glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 0, 0, 0.5f, 1);
			else
			{
				glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 0, 0.5f + h / 40.0f, 0, 1);
			}
    
			float left = MINIMAP_X1 + x*tilespanx;
			float right = left + tilespanx;
			float top = MINIMAP_Y1 + z*tilespany;
			float bottom = top + tilespany;

			float vertices[] =
			{
				//posx, posy    texx, texy
				left, top, 0,          0, 0,
				left, bottom,0,         1, 0,
				right, bottom,0,      1, 1,
        
				right, top,0,      1, 1
			};
    
			glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
    
			glDrawArrays(GL_QUADS, 0, 4);
		}

		
	float tilespanx2 = spanx / (g_hmap.m_widthX);
	float tilespany2 = spany / (g_hmap.m_widthZ);
		
	CRoad* r;
	glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 0.5f, 0.5f, 0.5f, 0.75f);
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			r = RoadAt(x, z);

			if(!r->on)
				continue;

			float left = MINIMAP_X1 + x*tilespanx2;
			float right = left + tilespanx2;
			float top = MINIMAP_Y1 + z*tilespany2;
			float bottom = top + tilespany2;

			float vertices[] =
			{
				//posx, posy    texx, texy
				left, top, 0,          0, 0,
				left, bottom,0,         1, 0,
				right, bottom,0,      1, 1,
        
				right, top,0,      1, 1
			};
    
			glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
    
			glDrawArrays(GL_QUADS, 0, 4);
		}
		
	float scalex = spanx / (float)(g_hmap.m_widthX*TILE_SIZE);
	float scaley = spany / (float)(g_hmap.m_widthZ*TILE_SIZE);

	CBuilding* b;
	Vec3f pos;
	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		const float* owncol = facowncolor[b->owner];
		
		glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], owncol[0], owncol[1], owncol[2], 0.75f);

		pos = b->pos;

		float left = MINIMAP_X1 + pos.x*scalex;
		float right = left + tilespanx2;
		float top = MINIMAP_Y1 + pos.z*scaley;
		float bottom = top + tilespany2;

		float vertices[] =
		{
			//posx, posy    texx, texy
			left, top, 0,          0, 0,
			left, bottom,0,         1, 0,
			right, bottom,0,      1, 1,
        
			right, top,0,      1, 1
		};
    
		glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
    
		glDrawArrays(GL_QUADS, 0, 4);
	}

	LastNum("minimap u");

	CUnit* u;
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->owner < 0)
		{
			glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 1, 1, 1, 0.75f);
		}
		else
		{
			const float* owncol = facowncolor[u->owner];
			glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], owncol[0], owncol[1], owncol[2], 0.75f);
		}

		pos = u->camera.Position();

		float left = MINIMAP_X1 + pos.x*scalex;
		float right = left + tilespanx2;
		float top = MINIMAP_Y1 + pos.z*scaley;
		float bottom = top + tilespany2;

		float vertices[] =
		{
			//posx, posy    texx, texy
			left, top, 0,          0, 0,
			left, bottom,0,         1, 0,
			right, bottom,0,      1, 1,
        
			right, top,0,      1, 1
		};
    
		glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
    
		glDrawArrays(GL_QUADS, 0, 4);
	}

	
	LastNum("after minimap u");

	/*
	Vec3f ray = OnNear(MMBAR_WIDTH, 0);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = g_camera.Position();
	lineTopLeft[1] = g_camera.Position() + (ray * 100000.0f);

	ray = OnNear(g_width, 0);
	Vec3f lineTopRight[2];
	lineTopRight[0] = g_camera.Position();
	lineTopRight[1] = g_camera.Position() + (ray * 100000.0f);

	ray = OnNear(LBAR_WIDTH, g_height);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = g_camera.Position();
	lineBottomLeft[1] = g_camera.Position() + (ray * 100000.0f);

	ray = OnNear(g_width, g_height);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = g_camera.Position();
	lineBottomRight[1] = g_camera.Position() + (ray * 100000.0f);*/
	
	Vec3f ray = g_camera.View() - g_camera.Position();

	Vec3f onnear = OnNear(MMBAR_WIDTH, 0);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = onnear;
	lineTopLeft[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(g_width, 0);
	Vec3f lineTopRight[2];
	lineTopRight[0] = onnear;
	lineTopRight[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(LBAR_WIDTH, g_height);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = onnear;
	lineBottomLeft[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(g_width, g_height);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = onnear;
	lineBottomRight[1] = onnear + (ray * 100000.0f);
	
	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;
	
	//if(!GetMapIntersection(lineTopLeft, &interTopLeft))
		GetMapIntersection2(lineTopLeft, &interTopLeft);
	//if(!GetMapIntersection(lineTopRight, &interTopRight))
		GetMapIntersection2(lineTopRight, &interTopRight);
	//if(!GetMapIntersection2(lineBottomLeft, &interBottomLeft))
		GetMapIntersection2(lineBottomLeft, &interBottomLeft);
	//if(!GetMapIntersection2(lineBottomRight, &interBottomRight))
		GetMapIntersection2(lineBottomRight, &interBottomRight);
	
	float topleft[] = {interTopLeft.x * scalex, interTopLeft.z * scaley};
	float topright[] = {interTopRight.x * scalex, interTopRight.z * scaley};
	float botleft[] = {interBottomLeft.x * scalex, interBottomLeft.z * scaley};
	float botright[] = {interBottomRight.x * scalex, interBottomRight.z * scaley};

	float vertices[] =
	{
		//posx, posy    texx, texy
		topleft[0], topleft[1], 0,          0, 0,
		botleft[0], botleft[1],0,         1, 0,
		botright[0], botright[1],0,      1, 1,
        
		topright[0], topright[1],0,      1, 1,
		topleft[0], topleft[1], 0,          0, 0
	};
    
	glVertexAttribPointer(g_shader[SHADER::COLOR2D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
    
	glUniform4f(g_shader[SHADER::COLOR2D].m_slot[SLOT::COLOR], 1, 1, 1, 0.75f);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
	
	Use(ORTHO);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER::ORTHO].m_slot[SLOT::HEIGHT], (float)g_height);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::POSITION]);
    glEnableVertexAttribArray(g_shader[SHADER::ORTHO].m_slot[SLOT::TEXCOORD0]);
}