
#include "main.h"
#include "selection.h"
#include "unit.h"
#include "building.h"
#include "powerline.h"
#include "pipeline.h"
#include "road.h"
#include "3dmath.h"
#include "player.h"
#include "model.h"
#include "chat.h"
#include "image.h"
#include "gui.h"
#include "shader.h"
#include "menu.h"
#include "editor.h"
#include "inspect.h"
#include "scenery.h"
#include "debug.h"

vector<int> g_selection;
vector<POINT> g_selection2;
int g_selectType;

Vec3f g_normalLeft;
Vec3f g_normalTop;
Vec3f g_normalRight;
Vec3f g_normalBottom;
float g_distLeft;
float g_distTop;
float g_distRight;
float g_distBottom;
void (*SelectionFunc)() = NULL;

bool PointInsidePlanes(Vec3f p)
{
	if(!PointBehindPlane(p, g_normalLeft, g_distLeft))
		return false;

	if(!PointBehindPlane(p, g_normalTop, g_distTop))
		return false;

	if(!PointBehindPlane(p, g_normalRight, g_distRight))
		return false;

	if(!PointBehindPlane(p, g_normalBottom, g_distBottom))
		return false;

	return true;
}
void DrawSelectionBuilding(Matrix projection, Matrix viewmat, Matrix modelmat)
{
	Use(COLOR3D);
    glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 0, 1, 0, 1);
	glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION]);

	int i = g_selection[0];
	CBuilding* b = &g_building[i];
	CBuildingType* t = &g_buildingType[b->type];
	Vec3f p = b->pos;
	float hwx = t->widthX*TILE_SIZE/2;
	float hwz = t->widthZ*TILE_SIZE/2;

	//glDisable(GL_TEXTURE_2D);
	//glColor4f(0, 1, 0, 1);
	//glBegin(GL_LINE_STRIP);
	
	//glVertex3f(p.x - hwx, p.y + 1, p.z - hwz);
	//glVertex3f(p.x - hwx, p.y + 1, p.z + hwz);
	//glVertex3f(p.x + hwx, p.y + 1, p.z + hwz);
	//glVertex3f(p.x + hwx, p.y + 1, p.z - hwz);
	//glVertex3f(p.x - hwx, p.y + 1, p.z - hwz);

	Shader* s = &g_shader[COLOR3D];
	
	float vertices[] =
    {
            //posx, posy posz   texx, texy
            p.x - hwx, p.y + 1, p.z - hwz,          1, 0,
            p.x - hwx, p.y + 1, p.z + hwz,          1, 1,
            p.x + hwx, p.y + 1, p.z + hwz,          0, 1,
            
            p.x + hwx, p.y + 1, p.z - hwz,          0, 1,
            p.x - hwx, p.y + 1, p.z - hwz,          0, 0
     };
		

	//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
	//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		
	glVertexAttribPointer(s->m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
	//glVertexAttribPointer(s->m_slot[SLOT::TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
	//glVertexAttribPointer(s->m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

    glDrawArrays(GL_LINE_STRIP, 0, 5);

	//glEnd();
	//glColor4f(1, 1, 1, 1);
	//glEnable(GL_TEXTURE_2D);
}

void DrawSelectionUnits(Matrix projection, Matrix viewmat, Matrix modelmat)
{
	Use(MODEL);
    glUniformMatrix4fv(g_shader[SHADER::MODEL].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::MODEL].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::MODEL].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
	glUniform4f(g_shader[SHADER::MODEL].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	glEnableVertexAttribArray(g_shader[SHADER::MODEL].m_slot[SLOT::POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER::MODEL].m_slot[SLOT::TEXCOORD0]);
	glEnableVertexAttribArray(g_shader[SHADER::MODEL].m_slot[SLOT::NORMAL]);

	glBindTexture(GL_TEXTURE_2D, g_circle);
	//glBegin(GL_QUADS);
	
	int j;
	CUnit* u;
	CUnitType* t;
	Vec3f p;
	float r;

	Shader* s = &g_shader[MODEL];

	for(int i=0; i<g_selection.size(); i++)
	{
		j = g_selection[i];
		u = &g_unit[j];
		t = &g_unitType[u->type];
		p = u->camera.Position();
		r = t->radius*1.5f;

		if(!u->on || u->hp <= 0.0f)
		{
			g_selection.erase( g_selection.begin() + i );
			i--;
			continue;
		}
	
		//glTexCoord2f(0, 0);		glVertex3f(p.x - r, p.y + 1, p.z - r);
		//glTexCoord2f(0, 1);		glVertex3f(p.x - r, p.y + 1, p.z + r);
		//glTexCoord2f(1, 1);		glVertex3f(p.x + r, p.y + 1, p.z + r);
		//glTexCoord2f(1, 0);		glVertex3f(p.x + r, p.y + 1, p.z - r);

		//glUniform4f(s->m_slot[SLOT::COLOR], 1, 1, 1, 1);

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
	}

	//glEnd();
}

void DrawSelection(Matrix projection, Matrix viewmat, Matrix modelmat)
{
	if(g_selection.size() <= 0)
		return;

	if(g_selectType == SELECT_BUILDING)
		DrawSelectionBuilding(projection, viewmat, modelmat);
	else if(g_selectType == SELECT_UNIT)
		DrawSelectionUnits(projection, viewmat, modelmat);
}

bool BuildingIntersect(int i, Vec3f line[])
{
	CBuilding* b = &g_building[i];
	CBuildingType* t = &g_buildingType[b->type];/*
	CModel* m = &g_model[t->model];
	if(!b->finished)
		m = &g_model[t->conmodel];

	CVertexArray* va = &m->vertexArrays[b->frame];
	Vec3f poly[3];

	for(int v=0; v<va->numverts; v+=3)
	{
		poly[0] = va->vertices[v+0] + b->pos;
		poly[1] = va->vertices[v+1] + b->pos;
		poly[2] = va->vertices[v+2] + b->pos;

		if(IntersectedPolygon(poly, line, 3))
			return true;
	}*/

	
	Vec3f norms[6];
	float ds[6];
	MakeHull(norms, ds, b->pos, t->widthX*TILE_SIZE/2.0f, t->widthZ*TILE_SIZE/2.0f, TILE_SIZE/3.0f);

	if(LineInterHull(line, norms, ds, 6))
		return true;

	return false;
}

//Rotate(Vec3f v, float rad, float x, float y, float z)
bool UnitIntersect(int i, Vec3f line[])
{
	CUnit* u = &g_unit[i];
	CUnitType* t = &g_unitType[u->type];/*
	CModel* m = &g_model[t->model];

	CVertexArray* va = &m->vertexArrays[(int)u->frame[BODY_LOWER]];
	Vec3f vPoly[3];
	float yaw = DEGTORAD(u->camera.Yaw());
	Vec3f p = u->camera.Position();

	for(int v=0; v<va->numverts; v+=3)
	{
		vPoly[0] = va->vertices[v+0];
		vPoly[1] = va->vertices[v+1];
		vPoly[2] = va->vertices[v+2];
		
		vPoly[0] = Rotate(vPoly[0], yaw, 0, 1, 0);
		vPoly[1] = Rotate(vPoly[1], yaw, 0, 1, 0);
		vPoly[2] = Rotate(vPoly[2], yaw, 0, 1, 0);

		vPoly[0] = vPoly[0] + p;
		vPoly[1] = vPoly[1] + p;
		vPoly[2] = vPoly[2] + p;

		if(IntersectedPolygon(vPoly, line, 3))
			return true;
	}*/
	
	Vec3f norms[6];
	float ds[6];
	MakeHull(norms, ds, u->camera.Position(), t->radius*1.5f, t->height*1.5f);

	if(LineInterHull(line, norms, ds, 6))
		return true;

	return false;
}

bool SelectUnit(Vec3f line[])
{
	for(int i=0; i<UNITS; i++)
	{
		if(!g_unit[i].on)
			continue;

		if(g_unit[i].hp <= 0.0f)
			continue;

		//if(g_unit[i].owner != g_localP)
		//	continue;

		if(!UnitIntersect(i, line))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_UNIT;
		UDebug(g_selection[0]);
		return true;
	}

	if(g_selection.size() > 0)
	{
		//UDebug(g_selection[0]);
		return true;
	}

	return false;
}

bool SceneryIntersect(int i, Vec3f line[])
{
	Scenery* s = &g_scenery[i];
	SceneryType* t = &g_sceneryType[s->type];
	Vec3f norms[6];
	float ds[6];
	MakeHull(norms, ds, s->pos, t->hsize, t->vsize);

	if(LineInterHull(line, norms, ds, 6))
		return true;

	return false;
}

bool SelectScenery(Vec3f line[])
{
	for(int i=0; i<SCENERY; i++)
	{
		if(!g_scenery[i].on)
			continue;

		if(!SceneryIntersect(i, line))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_SCENERY;
		return true;
	}

	if(g_selection.size() > 0)
		return true;

	return false;
}

bool SelectBuilding(Vec3f line[])
{
	for(int i=0; i<BUILDINGS; i++)
	{
		if(!g_building[i].on)
			continue;

		//if(g_building[i].owner != g_localP && g_mode == PLAY)
		//	continue;

		if(!BuildingIntersect(i, line))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_BUILDING;

		if(g_mode == PLAY)
			OpenBSelect();
	}

	if(g_selection.size() > 0)
		return true;

	return false;
}

bool SelectRoad(Vec3f line[])
{
	int z;
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
		{
			if(RoadIntersect(x, z, line))
			{
				POINT point;
				point.x = x;
				point.y = z;
				g_selection2.push_back(point);
				g_selectType = SELECT_ROAD;
				return true;
			}
		}



	return false;
}

bool SelectPowerline(Vec3f line[])
{
	int z;
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
		{
			if(PowlIntersect(x, z, line))
			{
				POINT point;
				point.x = x;
				point.y = z;
				g_selection2.push_back(point);
				g_selectType = SELECT_POWERLINE;
				return true;
			}
		}

	return false;
}

bool SelectPipeline(Vec3f line[])
{
	int z;
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
		{
			if(PipeIntersect(x, z, line))
			{
				POINT point;
				point.x = x;
				point.y = z;
				g_selection2.push_back(point);
				g_selectType = SELECT_PIPELINE;
				return true;
			}
		}

	return false;
}

void SelectOne()
{
	/*
	Vec3f ray = OnNear(g_mouse.x, g_mouse.y);
	Vec3f line[2];
	line[0] = g_camera.Position();
	line[1] = g_camera.Position() + (ray * 100000.0f);*/

	Vec3f ray = g_camera.View() - g_camera.Position();
	Vec3f onnear = OnNear(g_mouse.x, g_mouse.y);
	Vec3f line[2];
	line[0] = onnear;
	line[1] = onnear + (ray * 100000.0f);

	if(SelectUnit(line))
		return;
	else if(SelectPowerline(line))
		return;
	else if(SelectPipeline(line))
		return;
	else if(SelectBuilding(line))
		return;
	else if(SelectRoad(line))
		return;
	else if(SelectScenery(line))
		return;
}

bool SelectUnits()
{	
	CUnit* u;
	Vec3f p;

	// Select military units first
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hp <= 0.0f)
			continue;

		if(u->type == LABOURER || u->type == TRUCK)
			continue;

		if(u->owner != g_localP)
			continue;

		p = u->camera.Position();

		if(!PointInsidePlanes(p))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_UNIT;
	}

	if(g_selection.size() > 0 && g_mode != EDITOR)
		return true;

	// Otherwise select one truck
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hp <= 0.0f)
			continue;

		if(u->type != TRUCK)
			continue;

		//if(u->owner != g_localP)
		//	continue;

		p = u->camera.Position();

		if(!PointInsidePlanes(p))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_UNIT;

		if(g_mode != EDITOR || g_edTool != DESTROY)
			return true;
	}

	// Otherwise select one labourer
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hp <= 0.0f)
			continue;

		if(u->type != LABOURER)
			continue;

		p = u->camera.Position();

		if(!PointInsidePlanes(p))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_UNIT;
		
		if(g_mode != EDITOR || g_edTool != DESTROY)
			return true;
	}

	//Otherwise select one foreign unit
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;
		
		if(u->hp <= 0.0f)
			continue;
		
		if(u->type == LABOURER || u->type == TRUCK)
			continue;

		p = u->camera.Position();

		if(!PointInsidePlanes(p))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_UNIT;
		
		if(g_mode != EDITOR || g_edTool != DESTROY)
			return true;
	}

	if(g_selection.size() > 0)
		return true;

	return false;
}

bool SelectBuilding()
{
	CBuilding* b;
	Vec3f p;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		//if(b->owner != g_localP)
		//	continue;

		p = b->pos;

		if(!PointInsidePlanes(p))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_BUILDING;

		if(g_mode == PLAY)
			OpenBSelect();

		if(g_mode != EDITOR || g_edTool != DESTROY)
			return true;
	}

	if(g_selection.size() > 0)
		return true;

	return false;
}

bool SelectScenery()
{
	Scenery* s;
	Vec3f p;

	for(int i=0; i<SCENERY; i++)
	{
		s = &g_scenery[i];

		if(!s->on)
			continue;

		p = s->pos;

		if(!PointInsidePlanes(p))
			continue;

		g_selection.push_back(i);
		g_selectType = SELECT_SCENERY;
	}

	if(g_selection.size() > 0)
		return true;

	return false;
}

bool SelectRoad()
{
	Vec3f p;
	int z;
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
		{
			if(!RoadAt(x, z)->on)
				continue;

			p = RoadPosition(x, z);

			if(!PointInsidePlanes(p))
				continue;

			POINT point;
			point.x = x;
			point.y = z;

			g_selection2.push_back(point);
			g_selectType = SELECT_ROAD;

			if(g_mode != EDITOR || g_edTool != DESTROY)
				return true;
		}

	if(g_selection2.size() > 0)
		return true;

	return false;
}

bool SelectPowerline()
{
	Vec3f p;
	int z;
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
		{
			if(!PowlAt(x, z)->on)
				continue;

			p = PowerlinePosition(x, z);

			if(!PointInsidePlanes(p))
				continue;

			POINT point;
			point.x = x;
			point.y = z;

			g_selection2.push_back(point);
			g_selectType = SELECT_POWERLINE;

			if(g_mode != EDITOR || g_edTool != DESTROY)
				return true;
		}

	if(g_selection2.size() > 0)
		return true;

	return false;
}

bool SelectPipeline()
{	
	Vec3f p;
	int z;
	for(int x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
		{
			if(!PipeAt(x, z)->on)
				continue;

			p = PipelinePosition(x, z);

			if(!PointInsidePlanes(p))
				continue;

			POINT point;
			point.x = x;
			point.y = z;

			g_selection2.push_back(point);
			g_selectType = SELECT_PIPELINE;

			if(g_mode != EDITOR || g_edTool != DESTROY)
				return true;

			//char msg[128];
			//sprintf(msg, "sel pipe %d,%d", x, z);
			//Chat(msg);
		}

	if(g_selection2.size() > 0)
		return true;

	//Chat("no pipe");

	return false;
}

void SelectArea()
{
	int minx = min(g_mousestart.x, g_mouse.x);
	int maxx = max(g_mousestart.x, g_mouse.x);
	int miny = min(g_mousestart.y, g_mouse.y);
	int maxy = max(g_mousestart.y, g_mouse.y);
	
	/*Vec3f ray = OnNear(minx, miny);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = g_camera.Position();
	lineTopLeft[1] = g_camera.Position() + (ray * 100000.0f);

	ray = OnNear(maxx, miny);
	Vec3f lineTopRight[2];
	lineTopRight[0] = g_camera.Position();
	lineTopRight[1] = g_camera.Position() + (ray * 100000.0f);

	ray = OnNear(minx, maxy);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = g_camera.Position();
	lineBottomLeft[1] = g_camera.Position() + (ray * 100000.0f);

	ray = OnNear(maxx, maxy);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = g_camera.Position();
	lineBottomRight[1] = g_camera.Position() + (ray * 100000.0f);*/

	Vec3f ray = g_camera.View() - g_camera.Position();
	Vec3f onnear = OnNear(minx, miny);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = onnear;
	lineTopLeft[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(maxx, miny);
	Vec3f lineTopRight[2];
	lineTopRight[0] = onnear;
	lineTopRight[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(minx, maxy);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = onnear;
	lineBottomLeft[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(maxx, maxy);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = onnear;
	lineBottomRight[1] = onnear + (ray * 100000.0f);
	
	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;
	
	GetMapIntersection2(lineTopLeft, &interTopLeft);
	GetMapIntersection2(lineTopRight, &interTopRight);
	GetMapIntersection2(lineBottomLeft, &interBottomLeft);
	GetMapIntersection2(lineBottomRight, &interBottomRight);
	
	Vec3f leftPoly[3];
	Vec3f topPoly[3];
	Vec3f rightPoly[3];
	Vec3f bottomPoly[3];
	
	leftPoly[0] = g_camera.Position();
	leftPoly[1] = interBottomLeft;
	leftPoly[2] = interTopLeft;
	
	topPoly[0] = g_camera.Position();
	topPoly[1] = interTopLeft;
	topPoly[2] = interTopRight;

	rightPoly[0] = g_camera.Position();
	rightPoly[1] = interTopRight;
	rightPoly[2] = interBottomRight;

	bottomPoly[0] = g_camera.Position();
	bottomPoly[1] = interBottomRight;
	bottomPoly[2] = interBottomLeft;

	g_normalLeft = Normal(leftPoly);
	g_normalTop = Normal(topPoly);
	g_normalRight = Normal(rightPoly);
	g_normalBottom = Normal(bottomPoly);

	g_distLeft = PlaneDistance(g_normalLeft, leftPoly[0]);
	g_distTop = PlaneDistance(g_normalTop, topPoly[0]);
	g_distRight = PlaneDistance(g_normalRight, rightPoly[0]);
	g_distBottom = PlaneDistance(g_normalBottom, bottomPoly[0]);

	if(SelectUnits())
		return;
	else if(SelectBuilding())
		return;
	else if(SelectPowerline())
		return;
	else if(SelectPipeline())
		return;
	else if(SelectRoad())
		return;
	else if(SelectScenery())
		return;
}

void AfterSelection()
{
	if(g_selection.size() > 0)
	{
		if(g_selectType == SELECT_BUILDING)
		{
			OpenAnotherView("binspect");
			FillBInspect(g_selection[0]);
		}
	}
}

void Selection()
{
	if(!g_canselect)
		return;

	if(g_build != NOTHING)
		return;

	g_selection.clear();
	g_selection2.clear();

	if(g_mousestart.x == g_mouse.x && g_mousestart.y == g_mouse.y)
		SelectOne();
	else
		SelectArea();

	AfterSelection();

	if(SelectionFunc != NULL)
		SelectionFunc();

	//RedoLeftPanel();
	//RedoGUI();
}

void RecheckSelection()
{
	int i;
	if(g_selectType == SELECT_BUILDING)
	{
		CBuilding* b;
		
		for(int j=0; j<g_selection.size(); j++)
		{
			i = g_selection[j];
			b = &g_building[i];

			if(!b->on || b->hp <= 0.0f)
			{
				g_selection.erase( g_selection.begin() + j );
			}
		}
	}
	else if(g_selectType == SELECT_UNIT)
	{
		CUnit* u;
		
		for(int j=0; j<g_selection.size(); j++)
		{
			i = g_selection[j];
			u = &g_unit[i];

			if(!u->on || u->hp <= 0.0f)
				g_selection.erase( g_selection.begin() + j );
			
			int m = u->mode;

			if(m == CONJOB || m == ROADJOB || m == NORMJOB || m == POWLJOB || m == PIPEJOB || m == DRIVING || m == SHOPPING || m == RESTING)
				g_selection.erase( g_selection.begin() + j );
		}
	}
}

bool UnitSelected(CUnit* u)
{
	if(g_selectType != SELECT_UNIT)
		return false;

	if(g_selection.size() <= 0)
		return false;

	int num = g_selection.size();
	CUnit* u2;
	for(int i=0; i<num; i++)
	{
		u2 = &g_unit[ g_selection[i] ];
		if(u2 == u)
			return true;
	}

	return false;
}