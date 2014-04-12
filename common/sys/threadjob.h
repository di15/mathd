

#ifndef THREADJOB_H
#define THREADJOB_H

#include "../platform.h"

class ThreadJob
{
public:
	short tjtype;
	virtual bool process();
};

#define THREADJOB_PATH		0

#endif