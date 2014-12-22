

#ifndef SOUNDCH_H
#define SOUNDCH_H

#include "../platform.h"
#include "../utils.h"

#define SOUND_CHANNELS	16

class SoundCh
{
public:
	unsigned long long last;

	SoundCh(){ last = GetTickCount64(); }
};

extern SoundCh g_soundch[SOUND_CHANNELS];

int NewChan();

#endif
