
#include "projectile.h"
#include "image.h"
#include "main.h"
#include "shader.h"

CProjectileType g_projectileType[PROJECTILE_TYPES];
CProjectile g_projectile[PROJECTILES];

void CProjectileType::Define(char* texpath)
{
	//CreateTexture(tex, texpath);
	QueueTexture(&tex, texpath, true);
}

void LoadProjectiles()
{
	g_projectileType[GUNPROJ].Define("particles\\gunproj.png");
}

int NewProjectile()
{
	for(int i=0; i<PROJECTILES; i++)
		if(!g_projectile[i].on)
			return i;

	return -1;
}

void NewProjectile(Vec3f start, Vec3f end, int type)
{
	//g_projectile.push_back(CProjectile(start, end, type));
	int i = NewProjectile();
	if(i < 0)
		return;

	g_projectile[i] = CProjectile(start, end, type);
}

void DrawProjectiles()
{
	CProjectile* proj;
	CProjectileType* t;
	Vec3f start;
	Vec3f end;
	
	float dist;
	float sizeparallel = 8, sizeperpindicular = 1;
	float ratio;
	Vec3f delta;
	Vec3f center;
	Vec3f parallel;
	Vec3f perpindicular;
	Vec3f view = Normalize(g_camera.View() - g_camera.Position());
	Vec3f a, b, c, d;

	Shader* s = &g_shader[SHADER::BILLBOARD];
	glUniform4f(s->m_slot[SLOT::COLOR], 1, 1, 1, 1);

	for(int i=0; i<PROJECTILES; i++)
	{
		proj = &g_projectile[i];
		if(!proj->on)
			continue;

		start = proj->start;
		end = proj->end;
		delta = end - start;
		t = &g_projectileType[proj->type];
		dist = Magnitude(delta);
		ratio = (float)(rand()%1000)/1000.0f;
		center = start + delta * ratio;
		parallel = Normalize(delta) * sizeparallel/2.0f;
		perpindicular = Normalize(Cross(parallel, view)) * sizeperpindicular/2.0f;
		a = center - parallel + perpindicular;
		b = center - parallel - perpindicular;
		c = center + parallel - perpindicular;
		d = center + parallel + perpindicular;

		glBindTexture(GL_TEXTURE_2D, t->tex);
		/*
		glBindTexture(GL_TEXTURE_2D, t->tex);
		glBegin(GL_QUADS);
		
		glTexCoord2f(0, 0);		glVertex3f(a.x, a.y, a.z);
		glTexCoord2f(0, 1);		glVertex3f(b.x, b.y, b.z);
		glTexCoord2f(1, 1);		glVertex3f(c.x, c.y, c.z);
		glTexCoord2f(1, 0);		glVertex3f(d.x, d.y, d.z);

		glEnd();*/
		
        float vertices[] =
        {
            //posx, posy posz   texx, texy
            a.x, a.y, a.z,          0, 0,
            b.x, b.y, b.z,          0, 1,
            c.x, c.y, c.z,          1, 1,
            
            c.x, c.y, c.z,          1, 1,
            d.x, d.y, d.z,          1, 0,
            a.x, a.y, a.z,          0, 0
        };

		glVertexAttribPointer(s->m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexAttribPointer(s->m_slot[SLOT::TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->m_slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);
		
        glDrawArrays(GL_TRIANGLES, 0, 6);

		proj->on = false;
	}
}