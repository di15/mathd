

#ifndef UPDTHREAD_H
#define UPDTHREAD_H

#include "../platform.h"

#define UPD_FRAME_RATE		30

extern HANDLE g_hupdthread;
extern double g_instantupdfps;
extern double g_updfrinterval;

DWORD WINAPI UpdMain(PVOID pparam);

#endif