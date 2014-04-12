


#include "../texture.h"
#include "foliage.h"
#include "water.h"
#include "shader.h"
#include "../math/camera.h"
#include "model.h"
#include "../math/frustum.h"
#include "vertexarray.h"
#include "heightmap.h"
#include "../utils.h"

FoliageT g_foliageT[FOLIAGE_TYPES];
Foliage g_foliage[FOLIAGES];
Matrix g_folmodmat[FOLIAGES];
int g_folonsw[FOLIAGES];
Forest *g_forest = NULL;
int g_forestmodel;

Foliage::Foliage()
{
	on = false;
}

void Foliage::reinstance()
{
	int i = this - g_foliage;
	g_folonsw[i] = on ? 1 : 0;
	Matrix* m = &g_folmodmat[i];

	if(on)
	{
		float pitch = 0;
		m->loadIdentity();
		float radians[] = {DEGTORAD(pitch), DEGTORAD(yaw), 0};
		m->setTranslation((const float*)&pos);
		Matrix rotation;
		rotation.setRotationRadians(radians);
		m->postMultiply(rotation);
	}
	else
	{
		memset(m->m_matrix, 0, sizeof(float)*16);
	}
}

void Forest::remesh()
{
	Model* m = &g_model[g_forestmodel];
    
	VertexArray* fva = &va;
	VertexArray* mva = &m->m_va[0];
    
	fva->free();
    
	//g_log<<"meshroad allocating "<<mva->numverts<<"...";
	//g_log.flush();
    
	fva->numverts = mva->numverts;
	fva->vertices = new Vec3f[ mva->numverts ];
	fva->texcoords = new Vec2f[ mva->numverts ];
	fva->normals = new Vec3f[ mva->numverts ];
    
	int fi = this - g_forest;
	int tz = fi / g_hmap.m_widthx;
	int tx = fi % g_hmap.m_widthx;

	float realx, realz;
	drawpos = Vec3f(tx+0.5f, 0, tz+0.5f) * TILE_SIZE;
    
	for(int i=0; i<fva->numverts; i++)
	{
		fva->vertices[i] = mva->vertices[i];
		fva->texcoords[i] = mva->texcoords[i];
		fva->normals[i] = mva->normals[i];
        
		//realx = ((float)x+0.5f)*(float)TILE_SIZE + rva->vertices[i].x;
		//realz = ((float)z+0.5f)*(float)TILE_SIZE + rva->vertices[i].z;
		realx = drawpos.x + mva->vertices[i].x;
		realz = drawpos.z + mva->vertices[i].z;
        
#if 0
		rva->vertices[i].y += Bilerp(&g_hmap, realx, realz);
#else
		fva->vertices[i].y += g_hmap.accheight2(realx, realz);
#endif
        
		//char msg[128];
		//sprintf(msg, "(%f,%f,%f)", mva->vertices[i].x, mva->vertices[i].y, mva->vertices[i].z);
		//Chat(msg);
	}
}

void DefineFoliage(int type, const char* modelrelative, Vec3f scale, Vec3f translate, Vec3i size)
{
	FoliageT* t = &g_foliageT[type];
	//QueueTexture(&t->texindex, texrelative, true);
	QueueModel(&t->model, modelrelative, scale, translate);
	t->size = size;
}

int NewFoliage()
{
	for(int i=0; i<FOLIAGES; i++)
	{
		if(!g_foliage[i].on)
			return i;
	}

	return -1;
}

#if 0
void PlaceFoliage()
{
	if(g_scT < 0)
		return;

	int i = NewFoliage();

	if(i < 0)
		return;

	Foliage* s = &g_foliage[i];
	s->on = true;
	s->pos = g_vMouse;
	s->type = g_scT;
	s->yaw = DEGTORAD((rand()%360));

	if(s->pos.y <= WATER_LEVEL)
		s->on = false;
}
#endif

#if 0
void DrawFoliage()
{
	Vec3f vertical = g_camera.up2();
	Vec3f horizontal = g_camera.m_strafe;
	Vec3f a, b, c, d;
	Vec3f vert, horiz;

	Shader* s = &g_shader[g_curS];
	
	glActiveTextureARB(GL_TEXTURE0);
	glUniform1iARB(s->m_slot[SSLOT_TEXTURE0], 0);

	//glDisable(GL_CULL_FACE);

	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		FoliageT* t = &g_foliageT[f->type];
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_texture[t->texindex].texname);
		glUniform1iARB(s->m_slot[SSLOT_TEXTURE0], 0);

		Vec3i* size = &t->size;

		vert = vertical*size->y;
		horiz = horizontal*(size->x/2);

		a = f->pos + horiz + vert;
		b = f->pos + horiz;
		c = f->pos - horiz;
		d = f->pos - horiz + vert;

		float vertices[] =
		{
			//posx, posy posz   texx, texy
			a.x, a.y, a.z,          1, 0,
			b.x, b.y, b.z,          1, 1,
			c.x, c.y, c.z,          0, 1,

			c.x, c.y, c.z,          0, 1,
			d.x, d.y, d.z,          0, 0,
			a.x, a.y, a.z,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	
	//glEnable(GL_CULL_FACE);
}
#endif

VertexArray g_forestva;

void DrawFoliage(Vec3f zoompos, Vec3f vertical, Vec3f horizontal)
{
#if 0
	FoliageT* t = &g_foliageT[FOLIAGE_TREE1];
	Model* m = &g_model[t->model];

	if(g_forestva.numverts <= 0)
	{
		VertexArray* tva = &m->m_va[0];
		g_forestva.alloc( FOLIAGES * tva->numverts );

		for(int i=0; i<FOLIAGES; i++)
		{
			Foliage* f = &g_foliage[i];
			
			Vec3f* fverts = &g_forestva.vertices[ i*tva->numverts ];
			Vec3f* fnorms = &g_forestva.normals[ i*tva->numverts ];
			Vec2f* ftexcoords = &g_forestva.texcoords[ i*tva->numverts ];

			if(!f->on)
			{
				for(int j=0; j<tva->numverts; j++)
					fverts[j] = Vec3f(0,0,0);

				continue;
			}
			
			for(int j=0; j<tva->numverts; j++)
			{
				fverts[j] = tva->vertices[j] + f->pos;
				fnorms[j] = tva->normals[j];
				ftexcoords[j] = tva->texcoords[j];
			}
		}
	}
	else
	{
		m->usetex();
		
		Shader* s = &g_shader[g_curS];
		Matrix im;

		glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, im.m_matrix);
		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, g_forestva.vertices);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, g_forestva.texcoords);
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, g_forestva.normals);

		glDrawArrays(GL_TRIANGLES, 0, g_forestva.numverts);
	}

	return;
#endif

#if 0
	Model* m = &g_model[g_forestmodel];
	m->usetex();

	for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthz; i++)
	{
		Forest* f = &g_forest[i];
    
		if(!f->on)
			continue;

#if 1
		DrawVA(&f->va, f->drawpos);
#endif
	}
#endif
	
	//Vec3f vertical = g_camera.up2();
	//Vec3f vertical = g_camera.m_up;
	//Vec3f horizontal = g_camera.m_strafe;
	//Vec3f zoompos = g_camera.zoompos();
	Vec3f a, b, c, d;
	Vec3f vert, horiz;

#if 1
	
	Shader* s = &g_shader[g_curS];
	
	glActiveTextureARB(GL_TEXTURE0);
	glUniform1iARB(s->m_slot[SSLOT_TEXTURE0], 0);

	FoliageT* t = &g_foliageT[FOLIAGE_TREE1];
	Vec3i* size = &t->size;
	Model* m = &g_model[t->model];	
	VertexArray* va = &m->m_va[0];
	m->usetex();
	Matrix im;

	Vec3f viewdir = Normalize(g_camera.m_view - zoompos);
	Vec3f horizontal2 = horizontal;

	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;
		
		Vec3f vmin(f->pos.x - t->size.x/2, f->pos.y, f->pos.z - t->size.x/2);
		Vec3f vmax(f->pos.x + t->size.x/2, f->pos.y + t->size.y, f->pos.z + t->size.x/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		if(Magnitude2( f->pos - zoompos ) > 8000*8000*2*2)
		//if(false)
		{
#if 0
			horizontal2 = horizontal;
			
			if(Magnitude2( f->pos - zoompos ) < 8000*8000*2*2)
			{
				viewdir = Normalize( f->pos - zoompos );
				//horizontal = Cross( vertical, viewdir );
				horizontal2 = Cross( viewdir, vertical );
			}
#endif

			vert = vertical*size->y;
			horiz = horizontal2*(size->x/2);

			a = f->pos + horiz + vert;
			b = f->pos + horiz;
			c = f->pos - horiz;
			d = f->pos - horiz + vert;

			float vertices[] =
			{
				//posx, posy posz   texx, texy		normx, normy, normz
				a.x, a.y, a.z,          1, 0,		horiz.x, horiz.y, horiz.z,
				b.x, b.y, b.z,          1, 1,		horiz.x, horiz.y, horiz.z,
				c.x, c.y, c.z,          0, 1,		horiz.x, horiz.y, horiz.z,

				c.x, c.y, c.z,          0, 1,		-horiz.x, -horiz.y, -horiz.z,
				d.x, d.y, d.z,          0, 0,		-horiz.x, -horiz.y, -horiz.z,
				a.x, a.y, a.z,          1, 0,		horiz.x, horiz.y, horiz.z
			};

			//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
			//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
			
			glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, im.m_matrix);
			glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, &vertices[0]);
			glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, &vertices[3]);
			glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, &vertices[5]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		else
		{
			glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, g_folmodmat[i].m_matrix);
			glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
		}
	}
	
#elif 1

	FoliageT* t = &g_foliageT[FOLIAGE_TREE1];
	Model* m = &g_model[t->model];

	m->usetex();
	VertexArray* va = &m->m_va[0];
	Shader* s = &g_shader[g_curS];
    
    glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
    glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
    glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
	
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMATS], FOLIAGES, false, (const float*)g_folmodmat);
	glUniform1iv(s->m_slot[SSLOT_ONSWITCHES], FOLIAGES, g_folonsw);
	glDrawArraysInstanced(GL_TRIANGLES, 0,  va->numverts, FOLIAGES);
#endif
}

bool PlaceFoliage(int type, Vec3i ipos)
{
	int i = NewFoliage();

	if(i < 0)
		return false;

	Foliage* f = &g_foliage[i];
	f->on = true;
	f->type = type;
	f->pos = Vec3f(ipos.x, ipos.y, ipos.z);
	f->reinstance();

	return true;
}

void ClearFoliage(int minx, int minz, int maxx, int maxz)
{
	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		if(f->pos.x >= minx && f->pos.x <= maxx && f->pos.z >= minz && f->pos.z <= maxz)
		{
			f->on = false;
			f->reinstance();
		}
	}
}

void FillForest()
{
#if 0
	for(int tx = 0; tx < g_hmap.m_widthx; tx++)
		for(int tz = 0; tz < g_hmap.m_widthz; tz++)
		{
				int x = tz*TILE_SIZE + TILE_SIZE/2;
				int z = tz*TILE_SIZE + TILE_SIZE/2;

				Vec3f norm = g_hmap.getnormal(tx, tz);

				float y = g_hmap.accheight2(x, z);

				if(y >= ELEV_SANDONLYMAXY && y <= ELEV_GRASSONLYMAXY && 1.0f - norm.y <= 0.3f)
				{
					Forest* f = ForestAt(tx, tz);
					f->on = true;
					f->remesh();
				}
		}
#endif
	//for(int condensation = 0; condensation < sqrt(g_hmap.m_widthx * g_hmap.m_widthz); condensation++)
	{

		int maxfoliage = FOLIAGES*g_hmap.m_widthx*g_hmap.m_widthz/MAX_MAP/MAX_MAP;
		maxfoliage = min(FOLIAGES, maxfoliage);
		float mindesert = g_hmap.m_widthz*TILE_SIZE/2.0f - g_hmap.m_widthz*TILE_SIZE*0.2f;
		float maxdesert = g_hmap.m_widthz*TILE_SIZE/2.0f + g_hmap.m_widthz*TILE_SIZE*0.2f;
		float desertrange = g_hmap.m_widthz*TILE_SIZE*0.3f;

		for(int i=0; i<maxfoliage; i++)
		{
			//break;
			for(int j=0; j<30; j++)
			{
				int x = (rand()%g_hmap.m_widthx)*TILE_SIZE + rand()%TILE_SIZE;
				int z = (rand()%g_hmap.m_widthz)*TILE_SIZE + rand()%TILE_SIZE;

				int tx = x / TILE_SIZE;
				int tz = z / TILE_SIZE;

				if(ForestAt(tx, tz)->on)
					continue;
			
				if(NTree(tx, tz) > 2)
					continue;

				float y = g_hmap.accheight2(x, z);

				Vec3f norm = g_hmap.getnormal(x/TILE_SIZE, z/TILE_SIZE);

				float offequator = fabs( (float)g_hmap.m_widthz*TILE_SIZE/2.0f - z );

				if(y >= ELEV_SANDONLYMAXY && y <= ELEV_GRASSONLYMAXY && (rand()%((int)desertrange) < offequator) && 1.0f - norm.y <= 0.3f)
				{
					//int type = rand()%10 == 1 ? UNIT_MECH : UNIT_LABOURER;
	#if 0
					int type = UNIT_LABOURER;

					if(rand()%10 == 1)
						type = UNIT_MECH;
					if(rand()%10 == 1)
						type = UNIT_GEPARDAA;

					PlaceUnit(type, Vec3i(x, y, z), -1, -1);
	#endif
	#if 1

					int type = FOLIAGE_TREE1;

					PlaceFoliage(type, Vec3i(x, y, z));
	#endif
					break;
				}
			}
		}

		//CondenseForest(0, 0, g_hmap.m_widthx-1, g_hmap.m_widthz-1);
	}
}

int NTree(int tx, int tz)
{
	int minx = tx * TILE_SIZE;
	int minz = tz * TILE_SIZE;
	int maxx = minx + TILE_SIZE;
	int maxz = minz + TILE_SIZE;

	int cnt = 0;

	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		if(f->pos.x >= minx && f->pos.x <= maxx && f->pos.z >= minz && f->pos.z <= maxz)
			cnt++;
	}

	return cnt;
}

Forest* ForestAt(int tx, int tz)
{
	return &g_forest[ tz*g_hmap.m_widthx + tx ];
}

void CondenseForest(int tminx, int tminz, int tmaxx, int tmaxz)
{
	for(int tx = tminx; tx <= tmaxx; tx++)
	{
		for(int tz = tminz; tz <= tmaxz; tz++)
		{
			bool northhas = true;
			bool southhas = true;
			bool westhas = true;
			bool easthas = true;

			if(tx > 0 && NTree(tx-1, tz) <= 0)
				westhas = false;

			if(tx < g_hmap.m_widthx-1 && NTree(tx+1, tz) <= 0)
				easthas = false;

			if(tz > 0 && NTree(tx, tz-1) <= 0)
				northhas = false;

			if(tz < g_hmap.m_widthz-1 && NTree(tx, tz+1) <= 0)
				southhas = false;
				
			Forest* f = ForestAt(tx, tz);

			if(northhas && southhas && westhas && easthas)
			//if(true)
			{
				f->on = true;
				f->remesh();
				ClearFoliage( tx * TILE_SIZE, tz * TILE_SIZE, (tx+1) * TILE_SIZE, (tz+1) * TILE_SIZE );
			}
			else
			{
				f->on = false;
			}
		}
	}
}

void FreeFoliage()
{
	for(int i=0; i<FOLIAGES; i++)
	{
		g_foliage[i].on = false;
		g_foliage[i].reinstance();
	}
}