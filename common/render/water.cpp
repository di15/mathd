

#include "water.h"
#include "shader.h"
#include "../math/vec3f.h"
#include "../texture.h"
#include "heightmap.h"
#include "../window.h"
#include "../save/savemap.h"

unsigned int g_water;
unsigned int g_watertex[WATER_TEXS];

void DrawWater()
{
	Shader* s = &g_shader[g_curS];
    
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_water].texname);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);
	
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_GRADIENT] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_GRADIENTTEX], 0);

	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_DETAIL] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_DETAILTEX], 1);

	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_SPECULAR] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_SPECULARMAP], 2);

	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_NORMAL] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_NORMALMAP], 3);
    
	Vec3f a, b, c, d;
    
	int wx = g_hmap.m_widthx;
	int wz = g_hmap.m_widthz;
    
	a = Vec3f(wx * TILE_SIZE, WATER_LEVEL, wz * TILE_SIZE);
	b = Vec3f(0, WATER_LEVEL, wz * TILE_SIZE);
	c = Vec3f(0, WATER_LEVEL, 0);
	d = Vec3f(wx * TILE_SIZE, WATER_LEVEL, 0);
    
	float vertices[] =
	{
		a.x, a.y, a.z,
		b.x, b.y, b.z,
		c.x, c.y, c.z,
		d.x, d.y, d.z,
		a.x, a.y, a.z,
		c.x, c.y, c.z
	};
    
	float texcoords0[] =
	{
		(float)(wx+1), (float)(wz+1),
		0, (float)(wz+1),
		0, 0,
		(float)(wx+1), 0,
		(float)(wx+1), (float)(wz+1),
		0, 0
	};
    
	static const float normals[] =
	{
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0
	};

	
	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);
	glPolygonOffset(1.0, 0.01/(g_zoom));
	//glPolygonOffset(1.0, 250.0);
    
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, texcoords0);
	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, normals);
    
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_POLYGON_OFFSET_FILL);
}

void DrawWater2()
{
	Shader* s = &g_shader[g_curS];
    
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_water].texname);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);
	
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_GRADIENT] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_GRADIENTTEX], 0);

	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_DETAIL] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_DETAILTEX], 1);

	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_SPECULAR] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_SPECULARMAP], 2);

	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_NORMAL] ].texname);
	glUniform1iARB(s->m_slot[SSLOT_NORMALMAP], 3);
    
	Vec3f a, b, c, d;
    
	int wx = g_hmap.m_widthx;
	int wz = g_hmap.m_widthz;
	
	glUniform1f(s->m_slot[SSLOT_MAPMINZ], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXZ], wx*TILE_SIZE);
	glUniform1f(s->m_slot[SSLOT_MAPMINX], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXX], wz*TILE_SIZE);
	glUniform1f(s->m_slot[SSLOT_MAPMINY], ConvertHeight(0));
	glUniform1f(s->m_slot[SSLOT_MAPMAXY], ConvertHeight(255));
    
	a = Vec3f(wx * TILE_SIZE, WATER_LEVEL, wz * TILE_SIZE);
	b = Vec3f(0, WATER_LEVEL, wz * TILE_SIZE);
	c = Vec3f(0, WATER_LEVEL, 0);
	d = Vec3f(wx * TILE_SIZE, WATER_LEVEL, 0);
    
	float vertices[] =
	{
		a.x, a.y, a.z,
		b.x, b.y, b.z,
		c.x, c.y, c.z,
		d.x, d.y, d.z,
		a.x, a.y, a.z,
		c.x, c.y, c.z
	};
    
	float texcoords0[] =
	{
		(float)(wx+1), (float)(wz+1),
		0, (float)(wz+1),
		0, 0,
		(float)(wx+1), 0,
		(float)(wx+1), (float)(wz+1),
		0, 0
	};
    
	static const float normals[] =
	{
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0
	};

	
	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);
	glPolygonOffset(1.0, 0.01/(g_zoom));
	//glPolygonOffset(1.0, 250.0);
    
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, texcoords0);
	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, normals);
    
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_POLYGON_OFFSET_FILL);
}