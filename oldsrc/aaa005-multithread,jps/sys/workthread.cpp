

#include "workthread.h"
#include "../platform.h"
#include "../render/heightmap.h"
#include "../ai/collidertile.h"
#include "../ai/pathnode.h"
#include "../../game/gmain.h"
#include "threadjob.h"
#include "../sim/sim.h"

WorkThread g_workthread[WORKTHREADS];
short g_curthread = 0;
HANDLE g_hmutex;

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

	auto tjiter = jobs.begin();
	while(tjiter != jobs.end())
	{
		ThreadJob* tj = *tjiter;
		delete tj;
		tjiter = jobs.erase(tjiter);
	}
	
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

DWORD WINAPI ThreadMain(PVOID pparam)
{
#if 0
	try{
#endif

	WorkThread* wt = (WorkThread*)pparam;

	while(true)
	{
		MutexWait(g_hmutex);
		if(g_quit)
		{
			MutexRelease(g_hmutex);
			break;
		}
		MutexRelease(g_hmutex);
		
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

		auto tjiter = wt->jobs.begin();

		if(tjiter == wt->jobs.end())
		{
			MutexWait(wt->mutex);
			wt->finished = true;
			MutexRelease(wt->mutex);
			Sleep(1);
			continue;
		}

		ThreadJob* tj = *tjiter;

		if(tj->process())
		{
			delete tj;
			wt->jobs.erase(tjiter);
		}
		
		MutexWait(wt->mutex);
		wt->finished = true;
		MutexRelease(wt->mutex);
	}

	return 0;
}

void StartThreads()
{
	g_hmutex = CreateMutex(NULL, FALSE, NULL);

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
		wt->hthread = CreateThread(NULL, 0, ThreadMain, (PVOID)wt, 0, &wt->threadid);
	}
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
		WaitForMultipleObjects(1, &wt->hthread, TRUE, INFINITE);
		CloseHandle(wt->hthread);
	}
	
	CloseHandle(g_hmutex);
}

bool MutexWait(HANDLE hmutex)
{
	DWORD dwWaitResult = WaitForSingleObject( 
		hmutex,    // handle to mutex
		INFINITE);  // no time-out interval

#if 0
	switch (dwWaitResult) 
	{
		// The thread got ownership of the mutex
	case WAIT_OBJECT_0: 
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
		break; 

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
	case WAIT_ABANDONED: 
		return false; 
	}
#endif

	return true;
}

void MutexRelease(HANDLE hmutex)
{
	ReleaseMutex(hmutex);
}

void ThreadPhase()
{
	for(int i=0; i<WORKTHREADS; i++)
	{
		WorkThread* wt = &g_workthread[i];
		MutexWait(wt->mutex);
		wt->startsignal = true;
		MutexRelease(wt->mutex);
	}

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
}