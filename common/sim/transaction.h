

#include "3dmath.h"

class CTransaction
{
public:
	bool on;
	float halfwidth;
	char text[32];
	Vec3f pos;
	float life;

	CTransaction()
	{
		on = false;
	}
};

#define TRANSACTION_RISE		0.3f
#define TRANSACTION_DECAY		0.025f

#define TRANSACTIONS	128
extern CTransaction g_transx[TRANSACTIONS];

void DrawTransactions(Matrix projmodlview);
void NewTransx(Vec3f pos, int res1, float res1d, int res2, float res2d);
void NewTransx(Vec3f pos, int res1, float res1d);
void NewTransx(Vec3f pos, const char* text);
void TransxAppend(char* str, int res1, float res1d);