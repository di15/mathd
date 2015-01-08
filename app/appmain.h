#ifndef GMAIN_H
#define GMAIN_H



void WriteConfig();
void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float lightpos[3], float lightdir[3]);
void DrawSceneDepth();

#endif
