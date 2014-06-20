

#include "../platform.h"
#include "skybox.h"
#include "../math/3dmath.h"
#include "shader.h"
#include "../texture.h"
#include "../math/matrix.h"
#include "shader.h"
#include "../math/vec3f.h"

unsigned int g_front;
unsigned int g_left;
unsigned int g_right;
unsigned int g_bottom;
unsigned int g_top;
unsigned int g_back;

void LoadSkyBox(const char* basename)
{
	char front[MAX_PATH+1];
	char left[MAX_PATH+1];
	char right[MAX_PATH+1];
	char bottom[MAX_PATH+1];
	char top[MAX_PATH+1];
	char back[MAX_PATH+1];
	
	sprintf(front, "%sft^", basename);
	sprintf(left, "%slf^", basename);
	sprintf(right, "%srt^", basename);
	sprintf(bottom, "%sdn^", basename);
	sprintf(top, "%sup^", basename);
	sprintf(back, "%sbk^", basename);
	
	FindTextureExtension(front);
	FindTextureExtension(left);
	FindTextureExtension(right);
	FindTextureExtension(bottom);
	FindTextureExtension(top);
	FindTextureExtension(back);
	
	QueueTexture(&g_front, front, true, true);
	QueueTexture(&g_left, left, true, true);
	QueueTexture(&g_right, right, true, true);
	QueueTexture(&g_bottom, bottom, true, true);
	QueueTexture(&g_top, top, true, true);
	QueueTexture(&g_back, back, true, true);
}

void DrawSkyBox(Vec3f pos)
{
	glDisable(GL_DEPTH_TEST);

	Shader* s = &g_shader[g_curS];
	
	Matrix modelmat;
    modelmat.setTranslation((const float*)&pos);
    glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
	
	// right
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_right ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);

	float vright[] =
	{
	SKY_TEX_1, SKY_TEX_0, SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE,
	SKY_TEX_1, SKY_TEX_1, SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_1,-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,	
	SKY_TEX_0, SKY_TEX_0, -SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE
	};

    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vright[2]);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vright[0]);
	glDrawArrays(GL_QUADS, 0, 4);
	
	// left
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_left ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
	
	float vleft[] =
	{
	SKY_TEX_1, SKY_TEX_1, -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_1, SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_0, SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_1, SKY_TEX_0, -SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE
	};

    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vleft[2]);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vleft[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	// front
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_front ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
	
	float vfront[] =
	{
	SKY_TEX_1, SKY_TEX_0, SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_1, SKY_TEX_1, SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_1, SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_0, SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE
	};

    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vfront[2]);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vfront[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	// back
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_back ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);

	float vback[] =
	{
	SKY_TEX_1, SKY_TEX_1, -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_1, -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_0, -SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_1, SKY_TEX_0, -SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE
	};

    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vback[2]);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vback[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	// bottom
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_bottom ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
	
	float vbottom[] =
	{
	SKY_TEX_1, SKY_TEX_1, SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_1, -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TEX_0, SKY_TEX_0, -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
	SKY_TEX_1, SKY_TEX_0, SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE
	};

    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vbottom[2]);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vbottom[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	// top
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_top ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
	
	float vtop[] =
	{
	SKY_TOP_0, SKY_TOP_1, -SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE,
	SKY_TOP_0, SKY_TOP_0, -SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TOP_1, SKY_TOP_0, SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,
	SKY_TOP_1, SKY_TOP_1, SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE
	};

    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vtop[2]);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vtop[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	glEnable(GL_DEPTH_TEST);
}

#if 0
void DrawSkyBox2(CVector3 pos)
{
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	glTranslatef(pos.x, pos.y + SKYBOX_SIZE/2, pos.z);

	glBindTexture(GL_TEXTURE_2D, g_front);
	glBegin(GL_QUADS);
	SKY_TEX_0, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	SKY_TEX_0, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	SKY_TEX_1, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);	
	SKY_TEX_1, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, g_back);
	glBegin(GL_QUADS);
	SKY_TEX_1, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TEX_0, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TEX_0, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TEX_1, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_right);
	glBegin(GL_QUADS);
	SKY_TEX_0, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	SKY_TEX_0, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	SKY_TEX_1, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TEX_1, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_left);
	glBegin(GL_QUADS);
	SKY_TEX_1, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	SKY_TEX_0, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TEX_0, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TEX_1, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_bottom);
	glBegin(GL_QUADS);
	SKY_TEX_1, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	SKY_TEX_0, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	SKY_TEX_0, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TEX_1, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_top);
	glBegin(GL_QUADS);
	SKY_TOP_1, SKY_TOP_1);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	SKY_TOP_1, SKY_TOP_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TOP_0, SKY_TOP_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	SKY_TOP_0, SKY_TOP_1);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	/*
	glBindTexture(GL_TEXTURE_2D, g_front);
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);	
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, g_back);
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_right);
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_left);
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_bottom);
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_top);
	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();
	*/

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}
#endif