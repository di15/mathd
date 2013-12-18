
#include "../platform.h"

#ifdef _IOS

extern CFHostRef g_cfHost;
extern NSData* g_hostAddr;
extern CFSocketRef g_cfSocket;

void InitNetIOS();

#endif