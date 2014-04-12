

#include "../render/heightmap.h"
#include "../texture.h"
#include "../utils.h"
#include "../math/vec3f.h"
#include "../window.h"
#include "../math/camera.h"
#include "../render/border.h"
#include "../sim/unit.h"
#include "../sim/unittype.h"
#include "../math/hmapmath.h"
#include "../render/foliage.h"
#include "../sim/unit.h"
#include "../render/foliage.h"
#include "savemap.h"
#include "../sim/building.h"
#include "../sim/road.h"
#include "../sim/powerline.h"
#include "../sim/crudepipeline.h"
#include "../sim/deposit.h"
#include "../render/water.h"
#include "../sim/country.h"
#include "../sim/selection.h"
#include "../ai/collidertile.h"
#include "../sys/workthread.h"
#include "../ai/pathjob.h"
#include "../ai/pathnode.h"

float ConvertHeight(unsigned char brightness)
{
	// Apply exponential scale to height data.
	float y = (float)brightness*TILE_Y_SCALE/255.0f - TILE_Y_SCALE/2.0f;
	y = y / fabs(y) * pow(fabs(y), TILE_Y_POWER) * TILE_Y_AFTERPOW;

	if(y <= WATER_LEVEL)
		y -= TILE_SIZE;
	else if(y > WATER_LEVEL && y < WATER_LEVEL + TILE_SIZE/100)
		y += TILE_SIZE/100;

	return y;
}

void PlaceUnits()
{
	UnitT* ut = &g_unitT[UNIT_LABOURER];
	PathJob pj;
	pj.airborne = ut->airborne;
	pj.landborne = ut->landborne;
	pj.seaborne = ut->seaborne;
	pj.roaded = ut->roaded;
	pj.ignoreb = NULL;
	pj.ignoreu = NULL;
	pj.thisu = NULL;
	pj.utype = UNIT_LABOURER;

	int lcnt[COUNTRIES];

	for(int ci=0; ci<COUNTRIES; ci++)
	{
		lcnt[ci] = 0;
	}

	for(int li=0; li<6*COUNTRIES; li++)
	{
		int ntries = 0;

		for(; ntries < 100; ntries++)
		{
			Vec2i cmpos(rand()%g_hmap.m_widthx*TILE_SIZE, rand()%g_hmap.m_widthz*TILE_SIZE);
			Vec2i tpos = cmpos / TILE_SIZE;
			Vec2i npos = cmpos / PATHNODE_SIZE;

			int ci = g_hmap.getcountry(tpos.x, tpos.y);

			if(lcnt[ci] >= 6)
			{
				continue;
			}

			pj.cmstartx = cmpos.x;
			pj.cmstartz = cmpos.y;

			if(Walkable2(&pj, cmpos.x, cmpos.y))
			{
				lcnt[ci] ++;
				PlaceUnit(UNIT_LABOURER, Vec3i(cmpos.x, g_hmap.accheight(cmpos.x, cmpos.y), cmpos.y), ci, -1, -1);
				//g_log<<"placed "<<li<<" at"<<tpos.x<<","<<tpos.y<<endl;
				break;
			}
			//else
				//g_log<<"not placed at"<<tpos.x<<","<<tpos.y<<endl;
		}
	}
}

void LoadJPGMap(const char* relative)
{
	g_hmap.destroy();
	g_hmap2.destroy();
	g_hmap4.destroy();
	g_hmap8.destroy();

	LoadedTex *pImage = NULL;

	char full[1024];
	FullPath(relative, full);

	pImage = LoadJPG(full);

	if(!pImage)
		return;
	
	g_hmap.allocate((pImage->sizeX-1), (pImage->sizeY-1));
#if 0
	g_hmap2.allocate((pImage->sizeX-1)/2, (pImage->sizeY-1)/2);
	g_hmap4.allocate((pImage->sizeX-1)/4, (pImage->sizeY-1)/4);
	g_hmap8.allocate((pImage->sizeX-1)/8, (pImage->sizeY-1)/8);
#endif

	for(int x=0; x<pImage->sizeX; x++)
	{
		for(int z=0; z<pImage->sizeY; z++)
		{
			float r = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 0 ];
			float g = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 1 ];
			float b = (unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 2 ];

			// Apply exponential scale to height data.
			float y = ConvertHeight((r+g+b)/3.0f);

			g_hmap.setheight(x, z, y);

#if 0
			if(x%2 == 0 && z%2 == 0)
				g_hmap2.setheight(x/2, z/2, y);

			if(x%4 == 0 && z%4 == 0)
				g_hmap4.setheight(x/4, z/4, y);

			if(x%8 == 0 && z%8 == 0)
				g_hmap8.setheight(x/8, z/8, y);
#endif
		}
	}
	
	g_hmap.remesh(1);
#if 0
	g_hmap2.remesh(2);
	g_hmap4.remesh(4);
	g_hmap8.remesh(8);
#endif
	MutexWait(g_drawmutex);
	g_hmap.prerender();
	MutexRelease(g_drawmutex);

#if 1
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			g_hmap.getcountry(x, z) = 0;

			if(z < g_hmap.m_widthz/2)
			{
				g_hmap.getcountry(x, z) = 0;
			}
			else
			{
				g_hmap.getcountry(x, z) += COUNTRIES/2;
			}

			if(x >= g_hmap.m_widthx*0/4 && x < g_hmap.m_widthx*1/4)
			{
				g_hmap.getcountry(x, z) += 0;
			} 
			else if(x >= g_hmap.m_widthx*1/4 && x < g_hmap.m_widthx*2/4)
			{
				g_hmap.getcountry(x, z) += 1;
			}
			else if(x >= g_hmap.m_widthx*2/4 && x < g_hmap.m_widthx*3/4)
			{
				g_hmap.getcountry(x, z) += 2;
			}
			else if(x >= g_hmap.m_widthx*3/4 && x < g_hmap.m_widthx*4/4)
			{
				g_hmap.getcountry(x, z) += 3;
			}
		}
		
	g_borders.alloc(g_hmap.m_widthx, g_hmap.m_widthz);
	AllocPathGrid(g_hmap.m_widthx*TILE_SIZE, g_hmap.m_widthz*TILE_SIZE);
	AllocGrid((pImage->sizeX-1), (pImage->sizeY-1));
	FillColliderGrid();
	DefineBorders(0, 0, g_hmap.m_widthx-1, g_hmap.m_widthz-1);
#endif

	FreeDeposits();
	
	for(int i=0; i<min(DEPOSITS, g_hmap.m_widthx*g_hmap.m_widthz/50); i++)
	//for(int i=0; i<DEPOSITS; i++)
	{
		Deposit* d = &g_deposit[i];

		d->on = true;
		d->occupied = false;

		for(int j=0; j<1000; j++)
		{
			d->restype = -1;

			int x = (rand()%g_hmap.m_widthx);
			int z = (rand()%g_hmap.m_widthz);

			d->tilepos.x = x;
			d->tilepos.y = z;
			
			d->drawpos.x = x * TILE_SIZE + TILE_SIZE/2;
			d->drawpos.z = z * TILE_SIZE + TILE_SIZE/2;
#if 0
			float y = Bilerp(&g_hmap, d->drawpos.x, d->drawpos.z);
#else
			float y = g_hmap.accheight(d->drawpos.x, d->drawpos.z);
#endif
			d->drawpos.y = y;

			Vec3f norm = g_hmap.getnormal(x, z);

			if(y < WATER_LEVEL)
			{
				y = WATER_LEVEL;
				d->drawpos.y = y;
				
				// place deposits
				int r = rand()%13;

				if(r >= 0 && r <= 12)
				{
					d->restype = RES_CRUDEOIL;
					d->amount = 1000;
					break;
				}
			}
			else if(y >= ELEV_SANDONLYMAXY && y <= ELEV_GRASSONLYMAXY && 1.0f - norm.y <= 0.3f)
			{
				// place deposits

				int r = rand()%17;

				if(r >= 0 && r <= 3)
				{
					d->restype = RES_CRUDEOIL;
					d->amount = 1000;
					break;
				}
				else if(r >= 3 && r <= 6)
				{
					d->restype = RES_STONE;
					d->amount = 100;
					break;
				}
				else if(r >= 6 && r <= 9)
				{
					d->restype = RES_URANIUMORE;
					d->amount = 500;
					break;
				}
				else if(r >= 9 && r <= 12)
				{
					d->restype = RES_COAL;
					d->amount = 1000;
					break;
				}
				else if(r >= 13 && r<= 15)
				{
					d->restype = RES_IRONORE;
					d->amount = 1000;
					break;
				}
			}
#if 0
			else
			{
				g_log<<"xz= "<<x<<","<<z<<", y = "<<y<<"(minmax="<<ELEV_SANDONLYMAXY<<","<<ELEV_GRASSONLYMAXY<<") norm.y="<<norm.y<<" (1-norm.y="<<(1.0f - norm.y)<<")"<<endl;
				g_log.flush();
			}
#endif
		}

		if(d->restype < 0)
			d->on = false;
	}

	FillForest();
	PlaceUnits();

	Vec3f center = Vec3f( g_hmap.m_widthx * TILE_SIZE/2.0f, g_hmap.getheight(g_hmap.m_widthx/2, g_hmap.m_widthz/2), g_hmap.m_widthz * TILE_SIZE/2.0f );
	Vec3f delta = center - g_camera.m_view;
	g_camera.move(delta);
	Vec3f viewvec = g_camera.m_view - g_camera.m_pos;
	viewvec = Normalize(viewvec) * max(g_hmap.m_widthx, g_hmap.m_widthz) * TILE_SIZE;
	g_camera.m_pos = g_camera.m_view - viewvec;
	g_zoom = INI_ZOOM;

	if(pImage)
	{
		if (pImage->data)							// If there is texture data
		{
			free(pImage->data);						// Free the texture data, we don't need it anymore
		}

		free(pImage);								// Free the image structure

		g_log<<relative<<"\n\r";
		g_log.flush();
	}
	
	g_camera.position(
		-1000.0f/3, 1000.0f/3 + 5000, -1000.0f/3, 
		0, 5000, 0, 
		0, 1, 0);
	
	g_camera.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	g_camera.move( Vec3f(g_hmap.m_widthx*TILE_SIZE/2, 1000, g_hmap.m_widthz*TILE_SIZE/2) );

	CalcMapView();
}

void FreeMap()
{
	g_hmap.destroy();
	FreeGrid();
	FreeUnits();
	FreeFoliage();
	FreeBuildings();
	FreeDeposits();
	g_selection.clear();
}

void SaveHeightmap(FILE *fp)
{
	fwrite(&g_hmap.m_widthx, sizeof(int), 1, fp);
	fwrite(&g_hmap.m_widthz, sizeof(int), 1, fp);
	
	fwrite(g_hmap.m_heightpoints, sizeof(float), (g_hmap.m_widthx+1)*(g_hmap.m_widthz+1), fp);
	fwrite(g_hmap.m_countryowner, sizeof(int), g_hmap.m_widthx*g_hmap.m_widthz, fp);
}

void ReadHeightmap(FILE *fp)
{
	int widthx=0, widthz=0;

	fread(&widthx, sizeof(int), 1, fp);
	fread(&widthz, sizeof(int), 1, fp);

	g_hmap.allocate(widthx, widthz);
#if 0
	g_hmap2.allocate(widthx/2, widthz/2);
	g_hmap4.allocate(widthx/4, widthz/4);
	g_hmap8.allocate(widthx/8, widthz/8);
#endif
	
	fread(g_hmap.m_heightpoints, sizeof(float), (g_hmap.m_widthx+1)*(g_hmap.m_widthz+1), fp);
	fread(g_hmap.m_countryowner, sizeof(int), g_hmap.m_widthx*g_hmap.m_widthz, fp);

#if 0
	for(int x=0; x<widthx+1; x++)
		for(int z=0; z<widthz+1; z++)
		{
			if(x%2 == 0 && z%2 == 0)
				g_hmap2.setheight(x/2, z/2, g_hmap.getheight(x, z));
			if(x%4 == 0 && z%4 == 0)
				g_hmap4.setheight(x/4, z/4, g_hmap.getheight(x, z));
			if(x%8 == 0 && z%8 == 0)
				g_hmap8.setheight(x/8, z/8, g_hmap.getheight(x, z));
		}
#endif

	g_hmap.remesh(1);
#if 0
	g_hmap2.remesh(2);
	g_hmap4.remesh(4);
	g_hmap8.remesh(8);
#endif
	MutexWait(g_drawmutex);
	g_hmap.prerender();
	MutexRelease(g_drawmutex);

	AllocGrid(widthx, widthz);

	g_borders.alloc(widthx, widthz);
	AllocPathGrid(widthx*TILE_SIZE, widthz*TILE_SIZE);
	DefineBorders(0, 0, g_hmap.m_widthx-1, g_hmap.m_widthz-1);
	CalcMapView();
}

void SaveFoliage(FILE *fp)
{
	for(int i=0; i<FOLIAGES; i++)
	{
#if 0
	bool on;
	unsigned char type;
	Vec3f pos;
	float yaw;
#endif

		Foliage *f = &g_foliage[i];

		fwrite(&f->on, sizeof(bool), 1, fp);

		if(!f->on)
			continue;

		fwrite(&f->type, sizeof(unsigned char), 1, fp);
		fwrite(&f->pos, sizeof(Vec3f), 1, fp);
	}
}

void ReadFoliage(FILE *fp)
{
	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage *f = &g_foliage[i];

		fread(&f->on, sizeof(bool), 1, fp);

		if(!f->on)
		{
			f->reinstance();
			continue;
		}

		fread(&f->type, sizeof(unsigned char), 1, fp);
		fread(&f->pos, sizeof(Vec3f), 1, fp);
	
		f->reinstance();
	}
}

void SaveDeposits(FILE *fp)
{
	for(int i=0; i<DEPOSITS; i++)
	{
#if 0
	bool on;
	bool occupied;
	int restype;
	int amount;
	Vec2i tilepos;
	Vec3f drawpos;
#endif

		Deposit *d = &g_deposit[i];

		fwrite(&d->on, sizeof(bool), 1, fp);

		if(!d->on)
			continue;
		
		fwrite(&d->occupied, sizeof(bool), 1, fp);
		fwrite(&d->restype, sizeof(int), 1, fp);
		fwrite(&d->amount, sizeof(int), 1, fp);
		fwrite(&d->tilepos, sizeof(Vec2i), 1, fp);
		fwrite(&d->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void ReadDeposits(FILE *fp)
{
	for(int i=0; i<DEPOSITS; i++)
	{
		Deposit *d = &g_deposit[i];

		fread(&d->on, sizeof(bool), 1, fp);

		if(!d->on)
			continue;
		
		fread(&d->occupied, sizeof(bool), 1, fp);
		fread(&d->restype, sizeof(int), 1, fp);
		fread(&d->amount, sizeof(int), 1, fp);
		fread(&d->tilepos, sizeof(Vec2i), 1, fp);
		fread(&d->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void SaveUnits(FILE *fp)
{
	for(int i=0; i<UNITS; i++)
	{
#if 0
		bool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;

	/*
	The f (floating-point) position vectory is used for drawing.
	*/
	Vec3f fpos;

	/*
	The real position is stored in integers.
	*/
	Vec2i cmpos;
	Vec3f facing;
	Vec2f rotation;

	list<Vec2i> path;
	Vec2i goal;
#endif

		Unit *u = &g_unit[i];

		fwrite(&u->on, sizeof(bool), 1, fp);

		if(!u->on)
			continue;

		fwrite(&u->type, sizeof(int), 1, fp);
		fwrite(&u->stateowner, sizeof(int), 1, fp);
		fwrite(&u->corpowner, sizeof(int), 1, fp);
		fwrite(&u->unitowner, sizeof(int), 1, fp);

		fwrite(&u->drawpos, sizeof(Vec3f), 1, fp);
		fwrite(&u->cmpos, sizeof(Vec2i), 1, fp);
		fwrite(&u->rotation, sizeof(Vec2f), 1, fp);

		int pathsz = u->path.size();

		fwrite(&pathsz, sizeof(int), 1, fp);

		for(auto pathiter = u->path.begin(); pathiter != u->path.end(); pathiter++)
			fwrite(&*pathiter, sizeof(Vec2i), 1, fp);
		
		fwrite(&u->goal, sizeof(Vec2i), 1, fp);

#if 0
	int target;
	int target2;
	bool targetu;
	bool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	bool passive;
	Vec2i prevpos;
	int taskframe;
	bool pathblocked;
	int frameslookjobago;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//vector<TransportJob> bids;

	float frame[2];

	Vec2i subgoal;
#endif
	
		fwrite(&u->target, sizeof(int), 1, fp);
		fwrite(&u->target2, sizeof(int), 1, fp);
		fwrite(&u->targetu, sizeof(bool), 1, fp);
		fwrite(&u->underorder, sizeof(bool), 1, fp);
		fwrite(&u->fuelstation, sizeof(int), 1, fp);
		fwrite(u->belongings, sizeof(int), RESOURCES, fp);
		fwrite(&u->hp, sizeof(int), 1, fp);
		fwrite(&u->passive, sizeof(bool), 1, fp);
		fwrite(&u->prevpos, sizeof(Vec2i), 1, fp);
		fwrite(&u->taskframe, sizeof(int), 1, fp);
		fwrite(&u->pathblocked, sizeof(bool), 1, fp);
		fwrite(&u->frameslookjobago, sizeof(int), 1, fp);
		fwrite(&u->supplier, sizeof(int), 1, fp);
		fwrite(&u->reqamt, sizeof(int), 1, fp);
		fwrite(&u->targtype, sizeof(int), 1, fp);
		fwrite(&u->home, sizeof(int), 1, fp);
		fwrite(&u->car, sizeof(int), 1, fp);
		fwrite(&u->frame, sizeof(float), 2, fp);
		fwrite(&u->subgoal, sizeof(Vec2i), 1, fp);

#if 0
	unsigned char mode;
	int pathdelay;
	long long lastpath;
	
	bool threadwait;
#endif

		
	}
}

void ReadUnits(FILE *fp)
{
	for(int i=0; i<UNITS; i++)
	{
		Unit *u = &g_unit[i];

		fread(&u->on, sizeof(bool), 1, fp);

		if(!u->on)
			continue;

		fread(&u->type, sizeof(int), 1, fp);
		fread(&u->stateowner, sizeof(int), 1, fp);
		fread(&u->corpowner, sizeof(int), 1, fp);
		fread(&u->unitowner, sizeof(int), 1, fp);

		fread(&u->drawpos, sizeof(Vec3f), 1, fp);
		fread(&u->cmpos, sizeof(Vec2i), 1, fp);
		fread(&u->rotation, sizeof(Vec2f), 1, fp);

		int pathsz = 0;

		fread(&pathsz, sizeof(int), 1, fp);
		u->path.clear();

		for(int pathindex=0; pathindex<pathsz; pathindex++)
		{
			Vec2i waypoint;
			fread(&waypoint, sizeof(Vec2i), 1, fp);
			u->path.push_back(waypoint);
		}
		
		fread(&u->goal, sizeof(Vec2i), 1, fp);
		
		fread(&u->target, sizeof(int), 1, fp);
		fread(&u->target2, sizeof(int), 1, fp);
		fread(&u->targetu, sizeof(bool), 1, fp);
		fread(&u->underorder, sizeof(bool), 1, fp);
		fread(&u->fuelstation, sizeof(int), 1, fp);
		fread(u->belongings, sizeof(int), RESOURCES, fp);
		fread(&u->hp, sizeof(int), 1, fp);
		fread(&u->passive, sizeof(bool), 1, fp);
		fread(&u->prevpos, sizeof(Vec2i), 1, fp);
		fread(&u->taskframe, sizeof(int), 1, fp);
		fread(&u->pathblocked, sizeof(bool), 1, fp);
		fread(&u->frameslookjobago, sizeof(int), 1, fp);
		fread(&u->supplier, sizeof(int), 1, fp);
		fread(&u->reqamt, sizeof(int), 1, fp);
		fread(&u->targtype, sizeof(int), 1, fp);
		fread(&u->home, sizeof(int), 1, fp);
		fread(&u->car, sizeof(int), 1, fp);
		fread(&u->frame, sizeof(float), 2, fp);
		fread(&u->subgoal, sizeof(Vec2i), 1, fp);
	}
}

void SaveBuildings(FILE *fp)
{
	for(int i=0; i<BUILDINGS; i++)
	{
#if 0
		bool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;

	Vec2i tilepos;	//position in tiles
	Vec3f drawpos;	//drawing position in centimeters

	bool finished;

	int pownetw;
	int crpipenetw;
	list<int> roadnetw;
#endif
		
		Building *b = &g_building[i];

		fwrite(&b->on, sizeof(bool), 1, fp);

		if(!b->on)
			continue;

		fwrite(&b->type, sizeof(int), 1, fp);
		fwrite(&b->stateowner, sizeof(int), 1, fp);
		fwrite(&b->corpowner, sizeof(int), 1, fp);
		fwrite(&b->unitowner, sizeof(int), 1, fp);

		fwrite(&b->tilepos, sizeof(Vec2i), 1, fp);
		fwrite(&b->drawpos, sizeof(Vec3f), 1, fp);

		fwrite(&b->finished, sizeof(bool), 1, fp);

		fwrite(&b->pownetw, sizeof(int), 1, fp);
		fwrite(&b->crpipenetw, sizeof(int), 1, fp);
		int nroadnetw = b->roadnetw.size();
		fwrite(&nroadnetw, sizeof(int), 1, fp);
		for(auto roadnetwiter = b->roadnetw.begin(); roadnetwiter != b->roadnetw.end(); roadnetwiter++)
			fwrite(&*roadnetwiter, sizeof(int), 1, fp);
	}
}

void ReadBuildings(FILE *fp)
{
	
	for(int i=0; i<BUILDINGS; i++)
	{
		Building *b = &g_building[i];

		fread(&b->on, sizeof(bool), 1, fp);

		if(!b->on)
			continue;

		fread(&b->type, sizeof(int), 1, fp);
		fread(&b->stateowner, sizeof(int), 1, fp);
		fread(&b->corpowner, sizeof(int), 1, fp);
		fread(&b->unitowner, sizeof(int), 1, fp);

		fread(&b->tilepos, sizeof(Vec2i), 1, fp);
		fread(&b->drawpos, sizeof(Vec3f), 1, fp);
		
		fread(&b->finished, sizeof(bool), 1, fp);

		fread(&b->pownetw, sizeof(int), 1, fp);
		fread(&b->crpipenetw, sizeof(int), 1, fp);
		int nroadnetw = -1;
		fread(&nroadnetw, sizeof(int), 1, fp);
		for(int i=0; i<nroadnetw; i++)
		{
			int roadnetw = -1;
			fread(&roadnetw, sizeof(int), 1, fp);
			b->roadnetw.push_back(roadnetw);
		}

		b->fillcollider();
	}
}

void SaveZoomCam(FILE *fp)
{
	fwrite(&g_camera, sizeof(Camera), 1, fp);
	fwrite(&g_zoom, sizeof(float), 1, fp);
}

void ReadZoomCam(FILE *fp)
{
	fread(&g_camera, sizeof(Camera), 1, fp);
	fread(&g_zoom, sizeof(float), 1, fp);
}

void SaveRoads(FILE* fp)
{
#if 0	
	bool on;
	int type;
	bool finished;
	int stateowner;
	int conmat[RESOURCES];
	int netw;
	VertexArray drawva;
	int transporter[RESOURCES];
	Vec3f drawpos;
#endif

	for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthz; i++)
	{
		RoadTile* r = &g_road[i];

		fwrite(&r->on, sizeof(bool), 1, fp);

		if(!r->on)
			continue;

		fwrite(&r->type, sizeof(int), 1, fp);
		fwrite(&r->finished, sizeof(bool), 1, fp);
		fwrite(&r->stateowner, sizeof(int), 1, fp);
		fwrite(r->conmat, sizeof(int), RESOURCES, fp);
		fwrite(&r->netw, sizeof(int), 1, fp);
		fwrite(r->transporter, sizeof(int), RESOURCES, fp);
		fwrite(&r->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void ReadRoads(FILE* fp)
{
	for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthz; i++)
	{
		RoadTile* r = &g_road[i];

		fread(&r->on, sizeof(bool), 1, fp);

		if(!r->on)
			continue;

		fread(&r->type, sizeof(int), 1, fp);
		fread(&r->finished, sizeof(bool), 1, fp);
		fread(&r->stateowner, sizeof(int), 1, fp);
		fread(r->conmat, sizeof(int), RESOURCES, fp);
		fread(&r->netw, sizeof(int), 1, fp);
		fread(r->transporter, sizeof(int), RESOURCES, fp);
		fread(&r->drawpos, sizeof(Vec3f), 1, fp);
	}
	
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			if(!RoadAt(x, z)->on)
				continue;

			MeshRoad(x, z, false);

			g_hmap.hidetile(x, z);

			if(RoadAt(x, z)->finished)
				RoadAt(x, z)->fillcollider();
		}
}

void SavePowls(FILE* fp)
{
#if 0
	bool on;
	int type;
	bool finished;
	int stateowner;
	int conmat[RESOURCES];
	int netw;	//power network
	VertexArray drawva;
	//bool inaccessible;
	int transporter[RESOURCES];
	Vec3f drawpos;
#endif
	
	for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthz; i++)
	{
		PowlTile* p = &g_powl[i];

		fwrite(&p->on, sizeof(bool), 1, fp);

		if(!p->on)
			continue;

		fwrite(&p->type, sizeof(int), 1, fp);
		fwrite(&p->finished, sizeof(bool), 1, fp);
		fwrite(&p->stateowner, sizeof(int), 1, fp);
		fwrite(p->conmat, sizeof(int), RESOURCES, fp);
		fwrite(&p->netw, sizeof(int), 1, fp);
		fwrite(p->transporter, sizeof(int), RESOURCES, fp);
		fwrite(&p->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void ReadPowls(FILE* fp)
{
	for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthz; i++)
	{
		PowlTile* p = &g_powl[i];

		fread(&p->on, sizeof(bool), 1, fp);

		if(!p->on)
			continue;

		fread(&p->type, sizeof(int), 1, fp);
		fread(&p->finished, sizeof(bool), 1, fp);
		fread(&p->stateowner, sizeof(int), 1, fp);
		fread(p->conmat, sizeof(int), RESOURCES, fp);
		fread(&p->netw, sizeof(int), 1, fp);
		fread(p->transporter, sizeof(int), RESOURCES, fp);
		fread(&p->drawpos, sizeof(Vec3f), 1, fp);
	}
	
	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			if(!PowlAt(x, z)->on)
				continue;

			MeshPowl(x, z, false);
		}
}

void SaveCrPipes(FILE* fp)
{
#if 0
	bool on;
	int type;
	bool finished;
	int stateowner;
	int conmat[RESOURCES];
	int netw;	//pipe network
	VertexArray drawva;
	//bool inaccessible;
	int transporter[RESOURCES];
	Vec3f drawpos;
#endif
	
	for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthz; i++)
	{
		CrPipeTile* p = &g_crpipe[i];

		fwrite(&p->on, sizeof(bool), 1, fp);

		if(!p->on)
			continue;

		fwrite(&p->type, sizeof(int), 1, fp);
		fwrite(&p->finished, sizeof(bool), 1, fp);
		fwrite(&p->stateowner, sizeof(int), 1, fp);
		fwrite(p->conmat, sizeof(int), RESOURCES, fp);
		fwrite(&p->netw, sizeof(int), 1, fp);
		fwrite(p->transporter, sizeof(int), RESOURCES, fp);
		fwrite(&p->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void ReadCrPipes(FILE* fp)
{
	for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthz; i++)
	{
		CrPipeTile* p = &g_crpipe[i];

		fread(&p->on, sizeof(bool), 1, fp);

		if(!p->on)
			continue;

		fread(&p->type, sizeof(int), 1, fp);
		fread(&p->finished, sizeof(bool), 1, fp);
		fread(&p->stateowner, sizeof(int), 1, fp);
		fread(p->conmat, sizeof(int), RESOURCES, fp);
		fread(&p->netw, sizeof(int), 1, fp);
		fread(p->transporter, sizeof(int), RESOURCES, fp);
		fread(&p->drawpos, sizeof(Vec3f), 1, fp);
	}

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			if(!CrPipeAt(x, z)->on)
				continue;

			MeshCrPipe(x, z, false);
		}
}

bool SaveMap(const char* fullpath)
{
	FILE *fp = NULL;

	fp = fopen(fullpath, "wb");

	if(!fp)
		return false;

	char tag[] = MAP_TAG;
	int version = MAP_VERSION;

	fwrite(&tag, sizeof(tag), 1, fp);
	fwrite(&version, sizeof(version), 1, fp);

	SaveZoomCam(fp);
	SaveHeightmap(fp);
	SaveDeposits(fp);
	SaveFoliage(fp);
	SaveBuildings(fp);
	SaveUnits(fp);
	SaveRoads(fp);
	SavePowls(fp);
	SaveCrPipes(fp);

	fclose(fp);

	return true;
}

bool LoadMap(const char* fullpath)
{
	FreeMap();
	
	FILE *fp = NULL;

	fp = fopen(fullpath, "rb");

	if(!fp)
		return false;

	char realtag[] = MAP_TAG;
	int version = MAP_VERSION;
	char tag[ sizeof(realtag) ];
	
	fread(&tag, sizeof(tag), 1, fp);

	if(memcmp(tag, realtag, sizeof(tag)) != 0)
	{
		MessageBox(g_hWnd, "Incorrect header tag in map file.", "Error", NULL);
		return false;
	}

	fread(&version, sizeof(version), 1, fp);

	if(version != MAP_VERSION)
	{
		char msg[128];
		sprintf(msg, "Map file version %i doesn't match %i.", version, MAP_VERSION);
		MessageBox(g_hWnd, msg, "Error", NULL);
		return false;
	}

	ReadZoomCam(fp);
	ReadHeightmap(fp);
	ReadDeposits(fp);
	ReadFoliage(fp);
	ReadBuildings(fp);
	ReadUnits(fp);
	ReadRoads(fp);
	ReadPowls(fp);
	ReadCrPipes(fp);

	//CondenseForest(0, 0, g_hmap.m_widthx-1, g_hmap.m_widthz-1);

	FillColliderGrid();
	
	fclose(fp);

	return true;
}