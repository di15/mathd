

#include "selection.h"
#include "../math/matrix.h"
#include "../window.h"
#include "../math/plane3f.h"
#include "../math/frustum.h"
#include "../math/brush.h"
#include "unittype.h"
#include "unit.h"
#include "buildingtype.h"
#include "building.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../math/hmapmath.h"
#include "../utils.h"
#include "../../game/gmain.h"

Selection g_selection;

// Selection circle texture index
unsigned int g_circle = 0;

void Selection::clear()
{
	units.clear();
	buildings.clear();
	roads.clear();
	powerlines.clear();
	crudepipes.clear();
}

// Selection frustum for drag/area-selection
static Vec3f normalLeft;
static Vec3f normalTop;
static Vec3f normalRight;
static Vec3f normalBottom;
static float distLeft;
static float distTop;
static float distRight;
static float distBottom;

static Frustum g_selfrust;	//selection frustum

void DrawMarquee()
{
	if(!g_mousekeys[0] || g_keyintercepted || g_mode != PLAY)
		return;

	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_COLOR], 0, 1, 0, 0.75f);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].m_slot[SSLOT_POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].m_slot[SSLOT_TEXCOORD0]);

	float vertices[] =
	{
		//posx, posy    texx, texy
		g_mousestart.x, g_mousestart.y, 0,          0, 0,
		g_mousestart.x, g_mouse.y,0,         1, 0,
		g_mouse.x, g_mouse.y,0,      1, 1,

		g_mouse.x, g_mousestart.y,0,      1, 1,
		g_mousestart.x, g_mousestart.y,0,       0, 1
	};

	glVertexAttribPointer(g_shader[SHADER_COLOR2D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);

	glDrawArrays(GL_LINE_STRIP, 0, 5);
}

#if 1
void DrawSelectionCircles(Matrix* projection, Matrix* modelmat, Matrix* viewmat)
{
	//if(g_projtype == PROJ_PERSPECTIVE)
	UseS(SHADER_BILLBOARD);
	//else
	//	UseS(SHADER_BILLBOAR);

	Shader* s = &g_shader[g_curS];

	glUniformMatrix4fv(s->m_slot[SSLOT_PROJECTION], 1, 0, projection->m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat->m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_VIEWMAT], 1, 0, viewmat->m_matrix);
	glUniform4f(s->m_slot[SSLOT_COLOR], 0, 1, 0, 0.5f);
	glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
	glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
	glEnableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);

	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_circle ].texname);
	glUniform1iARB(s->m_slot[SSLOT_TEXTURE0], 0);

	for(auto selectioniterator = g_selection.units.begin(); selectioniterator != g_selection.units.end(); selectioniterator++)
	{
#if 0
		MessageBox(g_hWnd, "dr s c", "asd", NULL);
#endif

		Unit* u = &g_unit[ *selectioniterator ];
		//Entity* e = g_entity[ 0 ];
		Vec3f p = u->drawpos;
		UnitT* t = &g_unitT[ u->type ];

		//Vec3f p = c->m_pos + Vec3f(0, t->vmin.y, 0) + Vec3f(0, 1.0f, 0);

		const float r = t->size.x * 0.5f;

#if 0
		float y1 = Bilerp(&g_hmap, p.x + r, p.z - r);
		float y2 = Bilerp(&g_hmap, p.x + r, p.z + r);
		float y3 = Bilerp(&g_hmap, p.x - r, p.z + r);
		float y4 = Bilerp(&g_hmap, p.x - r, p.z - r);
#elif 1
		float y1 = g_hmap.accheight(p.x + r, p.z - r);
		float y2 = g_hmap.accheight(p.x + r, p.z + r);
		float y3 = g_hmap.accheight(p.x - r, p.z + r);
		float y4 = g_hmap.accheight(p.x - r, p.z - r);
#else
		float y1 = p.y;
		float y2 = p.y;
		float y3 = p.y;
		float y4 = p.y;
#endif

		float vertices[] =
		{
			//posx, posy posz   texx, texy
			p.x + r, y1 + TILE_SIZE/100, p.z - r,          1, 0,
			p.x + r, y2	+ TILE_SIZE/100, p.z + r,          1, 1,
			p.x - r, y3 + TILE_SIZE/100, p.z + r,          0, 1,

			p.x - r, y3 + TILE_SIZE/100, p.z + r,          0, 1,
			p.x - r, y4 + TILE_SIZE/100, p.z - r,          0, 0,
			p.x + r, y1 + TILE_SIZE/100, p.z - r,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
#endif 

//Open appropriate menus and such
void AfterSelection()
{
}

int SelectOneUnit(Vec3f *line)
{
	int unitsel = -1;
	float closestd = -1;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UnitT* t = &g_unitT[ u->type ];

		Vec3f normals[6];
		float dists[6];
		
		Vec3f vmin = Vec3f(-t->size.x/2, 0, -t->size.z/2);
		Vec3f vmax = Vec3f(t->size.x/2, t->size.y, t->size.z/2);
		 
		MakeHull(normals, dists, u->drawpos, vmin, vmax);

		Vec3f intersection;

		if(!LineInterHull(line, normals, dists, 6, &intersection))
			continue;

		float thisd = Magnitude(line[0] - intersection);

		if(thisd < closestd || closestd < 0)
		{
			unitsel = i;
			closestd = thisd;
		}
	}

	return unitsel;
}

Selection SelectOne(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f line[2];
	line[0] = campos;
	line[1] = campos + (ray * 10000.0f);

	Selection selection;

	int unitsel = SelectOneUnit(line);

#if 0
	InfoMessage("sel one", "a");
#endif

	if(unitsel >= 0)
	{
#if 0
		InfoMessage("sel one", "b");
#endif
		selection.units.push_back( unitsel );
	}

	return selection;
}

bool PointBehindPlane(Vec3f p, Vec3f normal, float dist)
{
	float result = p.x*normal.x + p.y*normal.y + p.z*normal.z + dist;

	if(result <= EPSILON)
		return true;

	return false;
}

bool PointInsidePlanes(Vec3f p)
{
	if(!PointBehindPlane(p, normalLeft, distLeft))
		return false;

	if(!PointBehindPlane(p, normalTop, distTop))
		return false;

	if(!PointBehindPlane(p, normalRight, distRight))
		return false;

	if(!PointBehindPlane(p, normalBottom, distBottom))
		return false;

	return true;
}

#if 0
void SelectAreaOrtho()
{
	int minx = min(g_mousestart.x, g_mouse.x);
	int maxx = max(g_mousestart.x, g_mouse.x);
	int miny = min(g_mousestart.y, g_mouse.y);
	int maxy = max(g_mousestart.y, g_mouse.y);

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

	leftPoly[0] = g_camera.m_pos;
	leftPoly[1] = interBottomLeft;
	leftPoly[2] = interTopLeft;

	topPoly[0] = g_camera.m_pos;
	topPoly[1] = interTopLeft;
	topPoly[2] = interTopRight;

	rightPoly[0] = g_camera.m_pos;
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
}
#endif

#if 0
// This determines if a BOX is in or around our frustum by it's min and max points
bool BoxInsidePlanes( float x, float y, float z, float x2, float y2, float z2)
{
	// Go through all of the corners of the box and check then again each plane
	// in the frustum.  If all of them are behind one of the planes, then it most
	// like is not in the frustum.
	for(int i = 0; i < 6; i++ )
	{
		if(m_Frustum[i][A] * x  + m_Frustum[i][B] * y  + m_Frustum[i][C] * z  + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x2 + m_Frustum[i][B] * y  + m_Frustum[i][C] * z  + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x  + m_Frustum[i][B] * y2 + m_Frustum[i][C] * z  + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x2 + m_Frustum[i][B] * y2 + m_Frustum[i][C] * z  + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x  + m_Frustum[i][B] * y  + m_Frustum[i][C] * z2 + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x2 + m_Frustum[i][B] * y  + m_Frustum[i][C] * z2 + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x  + m_Frustum[i][B] * y2 + m_Frustum[i][C] * z2 + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x2 + m_Frustum[i][B] * y2 + m_Frustum[i][C] * z2 + m_Frustum[i][D] > 0)  continue;

		// If we get here, it isn't in the frustum
		return false;
	}

	// Return a true for the box being inside of the frustum
	return true;
}
#endif

list<int> SelectAreaUnits()
{
	list<int> unitsel;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UnitT* t = &g_unitT[ u->type ];

		Vec3f vmin = u->drawpos + Vec3f(-t->size.x/2, 0, -t->size.z/2);
#if 1
		Vec3f vmax = u->drawpos + Vec3f(t->size.x/2, t->size.y, t->size.z/2);
#else
		Vec3f vmax = u->drawpos + Vec3f(t->size.x/2, 0, t->size.z/2);
#endif

		if(!g_selfrust.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

#if 0
		MessageBox(g_hWnd, "u s", "asd", NULL);
#endif

		unitsel.push_back(i);
	}

	return unitsel;
}

Selection SelectAreaPersp(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	int minx = min(g_mousestart.x, g_mouse.x);
	int maxx = max(g_mousestart.x, g_mouse.x);
	int miny = min(g_mousestart.y, g_mouse.y);
	int maxy = max(g_mousestart.y, g_mouse.y);

	//Vec3f campos = g_camera.m_pos;
	//Vec3f camside = g_camera.m_strafe;
	//Vec3f camup2 = g_camera.up2();
	//Vec3f viewdir = Normalize( g_camera.m_view - g_camera.m_pos );

	Vec3f topLeftRay = ScreenPerspRay(minx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = campos;
	lineTopLeft[1] = campos + (topLeftRay * 10000.0f);

	Vec3f topRightRay = ScreenPerspRay(maxx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineTopRight[2];
	lineTopRight[0] = campos;
	lineTopRight[1] = campos + (topRightRay * 10000.0f);

	Vec3f bottomLeftRay = ScreenPerspRay(minx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = campos;
	lineBottomLeft[1] = campos + (bottomLeftRay * 10000.0f);

	Vec3f bottomRightRay = ScreenPerspRay(maxx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = campos;
	lineBottomRight[1] = campos + (bottomRightRay * 10000.0f);

#if 0
	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;

	GetMapIntersection2(lineTopLeft, &interTopLeft);
	GetMapIntersection2(lineTopRight, &interTopRight);
	GetMapIntersection2(lineBottomLeft, &interBottomLeft);
	GetMapIntersection2(lineBottomRight, &interBottomRight);
#elif 0
	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;

	GetMapIntersection(&g_hmap, lineTopLeft, &interTopLeft);
	GetMapIntersection(&g_hmap, lineTopRight, &interTopRight);
	GetMapIntersection(&g_hmap, lineBottomLeft, &interBottomLeft);
	GetMapIntersection(&g_hmap, lineBottomRight, &interBottomRight);
	
	PlaceUnit(UNIT_ROBOSOLDIER, Vec3i(interTopLeft.x, interTopLeft.y, interTopLeft.z), 2, -1, -1);
	PlaceUnit(UNIT_ROBOSOLDIER, Vec3i(interTopRight.x, interTopRight.y, interTopRight.z), 2, -1, -1);
	PlaceUnit(UNIT_ROBOSOLDIER, Vec3i(interBottomLeft.x, interBottomLeft.y, interBottomLeft.z), 2, -1, -1);
	PlaceUnit(UNIT_ROBOSOLDIER, Vec3i(interBottomRight.x, interBottomRight.y, interBottomRight.z), 2, -1, -1);
#endif

	Vec3f leftPoly[3];
	Vec3f topPoly[3];
	Vec3f rightPoly[3];
	Vec3f bottomPoly[3];
	Vec3f frontPoly[3];
	Vec3f backPoly[3];

	//Assemble polys in clockwise order so that their normals face outward

	leftPoly[0] = campos;
#if 0
	leftPoly[1] = interBottomLeft;
	leftPoly[2] = interTopLeft;
#endif
	leftPoly[1] = lineBottomLeft[1];
	leftPoly[2] = lineTopLeft[1];

	topPoly[0] = campos;
#if 0
	topPoly[1] = interTopLeft;
	topPoly[2] = interTopRight;
#endif
	topPoly[1] = lineTopLeft[1];
	topPoly[2] = lineTopRight[1];

	rightPoly[0] = campos;
#if 0
	rightPoly[1] = interTopRight;
	rightPoly[2] = interBottomRight;
#endif
	rightPoly[1] = lineTopRight[1];
	rightPoly[2] = lineBottomRight[1];

	bottomPoly[0] = campos;
#if 0
	bottomPoly[1] = interBottomRight;
	bottomPoly[2] = interBottomLeft;
#endif
	bottomPoly[1] = lineBottomRight[1];
	bottomPoly[2] = lineBottomLeft[1];

	frontPoly[0] = lineBottomLeft[1];
	frontPoly[1] = lineBottomRight[1];
	frontPoly[2] = lineTopRight[1];

	backPoly[0] = lineTopRight[0] + topLeftRay;
	backPoly[1] = lineBottomRight[0] + bottomRightRay;
	backPoly[2] = lineBottomLeft[0] + bottomLeftRay;

	// Normals will actually face inward
	// (using counter-clockwise function Normal2)

	normalLeft = Normal2(leftPoly);
	normalTop = Normal2(topPoly);
	normalRight = Normal2(rightPoly);
	normalBottom = Normal2(bottomPoly);
	Vec3f normalFront = Normal(frontPoly);
	Vec3f normalBack = Normal(backPoly);

	distLeft = PlaneDistance(normalLeft, leftPoly[0]);
	distTop = PlaneDistance(normalTop, topPoly[0]);
	distRight = PlaneDistance(normalRight, rightPoly[0]);
	distBottom = PlaneDistance(normalBottom, bottomPoly[0]);
	float distFront = PlaneDistance(normalFront, frontPoly[0]);
	float distBack = PlaneDistance(normalBack, backPoly[0]);

	g_selfrust.construct(
		Plane3f(normalLeft.x, normalLeft.y, normalLeft.z, distLeft),
		Plane3f(normalRight.x, normalRight.y, normalRight.z, distRight),
		Plane3f(normalTop.x, normalTop.y, normalTop.z, distTop),
		Plane3f(normalBottom.x, normalBottom.y, normalBottom.z, distBottom),
		Plane3f(normalFront.x, normalFront.y, normalFront.z, distFront),
		Plane3f(normalBack.x, normalBack.y, normalBack.z, distBack));

	Selection selection;

	selection.units = SelectAreaUnits();

	return selection;
}

Selection DoSelection(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	Selection sel;

	if(g_mousestart.x == g_mouse.x && g_mousestart.y == g_mouse.y)
		sel = SelectOne(campos, camside, camup2, viewdir);
	else
		sel = SelectAreaPersp(campos, camside, camup2, viewdir);

	AfterSelection();

	return sel;
}