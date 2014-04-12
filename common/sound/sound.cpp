

#include "sound.h"
#include "../utils.h"

vector<Sound> g_ordersnd;

void SoundPath(const char* from, char* to)
{
	char intermed[64];
	StripPathExtension(from, intermed);
	sprintf(to, "%s.wav", intermed);
}

Sound::Sound(const char* fp)
{
	FullPath(fp, filepath);
	//strcpy(filepath, fp);
}

void Sound::play()
{
	PlaySound(filepath, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
}

void Sound_Order()
{
	g_ordersnd[ rand()%g_ordersnd.size() ].play();
}