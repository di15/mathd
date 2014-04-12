

#ifndef PATHJOB_H
#define PATHJOB_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../sys/workthread.h"
#include "../sys/threadjob.h"

class Unit;
class Building;

#define PATH_DELAY		200

// byte-align structures
#pragma pack(push, 1)
class PathJob : public ThreadJob
{
public:
	unsigned char utype;
	unsigned char umode;
	int cmstartx;
	int cmstartz;
	int target;
	int target2;
	int targtype;
	list<Vec2i> *path;
	Vec2i *subgoal;
	short thisu;
	short ignoreu;
	short ignoreb;
	int cmgoalx;
	int cmgoalz;
	int cmgoalminx;
	int cmgoalminz;
	int cmgoalmaxx;
	int cmgoalmaxz;
	bool roaded;
	bool landborne;
	bool seaborne;
	bool airborne;
	WorkThread* wt;
	void (*callback)(bool result, PathJob* pj);

	virtual bool process();
};
#pragma pack(pop)

void Callback_UnitPath(bool result, PathJob* pj);

#endif