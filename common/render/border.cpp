

#include "border.h"
#include "heightmap.h"
#include "../sim/country.h"
#include "water.h"
#include "shader.h"
#include "../window.h"

Borders g_borders;

Borders::Borders()
{
	drawtris = NULL;
	colorcodes = NULL;
}

Borders::~Borders()
{
	destroy();
}

void Borders::destroy()
{
	if(drawtris)
	{
		delete [] drawtris;
		drawtris = NULL;
	}
	
	if(colorcodes)
	{
		delete [] colorcodes;
		colorcodes = NULL;
	}
}

void Borders::alloc(int widthx, int widthz)
{
	destroy();

	drawtris = new Vec3f [ widthx * widthz * 6 ];
	colorcodes = new float [ widthx * widthz * 6 * 4 ];

	for(int x=0; x<widthx; x++)
		for(int z=0; z<widthz; z++)
		{
			float *colorcode = &colorcodes[ ( widthx*z + x ) * 6 * 4 ];
			colorcode[0] = 0;
			colorcode[1] = 0;
			colorcode[2] = 0;
			colorcode[3] = 0;
			Vec3f *drawtri = &drawtris[ ( widthx*z + x ) * 6 ];
			drawtri[0] = Vec3f(0,0,0);
			drawtri[1] = Vec3f(0,0,0);
			drawtri[2] = Vec3f(0,0,0);
			drawtri[3] = Vec3f(0,0,0);
			drawtri[4] = Vec3f(0,0,0);
			drawtri[5] = Vec3f(0,0,0);
		}
}

void DefineBorders(int minx, int minz, int maxx, int maxz)
{
	int widthx = g_hmap.m_widthx;
	int widthz = g_hmap.m_widthz;

	for(int x=minx; x<=maxx; x++)
		for(int z=minz; z<=maxz; z++)
		{
			float *colorcode = &g_borders.colorcodes[ ( widthx*z + x ) * 6 * 4 ];
			Vec3f *drawtri = &g_borders.drawtris[ ( widthx*z + x ) * 6 ];
			int bcountry = g_hmap.getcountry(x, z);

			bool bordersothercountry = false;
			
			if(x-1 >= 0 && 
				z-1 >= 0 && 
				g_hmap.getcountry(x-1, z-1) != bcountry)
				bordersothercountry = true;
			
			if(z-1 >= 0 && 
				g_hmap.getcountry(x, z-1) != bcountry)
				bordersothercountry = true;
			
			if(x+1 < widthx-1 && 
				z-1 >= 0 && 
				g_hmap.getcountry(x+1, z-1) != bcountry)
				bordersothercountry = true;
			
			if(x+1 < widthx-1 && 
				g_hmap.getcountry(x+1, z) != bcountry)
				bordersothercountry = true;
			
			if(x+1 < widthx-1 && 
				z+1 < widthz-1 && 
				g_hmap.getcountry(x+1, z+1) != bcountry)
				bordersothercountry = true;
			
			if(z+1 < widthz-1 &&
				g_hmap.getcountry(x, z+1) != bcountry)
				bordersothercountry = true;
			
			if(x-1 >= 0 && 
				z+1 < widthz-1 && 
				g_hmap.getcountry(x-1, z+1) != bcountry)
				bordersothercountry = true;

			if(x-1 >= 0 && 
				g_hmap.getcountry(x-1, z) != bcountry)
				bordersothercountry = true;

			if(bordersothercountry)
			{
				for(int i=0; i<6*4; i+=4)
				{
					colorcode[i + 0] = 1;
					colorcode[i + 1] = 1;
					colorcode[i + 2] = 1;
					colorcode[i + 3] = 1;
				}

				if(bcountry >= 0)
				{
					Country* c = &g_country[bcountry];
					for(int i=0; i<6*4; i+=4)
					{
						colorcode[i + 0] = c->colorcode[0];
						colorcode[i + 1] = c->colorcode[1];
						colorcode[i + 2] = c->colorcode[2];
						colorcode[i + 3] = c->colorcode[3];
					}
				}
				
				Vec3f* tileverts = g_hmap.getcolltileverts(x, z);

				for(int i=0; i<6; i++)
				{
					drawtri[i] = tileverts[i] + Vec3f(0,TILE_SIZE/100,0);

					if(tileverts[i].y <= WATER_LEVEL)
						drawtri[i].y = WATER_LEVEL + TILE_SIZE/10;
				}
			}
			else
			{
				colorcode[0] = 0;
				colorcode[1] = 0;
				colorcode[2] = 0;
				colorcode[3] = 0;

				for(int i=0; i<6; i++)
					drawtri[i] = Vec3f(0,0,0);
			}
		}
}

void DrawBorders()
{
	if(g_hmap.m_widthx <= 0 || g_hmap.m_widthz <= 0)
		return;

	Shader* s = &g_shader[g_curS];
	
	/*
	for(int x=0; x<m_widthx; x++)
		for(int z=0; z<m_widthz; z++)
		{
			glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &m_drawvertices[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &m_texcoords0[ (z * m_widthx + x) * 3 * 2 + 0 ]);
			glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &m_normals[ (z * m_widthx + x) * 3 * 2 + 0 ]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		*/
	
	glUniform1f(s->m_slot[SSLOT_MIND], MIN_DISTANCE);
	glUniform1f(s->m_slot[SSLOT_MAXD], MAX_DISTANCE / g_zoom);

#if 0
	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(-2.0, -500.0);
	//glPolygonOffset(-10.0, -2500.0);
	glPolygonOffset(-0.0, -0.2);
#endif

	// Draw all tiles
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, g_borders.drawtris);
	glVertexAttribPointer(s->m_slot[SSLOT_VERTCOLORS], 4, GL_FLOAT, GL_FALSE, 0, g_borders.colorcodes);
	glDrawArrays(GL_TRIANGLES, 0, (g_hmap.m_widthx) * (g_hmap.m_widthz) * 3 * 2);
	
	glDisable(GL_POLYGON_OFFSET_FILL);
}

