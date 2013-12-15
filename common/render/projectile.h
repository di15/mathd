
#include <vector>
#include "3dmath.h"

using namespace std;

class CProjectileType
{
public:
	unsigned int tex;

	void Define(char* texpath);
};

enum PROJECTILE{GUNPROJ, PROJECTILE_TYPES};
extern CProjectileType g_projectileType[PROJECTILE_TYPES];

class CProjectile
{
public:
	bool on;
	Vec3f start;
	Vec3f end;
	int type;

	CProjectile()
	{
		on = false;
	}

	CProjectile(Vec3f s, Vec3f e, int t)
	{
		on = true;
		start = s;
		end = e;
		type = t;
	}
};

#define PROJECTILES	128
extern CProjectile g_projectile[PROJECTILES];

void LoadProjectiles();
void DrawProjectiles();
void NewProjectile(Vec3f start, Vec3f end, int type);
