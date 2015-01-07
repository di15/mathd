#ifndef SOUND_H
#define SOUND_H

#include "../platform.h"

class Sound
{
public:
	Sound() 
	{ 
		on = false;
		sample = NULL;
	}
	Sound(const char* fp);
	~Sound();

	bool on;
	Mix_Chunk *sample;
	char filepath[MAX_PATH+1];
	void play();
};

#define SOUNDS	256

extern Sound g_sound[SOUNDS];

class SoundLoad
{
	char fullpath[MAX_PATH+1];
	int* retindex;
};

//TO DO queue sound

void SoundPath(const char* from, char* to);
bool QueueSound(const char* relative, short* index);
bool LoadSound(const char* relative, short* index);
void FreeSounds();
void ReloadSounds();
void PlaySound(short si);

void Sound_Order();

#endif
