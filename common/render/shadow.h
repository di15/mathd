


extern unsigned int g_depth;

//enum UNIFORM{SHADOWMAP, LIGHTMATRIX, LIGHTPOS, LIGHTDIR, TEXTURE1, TEXTURE2, TEXTURE4, UNIFORMS};
//extern unsigned int g_uniform[UNIFORMS];

class Matrix;

void InitShadows();
void RenderToShadowMap(Matrix projection, Matrix viewmat, Matrix modelmat);
void RenderShadowedScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelview);