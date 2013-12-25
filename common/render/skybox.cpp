

#include "main.h"
#include "skybox.h"
#include "3dmath.h"
#include "shader.h"
#include "image.h"

unsigned int g_front;
unsigned int g_left;
unsigned int g_right;
unsigned int g_bottom;
unsigned int g_top;
unsigned int g_back;

void LoadSkyBox(const char* basename)
{
	char front[128];
	char left[128];
	char right[128];
	char bottom[128];
	char top[128];
	char back[128];
	
	sprintf(front, "enviro\\%sft^", basename);
	sprintf(left, "enviro\\%slf^", basename);
	sprintf(right, "enviro\\%srt^", basename);
	sprintf(bottom, "enviro\\%sdn^", basename);
	sprintf(top, "enviro\\%sup^", basename);
	sprintf(back, "enviro\\%sbk^", basename);
	
	FindTextureExtension(front);
	FindTextureExtension(left);
	FindTextureExtension(right);
	FindTextureExtension(bottom);
	FindTextureExtension(top);
	FindTextureExtension(back);
	
	CreateTexture(g_front, front, true);
	CreateTexture(g_left, left, true);
	CreateTexture(g_right, right, true);
	CreateTexture(g_bottom, bottom, true);
	CreateTexture(g_top, top, true);
	CreateTexture(g_back, back, true);
}

void DrawSkyBox(Vec3f pos)
{
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);

/*	
original should be
lf -> bk
rt -> ft
bk -> lf
fr -> rt

switch lf with rt
*/

	glBindTexture(GL_TEXTURE_2D, g_right);	//g_front);
	glBegin(GL_QUADS);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);	
	glTexCoord2f(SKY_TEX_1, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, g_left);	//g_back);
	glBegin(GL_QUADS);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_front);	//g_right);
	glBegin(GL_QUADS);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_back);	//g_left);
	glBegin(GL_QUADS);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_bottom);
	glBegin(GL_QUADS);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_0);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_0);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_0, SKY_TEX_1);	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TEX_1, SKY_TEX_1);	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, g_top);
	glBegin(GL_QUADS);
	glTexCoord2f(SKY_TOP_1, SKY_TOP_1);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(SKY_TOP_1, SKY_TOP_0);	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TOP_0, SKY_TOP_0);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(SKY_TOP_0, SKY_TOP_1);	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}
