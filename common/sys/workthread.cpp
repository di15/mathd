

#include "workthread.h"
#include "../platform.h"
#include "../render/heightmap.h"
#include "../ai/collidertile.h"
#include "../ai/pathnode.h"
#include "../../game/gmain.h"
#include "threadjob.h"
#include "../sim/sim.h"
#include "../utils.h"
#include "updthread.h"

WorkThread g_workthread[WORKTHREADS];
short g_curthread = 0;
HANDLE g_glovarmutex;
HANDLE g_drawmutex;

void WorkThread::alloc(int cwx, int cwz)
{
	destroy();
	pathnode = new PathNode [ cwx * cwz ];
	openlist.alloc( cwx * cwz );

	for(int x=0; x<cwx; x++)
		for(int z=0; z<cwz; z++)
		{
			PathNode* n = PathNodeAt(this, x, z);
			n->nx = x;
			n->nz = z;
			n->opened = false;
			n->closed = false;
		}
		
	lastpath = g_simframe;
}

void WorkThread::destroy()
{
	if(pathnode)
	{
		delete [] pathnode;
		pathnode = NULL;
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

WorkThread::WorkThread()
{
	pathnode = NULL;
	startsignal = false;
	finished = true;
	hthread = NULL;
}

WorkThread::~WorkThread()
{
	destroy();
}

void WorkThread::resetcells()
{
	for(int i = 0; i < g_pathdim.x * g_pathdim.y; i++)
	{
		PathNode* n = &pathnode[i];
		n->closed = false;
		n->opened = false;
	}
	openlist.resetelems();
}

DWORD WINAPI WorkMain(PVOID pparam)
{
	WorkThread* wt = (WorkThread*)pparam;

#ifdef MULTITHREADED
	while(true)
	{
		MutexWait(g_glovarmutex);
		if(g_quit)
		{
			MutexRelease(g_glovarmutex);
			break;
		}
		MutexRelease(g_glovarmutex);
		
		MutexWait(wt->mutex);
		if(!wt->startsignal)
		{
			MutexRelease(wt->mutex);
			Sleep(1);
			continue;
		}
		wt->finished = false;
		wt->startsignal = false;
		MutexRelease(wt->mutex);
#endif

		if(wt->jobs.size() <= 0)
		{
#ifdef MULTITHREADED
			MutexWait(wt->mutex);
			wt->finished = true;
			MutexRelease(wt->mutex);
			Sleep(1);
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
		MutexWait(wt->mutex);
		wt->finished = true;
		MutexRelease(wt->mutex);
	}
#endif

	return 0;
}

void StartThreads()
{
	g_glovarmutex = CreateMutex(NULL, FALSE, NULL);
	g_drawmutex = CreateMutex(NULL, FALSE, NULL);

	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
		wt->mutex = CreateMutex(NULL, FALSE, NULL);
	}

	for(int i=0; i<WORKTHREADS; i++)
		g_workthread[i].destroy();

	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
#ifdef MULTITHREADED
		wt->hthread = CreateThread(NULL, 0, WorkMain, (PVOID)wt, 0, &wt->threadid);
#endif
	}
	
	g_hupdthread = CreateThread(NULL, 0, UpdMain, (PVOID)NULL, 0, NULL);

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
		WaitForMultipleObjects(1, &wt->hthread, TRUE, INFINITE);
#endif
		CloseHandle(wt->hthread);
		CloseHandle(wt->mutex);
	}
	
	WaitForMultipleObjects(1, &g_hupdthread, TRUE, INFINITE);
	CloseHandle(g_hupdthread);
	
	CloseHandle(g_drawmutex);
	CloseHandle(g_glovarmutex);
}

void MutexWaitInf(HANDLE hmutex)
{
	MutexWait(hmutex, INFINITE);
}

bool MutexWait(HANDLE hmutex, DWORD millisecs)
{
#ifdef MULTITHREADED
	DWORD dwWaitResult = WaitForSingleObject( 
		hmutex,    // handle to mutex
		millisecs);  // no time-out interval

#if 1
	switch (dwWaitResult) 
	{
		// The thread got ownership of the mutex
	case WAIT_OBJECT_0:
		return true;
#if 0
		__try { 
			// TODO: Write to the database
			//printf("Thread %d writing to database...\n", GetCurrentThreadId());
			//dwCount++;
		} 

		__finally { 
			// Release ownership of the mutex object
			if (! ReleaseMutex(hmutex)) 
			{ 
				// Handle error.
			} 
		} 
#endif
		break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
	case WAIT_ABANDONED: 
		return true; 
	case WAIT_TIMEOUT:
		return false;
	}
#endif
#endif

	return false;
}

void MutexRelease(HANDLE hmutex)
{
#ifdef MULTITHREADED
	ReleaseMutex(hmutex);
#endif
}

void ThreadPhase()
{
	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
#ifdef MULTITHREADED
		MutexWait(wt->mutex);
		wt->startsignal = true;
		MutexRelease(wt->mutex);
#else
		WorkMain(&g_workthread[0]);
#endif
	}

#ifdef MULTITHREADED
	while(true)
	{
		bool allfinished = true;
		
		for(int i=0; i<WORKTHREADS; i++)
		{
			WorkThread* wt = &g_workthread[i];
			
			MutexWait(wt->mutex);

			if(wt->startsignal)
			{
				allfinished = false;
			}

			if(!wt->finished)
			{
				allfinished = false;
			}

			MutexRelease(wt->mutex);

			if(!allfinished)
				break;
		}

		if(!allfinished)
		{
			Sleep(1);
			continue;
		}

		break;
	}
#endif
}