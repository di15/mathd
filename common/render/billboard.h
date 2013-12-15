

#ifndef BILLBOARD_H
#define BILLBOARD_H

#include <WinSock2.h>	//winsock2.h must be included before windows.h
#include <Shlwapi.h>
#include <vector>
#include "3dmath.h"

#pragma comment(lib, "shlwapi.lib")

using namespace std;

class CBillboardType
{
public:
	bool on;
    char name[32];
    unsigned int tex;

	CBillboardType()
	{
		on = false;
	}
};

#define BILLBOARD_TYPES			64
extern CBillboardType g_billbT[BILLBOARD_TYPES];

class CBillboard
{
public:
    bool on;
    int type;
    float size;
    Vec3f pos;
    float dist;
	int particle;
    
    CBillboard()
    {
        on = false;
		particle = -1;
    }
};

#define BILLBOARDS  512
extern CBillboard g_billb[BILLBOARDS];

extern unsigned int g_muzzle[4];

void Effects();
int NewBillboard();
int NewBillboard(char* tex);
int Billboard(const char* name);
void SortBillboards();
void DrawBillboards();
void PlaceBillboard(const char* n, Vec3f pos, float size, int particle=-1);
void PlaceBillboard(int type, Vec3f pos, float size, int particle=-1);

#endif