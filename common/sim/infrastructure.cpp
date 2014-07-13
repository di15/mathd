

#include "infrastructure.h"
#include "../render/heightmap.h"
#include "building.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "player.h"
#include "../sim/selection.h"
#include "../gui/gui.h"
#include "../gui/widgets/spez/constructionview.h"
#include "../../game/gui/gviewport.h"
#include "../../game/gmain.h"
#include "../../game/gui/ggui.h"

ConduitType g_cotype[CONDUITS];


void ConduitType::updplans(char owner, Vec3f start, Vec3f end)
{
}

void ConduitType::clearplans()
{
	//if(!get)
	//	return;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
			get(x, z, true)->on = false;
}

void ConduitType::resetnetw()
{
	int lastnetw = 0;
	Building* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(blconduct)
		{
			short* netw = (short*)(((char*)b)+netwoff);
			*netw = -1;
		}
		else
		{
			std::list<short>* netw = (std::list<short>*)(((char*)b)+netwoff);
			netw->clear();
		}
	}

	//if(!get)
	//	return;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			ConduitTile* ct = get(x, z, false);

			if(!ct->on)
				continue;

			if(!ct->finished)
				continue;

			ct->netw = lastnetw++;
		}
}

// If buildings conduct the resource,
// merge the networks of touching buildings.
bool ConduitType::renetwb()
{
	if(!blconduct)
		return false;

	bool change = false;
	
	Building* b;
	Building* b2;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];
		short& netw = *(short*)(((char*)b)+netwoff);

		if(!b->on)
			continue;

		for(int j=i+1; j<BUILDINGS; j++)
		{
			b2 = &g_building[j];

			if(!b2->on)
				continue;

			if(!BlAdj(i, j))
				continue;
			
			short& netw2 = *(short*)(((char*)b2)+netwoff);

			if(netw < 0 && netw2 >= 0)
			{
				netw = netw2;
				change = true;
			}
			else if(netw2 < 0 && netw >= 0)
			{
				netw2 = netw;
				change = true;
			}
			else if(netw >= 0 && netw2 >= 0 && netw != netw2)
			{
				mergenetw(netw, netw2);
				change = true;
			}
		}
	}

	return change;
}

// Merge two networks that have been found to be touching.
void ConduitType::mergenetw(int A, int B)
{
	int mini = imin(A, B);
	int maxi = imax(A, B);

	Building* b;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;
		
		short& netw = *(short*)(((char*)b)+netwoff);

		if(netw == maxi)
			netw = mini;
	}

	ConduitTile* ct;

	//if(!get)
	//	return;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			ct = get(x, z, false);

			if(!ct->on)
				continue;

			if(!ct->finished)
				continue;

			if(ct->netw == maxi)
				ct->netw = mini;
		}
}

bool ConduitType::comparetiles(ConduitTile* ct, int x, int z)
{
	ConduitTile* ct2 = get(x, z, false);

	if(!ct2->on)
		return false;

	if(!ct2->finished)
		return false;

	if(ct2->netw < 0 && ct->netw >= 0)
	{
		ct2->netw = ct->netw;
		return true;
	}
	else if(ct->netw < 0 && ct2->netw >= 0)
	{
		ct->netw = ct2->netw;
		return true;
	}
	else if(ct->netw >= 0 && ct2->netw >= 0 && ct->netw != ct2->netw)
	{
		mergenetw(ct->netw, ct2->netw);
		return true;
	}

	return false;
}

// Building adjacent?
bool ConduitType::badj(int i, int x, int z)
{
	Building* b = &g_building[i];
	BuildingT* t = &g_bltype[b->type];

	Vec2i tp = b->tilepos;

	int minx = (tp.x - t->widthx/2)*TILE_SIZE;
	int minz = (tp.y - t->widthz/2)*TILE_SIZE;
	int maxx = minx + t->widthx*TILE_SIZE;
	int maxz = minz + t->widthz*TILE_SIZE;

	//Vec3i p2 = RoadPhysPos(x, z);
	Vec2i p2 = Vec2i(x, z) + physoff;

	const int hwx2 = TILE_SIZE/2;
	const int hwz2 = TILE_SIZE/2;
	int minx2 = p2.x - hwx2;
	int minz2 = p2.y - hwz2;
	int maxx2 = minx2 + TILE_SIZE;
	int maxz2 = minz2 + TILE_SIZE;

	if(maxx >= minx2 && maxz >= minz2 && minx <= maxx2 && minz <= maxz2)
		return true;

	return false;
}

bool ConduitType::compareb(Building* b, ConduitTile* ct)
{
	if(!blconduct)
	{
		std::list<short>& bnetw = *(std::list<short>*)(((char*)b)+netwoff);

		if(bnetw.size() <= 0 && ct->netw >= 0)
		{
			bnetw.push_back(ct->netw);
			return true;
		}/*
		 else if(r->netw < 0 && b->roadnetw >= 0)
		 {
		 pow->netw = b->pownetw;
		 return true;
		 }
		 else if(pow->netw >= 0 && b->pownetw >= 0 && pow->netw != b->pownetw)
		 {
		 MergePow(pow->netw, b->pownetw);
		 return true;
		 }*/
		else if(bnetw.size() > 0 && ct->netw >= 0)
		{
			bool found = false;
			for(auto netwiter = bnetw.begin(); netwiter != bnetw.end(); netwiter++)
			{
				if(*netwiter == ct->netw)
				{
					found = true;
					break;
				}
			}
			if(!found)
				bnetw.push_back(ct->netw);
		}

		return false;
	}
	else
	{
		short& bnetw = *(short*)(((char*)b)+netwoff);

		if(bnetw < 0 && ct->netw >= 0)
		{
			bnetw = ct->netw;
			return true;
		}
		else if(ct->netw < 0 && bnetw >= 0)
		{
			ct->netw = bnetw;
			return true;
		}
		else if(ct->netw >= 0 && bnetw >= 0 && ct->netw != bnetw)
		{
			mergenetw(ct->netw, bnetw);
			return true;
		}

		return false;
	}

	return false;
}

bool ConduitType::renetwtiles()
{
	bool change = false;

	//if(!get)
	//	return;

	ConduitTile* ct;
	Building* b;

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
			ct = get(x, z, false);

			if(!ct->on)
				continue;

			if(!ct->finished)
				continue;

			if(x > 0 && comparetiles(ct, x-1, z))
				change = true;
			if(x < g_hmap.m_widthx-1 && comparetiles(ct, x+1, z))
				change = true;
			if(z > 0 && comparetiles(ct, x, z-1))
				change = true;
			if(z < g_hmap.m_widthz-1 && comparetiles(ct, x, z+1))
				change = true;

			for(int i=0; i<BUILDINGS; i++)
			{
				b = &g_building[i];

				if(!b->on)
					continue;

				if(!badj(i, x, z))
					continue;

				if(compareb(b, ct))
					change = true;
			}
		}

	return change;
}

void ConduitType::renetw()
{
	resetnetw();

	bool change;

	do
	{
		change = false;

		if(renetwb())
			change = true;

		if(renetwtiles())
			change = true;
	}
	while(change);

#if 0
	CheckRoadAccess();
#endif
}

bool ConduitType::tilelevel(float iterx, float iterz, float testx, float testz, float dx, float dz, int i, float d, bool plantoo)
{
	//return true;

	bool n = false, e = false, s = false, w = false;
	int ix, iz;

	if(i > 0)
	{
		float x = iterx - dx;
		float z = iterz - dz;

		ix = x;
		iz = z;

		if(ix == testx)
		{
			if(iz == testz+1)	n = true;
			else if(iz == testz-1)	s = true;
		}
		else if(iz == testz)
		{
			if(ix == testx+1)	w = true;
			else if(ix == testx-1)	e = true;
		}

		float prevx = x - dx;
		float prevz = z - dz;

		if((int)x != prevx && (int)z != prevz)
		{
			ix = prevx;

			if(ix == testx)
			{
				if(iz == testz+1)	n = true;
				else if(iz == testz-1)	s = true;
			}
			else if(iz == testz)
			{
				if(ix == testx+1)	w = true;
				else if(ix == testx-1)	e = true;
			}
		}
	}

	if(i < d)
	{
		float x = iterx + dx;
		float z = iterz + dz;

		ix = x;
		iz = z;

		if(ix == testx)
		{
			if(iz == testz+1)	n = true;
			else if(iz == testz-1)	s = true;
		}
		else if(iz == testz)
		{
			if(ix == testx+1)	w = true;
			else if(ix == testx-1)	e = true;
		}

		if(i > 0)
		{
			float prevx = x - dx;
			float prevz = z - dz;

			if((int)x != prevx && (int)z != prevz)
			{
				ix = prevx;

				if(ix == testx)
				{
					if(iz == testz+1)	n = true;
					else if(iz == testz-1)	s = true;
				}
				else if(iz == testz)
				{
					if(ix == testx+1)	w = true;
					else if(ix == testx-1)	e = true;
				}
			}
		}
	}

	if(!cornerpl)
	{
		ix = testx;
		iz = testz;

		if(g_hmap.getheight(ix, iz) <= WATER_LEVEL)		return false;
		if(g_hmap.getheight(ix+1, iz) <= WATER_LEVEL)	return false;
		if(g_hmap.getheight(ix, iz+1) <= WATER_LEVEL)	return false;
		if(g_hmap.getheight(ix+1, iz+1) <= WATER_LEVEL)	return false;
	}
	else
	{
		//ix = (testx * TILE_SIZE + physoff.x)/TILE_SIZE;
		//iz = (testz * TILE_SIZE + physoff.y)/TILE_SIZE;
		ix = testx;
		iz = testz;

		if(g_hmap.getheight(ix, iz) <= WATER_LEVEL)		return false;
	}

	// Check which neighbours have conduits too

	if(ix > 0)
	{
		if(get(ix-1, iz, false)->on)	w = true;
		//if(RoadPlanAt(ix-1, iz)->on)	w = true;
		if(plantoo)
			if(get(ix-1, iz, true)->on) w = true;
	}

	if(ix < g_hmap.m_widthx-1)
	{
		if(get(ix+1, iz, false)->on)	e = true;
		//if(RoadPlanAt(ix+1, iz)->on)	e = true;
		if(plantoo)
			if(get(ix+1, iz, true)->on) e = true;
	}

	if(iz > 0)
	{
		if(get(ix, iz-1, false)->on)	s = true;
		//if(RoadPlanAt(ix, iz-1)->on)	s = true;
		if(plantoo)
			if(get(ix, iz-1, true)->on) s= true;
	}

	if(iz < g_hmap.m_widthz-1)
	{
		if(get(ix, iz+1, false)->on)	n = true;
		//if(RoadPlanAt(ix, iz+1)->on)	n = true;
		if(plantoo)
			if(get(ix, iz+1, true)->on) n = true;
	}
#if 0
	g_log<<"level? ix"<<ix<<","<<iz<<endl;
	g_log.flush();
#endif
	if((n && e && s && w) || (n && e && s && !w) || (n && e && !s && w) || (n && e && !s && !w) || (n && !e && s && w)
			|| (n && !e && !s && w) || (!n && e && s && !w) || (!n && !e && s && w) || (!n && !e && !s && !w) || (!n && e && s && w))
	{
		float compare = g_hmap.getheight(ix, iz);
		if(fabs(g_hmap.getheight(ix+1, iz) - compare) > maxsideincl)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - compare) > maxsideincl)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz+1) - compare) > maxsideincl)	return false;
	}
	else if((n && !e && s && !w) || (n && !e && !s && !w) || (!n && !e && s && !w))
	{
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix+1, iz)) > maxsideincl)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - g_hmap.getheight(ix+1, iz+1)) > maxsideincl)	return false;
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix, iz+1)) > maxforwincl)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz) - g_hmap.getheight(ix+1, iz+1)) > maxforwincl)	return false;
	}
	else if((!n && e && !s && w) || (!n && e && !s && !w) || (!n && !e && !s && w))
	{
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix+1, iz)) > maxforwincl)	return false;
		if(fabs(g_hmap.getheight(ix, iz+1) - g_hmap.getheight(ix+1, iz+1)) > maxforwincl)	return false;
		if(fabs(g_hmap.getheight(ix, iz) - g_hmap.getheight(ix, iz+1)) > maxsideincl)	return false;
		if(fabs(g_hmap.getheight(ix+1, iz) - g_hmap.getheight(ix+1, iz+1)) > maxsideincl)	return false;
	}

#if 0
	g_log<<"level yes! ix"<<ix<<","<<iz<<endl;
	g_log.flush();
#endif

	return true;
}

void ConduitType::remesh(int x, int z, bool plan)
{
	ConduitTile* ct = get(x, z, plan);

	if(!ct->on)
		return;
#if 0
	g_log<<"mesh on "<<x<<","<<z<<endl;
	g_log.flush();
#endif

	int mi = model[ct->conntype][plan];
	Model* m = &g_model[mi];

	VertexArray* cva = &ct->drawva;
	VertexArray* mva = &m->m_va[0];

	cva->free();

	//g_log<<"meshroad allocating "<<mva->numverts<<"...";
	//g_log.flush();

	cva->alloc( mva->numverts );

	if(!cva->vertices)
		OutOfMem(__FILE__, __LINE__);

	if(!cva->texcoords)
		OutOfMem(__FILE__, __LINE__);

	if(!cva->normals)
		OutOfMem(__FILE__, __LINE__);

	float realx, realz;
	ct->drawpos = Vec3f(x*TILE_SIZE, 0, z*TILE_SIZE) + drawoff;

	for(int i=0; i<cva->numverts; i++)
	{
		cva->vertices[i] = mva->vertices[i];
		cva->texcoords[i] = mva->texcoords[i];
		cva->normals[i] = mva->normals[i];

		realx = ct->drawpos.x + cva->vertices[i].x;
		realz = ct->drawpos.z + cva->vertices[i].z;

#if 0
		cva->vertices[i].y += Bilerp(&g_hmap, realx, realz);
#else
		cva->vertices[i].y += g_hmap.accheight2(realx, realz);
#endif
	}

	for(int i=0; i<cva->numverts; i+=3)
	{
		cva->normals[i+0] = Normal(&cva->vertices[i]);
		cva->normals[i+1] = Normal(&cva->vertices[i]);
		cva->normals[i+2] = Normal(&cva->vertices[i]);
	}

	//g_log<<"done meshroad"<<endl;
	//g_log.flush();

	cva->genvbo();
}

void ConduitType::updplans(char owner, Vec3f start, Vec3f end)
{
	clearplans();

	int x1 = Clipi(start.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z1 = Clipi(start.z/TILE_SIZE, 0, g_hmap.m_widthz-1);

	int x2 = Clipi(end.x/TILE_SIZE, 0, g_hmap.m_widthx-1);
	int z2 = Clipi(end.z/TILE_SIZE, 0, g_hmap.m_widthz-1);

#if 0
	g_log<<"road plan "<<x1<<","<<z1<<"->"<<x2<<","<<z2<<endl;
	g_log.flush();
#endif
	
	//PlacePowl(x1, z1, stateowner, true);
	//PlacePowl(x2, z2, stateowner, true);

	float dx = x2-x1;
	float dz = z2-z1;

	float d = sqrtf(dx*dx + dz*dz);

	dx /= d;
	dz /= d;

	int prevx = x1;
	int prevz = z1;

	int i = 0;

	//PlaceRoad(x1, z1, g_localPlayer, true);
	//if(RoadLevel(x2, z2, dx, dz, i, d))
	//	PlaceRoad(x2, z2, g_localPlayer, true);

	for(float x=x1, z=z1; i<=d; x+=dx, z+=dz, i++)
	{
		if(tilelevel(x, z, x, z, dx, dz, i, d, true))
		{
#if 0
			g_log<<"place road urp "<<x<<","<<z<<endl;
			g_log.flush();
#endif
			place(x, z, owner, true);
		}

		if((int)x != prevx && (int)z != prevz)
		{
			if(tilelevel(x, z, prevx, z, dx, dz, i, d, true))
				place(prevx, z, owner, true);
		}

		prevx = x;
		prevz = z;

#if 0
		g_log<<"place road "<<x<<","<<z<<endl;
		g_log.flush();
#endif
	}

	if((int)x2 != prevx && (int)z2 != prevz)
	{
		if(tilelevel(x2, z1, prevx, z2, dx, dz, i, d, true))
			place(prevx, z2, owner, true);
	}

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthz; z++)
			remesh(x, z, true);
}

void ConduitType::place(int x, int z, int owner, bool plan)
{
#if 0
	g_log<<"place road ...2"<<endl;
	g_log.flush();
#endif

	if(get(x, z, false)->on)
	{
#if 0
		g_log<<"place road ...2 repo"<<endl;
		g_log.flush();
#endif

		if(!plan)
		{
			RepossessRoad(x, z, owner);

			return;
		}
	}

	if(!RoadPlaceable(x, z))
	{
#if 0
		g_log<<"place road ...2 not placeable "<<x<<","<<z<<endl;
		g_log.flush();
#endif

		return;
	}

#if 0
	g_log<<"place road ...3"<<endl;
	g_log.flush();
#endif

	RoadTile* r = RoadAt(x, z);
	if(plan)
		r = RoadPlanAt(x, z);

	r->on = true;
	r->stateowner = stateowner;
	Zero(r->maxcost);

	//if(!plan && g_mode == APPMODE_PLAY)
	//	r->allocate();

	r->drawpos = RoadPosition(x, z);

	if(g_mode == APPMODE_PLAY)
		r->finished = false;
	//if(plan || g_mode == APPMODE_EDITOR)
	if(plan)
		r->finished = true;

	TypeRoad(x, z, plan);
	TypeRoadsAround(x, z, plan);

	for(int i=0; i<RESOURCES; i++)
		r->transporter[i] = -1;

	//if(!plan)
	//	ReRoadNetw();
#if 0
	g_log<<"place road ...4"<<endl;
	g_log.flush();
#endif

	if(!plan)
	{
		Vec3f pos = RoadPosition(x, z);
		ClearFoliage(pos.x - TILE_SIZE/2, pos.z - TILE_SIZE/2, pos.x + TILE_SIZE/2, pos.z + TILE_SIZE/2);

		g_hmap.hidetile(pos.x/TILE_SIZE, pos.z/TILE_SIZE);
	}
}

void ConduitType::place()
{
#if 0
	g_log<<"place road 1"<<endl;
	g_log.flush();
#endif
	Player* py = &g_player[g_curP];

	if(g_mode == APPMODE_PLAY)
		ClearSel(&py->sel);

	for(int x=0; x<g_hmap.m_widthx; x++)
	{
		for(int z=0; z<g_hmap.m_widthz; z++)
		{
#if 0
			g_log<<"place road 1.1 "<<x<<","<<z<<endl;
			g_log.flush();
#endif

			ConduitTile* plan = get(x, z, true);

			if(plan->on)
			{
#if 0
				g_log<<"place road 1.2 "<<x<<","<<z<<endl;
				g_log.flush();
#endif

				ConduitTile* actual = get(x, z, false);

				bool willchange = !actual->on;

				place(x, z, plan->owner, false);

				if(g_mode == APPMODE_PLAY && !actual->finished)
					py->sel.roads.push_back(Vec2i(x,z));

				if(g_mode == APPMODE_PLAY && willchange)
				{
#if 0
					NewJob(GOINGTOROADJOB, x, z);
#endif
				}
			}
		}
	}

	clearplans();
	renetw();
	g_hmap.genvbo();

	if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_curP];
		GUI* gui = &py->gui;

		if(py->sel.roads.size() > 0)
		{
			ConstructionView* cv = (ConstructionView*)gui->get("construction view")->get("construction view");
			cv->regen(&py->sel);
			gui->open("construction view");
		}
	}
}

ConduitTile::ConduitTile()
{
}

ConduitTile::~ConduitTile()
{
}

char ConduitTile::condtype()
{
	return CON_NONE;
}

int ConduitTile::netreq(int res)
{
	return 0;
}

void ConduitTile::destroy()
{
}

void ConduitTile::allocate()
{

}

bool ConduitTile::checkconstruction()
{

	return false;
}

void ConduitTile::fillcollider()
{
}

void ConduitTile::freecollider()
{
}

ConduitTile* ConduitType::get(int tx, int tz, bool plan)
{
	ConduitTile* tilesarr = *cotiles[(int)plan];
	return &tilesarr[ tx + tz*g_hmap.m_widthx ];
}

void DefCo(char ctype, unsigned short netwoff, unsigned short maxforwincl, unsigned short maxsideincl, bool blconduct, 
		   bool cornerpl, Vec2i physoff, Vec3f drawoff, ConduitTile** cotiles, ConduitTile** coplans
	//void (*checkplace)(int tx, int tz),
	//Vec3f (*drawpos)(int tx, int tz),
	//Vec2i (*physpos)(int tx, int tz),
	//void (*draw)(int tx, int ty, bool plan),
	//void (*updplans)(char owner, Vec3f start, Vec3f end),
	//void (*clearplans)(),
	//void (*renetw)(),
	//void (*place)(int tx, int tz, char owner, bool plan)
	//ConduitTile* (*get)(int tx, int tz, bool plan)
	)
{
	ConduitType* ct = &g_cotype[ctype];
	ct->netwoff = netwoff;
	ct->maxforwincl = maxforwincl;
	ct->maxsideincl = maxsideincl;
	ct->physoff = physoff;
	ct->drawoff = drawoff;
	ct->cotiles[0] = cotiles;
	ct->cotiles[1] = coplans;
	ct->cornerpl = cornerpl;
	//ct->checkplace = checkplace;
	//ct->drawpos = drawpos;
	//ct->physpos = physpos;
	//ct->draw = draw;
	//ct->updplans = updplans;
	//ct->clearplans = clearplans;
	//ct->renetw = renetw;
	//ct->place = place;
	//ct->get = get;

	// Test
	class RoadTile : public ConduitTile
	{
	public:
		int extra;
	};

	ConduitTile* conduits = new RoadTile[100];

	delete [] conduits;
}

void CoConMat(char ctype, char rtype, short ramt)
{
	ConduitType* ct = &g_cotype[ctype];
	ct->conmat[rtype] = ramt;
}