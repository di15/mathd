


#include "trystep.h"
#include "pathcell.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sys/workthread.h"
#include "../sim/unit.h"
#include "../sim/unittype.h"
#include "../sim/building.h"
#include "../sim/buildingtype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../sim/road.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/sim.h"
#include "../phys/trace.h"
#include "binheap.h"
#include "fullpath.h"

static vector<Vec3f> gridvecs;
Unit* g_pathunit = NULL;

void DrawGrid()
{
	glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glBegin(GL_LINES);
	
	glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &gridvecs[0]);
	glDrawArrays(GL_LINES, 0, gridvecs.size());

#if 1
	glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR],  0.5f, 0, 0, 1);
    
	if(g_selection.units.size() > 0)
	{
		int i = *g_selection.units.begin();
		Unit* u = &g_unit[i];
        
		bool roadVeh = false;
#if 0
		if(u->type == UNIT_TRUCK)
			roadVeh = true;
#endif
        
		UnitT* t = &g_unitT[u->type];
		
		for(int x=0; x<g_pathdim.x; x++)
			for(int z=0; z<g_pathdim.y; z++)
			{
				ColliderCell* cell = ColliderCellAt(x, z);
				if(cell->units.size() <= 0)
					continue;

				bool foundother = false;

				for(auto uiter = cell->units.begin(); uiter != cell->units.end(); uiter++)
				{
					if(&g_unit[*uiter] != u)
					{
						foundother = true;
						break;
					}
				}

				if(!foundother)
					continue;
                
				vector<Vec3f> vecs;
				vecs.push_back(Vec3f(x*PATHCELL_SIZE, 1, z*PATHCELL_SIZE));
				vecs.push_back(Vec3f((x+1)*PATHCELL_SIZE, 1, (z+1)*PATHCELL_SIZE));
				vecs.push_back(Vec3f((x+1)*PATHCELL_SIZE, 1, z*PATHCELL_SIZE));
				vecs.push_back(Vec3f(x*PATHCELL_SIZE, 1, (z+1)*PATHCELL_SIZE));
				
				vecs[0].y = g_hmap.accheight(vecs[0].x, vecs[0].z) + TILE_SIZE/100;
				vecs[1].y = g_hmap.accheight(vecs[1].x, vecs[1].z) + TILE_SIZE/100;
				vecs[2].y = g_hmap.accheight(vecs[2].x, vecs[2].z) + TILE_SIZE/100;
				vecs[3].y = g_hmap.accheight(vecs[3].x, vecs[3].z) + TILE_SIZE/100;

				glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
				glDrawArrays(GL_LINES, 0, vecs.size());
			}
	}
#endif

	if(gridvecs.size() > 0)
		return;
    
	for(int x=0; x<g_pathdim.x-1; x++)
	{
		for(int z=0; z<g_pathdim.y-1; z++)
		{
			int i = gridvecs.size();

			gridvecs.push_back(Vec3f(x*PATHCELL_SIZE, 0 + 1, z*PATHCELL_SIZE));
			gridvecs.push_back(Vec3f(x*PATHCELL_SIZE, 0 + 1, (z+1)*PATHCELL_SIZE));
			gridvecs[i+0].y = g_hmap.accheight(gridvecs[i+0].x, gridvecs[i+0].z) + TILE_SIZE/100;
			gridvecs[i+1].y = g_hmap.accheight(gridvecs[i+1].x, gridvecs[i+1].z) + TILE_SIZE/100;
			//glVertex3f(x*(MIN_RADIUS*2.0f), 0 + 1, 0);
			//glVertex3f(x*(MIN_RADIUS*2.0f), 0 + 1, g_map.m_widthZ*TILE_SIZE);
		}
	}
    
	for(int z=0; z<g_pathdim.y-1; z++)
	{
		for(int x=0; x<g_pathdim.x-1; x++)
		{
			int i = gridvecs.size();
			gridvecs.push_back(Vec3f(x*PATHCELL_SIZE, 0 + 1, z*PATHCELL_SIZE));
			gridvecs.push_back(Vec3f((x+1)*PATHCELL_SIZE, 0 + 1, z*PATHCELL_SIZE));
			gridvecs[i+0].y = g_hmap.accheight(gridvecs[i+0].x, gridvecs[i+0].z) + TILE_SIZE/100;
			gridvecs[i+1].y = g_hmap.accheight(gridvecs[i+1].x, gridvecs[i+1].z) + TILE_SIZE/100;
			//glVertex3f(0, 0 + 1, z*(MIN_RADIUS*2.0f));
			//glVertex3f(g_map.m_widthX*TILE_SIZE, 0 + 1, z*(MIN_RADIUS*2.0f));
		}
	}
  
	//glEnd();
	//glColor4f(1, 1, 1, 1);
}

void DrawUnitSquares()
{
	Unit* u;
	UnitT* t;
	Vec3f p;
    
	glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0.5f, 0, 0, 1);
    
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];
        
		if(!u->on)
			continue;
        
		t = &g_unitT[u->type];
		p = Vec3f(u->cmpos.x, u->drawpos.y + TILE_SIZE/100, u->cmpos.y);
        
#if 1
		if(u->collided)
			glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 1.0f, 0, 0, 1);
		else
			glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0.2f, 0, 0, 1);
#endif

		/*
         glVertex3f(p.x - r, 0 + 1, p.z - r);
         glVertex3f(p.x - r, 0 + 1, p.z + r);
         glVertex3f(p.x + r, 0 + 1, p.z + r);
         glVertex3f(p.x + r, 0 + 1, p.z - r);
         glVertex3f(p.x - r, 0 + 1, p.z - r);
         */
        
		Vec3i vmin(u->cmpos.x - t->size.x/2, 0, u->cmpos.y - t->size.z/2);

		vector<Vec3f> vecs;
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z));
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z + t->size.z));
		vecs.push_back(Vec3f(vmin.x + t->size.x, 0 + 1, vmin.z + t->size.z));
		vecs.push_back(Vec3f(vmin.x + t->size.x, 0 + 1, vmin.z));
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z));
		
		vecs[0].y = g_hmap.accheight(vecs[0].x, vecs[0].z) + TILE_SIZE/100;
		vecs[1].y = g_hmap.accheight(vecs[1].x, vecs[1].z) + TILE_SIZE/100;
		vecs[2].y = g_hmap.accheight(vecs[2].x, vecs[2].z) + TILE_SIZE/100;
		vecs[3].y = g_hmap.accheight(vecs[3].x, vecs[3].z) + TILE_SIZE/100;
		vecs[4].y = g_hmap.accheight(vecs[4].x, vecs[4].z) + TILE_SIZE/100;
        
		glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
		glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
	}
    
}

void DrawPaths()
{   
#if 1
	int i = 0;

	if(g_selection.units.size() <= 0)
		return;

	i = *g_selection.units.begin();
#else
	for(int i=0; i<UNITS; i++)
#endif
	{
		glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0, 1, 0, 1);
        
		Unit* u = &g_unit[i];

#if 0
		if(!u->on)
			continue;
#endif

		vector<Vec3f> vecs;

		Vec3f p;
		p.x = u->cmpos.x;
		p.z = u->cmpos.y;
		p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/100;
        
		for(auto piter = u->path.begin(); piter != u->path.end(); piter++)
		{
			p.x = piter->x;
			p.z = piter->y;
			p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/100;
			//glVertex3f(p->x, p->y + 5, p->z);
			vecs.push_back(p);
			//vecs.push_back(p+Vec3f(0,10,0));
		}
		
#if 0
		p.x = u->goal.x;
		p.z = u->goal.y;
		p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/100;
#endif
        
		if(vecs.size() > 1)
		{
            glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
            glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
		else
		{
			//vecs.push_back(u->camera.Position() + Vec3f(0,5,0));
			//vecs.push_back(u->goal + Vec3f(0,5,0));
            
			glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0.8f, 1, 0.8f, 1);
            
			glVertexAttribPointer(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
			glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
	}
}

#if 0
void DrawVelocities()
{
	CUnit* u;
	Vec3f p;
    
	glUniform4f(g_shader[SHADER::COLOR3D].m_slot[SLOT::COLOR], 1, 0, 1, 1);
    
	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];
        
		if(!u->on)
			continue;
        
		vector<Vec3f> vecs;
        
        vecs.push_back(u->camera.Position() + Vec3f(0, 1, 0));
        vecs.push_back(u->camera.Position() + u->camera.Velocity()*100.0f + Vec3f(0, 1, 0));
        
		if(vecs.size() > 0)
		{
            glVertexAttribPointer(g_shader[SHADER::COLOR3D].m_slot[SLOT::POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
            glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
	}
}

#endif


void LogPathDebug()
{
	if(g_pathunit)
	{
		g_pathunit = NULL;
		return;
	}

	if(g_selection.units.size() <= 0)
		return;

	int i = *g_selection.units.begin();

	g_pathunit = &g_unit[i];
}