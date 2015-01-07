#ifndef SPRITE_H
#define SPRITE_H

#include "../texture.h"

class Sprite
{
public:

	Sprite();
	~Sprite();

	unsigned int texindex;
	float offset[4];
	LoadedTex* pixels;
};


void ParseSprite(const char* relative, Sprite* s);
void DefS(const char* relative, Sprite* s, int offx, int offy);
bool PlayAnimation(float& frame, int first, int last, bool loop, float rate);
bool PlayAnimationB(float& frame, int first, int last, bool loop, float rate);	//Play animation backwards

#endif
