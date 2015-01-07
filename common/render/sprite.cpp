#include "sprite.h"
#include "../utils.h"
#include "../texture.h"

Sprite::Sprite()
{
	texindex = 0;
	pixels = NULL;
}

Sprite::~Sprite()
{
    if(pixels)
    {
        delete pixels;
        pixels = NULL;
    }
}

void ParseSprite(const char* relative, Sprite* s)
{
	char fullpath[MAX_PATH+1];
	FullPath(relative, fullpath);

	FILE* fp = fopen(fullpath, "r");
	if(!fp) return;

	float centerx;
	float centery;
	float width;
	float height;

	fscanf(fp, "%f %f %f %f", &centerx, &centery, &width, &height);
	s->offset[0] = -centerx;
	s->offset[1] = -centery;
	s->offset[2] = s->offset[0] + width;
	s->offset[3] = s->offset[1] + height;

	fclose(fp);
}

void DefS(const char* relative, Sprite* s, int offx, int offy)
{
	CreateTexture(s->texindex, relative, true, false);
	s->offset[0] = offx;
	s->offset[1] = offy;

	char full[1024];
	FullPath(relative, full);
    s->pixels = LoadTexture(full);
}

bool PlayAnimation(float& frame, int first, int last, bool loop, float rate)
{
    if(frame < first || frame > last+1)
    {
        frame = first;
        return false;
    }

    frame += rate;

    if(frame > last)
    {
        if(loop)
            frame = first;
		else
			frame = last;

        return true;
    }

    return false;
}

//Play animation backwards
bool PlayAnimationB(float& frame, int first, int last, bool loop, float rate)
{
    if(frame < first-1 || frame > last)
    {
        frame = last;
        return false;
    }

    frame -= rate;

    if(frame < first)
    {
        if(loop)
            frame = last;
		else
			frame = first;

        return true;
    }

    return false;
}
