

#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include "../platform.h"
#include "../ai/binheap.h"

class ThreadJob;
class PathNode;

#define MULTITHREADED

class WorkThread
{
public:
	//list<PathNode> openlist;
	PathNode *pathnode;
	Heap openlist;
	HANDLE mutex;
	bool startsignal;
	bool finished;
	HANDLE hthread;
	DWORD threadid;
	list<ThreadJob*> jobs;
	long long lastpath;
	list<PathNode*> toclear;

	void alloc(int cwx, int cwz);
	void destroy();
	void resetcells();
	WorkThread();
	~WorkThread();
};

#ifdef MULTITHREADED
#define WORKTHREADS		2
#else
#define WORKTHREADS		1
#endif

extern WorkThread g_workthread[WORKTHREADS];
extern short g_curthread;
extern HANDLE g_glovarmutex;
extern HANDLE g_drawmutex;

void StartThreads();
void StopThreads();
void ThreadPhase();
bool MutexWait(HANDLE hmutex, DWORD millisecs);
void MutexWaitInf(HANDLE hmutex);
void MutexRelease(HANDLE hmutex);

#endif