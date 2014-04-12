
#ifndef TRYSTEP_H
#define TRYSTEP_H

// JPS expansion directions
#define EXP_N			0
#define EXP_E			1
#define EXP_S			2
#define EXP_W			3
#define EXP_NE			4
#define EXP_NE_N		5
#define EXP_NE_E		6
#define EXP_SE			7
#define EXP_SE_S		8
#define EXP_SE_E		9
#define EXP_SW			10
#define EXP_SW_S		11
#define EXP_SW_W		12
#define EXP_NW			13
#define EXP_NW_N		14
#define EXP_NW_W		15

// byte-align structures
#pragma pack(push, 1)

class TryStep
{
public:
	int F;
	short cx;
	short cz;
	int runningD;
	unsigned char expansion;
	TryStep* previous;
	//bool tried;
	TryStep()
	{
		//tried = false;
	};
	TryStep(int startx, int startz, int endx, int endz, int cx, int cz, TryStep* prev, int totalD, int stepD);
	TryStep(int startx, int startz, int endx, int endz, int cx, int cz, TryStep* prev, int totalD, int stepD, unsigned char expan);
};

#pragma pack(pop)

#endif