
#include <vector>
#include "3dmath.h"

using namespace std;

class COrder
{
public:
	Vec3f pos;
	long tick;
	float radius;

	COrder(Vec3f p, long t, float r)
	{
		pos = p;
		tick = t;
		radius = r;
	}
};

#define ORDER_EXPIRE		2000

extern vector<COrder> g_order;

void DrawOrders();
void Order();