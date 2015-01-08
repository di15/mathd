#include "crpipe.h"
#include "../render/heightmap.h"
#include "../render/shader.h"
#include "../phys/collision.h"
#include "../render/model.h"
#include "../render/vertexarray.h"
#include "player.h"
#include "building.h"
#include "road.h"
#include "../math/hmapmath.h"
#include "bltype.h"
#include "building.h"
#include "../../app/appmain.h"
#include "../../app/gui/appgui.h"

#if 0
unsigned char CrPipeTile::cdtype()
{
	return CONDUIT_CRPIPE;
}
#endif