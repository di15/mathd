

#ifndef SIM_H
#define SIM_H

#include "../render/heightmap.h"
#include "../platform.h"
#include "../texture.h"
#include "../render/model.h"
#include "../sim/resources.h"
#include "../gui/icon.h"
#include "../sim/buildingtype.h"
#include "../sim/unittype.h"
#include "../render/foliage.h"
#include "../sim/road.h"
#include "../sim/powl.h"
#include "../sim/crpipe.h"
#include "../render/water.h"
#include "../sim/selection.h"
#include "../sound/sound.h"
#include "../render/particle.h"
#include "../render/skybox.h"
#include "../sim/player.h"
#include "../gui/sprite.h"
#include "../gui/cursor.h"

extern long long g_simframe;

#define SIM_FRAME_RATE		30

void Queue();

#endif
