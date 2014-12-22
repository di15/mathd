#ifndef WEVIEWPORT_H
#define WEVIEWPORT_H

#include "../../common/math/3dmath.h"
#include "../../common/math/camera.h"
#include "../../common/math/vec2i.h"

#define VIEWPORT_MINIMAP		0
#define VIEWPORT_ENTVIEW		1
#define VIEWPORT_TYPES			2

class VpType
{
public:
	Vec3f m_offset;
	Vec3f m_up;
	char m_label[32];
	bool m_axial;

	VpType() {}
	VpType(Vec3f offset, Vec3f up, const char* label, bool axial);
};

extern VpType g_vptype[VIEWPORT_TYPES];

#if 0
class Viewport
{
public:
	int m_type;
	bool m_ldown;
	bool m_rdown;
	bool m_mdown;
	Vec2i m_lastmouse;
	Vec2i m_curmouse;
	bool m_drag;

	Viewport();
	Viewport(int type);
	Vec3f up();
	Vec3f up2();
	Vec3f strafe();
	Vec3f focus();
	Vec3f viewdir();
	Vec3f pos();
};

extern Viewport g_viewport[4];
//extern Vec3f g_focus;
#endif

void DrawViewport(int which, int x, int y, int width, int height);

#if 0
bool ViewportLDown(int which, int relx, int rely, int width, int height);
bool ViewportLUp(int which, int relx, int rely, int width, int height);
bool ViewportMousemove(int which, int relx, int rely, int width, int height);
bool ViewportRDown(int which, int relx, int rely, int width, int height);
bool ViewportRUp(int which, int relx, int rely, int width, int height);
bool NULL(int which, int delta);
#endif

#endif
