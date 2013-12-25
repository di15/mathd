

#include "tileset.h"
#include "gui.h"
#include "menu.h"
#include "map.h"
#include "main.h"
#include "editor.h"

int g_selTS = -1;

void Retile()
{
	if(g_selTS < 0)
		return;

	int minx, maxx, minz, maxz;
	MinMaxCorners(minx, maxx, minz, maxz);

	int z;
	for(int x=minx; x<=maxx; x++)
		for(z=minz; z<=maxz; z++)
		{
			g_hmap.settile(x, z, g_selTS);
		}

	g_hmap.retexture();
}

void Click_Tileset(int param)
{
	g_edTool = EDTOOL::PLACETILE;
	g_selTS = param;
}

void Over_Tileset(int param)
{
}

void Click_TSNext()
{
	NextPage("tileset buttons");
}