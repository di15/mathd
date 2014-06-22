

#ifndef SHADOW_H
#define SHADOW_H

#include "../math/vec3f.h"

extern unsigned int g_depth;
extern Vec3f g_lightPos;
extern Vec3f g_lightEye;
extern Vec3f g_lightUp;
extern Vec3f g_lightOff;

class Matrix;

extern Matrix g_cameraModelViewMatrix;
extern Matrix g_cameraProjectionMatrix;
extern Matrix g_cameraViewMatrix;

void InitShadows();
void RenderToShadowMap(Matrix projection, Matrix viewmat, Matrix modelmat, Vec3f focus, Vec3f lightpos, void (*drawscenedepthfunc)());
void RenderShadowedScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelview, void (*drawscenefunc)(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]));
void UseShadow(int shader, Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]);


void Transpose(Matrix mat, Matrix &transpMat);
bool Inverse2(Matrix mat, Matrix &invMat);

#if 0
extern void (*DrawSceneDepthFunc)();
extern void (*DrawSceneFunc)(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]);
#endif

#endif