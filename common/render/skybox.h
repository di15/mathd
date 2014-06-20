


#define SKYBOX_SIZE	1600.0f

#define SKY_TEX_0	0.002f
#define SKY_TEX_1	0.998f

#define SKY_TOP_0	0.002f
#define SKY_TOP_1	0.998f

extern unsigned int g_front;
extern unsigned int g_left;
extern unsigned int g_right;
extern unsigned int g_bottom;
extern unsigned int g_top;
extern unsigned int g_back;

class Vec3f;

void DrawSkyBox(Vec3f pos);
void LoadSkyBox(const char* basename);