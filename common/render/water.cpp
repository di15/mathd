
#include "main.h"
#include "shader.h"
#include "map.h"
#include "image.h"
#include "water.h"

void DrawWater()
{			
	Shader* s = &g_shader[g_curS];

	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_water);
	glUniform1iARB(s->m_slot[SLOT::TEXTURE0], 0);

	Vec3f a, b, c, d;

	int wx = g_hmap.m_widthX;
	int wz = g_hmap.m_widthZ;
	
	a = Vec3f(wx * TILE_SIZE, WATER_HEIGHT, wz * TILE_SIZE);
	b = Vec3f(0, WATER_HEIGHT, wz * TILE_SIZE);
	c = Vec3f(0, WATER_HEIGHT, 0);
	d = Vec3f(wx * TILE_SIZE, WATER_HEIGHT, 0);

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
		wx+1, wz+1,
		0, wz+1,
		0, 0,
		wx+1, 0,
		wx+1, wz+1,
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
    
	glVertexAttribPointer(s->m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(s->m_slot[SLOT::TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, texcoords0);
	glVertexAttribPointer(s->m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, 0, normals);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}