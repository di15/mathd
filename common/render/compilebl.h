

#include "../math/vec2i.h"
#include "../math/vec2f.h"
#include "../platform.h"
#include "../texture.h"

#if 0
#define BUILDINGM_VERSION		1.0f

#define TAG_BUILDINGM		{'D', 'M', 'D', 'B', 'M'}
#endif

#define TEX_PER_BL		4

class TexFitInfo
{
public:
	Vec2i tiletimes;
	Vec2f newdim;
	Vec2i bounds[2];	//duplicate of TexFitRow member, needed for accessing directly without iterating rows

	TexFitInfo();
};

class TexFit
{
public:
	unsigned int texindex;
	Vec2i bounds[2];
};

class TexFitRow
{
public:
	list<TexFit> fits;
	Vec2i bounds[2];

	TexFitRow();
};


#if 0
class EdBuilding;

void CompileBuilding(const char* fullfile, EdBuilding* bldg);
#endif

void Resample(LoadedTex* original, LoadedTex* empty, Vec2i newdim);