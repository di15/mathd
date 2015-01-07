//
// gviewport.cpp
//
//

#include "../../common/render/shader.h"
#include "../../common/gui/gui.h"
#include "../../common/math/3dmath.h"
#include "../../common/window.h"
#include "../../common/platform.h"
#include "../../common/gui/font.h"
#include "../../common/math/camera.h"
#include "../../common/math/matrix.h"
#include "../../common/render/shadow.h"
#include "../../common/render/heightmap.h"
#include "../../common/math/vec4f.h"
#include "../../common/math/brush.h"
#include "../../common/math/frustum.h"
#include "../../common/sim/simdef.h"
#include "gviewport.h"
#include "../../common/math/hmapmath.h"
#include "../../common/render/water.h"
#include "../../common/save/savemap.h"
#include "../../common/gui/widgets/spez/botpan.h"
#include "../../common/sim/bltype.h"
#include "../../common/sim/road.h"
#include "../../common/sim/powl.h"
#include "../../common/sim/crpipe.h"
#include "../../common/sim/utype.h"
#include "../../common/sim/player.h"
#include "../../common/debug.h"
#include "../../common/sim/conduit.h"

VpType g_vptype[VIEWPORT_TYPES];
Viewport g_viewport[4];
//Vec3f g_focus;

VpType::VpType(Vec3f offset, Vec3f up, const char* label, bool axial)
{
	m_offset = offset;
	m_up = up;
	strcpy(m_label, label);
	m_axial = axial;
}

#if 0
Viewport::Viewport()
{
	m_drag = false;
	m_ldown = false;
	m_rdown = false;
}

Viewport::Viewport(int type)
{
	m_drag = false;
	m_ldown = false;
	m_rdown = false;
	m_mdown = false;
	m_type = type;
}

Vec3f Viewport::up()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f upvec = c->m_up;
	VpType* t = &g_vptype[m_type];

	if(t->m_axial)
		upvec = t->m_up;

	return upvec;
}

Vec3f Viewport::up2()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f upvec = c->up2();
	VpType* t = &g_vptype[m_type];

	if(t->m_axial)
		upvec = t->m_up;

	return upvec;
}

Vec3f Viewport::strafe()
{
	Vec3f upvec = up();
	VpType* t = &g_vptype[m_type];
	Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, upvec));

	//if(!t->m_axial)
	//	sidevec = c->m_strafe;

	return sidevec;
}

Vec3f Viewport::focus()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f viewvec = c->m_view;
	return viewvec;
}

Vec3f Viewport::viewdir()
{
	Vec3f focusvec = focus();
	Vec3f posvec = pos();
	//Vec3f viewvec = posvec + Normalize(focusvec-posvec);
	//return viewvec;
	return focusvec-posvec;
}

Vec3f Viewport::pos()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f posvec = c->m_pos;

#if 0
	if(g_projtype == PROJ_PERSP && !t->m_axial)
	{
		Vec3f dir = Normalize( c->m_view - c->m_pos );
		posvec = c->m_view - dir * 1000.0f / g_zoom;
	}
#endif

	VpType* t = &g_vptype[m_type];

	if(t->m_axial)
		posvec = c->m_view + t->m_offset;

	return posvec;
}
#endif

void DrawMMFrust()
{
	//return;

#if 0

	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f campos = c->zoompos();
	Vec3f camside = c->m_strafe;
	Vec3f camup2 = c->up2();
	Vec3f viewdir = Normalize(c->m_view - c->m_pos);

	int minx = 0;
	int maxx = g_width;
	int miny = 0;
	int maxy = g_height;

	//Vec3f campos = c->m_pos;
	//Vec3f camside = c->m_strafe;
	//Vec3f camup2 = c->up2();
	//Vec3f viewdir = Normalize( c->m_view - c->m_pos );

	Vec3f topLeftRay = ScreenPerspRay(minx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = campos;
	lineTopLeft[1] = campos + (topLeftRay * 1000000.0f);

	Vec3f topRightRay = ScreenPerspRay(maxx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineTopRight[2];
	lineTopRight[0] = campos;
	lineTopRight[1] = campos + (topRightRay * 1000000.0f);

	Vec3f bottomLeftRay = ScreenPerspRay(minx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = campos;
	lineBottomLeft[1] = campos + (bottomLeftRay * 1000000.0f);

	Vec3f bottomRightRay = ScreenPerspRay(maxx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = campos;
	lineBottomRight[1] = campos + (bottomRightRay * 1000000.0f);

	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;

	if(!FastMapIntersect(&g_hmap, lineTopLeft, &interTopLeft))
		GetMapIntersection2(&g_hmap, lineTopLeft, &interTopLeft);
	if(!FastMapIntersect(&g_hmap, lineTopRight, &interTopRight))
		GetMapIntersection2(&g_hmap, lineTopRight, &interTopRight);
	if(!FastMapIntersect(&g_hmap, lineBottomLeft, &interBottomLeft))
		GetMapIntersection2(&g_hmap, lineBottomLeft, &interBottomLeft);
	if(!FastMapIntersect(&g_hmap, lineBottomRight, &interBottomRight))
		GetMapIntersection2(&g_hmap, lineBottomRight, &interBottomRight);

#else

	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;

	MapFrust(&interTopLeft,
		&interTopRight,
		&interBottomLeft,
		&interBottomRight);
#endif

	float mmxscale = (float)MINIMAP_SIZE / (g_hmap.m_widthx*TILE_SIZE);
	float mmzscale = (float)MINIMAP_SIZE / (g_hmap.m_widthy*TILE_SIZE);

	interTopLeft.x = interTopLeft.x * mmxscale;
	interTopRight.x = interTopRight.x * mmxscale;
	interBottomLeft.x = interBottomLeft.x * mmxscale;
	interBottomRight.x = interBottomRight.x * mmxscale;

	interTopLeft.z = interTopLeft.z * mmzscale;
	interTopRight.z = interTopRight.z * mmzscale;
	interBottomLeft.z = interBottomLeft.z * mmzscale;
	interBottomRight.z = interBottomRight.z * mmzscale;

	float* color = g_player[g_localP].color;

	glUniform4f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_COLOR], color[0], color[1], color[2], 0.3f);

	float vertices[] =
	{
		//posx, posy
		interTopLeft.x, interTopLeft.z,0,
		interTopRight.x, interTopRight.z,0,
		interBottomRight.x, interBottomRight.z,0,
		interBottomLeft.x, interBottomLeft.z,0,
		interTopLeft.x, interTopLeft.z,0
	};

	//glVertexAttribPointer(g_shader[SHADER_COLOR2D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
	glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);

	glDrawArrays(GL_LINE_STRIP, 0, 5);
}

void DrawMinimap(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float lightpos[3], float lightdir[3])
{
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult(viewmat);

	//g_frustum.construct(projection.m_matrix, viewmat.m_matrix);

	CheckGLError(__FILE__, __LINE__);
#if 1
	UseShadow(SHADER_MAPTILESMM, projection, viewmat, modelmat, modelviewinv, lightpos, lightdir);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 8);
	g_hmap.draw2();
	EndS();
#endif
	CheckGLError(__FILE__, __LINE__);

#if 1
	UseShadow(SHADER_WATERMM, projection, viewmat, modelmat, modelviewinv, lightpos, lightdir);
	CheckGLError(__FILE__, __LINE__);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 8);
	CheckGLError(__FILE__, __LINE__);
	DrawWater2();
	CheckGLError(__FILE__, __LINE__);
	EndS();
#endif

	CheckGLError(__FILE__, __LINE__);
#if 0
	UseShadow(SHADER_BORDERSMM, projection, viewmat, modelmat, modelviewinv, lightpos, lightdir);
	//glActiveTexture(GL_TEXTURE8);
	//glBindTexture(GL_TEXTURE_2D, g_depth);
	//glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 8);
	Shader* s = &g_shader[g_curS];
	glUniform1f(s->m_slot[SSLOT_MAPMINZ], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXZ], g_hmap.m_widthy*TILE_SIZE);
	glUniform1f(s->m_slot[SSLOT_MAPMINX], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXX], g_hmap.m_widthx*TILE_SIZE);
	glUniform1f(s->m_slot[SSLOT_MAPMINY], ConvertHeight(0));
	glUniform1f(s->m_slot[SSLOT_MAPMAXY], ConvertHeight(255));
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
	DrawBorders();
	EndS();
#endif

	glDisable(GL_DEPTH_TEST);

	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)MINIMAP_SIZE);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)MINIMAP_SIZE);
	glUniform4f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
	DrawMMFrust();
	EndS();
	CheckGLError(__FILE__, __LINE__);
}

void DrawMinimapDepth()
{
	//if(rand()%2 == 1)
	g_hmap.draw2();
}

void DrawPreview(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float lightpos[3], float lightdir[3])
{
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult(viewmat);

	UseShadow(SHADER_OWNED, projection, viewmat, modelmat, modelviewinv, lightpos, lightdir);
	//UseShadow(SHADER_UNIT, projection, viewmat, modelmat, modelviewinv, lightpos, lightdir);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1i(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 8);

	Shader* s = &g_shader[g_curS];
	Player* py = &g_player[g_localP];
	float* color = py->color;
	glUniform4f(s->m_slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);
	//glUniform4f(s->m_slot[SSLOT_OWNCOLOR], 1, 0, 0, 0);

	Model* m = NULL;

	if(g_bptype >= 0 && g_bptype < BL_TYPES)
	{
		BlType* t = &g_bltype[g_bptype];
		m = &g_model[t->model];
	}
	else if(g_bptype == BL_ROAD)
	{
		CdType* ct = &g_cdtype[CONDUIT_ROAD];
		m = &g_model[ct->model[CONNECTION_EASTWEST][1]];
	}
	else if(g_bptype == BL_POWL)
	{
		CdType* ct = &g_cdtype[CONDUIT_POWL];
		m = &g_model[ct->model[CONNECTION_EASTWEST][1]];
	}
	else if(g_bptype == BL_CRPIPE)
	{
		CdType* ct = &g_cdtype[CONDUIT_CRPIPE];
		m = &g_model[ct->model[CONNECTION_EASTWEST][1]];
	}

	if(!m)
		return;

	g_bpyaw += g_drawfrinterval * 100.0f;
	int iyaw = (int)g_bpyaw;
	float rem = g_bpyaw - (float)iyaw;
	g_bpyaw = (int)g_bpyaw % 360 + rem;

	float pitch = 0;
	float yaw = g_bpyaw;
	int frame = 0;
	Vec3f pos(0,0,0);
	float radians[] = {(float)DEGTORAD(pitch), (float)DEGTORAD(yaw), 0};
	modelmat.reset();
	modelmat.translation((const float*)&pos);
	Matrix rotation;
	rotation.rotrad(radians);
	modelmat.postmult(rotation);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvp;
#if 0
	mvp.set(modelview.m_matrix);
	mvp.postmult(g_camproj);
#elif 0
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(modelview);
#else
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
#endif
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	VertexArray* va = &m->m_va[frame];

	m->usetex();

	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
	glNormalPointer(GL_FLOAT, 0, va->normals);

	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
	EndS();
}

void DrawPreviewDepth()
{
	Shader* s = &g_shader[g_curS];

	Model* m = NULL;

	Player* py = &g_player[g_localP];

	if(g_bptype >= 0 && g_bptype < BL_TYPES)
	{
		BlType* t = &g_bltype[g_bptype];
		m = &g_model[t->model];
	}
	else if(g_bptype == BL_ROAD)
	{
		CdType* ct = &g_cdtype[CONDUIT_ROAD];
		m = &g_model[ct->model[CONNECTION_EASTWEST][1]];
	}
	else if(g_bptype == BL_POWL)
	{
		CdType* ct = &g_cdtype[CONDUIT_POWL];
		m = &g_model[ct->model[CONNECTION_EASTWEST][1]];
	}
	else if(g_bptype == BL_CRPIPE)
	{
		CdType* ct = &g_cdtype[CONDUIT_CRPIPE];
		m = &g_model[ct->model[CONNECTION_EASTWEST][1]];
	}

	if(!m)
		return;

	//g_bpyaw = (int)(g_bpyaw+2)%360;

	float pitch = 0;
	float yaw = g_bpyaw;
	int frame = 0;
	Vec3f pos(0,0,0);
	Matrix modelmat;
	float radians[] = {(float)DEGTORAD(pitch), (float)DEGTORAD(yaw), 0};
	modelmat.translation((const float*)&pos);
	Matrix rotation;
	rotation.rotrad(radians);
	modelmat.postmult(rotation);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvp;
#if 0
	mvp.set(modelview.m_matrix);
	mvp.postmult(g_camproj);
#elif 0
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(modelview);
#else
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
#endif
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	VertexArray* va = &m->m_va[frame];

	m->usetex();

	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

void DrawViewport(int which, int x, int y, int width, int height)
{
	//return;

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	Viewport* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;


	Matrix oldview = g_camview;

	if(which == VIEWPORT_ENTVIEW)
	{
		float aspect = fabsf((float)width / (float)height);
		Matrix projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		//Matrix projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);

		Vec3f focusvec = g_bpcam.m_view;
		Vec3f posvec = g_bpcam.m_pos;
		Vec3f upvec = g_bpcam.m_up;

#if 1
		Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
#else
		Matrix viewmat = gluLookAt2(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
#endif

		g_camview = viewmat;

		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.translation(translation);
		modelmat.translation(translation);
		modelview.postmult(viewmat);

		Matrix mvpmat;
		mvpmat.set(projection.m_matrix);
		mvpmat.postmult(viewmat);

		//RenderToShadowMap(projection, viewmat, modelmat, c->m_view);
		//RenderToShadowMap(projection, viewmat, modelmat, Vec3f(0,0,0));
		Vec3f focus(0, 0, 0);
		Vec3f vLine[2];
		Vec3f ray = Normalize(c->m_view - posvec);
		Vec3f onnear = posvec;	//OnNear(g_width/2, g_height/2);
#if 0
		vLine[0] = onnear;
		vLine[1] = onnear + (ray * 100000.0f);
		//if(!GetMapIntersection(&g_hmap, vLine, &focus))
		if(!FastMapIntersect(&g_hmap, vLine, &focus))
			//if(!GetMapIntersection(&g_hmap, vLine, &focus))
			GetMapIntersection2(&g_hmap, vLine, &focus);
#endif

		CheckGLError(__FILE__, __LINE__);
		//RenderToShadowMap(projection, viewmat, modelmat, focus, focus + g_lightoff, DrawPreviewDepth);
		CheckGLError(__FILE__, __LINE__);
		RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawPreview);
		CheckGLError(__FILE__, __LINE__);
	}

	if(which == VIEWPORT_MINIMAP)
	{
		StartTimer(TIMER_DRAWMINIMAP);

		float aspect = fabsf((float)width / (float)height);
		Matrix projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE/g_zoom);
		//Matrix projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);

		Vec3f focusvec = c->m_view;
		Vec3f posvec = c->zoompos();
		Vec3f upvec = c->m_up;

		Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

		g_camview = viewmat;

		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.translation(translation);
		modelmat.translation(translation);
		modelview.postmult(viewmat);

		Matrix mvpmat;
		mvpmat.set(projection.m_matrix);
		mvpmat.postmult(viewmat);

		//if(v->m_type == VIEWPORT_MAIN3D)
		{
			//RenderToShadowMap(projection, viewmat, modelmat, c->m_view);
			//RenderToShadowMap(projection, viewmat, modelmat, Vec3f(0,0,0));
			Vec3f focus(g_hmap.m_widthx*TILE_SIZE, 0, g_hmap.m_widthy*TILE_SIZE);
			Vec3f vLine[2];
			Vec3f ray = Normalize(c->m_view - posvec);
			Vec3f onnear = posvec;	//OnNear(g_width/2, g_height/2);
#if 0
			vLine[0] = onnear;
			vLine[1] = onnear + (ray * 100000.0f);
			//if(!GetMapIntersection(&g_hmap, vLine, &focus))
			if(!FastMapIntersect(&g_hmap, vLine, &focus))
				//if(!GetMapIntersection(&g_hmap, vLine, &focus))
				GetMapIntersection2(&g_hmap, vLine, &focus);
#endif
			CheckGLError(__FILE__, __LINE__);
			//RenderToShadowMap(projection, viewmat, modelmat, focus, focus + g_lightoff / MIN_ZOOM, DrawMinimapDepth);
			CheckGLError(__FILE__, __LINE__);
			RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawMinimap);
			CheckGLError(__FILE__, __LINE__);
		}

		StopTimer(TIMER_DRAWMINIMAP);
	}

	g_camview = oldview;

#if 0
	EndS();
	CheckGLError(__FILE__, __LINE__);
	Ortho(width, height, 1, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	RichText rt = RichText(t->m_label);
	DrawShadowedText(MAINFONT8, 0, 0, &rt, NULL, -1);
	EndS();
#endif

	//if(persp)
	//	DrawVertexDebug(&projection, &modelmat, &viewmat, width, height);

	glDisable(GL_DEPTH_TEST);
	glFlush();
	CheckGLError(__FILE__, __LINE__);
}

#if 0
bool ViewportLDown(int which, int relx, int rely, int width, int height)
{
	//return false;

	Viewport* v = &g_viewport[which];
	v->m_ldown = true;
	v->m_lastmouse = Vec2i(relx, rely);
	v->m_curmouse = Vec2i(relx, rely);


	VpType* t = &g_vptype[v->m_type];

	//g_log<<"vp["<<which<<"] l down"<<std::endl;
	//g_log.flush();

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	bool persp = false;

#if 0
	if(v->!t->m_axial && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}
#endif

	//Vec3f viewvec = g_focus; //c->m_view;
	//Vec3f viewvec = c->m_view;
	Vec3f focusvec = v->focus();
	//Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = c->m_pos;
	Vec3f posvec = v->pos();

	//if(v->t->m_axial)
	{
		//	posvec = c->m_view + t->m_offset;
		//viewvec = posvec + Normalize(c->m_view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = c->lookpos() + t->m_offset;
	//Vec3f upvec = t->m_up;
	//Vec3f upvec = c->m_up;
	Vec3f upvec = v->up();

	//if(v->t->m_axial)
	//	upvec = t->m_up;

	Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult(viewmat);

	return true;
}


bool ViewportRDown(int which, int relx, int rely, int width, int height)
{
	Viewport* v = &g_viewport[which];
	v->m_rdown = true;
	v->m_lastmouse = Vec2i(relx, rely);
	v->m_curmouse = Vec2i(relx, rely);

	return true;
}

bool ViewportLUp(int which, int relx, int rely, int width, int height)
{
	Viewport* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	if(v->m_ldown)
	{
		//return true;
		v->m_ldown = false;

	}

	//g_sel1b = NULL;
	//g_dragV = -1;
	//g_dragS = -1;

	return false;
}

bool ViewportRUp(int which, int relx, int rely, int width, int height)
{
	Viewport* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	v->m_rdown = false;

	return false;
}

bool NULL(int which, int delta)
{
	Viewport* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

#if 0
	//if(v->!t->m_axial)
	{
		g_zoom *= 1.0f + (float)delta / 10.0f;
		return true;
	}
#endif

	return false;
}

bool ViewportMousemove(int which, int relx, int rely, int width, int height)
{
	Viewport* v = &g_viewport[which];

	v->m_lastmouse = Vec2i(relx, rely);
	v->m_curmouse = Vec2i(relx, rely);

	return false;
}
#endif