

#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include "../platform.h"
#include "../ai/binheap.h"

class ThreadJob;
class PathNode;

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

#define WORKTHREADS		4

extern WorkThread g_workthread[WORKTHREADS];
extern short g_curthread;
extern HANDLE g_hmutex;

void StartThreads();
void StopThreads();
void ThreadPhase();
bool MutexWait(HANDLE hmutex);
void MutexRelease(HANDLE hmutex);

#endif