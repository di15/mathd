

#include "workthread.h"
#include "../platform.h"
#include "../render/heightmap.h"
#include "../path/collidertile.h"
#include "../path/pathnode.h"
#include "../../game/gmain.h"
#include "threadjob.h"
#include "../sim/simflow.h"
#include "../utils.h"

#if 1	//MT TODO

WorkThread g_workthread[WORKTHREADS];
unsigned char g_curthread = 0;
SDL_mutex* g_glovarmutex = NULL;
SDL_mutex* g_drawmutex = NULL;

void WorkThread::alloc(int cwx, int cwz)
{
	destroy();
	pathnodes = new PathNode [ cwx * cwz ];
	openlist.alloc( cwx * cwz );

	for(int x=0; x<cwx; x++)
		for(int z=0; z<cwz; z++)
		{
			PathNode* n = PathNodeAt(this, x, z);
			n->opened = false;
			n->closed = false;
		}
		
	lastpath = g_simframe;
}

void WorkThread::destroy()
{
	if(pathnodes)
	{
		delete [] pathnodes;
		pathnodes = NULL;
	}

	openlist.freemem();

#if 0
	auto tjiter = jobs.begin();
	while(tjiter != jobs.end())
	{
		ThreadJob* tj = *tjiter;
		delete tj;
		tjiter = jobs.erase(tjiter);
	}
#elif 1
	if(jobs.size() > 0)
	{
		for(auto tjiter = jobs.begin(); tjiter != jobs.end(); tjiter++)
		{
			ThreadJob* tj = *tjiter;
			delete tj;
		}

		jobs.clear();
	}
#endif

	startsignal = false;
	finished = true;
}

WorkThread::WorkThread() : openlist(CompareNodes)
{
	//return;

	pathnodes = NULL;
	startsignal = false;
	finished = true;
	hthread = NULL;
	mutex = NULL;
	//openlist = BinHeap(CompareNodes);
}

WorkThread::~WorkThread()
{
	destroy();
}

void WorkThread::resetnodes()
{
	//return;

	for(int i = 0; i < g_pathdim.x * g_pathdim.y; i++)
	{
		PathNode* n = &pathnodes[i];
		n->closed = false;
		n->opened = false;
	}
	openlist.resetelems();
}

static int WorkMain(void *pparam)
{
	//return 0;

	WorkThread *wt = (WorkThread*)pparam;

#ifdef MULTITHREADED
	while(true)
	{
		if(MutexWait(g_glovarmutex, 1))
		{
			if(g_quit)
			{
				MutexUnlock(g_glovarmutex);
				break;
			}
			MutexUnlock(g_glovarmutex);
		}
		
		MutexWaitInf(wt->mutex);
		if(!wt->startsignal)
		{
			MutexUnlock(wt->mutex);
			SDL_Delay(1);
			continue;
		}
		wt->finished = false;
		wt->startsignal = false;
		MutexUnlock(wt->mutex);
#endif

		if(wt->jobs.size() <= 0)
		{
#ifdef MULTITHREADED
			MutexWaitInf(wt->mutex);
			wt->finished = true;
			MutexUnlock(wt->mutex);
			SDL_Delay(1);
			continue;
#else
			return 0;
#endif
		}
		
		auto tjiter = wt->jobs.begin();

		while(tjiter != wt->jobs.end())
		{
			ThreadJob* tj = *tjiter;

			if(tj->process())
			{
				tjiter = wt->jobs.erase(tjiter);
				delete tj;
				continue;
			}

			tjiter++;
		}

#ifdef MULTITHREADED
		MutexWaitInf(wt->mutex);
		wt->finished = true;
		MutexUnlock(wt->mutex);
	}
#endif

	return 0;
}

void StartThreads()
{
	//return;

	g_glovarmutex = SDL_CreateMutex();
	g_drawmutex = SDL_CreateMutex();

#if 1
	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
		wt->mutex = SDL_CreateMutex();
	}

	for(int i=0; i<WORKTHREADS; i++)
		g_workthread[i].destroy();

	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
		char name[16];
		sprintf(name, "w%d", i);
#ifdef MULTITHREADED
		//wt->hthread = CreateThread(NULL, 0, WorkMain, (PVOID)wt, 0, &wt->threadid);
		wt->hthread = SDL_CreateThread(WorkMain, name, (void *)wt);
#endif
	}
#endif

	//g_hupdthread = CreateThread(NULL, 0, UpdMain, (PVOID)NULL, 0, NULL);
}

void StopThreads()
{
#if 0
	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
		wt->destroy();
		TerminateThread(wt->hthread, 0);
		CloseHandle(wt->hthread);
		wt->hthread = NULL;
	}
#endif
	
	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
#ifdef MULTITHREADED
		g_log<<"wai "<<i<<std::endl;
		g_log.flush();
		SDL_WaitThread(wt->hthread, NULL);
		wt->hthread = NULL;
#endif
		g_log<<"des "<<i<<std::endl;
		g_log.flush();
		//CloseHandle(wt->hthread);
		SDL_DestroyMutex(wt->mutex);
		wt->mutex = NULL;
	}
	
	//WaitForMultipleObjects(1, &g_hupdthread, TRUE, INFINITE);
	//CloseHandle(g_hupdthread);
	
	SDL_DestroyMutex(g_drawmutex);
	SDL_DestroyMutex(g_glovarmutex);
	g_drawmutex = NULL;
	g_glovarmutex = NULL;
}

//TODO Better to use SDL conditions so the mutexes don't use up idle CPU cycles.

void MutexWaitInf(SDL_mutex* hmutex)
{
	//return;

	if(!hmutex)
		return;

	while(SDL_LockMutex(hmutex) != 0)
		SDL_Delay(1);
}

bool MutexWait(SDL_mutex* hmutex, int millisecs)
{
	//return true;

	if(!hmutex)
		return true;

	unsigned long long start = GetTickCount64();

	do
	{
		if(SDL_LockMutex(hmutex) == 0)
			return true;

		SDL_Delay(1);

	}while(GetTickCount64() - start < millisecs);

	return false;
}

void MutexUnlock(SDL_mutex* hmutex)
{
	//return;
	
	if(!hmutex)
		return;

#ifdef MULTITHREADED
	//SDL_DestroyMutex(hmutex);
	SDL_UnlockMutex(hmutex);
#endif
}

//TO DO: threads need to do jobs immediately as soon as they start arriving,
//not waiting for this function. Need thread-safe way to add and consume jobs.
void ThreadPhase()
{
	//g_log<<"th p"<<std::endl;
	//g_log.flush();

	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
#ifdef MULTITHREADED
		MutexWaitInf(wt->mutex);
		wt->startsignal = true;
		MutexUnlock(wt->mutex);
#else
		WorkMain(&g_workthread[0]);
#endif
	}

	//g_log<<"th p2"<<std::endl;
	//g_log.flush();

#ifdef MULTITHREADED
	while(true)
	{
		bool allfinished = true;
		
		for(int i=0; i<WORKTHREADS; i++)
		{
			WorkThread* wt = &g_workthread[i];
			
			MutexWaitInf(wt->mutex);

			if(wt->startsignal)
			{
				allfinished = false;
			}

			if(!wt->finished)
			{
				allfinished = false;
			}

			MutexUnlock(wt->mutex);

			if(!allfinished)
				break;
		}

		if(!allfinished)
		{
			SDL_Delay(1);
			continue;
		}

		break;
	}
#endif

	//g_log<<"th p3"<<std::endl;
	//g_log.flush();
}

#endif