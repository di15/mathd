

#include "main.h"
#include "shadow.h"
#include "map.h"
#include "shader.h"
#include "3dmath.h"
#include "model.h"
#include "building.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "particle.h"
#include "unit.h"
#include "selection.h"
#include "order.h"
#include "pathfinding.h"
#include "projectile.h"
#include "debug.h"
#include "billboard.h"
#include "skybox.h"
#include "chat.h"
#include "water.h"
#include "scenery.h"

unsigned int g_depth;
int g_depthSizeX = 512;	//4096;
int g_depthSizeY = 512;	//4096;
unsigned int g_rbDepth;
unsigned int g_fbDepth;

Vec3f g_lightPos;	//(0, 160, MAP_SIZE*TILE_SIZE);
Vec3f g_lightEye;	//(MAP_SIZE*TILE_SIZE/2, 0, MAP_SIZE*TILE_SIZE/2);
//float g_lightPos[] = {0, 16, 4*TILE_SIZE};
//float g_lightEye[] = {0, 0, 0};
//Vec3f g_lightPos;
//Vec3f g_lightEye;
Vec3f g_lightUp(0,1,0);
/*
Matrix g_lightProjectionMatrix;
Matrix g_lightModelViewMatrix;

Matrix g_cameraInverseModelViewMatrix;
Matrix g_lightMatrix;

Matrix g_cameraModelViewMatrix;*/

//float g_lightProjectionMatrix[16];
Matrix g_lightProjectionMatrix;
//float g_lightModelViewMatrix[16];
Matrix g_lightModelViewMatrix;

//float g_cameraInverseModelViewMatrix[16];
Matrix g_cameraInverseModelViewMatrix;
//float g_lightMatrix[16];
Matrix g_lightMatrix;

//float g_cameraModelViewMatrix[16];
Matrix g_cameraModelViewMatrix;
//float g_cameraProjectionMatrix[16];
Matrix g_cameraProjectionMatrix;

//unsigned int g_uniform[UNIFORMS];

void InitShadows()
{
	glGenTextures(1, &g_depth);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, g_depthSizeX, g_depthSizeY, 0, GL_RGBA, GL_UNSIGNED_SHORT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffersEXT(1, &g_rbDepth);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_rbDepth);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, g_depthSizeX, g_depthSizeY);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	
	glGenFramebuffersEXT(1, &g_fbDepth);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_fbDepth);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_depth, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_rbDepth);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	g_shader[DEPTH].InitShaders("shaders\\depth.vert", "shaders\\depth.frag");
	g_shader[SHADOW].InitShaders("shaders\\shadow.vert", "shaders\\shadow.frag");
	g_shader[SHADOWOWN].InitShaders("shaders\\shadowown.vert", "shaders\\shadowown.frag");
	
	//g_uniform[SHADOWMAP] = g_shader[SHADOW].GetVariable("shadowMap");
	//g_uniform[LIGHTMATRIX] = g_shader[SHADOW].GetVariable("lightMatrix");
	//g_uniform[LIGHTPOS] = g_shader[SHADOW].GetVariable("lightPos");
	//g_uniform[LIGHTDIR] = g_shader[SHADOW].GetVariable("lightDir");
	//g_uniform[TEXTURE1] = g_shader[SHADOW].GetVariable("texture1");
	//g_uniform[TEXTURE2] = g_shader[SHADOW].GetVariable("texture2");
	//g_uniform[TEXTURE4] = g_shader[SHADOW].GetVariable("texture4");
}

void InverseMatrix(Matrix* dstm, Matrix srcm)
{
	float dst[16];
	const float* src = srcm.getMatrix();

	dst[0] = src[0];
	dst[1] = src[4];
	dst[2] = src[8];
	dst[3] = 0.0;
	dst[4] = src[1];
	dst[5] = src[5];
	dst[6]  = src[9];
	dst[7] = 0.0;
	dst[8] = src[2];
	dst[9] = src[6];
	dst[10] = src[10];
	dst[11] = 0.0;
	dst[12] = -(src[12] * src[0]) - (src[13] * src[1]) - (src[14] * src[2]);
	dst[13] = -(src[12] * src[4]) - (src[13] * src[5]) - (src[14] * src[6]);
	dst[14] = -(src[12] * src[8]) - (src[13] * src[9]) - (src[14] * src[10]);
	dst[15] = 1.0;

	dstm->set(dst);
}

void InverseMatrix(float dst[16], float src[16])
{
	dst[0] = src[0];
	dst[1] = src[4];
	dst[2] = src[8];
	dst[3] = 0.0;
	dst[4] = src[1];
	dst[5] = src[5];
	dst[6]  = src[9];
	dst[7] = 0.0;
	dst[8] = src[2];
	dst[9] = src[6];
	dst[10] = src[10];
	dst[11] = 0.0;
	dst[12] = -(src[12] * src[0]) - (src[13] * src[1]) - (src[14] * src[2]);
	dst[13] = -(src[12] * src[4]) - (src[13] * src[5]) - (src[14] * src[6]);
	dst[14] = -(src[12] * src[8]) - (src[13] * src[9]) - (src[14] * src[10]);
	dst[15] = 1.0;
}

bool gluInvertMatrix(const float m[16], float invOut[16])
{
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

bool InverseMatrix2(Matrix mat, Matrix invMat)
{
    double inv[16], det;
    int i;

	const float* m = mat.getMatrix();

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

	float invOut[16];
    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

	invMat.set(invOut);

    return true;
}

void Transpose(Matrix mat, Matrix transpMat)
{
	const float* m = mat.getMatrix();
	float transp[16];

	int j;
	int index_1;
	int index_2;

	for (int i = 0; i < 4; i++)
	{
		for (j = 0; j < 4;  j++)
		{
			index_1 = i * 4 + j;
			index_2 = j * 4 + i;
			transp[index_1] = m[index_2];
		}
	}

	transpMat.set(transp);
}

Vec3f g_viewInter;

void RenderToShadowMap(Matrix projection, Matrix viewmat, Matrix modelmat)
{
	Vec3f vLine[2];
	Vec3f ray = g_camera.View() - g_camera.Position();
	Vec3f onnear = g_camera.Position();	//OnNear(g_width/2, g_height/2);
	vLine[0] = onnear;
	vLine[1] = onnear + (ray * 100000.0f);
	GetMapIntersection(vLine, &g_lightEye);
	float viewD = Magnitude(onnear - g_lightEye) * 1.0f;
	g_lightPos = g_lightEye + Vec3f(-viewD, viewD, viewD);

	glViewport(0, 0, g_depthSizeX, g_depthSizeY);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_fbDepth);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0, 500.0);
	
	//g_lightProjectionMatrix = BuildPerspProjMat(90.0, 1.0, 30.0, 10000.0);
	g_lightProjectionMatrix = setorthographicmat(-PROJ_RIGHT, PROJ_RIGHT, PROJ_RIGHT, -PROJ_RIGHT, 30.0, 10000.0);
	g_lightModelViewMatrix = gluLookAt2(g_lightPos.x, g_lightPos.y, g_lightPos.z,
		g_lightEye.x, g_lightEye.y, g_lightEye.z, 
		g_lightUp.x, g_lightUp.y, g_lightUp.z);
	
	UseS(DEPTH);
	//UseS(ORTHO);
	glUniformMatrix4fv(g_shader[SHADER::DEPTH].m_slot[SLOT::PROJECTION], 1, 0, g_lightProjectionMatrix.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::DEPTH].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
	glUniformMatrix4fv(g_shader[SHADER::DEPTH].m_slot[SLOT::VIEWMAT], 1, 0, g_lightModelViewMatrix.getMatrix());
	glUniform4f(g_shader[SHADER::MODEL].m_slot[SLOT::COLOR], 1, 1, 1, 1);
	glEnableVertexAttribArray(g_shader[SHADER::DEPTH].m_slot[SLOT::POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER::DEPTH].m_slot[SLOT::TEXCOORD0]);
	
	g_hmap.draw();
	DrawBuildings();
	DrawPowerlines();
	DrawRoads();
	DrawPipelines();
	DrawSelB();
	DrawUnits();
	DrawScenery();
	
	TurnOffShader();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	UseS(ORTHO);
	
	glViewport(0, 0, g_width, g_height);
}

void ShadowShader(int shader, Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
	UseS(shader);
	Shader* s = &g_shader[g_curS];
	glUniformMatrix4fv(s->m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
	glUniformMatrix4fv(s->m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
	glUniformMatrix4fv(s->m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
	//glUniformMatrix4fv(s->m_slot[SLOT::NORMALMAT], 1, 0, modelviewinv.getMatrix());
	//glUniformMatrix4fv(s->m_slot[SLOT::INVMODLVIEWMAT], 1, 0, modelviewinv.getMatrix());
	glUniform4f(s->m_slot[SLOT::COLOR], 1, 1, 1, 1);
	glEnableVertexAttribArray(s->m_slot[SLOT::POSITION]);
	glEnableVertexAttribArray(s->m_slot[SLOT::TEXCOORD0]);
	//glEnableVertexAttribArray(s->m_slot[SLOT::TEXCOORD1]);
	//glEnableVertexAttribArray(s->m_slot[SLOT::NORMAL]);
	
	//glUniformMatrix4fvARB(s->m_slot[SLOT::LIGHTMATRIX], 1, false, g_lightMatrix);
	glUniformMatrix4fvARB(s->m_slot[SLOT::LIGHTMATRIX], 1, false, g_lightMatrix.getMatrix());
	
	glUniform3fARB(s->m_slot[SLOT::LIGHTPOS], mvLightPos[0], mvLightPos[1], mvLightPos[2]);
	glUniform3fARB(s->m_slot[SLOT::LIGHTDIR], lightDir[0], lightDir[1], lightDir[2]);
}

void RenderShadowedScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelview)
{
	StartProfile(SHADOWS);

	glViewport(0, 0, g_width, g_height);
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective(FIELD_OF_VIEW, double(g_width) / double(g_height), MIN_DISTANCE, MAX_DISTANCE);
	//g_cameraProjectionMatrix = BuildPerspProjMat(FIELD_OF_VIEW, double(g_width) / double(g_height), MIN_DISTANCE, MAX_DISTANCE);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//g_camera.Look();

	//glGetFloatv(GL_PROJECTION_MATRIX, g_cameraProjectionMatrix);
	g_cameraProjectionMatrix = projection;
	//glGetFloatv(GL_MODELVIEW_MATRIX, g_cameraModelViewMatrix);
	g_cameraModelViewMatrix = modelview;

	EndProfile(SHADOWS);

	// Do non-shadowed drawing here
	//DrawSkyBox(g_camera.LookPos());

	StartProfile(SHADOWS);
	
	InverseMatrix(&g_cameraInverseModelViewMatrix, modelview);
	//InverseMatrix(&g_cameraInverseModelViewMatrix, g_cameraModelViewMatrix);
	//InverseMatrix(g_cameraInverseModelViewMatrix, g_cameraModelViewMatrix);

	/*
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.5, 0.5, 0.5); // + 0.5
	glScalef(0.5, 0.5, 0.5); // * 0.5
	//glMultMatrixf(g_lightProjectionMatrix);
	glMultMatrixf(g_lightProjectionMatrix.getMatrix());
	//glMultMatrixf(g_lightModelViewMatrix);
	glMultMatrixf(g_lightModelViewMatrix.getMatrix());
	//glMultMatrixf(g_cameraInverseModelViewMatrix);
	glMultMatrixf(g_cameraInverseModelViewMatrix.getMatrix());
	float lm[16];
	//glGetFloatv(GL_MODELVIEW_MATRIX, g_lightMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, lm);
	g_lightMatrix.set(lm);
	glPopMatrix();
	*/
	
	//glPushMatrix();
	//glLoadIdentity();
	float trans[] = { 0.5f, 0.5f, 0.5f };
	//float trans[] = { 0, 0, 0 };
	//Matrix transm;
	//transm.setTranslation(trans);
	//g_lightMatrix.postMultiply(transm);
	g_lightMatrix.loadIdentity();
	g_lightMatrix.setTranslation(trans);
	//glTranslatef(0.5, 0.5, 0.5); // + 0.5
	//glScalef(0.5, 0.5, 0.5); // * 0.5
	float scalef[] = { 0.5f, 0.5f, 0.5f };
	Matrix scalem;
	scalem.setScale(scalef);
	g_lightMatrix.postMultiply(scalem);
	g_lightMatrix.postMultiply(g_lightProjectionMatrix);
	g_lightMatrix.postMultiply(g_lightModelViewMatrix);
	g_lightMatrix.postMultiply(g_cameraInverseModelViewMatrix);

	Matrix modelviewinv;
	InverseMatrix2(modelview, modelviewinv);
	Transpose(modelviewinv, modelviewinv);
	
	//const float* mv = g_cameraModelViewMatrix;
	const float* mv = g_cameraModelViewMatrix.getMatrix();
	//const float* mv = modelview.getMatrix();
	float mvLightPos[3];
	mvLightPos[0] = mv[0] * g_lightPos.x + mv[4] * g_lightPos.y + mv[8] * g_lightPos.z + mv[12];
	mvLightPos[1] = mv[1] * g_lightPos.x + mv[5] * g_lightPos.y + mv[9] * g_lightPos.z + mv[13];
	mvLightPos[2] = mv[2] * g_lightPos.x + mv[6] * g_lightPos.y + mv[10] * g_lightPos.z + mv[14];
	
	float lightDir[3];
	lightDir[0] = g_lightEye.x - g_lightPos.x;
	lightDir[1] = g_lightEye.y - g_lightPos.y;
	lightDir[2] = g_lightEye.z - g_lightPos.z;
	
	EndProfile(SHADOWS);
	
	ShadowShader(SHADOW, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);

	//glActiveTextureARB(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, g_transparency);
	//glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::TEXTURE0], 0);

	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_transparency);
	glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::TEXTURE1], 1);
	
	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_transparency);
	glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::TEXTURE2], 2);
	
	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_transparency);
	glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::TEXTURE3], 3);
	
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::SHADOWMAP], 4);
	
	g_hmap.draw();
	
	//glActiveTextureARB(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, g_transparency);
	//glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::TEXTURE0], 0);

	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_transparency);
	glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::TEXTURE1], 1);
	
	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_transparency);
	glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::TEXTURE2], 2);
	
	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_transparency);
	glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::TEXTURE3], 3);

	DrawWater();
	
	ShadowShader(SHADOWOWN, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[SHADER::SHADOW].m_slot[SLOT::SHADOWMAP], 4);

	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_transparency);
	glUniform1iARB(g_shader[SHADER::SHADOWOWN].m_slot[SLOT::TEXTURE1], 1);
	glActiveTextureARB(GL_TEXTURE0);

	DrawBuildings();
	DrawPowerlines();
	DrawRoads();
	DrawPipelines();
	DrawSelB();
	DrawUnits();
	DrawScenery();

	StartProfile(SHADOWS);
	TurnOffShader();
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	EndProfile(SHADOWS);

	if(g_mode == EDITOR)
	{
		UseS(COLOR3D);
		glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::PROJECTION], 1, 0, projection.getMatrix());
		glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::MODELMAT], 1, 0, modelmat.getMatrix());
		glUniformMatrix4fv(g_shader[SHADER::COLOR3D].m_slot[SLOT::VIEWMAT], 1, 0, viewmat.getMatrix());
		glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 0, 1, 0, 1);
		glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION]);
		glEnableVertexAttribArray(g_shader[SHADER::COLOR3D].m_slot[SLOT::NORMAL]);
		DrawTileSq();
	}
	
	TurnOffShader();
	//g_camera.Look();
}


