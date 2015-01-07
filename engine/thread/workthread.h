

#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include "../platform.h"
#include "../sys/binheap.h"
#include "../path/pathnode.h"

#if 1	//MT TODO
class ThreadJob;
class PathNode;

#define MULTITHREADED

#endif

class WorkThread
{
public:
#if 1	//MT TODO
	//list<PathNode> openlist;
	PathNode *pathnodes;
	BinHeap openlist;
	SDL_mutex* mutex;
	bool startsignal;
	bool finished;
	SDL_Thread* hthread;
	std::list<ThreadJob*> jobs;
	unsigned int lastpath;
	std::list<PathNode*> toclear;

	void alloc(int cwx, int cwz);
	void destroy();
	void resetnodes();
	WorkThread();
	~WorkThread();
#endif
};

#if 1	//MT TODO

#ifdef MULTITHREADED
#define WORKTHREADS		1
#else
#define WORKTHREADS		1
#endif

extern WorkThread g_workthread[WORKTHREADS];
extern unsigned char g_curthread;
extern SDL_mutex* g_glovarmutex;
extern SDL_mutex* g_drawmutex;

void StartThreads();
void StopThreads();
void ThreadPhase();
bool MutexWait(SDL_mutex *hmutex, int millisecs);
void MutexWaitInf(SDL_mutex *hmutex);
void MutexUnlock(SDL_mutex *hmutex);

#endif

#endif