
#include "main.h"
#include "building.h"
#include "model.h"
#include "map.h"
#include "physics.h"
#include "powerline.h"
#include "pipeline.h"
#include "road.h"
#include "image.h"
#include "particle.h"
#include "player.h"
#include "job.h"
#include "chat.h"
#include "selection.h"
#include "gui.h"
#include "resource.h"
#include "unit.h"
#include "shader.h"
#include "transaction.h"
#include "sound.h"
#include "script.h"
#include "inspect.h"
#include "font.h"
#include "scenery.h"

CBuildingType g_buildingType[BUILDING_TYPES];
CBuilding g_building[BUILDINGS];
int g_build = NOTHING;
bool g_canPlace = false;
int g_lastBT = -1;
unsigned int g_hovertex[HOVERTEXS];

void BSprite(int type, char* file)
{
	g_lastBT = type;
	CBuildingType* t = &g_buildingType[type];
	//CreateTexture(sprite, file);
	QueueTexture(&t->sprite, file, true);
}

void BDefine(int type, char* n, char* modelfile, char* conmodelfile, int wX, int wZ, float hp, float maxprodq)
{
	g_lastBT = type;
	CBuildingType* t = &g_buildingType[type];
	strcpy(t->name, n);
	QueueModel(&t->model, modelfile, Vec3f(1,1,1), Vec3f(0,0,0));
	QueueModel(&t->conmodel, conmodelfile, Vec3f(1,1,1), Vec3f(0,0,0));
	t->widthX = wX;
	t->widthZ = wZ;
	t->hitpoints = hp;
	//BSprite(type, sprite);
	Zero(t->cost);
	Zero(t->input);
	Zero(t->output);
	t->maxprodquota = maxprodq;
}

void BCost(int res, float value)
{
	CBuildingType* t = &g_buildingType[g_lastBT];
	t->cost[res] = value;
}

void BIn(int res, float value)
{
	CBuildingType* t = &g_buildingType[g_lastBT];
	t->input[res] = value;
}

void BOut(int res, float value)
{
	CBuildingType* t = &g_buildingType[g_lastBT];
	t->output[res] = value;
}

void BBuildable(int type)
{
	CBuildingType* t = &g_buildingType[g_lastBT];
	t->buildable.push_back(type);
}

void BEmitter(int etype, Vec3f offset) 
{ 
	CBuildingType* t = &g_buildingType[g_lastBT];
	t->emitter.push_back(EmitterPlace(etype, offset)); 
}

void CBuilding::draw() 
{ 
	const float* owncol = facowncolor[owner];
	glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);
	g_buildingType[type].draw(frame, finished, pos); 
}

void CBuilding::fillemitters()
{	
	for(int i=0; i<g_buildingType[type].emitter.size(); i++)
		emitter.push_back(EmitterCounter());
}

bool CBuilding::trybuy(int buyer)
{
	if(!forsale)
		return false;

	CPlayer* buyp = &g_player[buyer];
	CPlayer* sellp = &g_player[owner];

	if(buyp->global[CURRENC] < saleprice)
		return false;

	buyp->global[CURRENC] -= saleprice;
	sellp->global[CURRENC] += saleprice;

	if(buyer == g_localP)
		NewTransx(pos, CURRENC, -saleprice);
	else if(owner == g_localP)
		NewTransx(pos, CURRENC, saleprice);
	//FillBInspect(g_selection[0]);

	int prevowner = owner;
	owner = buyer;
	forsale = false;
	//cycleh.clear();

	//Chat(g_buildingType[type].name);

	CheckGameOver(prevowner);

	ClearAttackers(BuildingID(this), false);

	char msg[128];
	sprintf(msg, "sell %s", g_buildingType[type].name);
	LogTransx(prevowner, saleprice, msg);
	sprintf(msg, "buy %s", g_buildingType[type].name);
	LogTransx(buyer, -saleprice, msg);

	return true;
}

void CBuilding::set(int t, int o, Vec3f p) 
{
	on = true;
	finished = false;
	owner = o;
	type = t;
	pos = p;
	hp = g_buildingType[t].hitpoints;
	pownetw = -1;
	pipenetw = -1;
	prodquota = 1;
	Zero(quotafilled);
	Zero(conmat);
	framesago = 0;
	occupier.clear();

	cycleh.clear();
	recenth.reset();

	fillemitters();

	for(int i=0; i<RESOURCES; i++)
		transporter[i] = -1;

	produ.clear();

	forsale = false;
	//forsale = true;
	saleprice = 0.50f;
}

void CBuilding::Allocate()
{
	CBuildingType* t = &g_buildingType[type];
	CPlayer* p = &g_player[owner];

	float alloc;

	char transx[32];
	transx[0] = '\0';

	for(int i=0; i<RESOURCES; i++)
	{
		if(t->cost[i] <= 0)
			continue;

		if(i == LABOUR)
			continue;

		alloc = t->cost[i] - conmat[i];

		if(p->global[i] < alloc)
			alloc = p->global[i];

		conmat[i] += alloc;
		p->global[i] -= alloc;

		if(alloc > 0.0f)
			TransxAppend(transx, i, -alloc);
	}

	if(transx[0] != '\0'
#ifdef LOCAL_TRANSX
		&& owner == g_localP
#endif
		)
		NewTransx(pos, transx);

	CheckConstruction();
}

void CBuilding::Emit(int t)
{
	EmitParticle(t, pos + Vec3f(0, 10, 0));
}


bool CBuilding::QueueU(int ut, int buyerplayer)
{ 
	if(owner != buyerplayer)
	{
		CPlayer* buyer = &g_player[buyerplayer];
		CPlayer* seller = &g_player[owner];

		float uprice = seller->uprice[ut];

		if(buyer->global[CURRENC] < uprice)
			return false;

		buyer->global[CURRENC] -= uprice;
		seller->global[CURRENC] += uprice;

		char msg[128];
		sprintf(msg, "order %s", g_unitType[ut].name);
		LogTransx(buyerplayer, -seller->uprice[ut], msg);
		LogTransx(owner, seller->uprice[ut], msg);
	}

	ManufJob mj;
	mj.utype = ut;
	mj.owner = buyerplayer;
	produ.push_back(mj);

	if(g_selectType == SELECT_BUILDING && g_selection.size() > 0)
	{
		int thisID = BuildingID(this);

		if(g_selection[0] == thisID)
			UpdateQueueCount(ut);
	}

	return true;
}

bool CBuilding::DequeueU(int ut, int ownr)
{ 
	//char msg[128];
	//sprintf(msg, "deq %s", g_unitType[ut].name);
	//Chat(msg);

	for(int i=produ.size()-1; i>=0; i--)
	{	
		ManufJob mj = produ[i];

		if(mj.utype == ut && mj.owner == ownr)
		{
			produ.erase( produ.begin() + i );
			return true;
		}
	}

	return false;
}

void CBuilding::Explode()
{
	CBuildingType* t = &g_buildingType[type];
	float hwx = t->widthX*TILE_SIZE/2.0f;
	float hwz = t->widthZ*TILE_SIZE/2.0f;
	Vec3f p;

	for(int i=0; i<5; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = 4;
		p.z = hwx * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(FIREBALL, p + pos);
	}
	
	for(int i=0; i<5; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = 4;
		p.z = hwx * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(FIREBALL2, p + pos);
	}
	/*
	for(int i=0; i<5; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = 8;
		p.z = hwx * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(SMOKE, p + pos);
	}
	
	for(int i=0; i<5; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = 8;
		p.z = hwx * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(SMOKE2, p + pos);
	}
	*/
	for(int i=0; i<50; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = 4;
		p.z = hwx * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(DEBRIS, p + pos);
	}
}

void CBuildingType::draw(int frame, bool finished, Vec3f pos)	
{ 
	if(finished)
		g_model[model].draw(frame, pos, 0);
	else
		g_model[conmodel].draw(frame, pos, 0); 
}

void CBuilding::EmitParticles()
{
	CBuildingType* b = &g_buildingType[type];
	EmitterPlace* e;
	CParticleType* p;

	for(int i=0; i<emitter.size(); i++)
	{
		//first = true;

		e = &b->emitter[i];
		p = &g_particleType[e->type];

		if(emitter[i].EmitNext(p->delay))
			EmitParticle(e->type, pos + e->offset);
	}
}

void CBuilding::spawn(int ut, int ownr)
{
	int i = NewUnit();
	CUnit* u = &g_unit[i];
	CUnitType* t = &g_unitType[ut];

	u->on = true;
	u->owner = ownr;
	u->type = ut;
	u->hp = t->hitpoints;
	u->ResetTarget();
	u->passive = false;
	//if(ownr == g_localP)
	//	u->passive = true;
	u->camera.PositionCamera(pos.x, pos.y, pos.z, pos.x, pos.z+1, pos.z, 0, 1, 0);
	u->accum = 0;
	u->fuelStation = -1;
	u->target = -1;
	u->target2 = -1;
	u->driver = -1;
	u->frame[BODY_LOWER] = 0;
	u->frame[BODY_UPPER] = 0;

	if(ut == TRUCK)
		u->fuel = TRUCK_FUEL;

	CBuildingType* bt = &g_buildingType[type];

	PlaceUAround(u, pos.x - bt->widthX*TILE_SIZE/2, pos.z - bt->widthZ*TILE_SIZE/2, pos.x + bt->widthX*TILE_SIZE/2, pos.z + bt->widthZ*TILE_SIZE/2, true);
	u->ResetMode();
}

void CBuilding::UpdateQueueU()
{
	if(produ.size() <= 0)
		return;
	
	ManufJob mj = produ[0];

	CPlayer* p = &g_player[owner];
	CUnitType* t = &g_unitType[ mj.utype ];

	float netch[RESOURCES];

	if(!TrySubtract(t->cost, p->global, stock, p->local, netch))
		return;

	for(int i=0; i<RESOURCES; i++)
	{
		recenth.consumed[i] += netch[i];
	}

	spawn(mj.utype, mj.owner);
	produ.erase( produ.begin() + 0 );

	//if(owner == g_localP)
	{
		if(g_selection.size() > 0 && g_selectType == SELECT_BUILDING && g_selection[0] == BuildingID(this))
			UpdateQueueCount(mj.utype);

		if(mj.owner == g_localP)
		{
			char finmsg[128];
			sprintf(finmsg, "%s manufactured.", t->name);
			Chat(finmsg);
			WaiFoO();
			OnBuildU(mj.utype);
		}
	}
}

bool CBuilding::excessinput(int res)
{
	CBuildingType* bt = &g_buildingType[type];
	CPlayer* p = &g_player[owner];

	float got = stock[res];

	if(res != LABOUR)
		got += p->global[res];
	
	//if(got >= prodquota * bt->input[res])
	if(got >= netreq(res))
		return true;

	return false;
}

bool CBuilding::excessoutput()
{/*
	bool excess = true;
	CBuildingType* t = &g_buildingType[type];

	for(int r=0; r<RESOURCES; r++)
	{
		if(stock[r] < 50*t->output[r])
		{
			excess = false;
			break;
		}
	}
	*/

	//bool excess = true;
	CBuildingType* t = &g_buildingType[type];
	
	for(int r=0; r<RESOURCES; r++)
	{
		if(t->output[r] > 0.0f && (quotafilled[r] < t->output[r] * prodquota || netreq(r) > 0.0f))
		//if(t->output[r] > 0.0f && quotafilled[r] < netreq(r))
		{/*
			if(type == FACTORY)
			{
				char msg[128];
				sprintf(msg, "fac under %s", g_resource[r].name);
				Chat(msg);
			}*/
			/*
			if(type == FARM)
			{
				char msg[128];
				sprintf(msg, "farm FALSE prodquota = %f", prodquota);
				Chat(msg);
				//Chat("farm FALSE");
			}*/

			return false;
			//excess = false;
			//break;
		}
	}
	
			/*if(type == FARM)
			{
				char msg[128];
				sprintf(msg, "farm TRUE prodquota = %f", prodquota);
				Chat(msg);
				//Chat("farm TRUE");
			}*/
	//return excess;
	return true;
}

void CBuilding::EmitRes(int r)
{

}

bool CBuilding::BuyE()
{
	if(pownetw < 0)
		return false;

	CBuildingType* t;
	CPlayer* buyp = &g_player[owner];
	CPlayer* sellp;
	float lowest = 9999999999.0f;
	int lowestB = -1;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];
		
		if(!b->on)
			continue;

		if(!b->finished)
			continue;
		
		t = &g_buildingType[b->type];

		if(t->output[ELECTRICITY] <= 0.0f)
			continue;

		if(!g_diplomacy[b->owner][owner])
			continue;

		if(pownetw != b->pownetw)
			continue;

		sellp = &g_player[ b->owner ];

		if(b->stock[ELECTRICITY] <= 0.0f && sellp->global[ELECTRICITY] <= 0.0f)
			continue;

		if(b->owner != owner)
		{
			if(buyp->global[CURRENC] < sellp->price[ELECTRICITY])
				continue;

			if(sellp->price[ELECTRICITY] > lowest)
				continue;

			lowest = sellp->price[ELECTRICITY];
			lowestB = i;
			continue;
		}

		lowest = 0.001f;
		lowestB = i;
	}

	if(lowestB < 0)
		return false;

	CBuildingType* thist = &g_buildingType[type];
	float required = thist->input[ELECTRICITY] - stock[ELECTRICITY];
	float got = required;

	float paid;

	CBuilding* sellb = &g_building[lowestB];
	sellp = &g_player[sellb->owner];
	
	//if(b->stock[ELECTRICITY] + bp->global[ELECTRICITY] <= required)
	if(sellb->stock[ELECTRICITY] <= required)
		//got = b->stock[ELECTRICITY] + bp->global[ELECTRICITY];
		got = sellb->stock[ELECTRICITY];

	if(sellb->owner != owner)
	{
		float canafford = got;
		if(sellp->price[ELECTRICITY] > 0.0f)
			canafford = buyp->global[CURRENC] / sellp->price[ELECTRICITY];

		if(canafford < got)
			got = canafford;

		paid = sellp->price[ELECTRICITY] * got;
		buyp->global[CURRENC] -= paid;
		sellp->global[CURRENC] += paid;
		recenth.consumed[CURRENC] += paid;
		sellb->recenth.produced[CURRENC] += paid;
		//thisp->local[ELECTRICITY] 
		sellp->recentph[ELECTRICITY].earnings += paid;

		char msg[128];
		sprintf(msg, "purchase %d %s", (int)got, g_resource[ELECTRICITY].name);
		LogTransx(sellb->owner, paid, msg);
		LogTransx(owner, -paid, msg);
	}

	sellb->stock[ELECTRICITY] -= got;
	recenth.consumed[ELECTRICITY] += got;
	if(owner == sellb->owner)
		sellb->recenth.consumed[ELECTRICITY] += got;
	stock[ELECTRICITY] += got;

#ifdef LOCAL_TRANSX
		NewTransx(sellb->pos, ELECTRICITY, -got);
#else
	//b->Emit(DOLLAR);
	if(owner == g_localP && sellb->owner != owner)
		NewTransx(sellb->pos, ELECTRICITY, got, CURRENC, -paid);
	else if(sellb->owner == g_localP && sellb->owner != owner)
		NewTransx(sellb->pos, CURRENC, paid, ELECTRICITY, -got);
#endif

	return true;
}

void CBuilding::convert(int newowner)
{
	owner = newowner;
	CBuildingType* t = &g_buildingType[type];
	hp = t->hitpoints;
}

bool CBuilding::BuyCrude()
{
	if(pipenetw < 0)
		return false;

	CBuildingType* t;
	CPlayer* buyp = &g_player[owner];
	CPlayer* sellp;
	float lowest = 9999999999.0f;
	int lowestB = -1;

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];
		
		if(!b->on)
			continue;

		if(!b->finished)
			continue;
		
		t = &g_buildingType[b->type];

		if(t->output[CRUDE] <= 0.0f)
			continue;

		if(!g_diplomacy[b->owner][owner])
			continue;

		if(pipenetw != b->pipenetw)
			continue;

		sellp = &g_player[ b->owner ];

		if(b->stock[CRUDE] <= 0.0f && sellp->global[CRUDE] <= 0.0f)
			continue;

		if(b->owner != owner)
		{
			if(buyp->global[CURRENC] < sellp->price[CRUDE])
				continue;

			if(sellp->price[CRUDE] > lowest)
				continue;

			lowest = sellp->price[CRUDE];
			lowestB = i;
			continue;
		}

		lowest = 0.001f;
		lowestB = i;
	}

	if(lowestB < 0)
		return false;

	CBuildingType* thist = &g_buildingType[type];
	float required = thist->input[CRUDE] - stock[CRUDE];
	float got = required;

	float paid;

	CBuilding* sellb = &g_building[lowestB];
	sellp = &g_player[sellb->owner];

	if(sellb->stock[CRUDE] + sellp->global[CRUDE] <= required)
		got = sellb->stock[CRUDE] + sellp->global[CRUDE];

	if(sellb->owner != owner)
	{
		float canafford = got;
		if(sellp->price[CRUDE] > 0.0f)
			canafford = buyp->global[CURRENC] / sellp->price[CRUDE];

		if(canafford < got)
			got = canafford;

		paid = sellp->price[CRUDE] * got;
		buyp->global[CURRENC] -= paid;
		sellp->global[CURRENC] += paid;
		recenth.consumed[CURRENC] += paid;
		sellb->recenth.produced[CURRENC] += paid;
		buyp->local[CRUDE] += got;
		sellp->local[CRUDE] -= got;
		sellp->recentph[CRUDE].earnings += paid;
		
		char msg[128];
		sprintf(msg, "purchase %d %s", (int)got, g_resource[CRUDE].name);
		LogTransx(sellb->owner, paid, msg);
		LogTransx(owner, -paid, msg);
	}

	if(sellp->global[CRUDE] > got)
	{
		sellp->global[CRUDE] -= got;
	}
	else
	{
		float subgot = got;
		subgot -= sellp->global[CRUDE];
		sellp->global[CRUDE] = 0;
		sellb->stock[CRUDE] -= subgot;
	}
	recenth.consumed[CRUDE] += got;
	if(owner == sellb->owner)
		sellb->recenth.consumed[CRUDE] += got;
	stock[CRUDE] += got;

#ifdef LOCAL_TRANSX
		NewTransx(sellb->pos, CRUDE, -got);
#else
	//b->Emit(DOLLAR);
	if(owner == g_localP && sellb->owner != owner)
		NewTransx(sellb->pos, CRUDE, got, CURRENC, -paid);
	else if(sellb->owner == g_localP && sellb->owner != owner)
		NewTransx(sellb->pos, CURRENC, paid, CRUDE, -got);
#endif

	return true;
}


void CBuilding::process()
{
	if(excessoutput())
		return;

	CBuildingType* t = &g_buildingType[type];
	CPlayer* p = &g_player[owner];

	if(stock[ELECTRICITY] < t->input[ELECTRICITY])
		BuyE();
	
	if(stock[CRUDE] < t->input[CRUDE])
	{
		//Chat("buy cr");
		BuyCrude();
	}

	float netch[RESOURCES];

	if(!TrySubtract(t->input, p->global, stock, p->local, netch))
		return;

	char transx[64];
	transx[0] = '\0';

	CResource* res;
	for(int r=0; r<RESOURCES; r++)
	{
		recenth.consumed[r] += netch[r];

		if(t->output[r] <= 0.0f)
			continue;

		stock[r] += t->output[r];
		p->local[r] += t->output[r];
		quotafilled[r] += t->output[r];
		recenth.produced[r] += t->output[r];

		res = &g_resource[r];

		if(owner == g_localP && !res->capacity)
			TransxAppend(transx, r, t->output[r]);
	}
	
#ifdef LOCAL_TRANSX
	if(owner == g_localP)
#endif
	{
		for(int r=0; r<RESOURCES; r++)
		{
			if(t->input[r] <= 0.0f)
				continue;

			TransxAppend(transx, r, -t->input[r]);
		}
		
		if(transx[0] != '\0')
			NewTransx(pos + Vec3f(0, 5, 0), transx);
	}

	//if(owner == g_localP && g_selectType == SELECT_BUILDING && g_selection[0] == BuildingID(this))
	//	RedoLeftPanel();
}

bool CBuilding::CheckConstruction()
{
	CBuildingType* t = &g_buildingType[type];

	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < t->cost[i])
			return false;

	if(owner == g_localP)
	{
		char msg[128];
		sprintf(msg, "%s construction complete.", t->name);
		Chat(msg);
		ConCom();
	}

	finished = true;

	//if(owner == g_localP && g_selectType == SELECT_BUILDING && g_selection[0] == BuildingID(this))
	//	RedoLeftPanel();

	RePow();
	RePipe();
	ReRoadNetw();
	
	if(owner == g_localP)
		OnFinishedB(type);

	return true;
}

void CBuilding::addoccupier(CUnit* u)
{
	int id = UnitID(u);
	occupier.push_back(id);
}

void CBuilding::RemoveWorker(CUnit* u)
{
	for(int i=0; i<occupier.size(); i++)
	{
		if(&g_unit[occupier[i]] != u)
			continue;

		occupier.erase( occupier.begin() + i );

		return;
	}
}

bool CBuilding::Working(CUnit* u)
{
	for(int i=0; i<occupier.size(); i++)
	{
		if(&g_unit[occupier[i]] == u)
			return true;
	}

	return false;
}

void CBuilding::destroy()
{
	CUnit* u;
	int thisID = BuildingID(this);

	cycleh.clear();
	recenth.reset();

	CBuildingType* bt = &g_buildingType[type];

	if(bt->output[HOUSING] > 0.0f)
	{
		for(int i=0; i<occupier.size(); i++)
		{
			u = &g_unit[i];
			u->home = -1;
		}
	}

	occupier.clear();
	emitter.clear();
	roadnetw.clear();
	pownetw = -1;
	pipenetw = -1;
	Zero(quotafilled);
	Zero(stock);
	Zero(conmat);
	prodquota = 1;

	CTrigger* t = g_scripthead;
	while(t)
	{
		for(int i=0; i<t->conditions.size(); i++)
		{
			CCondition* c = &t->conditions[i];
			if(c->type == CONDITION_CAPTUREB && c->target == thisID)
				c->target = -1;
		}
		t = t->next;
	}

	freecollidercells();

	on = false;
	finished = false;
}

void CBuilding::fillcollidercells()
{
	int bID = BuildingID(this);

	CBuildingType* bt = &g_buildingType[type];
	float hwx = bt->widthX*TILE_SIZE/2.0f;
	float hwz = bt->widthZ*TILE_SIZE/2.0f;
		
	float fstartx = pos.x-hwx;
	float fstartz = pos.z-hwz;
	float fendx = pos.x+hwx;
	float fendz = pos.z+hwz;
		
	//int extentx = ceil(r/cellwx);
	//int extentz = ceil(r/cellwz);
	const float cellwx = MIN_RADIUS*2.0f;
	const float cellwz = MIN_RADIUS*2.0f;
		
	int startx = max(0, fstartx/cellwx);
	int startz = max(0, fstartz/cellwz);
	int endx = min(ceil(fendx/cellwx), g_hmap.m_widthX*TILE_SIZE/cellwx-1);
	int endz = min(ceil(fendz/cellwz), g_hmap.m_widthZ*TILE_SIZE/cellwz-1);
	//int endx = startx+extentx;
	//int endz = startz+extentz;

	for(int x=startx; x<endx; x++)
		for(int z=startz; z<endz; z++)
		{
			ColliderCell* cell = ColliderCellAt(x, z);
			cell->colliders.push_back(Collider(COLLIDER_BUILDING, bID));
		}
}

void CBuilding::freecollidercells()
{
	int bID = BuildingID(this);

	CBuildingType* bt = &g_buildingType[type];
	float hwx = bt->widthX*TILE_SIZE/2.0f;
	float hwz = bt->widthZ*TILE_SIZE/2.0f;
		
	float fstartx = pos.x-hwx;
	float fstartz = pos.z-hwz;
	float fendx = pos.x+hwx;
	float fendz = pos.z+hwz;
		
	//int extentx = ceil(r/cellwx);
	//int extentz = ceil(r/cellwz);
	const float cellwx = MIN_RADIUS*2.0f;
	const float cellwz = MIN_RADIUS*2.0f;
		
	int startx = max(0, fstartx/cellwx);
	int startz = max(0, fstartz/cellwz);
	int endx = min(ceil(fendx/cellwx), g_hmap.m_widthX*TILE_SIZE/cellwx-1);
	int endz = min(ceil(fendz/cellwz), g_hmap.m_widthZ*TILE_SIZE/cellwz-1);
	//int endx = startx+extentx;
	//int endz = startz+extentz;

	for(int x=startx; x<endx; x++)
		for(int z=startz; z<endz; z++)
		{
			ColliderCell* cell = ColliderCellAt(x, z);
			//cell->colliders.push_back(Collider(COLLIDER_UNIT, uID));
			for(int i=0; i<cell->colliders.size(); i++)
			{
				Collider* c = &cell->colliders[i];
				if(c->type == COLLIDER_BUILDING && c->ID == bID)
				{
					cell->colliders.erase( cell->colliders.begin() + i );
					break;
				}
			}
		}
}

int BuildingID(CBuilding* b)
{
	for(int i=0; i<BUILDINGS; i++)
		if(&g_building[i] == b)
			return i;

	return -1;
}

void InitBuildings()
{
	//transport = 6*2 = 12

	//		Building	Name					Model									Construction model							wX wZ	HP		Qta
	BDefine(APARTMENT,	"Apartment",		"models\\apartment\\apartment.ms3d",		"models\\apartment\\apartment_c.ms3d",		2, 2,	100,	1);
	BCost(LABOUR, 4);
	BCost(CEMENT, 4);
	BCost(STONE, 2);
	//BIn(ELECTRICITY, 1);
	BOut(HOUSING, 8);	// = 1

	BDefine(SHOPCPLX,	"Shopping Complex",	"models\\shopcplx\\shopcplx.ms3d",			"models\\shopcplx\\shopcplx_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(ELECTRONICS, 2);
	BIn(LABOUR, 10);		//+10
	BIn(PRODUCE, 5);		//+70
	BIn(PRODUCTION, 5);		//+15
	BIn(ELECTRICITY, 3);	//+6
	BOut(CONSUMERGOODS, 39);	// 101/10+1+2*12 = 35.1

	BDefine(FACTORY,	"Factory",			"models\\factory\\factory.ms3d",			"models\\factory\\factory_c.ms3d",			2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 2);
	BCost(CEMENT, 3);
	BCost(ELECTRONICS, 10);
	BIn(LABOUR, 10);		//+10
	BIn(ELECTRICITY, 10);	//+20
	BIn(CHEMICALS, 3);		//+12
	BIn(METAL, 3);
	BOut(PRODUCTION, 18);	// 32/20+1+1*12 = 15
	BBuildable(TRUCK);
	BBuildable(BATTLECOMP);
	BBuildable(CARLYLE);

	BDefine(FARM,		"Farm",				"models\\farm\\farm.ms3d",					"models\\farm\\farm_c.ms3d",				4, 2,	100,	10);
	BCost(LABOUR, 4);
	BIn(LABOUR, 10);		//+10
	BIn(ELECTRICITY, 1);	//+2
	BIn(CHEMICALS, 1);		//+4
	BOut(PRODUCE, 5);		//16/5+1+1*12 = 16

	BDefine(MINE,		"Mine",				"models\\mine\\mine.ms3d",					"models\\mine\\mine_c.ms3d",				2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	//BCost(METAL, 4);
	BIn(LABOUR, 10);		//+10
	BOut(ORE, 5);			// 10/15+1 = 2
	BOut(URONIUM, 5);
	BOut(COHL, 5);

	BDefine(SMELTER,	"Smelter",			"models\\smelter\\smelter.ms3d",			"models\\smelter\\smelter_c.ms3d",			2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BIn(LABOUR, 10);		//+10
	BIn(ELECTRICITY, 10);	//+20
	BIn(ORE, 5);			//+10
	BIn(CHEMICALS, 10);		//+40
	BOut(METAL, 5);			// 80/5+1+2*12 = 41
	BEmitter(EXHAUST, Vec3f(-7.0f, 23.2f, -11.5f));
	BEmitter(EXHAUST, Vec3f(-12.1f, 23.2f, -6.0f));

	BDefine(DERRICK,	"Derrick",			"models\\derrick\\derrick.ms3d",			"models\\derrick\\derrick_c.ms3d",			1, 1,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 3);
	BIn(LABOUR, 10);	//+10
	BOut(CRUDE, 3);		// 10/3+1 = 4

	BDefine(REFINERY,	"Refinery",			"models\\refinery\\refinery.ms3d",			"models\\refinery\\refinery_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 3);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BIn(LABOUR, 1);			//+1
	BIn(ELECTRICITY, 1);	//+2
	BIn(CHEMICALS, 1);		//+4
	BIn(CRUDE, 3);			//+4
	BOut(ZETROL, 21);		// 11/7+1+1*12 = 17
	BEmitter(EXHAUST, Vec3f(-4.0f, 22.4f, -6.5f));
	BEmitter(EXHAUST, Vec3f(8.9f, 22.4f, 9.0f));

	BDefine(REACTOR,	"Reactor",			"models\\reactor\\reactor.ms3d",			"models\\reactor\\reactor_c.ms3d",			2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(ELECTRONICS, 3);
	BIn(LABOUR, 10);	//+10
	BIn(URONIUM, 1);	//+2
	BOut(ELECTRICITY, 10);	// 12/10+1+1*12 = 14
	BEmitter(EXHAUSTBIG, Vec3f(1.8f, 15.69f, -7.99f));
	BEmitter(EXHAUSTBIG, Vec3f(-8.0f, 15.69f, 2.35f));

	BDefine(COMBUSTOR,	"Combustor",		"models\\combustor\\combustor.ms3d",		"models\\combustor\\combustor_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(ELECTRONICS, 1);
	BIn(LABOUR, 10);	//+10
	BIn(COHL, 1);		//+2
	BOut(ELECTRICITY, 10);	// 12/10+1+1*12 = 14
	BEmitter(EXHAUST, Vec3f(-8.7f, 23.1f, -11.3f));
	BEmitter(EXHAUST, Vec3f(-10.7f, 23.1f, -8.8f));
	BEmitter(EXHAUST, Vec3f(-10.3f, 23.1f, 9.6f));
	BEmitter(EXHAUST, Vec3f(-8.4f, 23.1f, 11.8f));

	BDefine(QUARRY,		"Quarry",			"models\\quarry\\quarry.ms3d",				"models\\quarry\\quarry_c.ms3d",			2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 3);
	BIn(LABOUR, 10);	//+10
	BOut(STONE, 5);		// 10/5+1 = 3

	BDefine(CEMPLANT,	"Cement Plant",		"models\\cemplant\\cemplant.ms3d",			"models\\cemplant\\cemplant_c.ms3d",		1, 1,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 3);
	BIn(LABOUR, 10);	//+10
	BIn(STONE, 5);		//+15
	BOut(CEMENT, 5);	//  (25+1*12)/5+1 = 18
	
	BDefine(CHEMPLANT,	"Chemical Plant",	"models\\chemplant\\chemplant.ms3d",		"models\\chemplant\\chemplant_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 2);
	BCost(STONE, 1);
	BCost(ELECTRONICS, 1);
	BCost(METAL, 1);
	BIn(LABOUR, 10);		//+10
	BIn(ELECTRICITY, 10);	//+20
	BOut(CHEMICALS, 42);	// 30/10+1 = 4

	BDefine(ELECPLANT,	"Electronics Plant", "models\\elecplant\\elecplant.ms3d",		"models\\elecplant\\elecplant_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(METAL, 3);
	BIn(LABOUR, 1);		//+10
	BIn(ELECTRICITY, 1);	//+20
	BIn(METAL, 1);			//+27
	BIn(CHEMICALS, 1);		//+40
	BOut(ELECTRONICS, 25);	// 97/5+1+2*12 = 45
	
	BDefine(RNDFACILITY,	"R&D Facility",		"models\\rndfac\\rndfac.ms3d",				"models\\rndfac\\rndfac_c.ms3d",			2, 2,	100,	1);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(METAL, 1);
	BIn(LABOUR, 1);			//+10
	BIn(ELECTRICITY, 1);		//+20
	BIn(METAL, 1);				//+27
	BIn(ELECTRONICS, 1);		//+21
	BIn(CHEMICALS, 1);			//+40
	BOut(RESEARCH, 1);			// 118/1+1+3*12 = 155

	//CURRENC, LABOUR, CONSUMERGOODS, PRODUCE, PRODUCTION, ORE, METAL, CRUDE, ZETROL, STONE, CEMENT, ELECTRICITY, URONIUM, COHL, CHEMICALS, ELECTRONICS, RESOURCES
	
	//g_buildingType[FACTORY].Emitter(EXHAUST, Vec3f(2.929f, 12.625f, -5.5f));
	//g_buildingType[FACTORY].Emitter(EXHAUST, Vec3f(5.429f, 12.625f, -5.5f));
	//g_buildingType[FACTORY].Emitter(HAMMER, Vec3f(0.0f, 5.0f, 0.0f));
}

void LoadHoverTex()
{
	QueueTexture(&g_hovertex[HOVER_NOELEC], "gui\\hover\\noelec.png", true);
	QueueTexture(&g_hovertex[HOVER_NOCRUDE], "gui\\hover\\nocrude.png", true);
	QueueTexture(&g_hovertex[HOVER_NOROAD], "gui\\hover\\noroad.png", true);
}

void BSprites()
{
	BSprite(APARTMENT, "gui\\centerp\\apartmenticon.png");
	BSprite(SHOPCPLX, "gui\\centerp\\shopcplxicon.png");
	BSprite(FACTORY, "gui\\centerp\\factoryicon.png");
	BSprite(FARM, "gui\\centerp\\farmicon.png");
	BSprite(MINE, "gui\\centerp\\mineicon.png");
	BSprite(SMELTER, "gui\\centerp\\smeltericon.png");
	BSprite(DERRICK, "gui\\centerp\\derrickicon.png");
	BSprite(REFINERY, "gui\\centerp\\refineryicon.png");
	BSprite(REACTOR, "gui\\centerp\\reactoricon.png");
	BSprite(COMBUSTOR, "gui\\centerp\\combustoricon.png");
	BSprite(QUARRY, "gui\\centerp\\quarryicon.png");
	BSprite(CEMPLANT, "gui\\centerp\\cemplanticon.png");
	BSprite(CHEMPLANT, "gui\\centerp\\chemplanticon.png");
	BSprite(ELECPLANT, "gui\\centerp\\elecplanticon.png");
	BSprite(RNDFACILITY, "gui\\centerp\\rndfacicon.png");
}

void DrawSelB()
{
	if(g_build >= BUILDING_TYPES || g_build < 0)
		return;

	//g_log<<"g_build = "<<g_build<<endl;
	//g_log.flush();

	CBuildingType* t = &g_buildingType[g_build];

	Shader* s = &g_shader[g_curS];

	const float* owncol;

	if(g_mode == EDITOR)	owncol = facowncolor[g_selP];
	else					owncol = facowncolor[g_localP];

	glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);

	if(g_canPlace)
        glUniform4f(s->m_slot[SLOT::COLOR], 1, 1, 1, 0.5f);
		//glColor4f(1,1,1,0.5f);
	else
        glUniform4f(s->m_slot[SLOT::COLOR], 1, 0, 0, 0.5f);
		//glColor4f(1,0,0,0.5f);
	t->draw(0, true, g_vTile);
	//glColor4f(1,1,1,1.0f);
	glUniform4f(s->m_slot[SLOT::COLOR], 1, 1, 1, 1);
}

void DrawBuildings()
{
	StartProfile(DRAWBUILDINGS);
	for(int i=0; i<BUILDINGS; i++)
		if(g_building[i].on)
			g_building[i].draw();
	EndProfile(DRAWBUILDINGS);
}

void DrawBStatus(Matrix projmodlview)
{
	Vec4f screenpos;
	string status;
	Vec4f imgpos;
	CFont* f = &g_font[MAINFONT8];
	const float color[] = {1,1,1,STATUS_ALPHA};

	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(!b->on)
			continue;

		screenpos.x = b->pos.x;
		screenpos.y = b->pos.y;
		screenpos.z = b->pos.z;
		screenpos.w = 1;

		screenpos.Transform(projmodlview);
		screenpos = screenpos / screenpos.w;
		screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
		screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;

		if(b->finished)
		{
			int secsleft = (CYCLE_FRAMES - b->framesago)/FRAME_RATE;
			char secstr[64];
			sprintf(secstr, "%c%d", TIME_CHAR, secsleft);
			//sprintf(secstr, "%c%d pipe=%d", TIME_CHAR, secsleft, b->pipenetw);

			CBuildingType* t = &g_buildingType[b->type];
			if(t->output[HOUSING] > 0.0f)
			{
				char add[32];
				sprintf(add, " %c%d/%d", g_resource[HOUSING].icon, (int)b->occupier.size(), (int)t->output[HOUSING]);
				strcat(secstr, add);
			}

			DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y-f->gheight, secstr, color);
		}
		else
		{	
			CBuildingType* t = &g_buildingType[b->type];

			char st[32];
			st[0] = '\0';
			for(int i=0; i<RESOURCES; i++)
			{
				if(t->cost[i] <= 0.0f)
					continue;

				char add[16];
				sprintf(add, "%c%d/%d", g_resource[i].icon, (int)b->conmat[i], (int)t->cost[i]);
				strcat(st, add);
			}
			DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y-f->gheight, st, color);
		}

#ifdef LOCAL_TRANSX
		if(b->owner == g_localP)
#endif
		{
			CBuildingType* t = &g_buildingType[b->type];

			if(b->roadnetw.size() <= 0 ||
				(b->pipenetw < 0 && t->input[CRUDE] > 0.0f) ||
				(b->pownetw < 0 && t->input[ELECTRICITY] > 0.0f))
			{
		
				screenpos.x = b->pos.x;
				screenpos.y = b->pos.y + TILE_SIZE;
				screenpos.z = b->pos.z;
				screenpos.w = 1;

				screenpos.Transform(projmodlview);
				screenpos = screenpos / screenpos.w;
				screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
				screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;

				imgpos = screenpos;
				imgpos.y -= HOVER_SIZE/2;

				status = "";

				if(b->roadnetw.size() <= 0)
				{
					status = "No road";
					imgpos.x -= HOVER_SIZE/2;
				}

				if(b->pipenetw < 0 && t->input[CRUDE] > 0.0f)
				{
					if(status.c_str()[0] != '\0')
						status += "; ";
					status += "No crude";
					imgpos.x -= HOVER_SIZE/2;
				}

				if(b->pownetw < 0 && t->input[ELECTRICITY] > 0.0f)
				{
					if(status.c_str()[0] != '\0')
						status += "; ";
					status += "No power";
					imgpos.x -= HOVER_SIZE/2;
				}

				if(b->roadnetw.size() <= 0)
				{
					DrawImage(g_hovertex[HOVER_NOROAD], imgpos.x, imgpos.y, imgpos.x + HOVER_SIZE, imgpos.y + HOVER_SIZE);
					imgpos.x += HOVER_SIZE;
				}
		
				if(b->pipenetw < 0 && t->input[CRUDE] > 0.0f)
				{
					DrawImage(g_hovertex[HOVER_NOCRUDE], imgpos.x, imgpos.y, imgpos.x + HOVER_SIZE, imgpos.y + HOVER_SIZE);
					imgpos.x += HOVER_SIZE;
				}
		
				if(b->pownetw < 0 && t->input[ELECTRICITY] > 0.0f)
				{
					DrawImage(g_hovertex[HOVER_NOELEC], imgpos.x, imgpos.y, imgpos.x + HOVER_SIZE, imgpos.y + HOVER_SIZE);
					//imgpos.x += HOVER_SIZE;
				}

				/*
				pos.Transform(projmodlview);
				pos.x = (pos.x * 0.5f + 0.5f) * g_width;
				pos.y = (pos.y * 0.5f + 0.5f) * g_height;*/
		
				DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, status.c_str(), color);
			}
		}

		if(b->forsale)
		{
			screenpos.x = b->pos.x;
			screenpos.y = b->pos.y;
			screenpos.z = b->pos.z;
			screenpos.w = 1;

			screenpos.Transform(projmodlview);
			screenpos = screenpos / screenpos.w;
			screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
			screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height - f->gheight*2;

			char salestr[32];
			sprintf(salestr, "FOR SALE %c%0.2f", g_resource[CURRENC].icon, b->saleprice);
			DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, salestr, color);
		}
	}

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CRoad* road = RoadAt(x, z);

			if(!road->on)
				continue;

			if(road->finished)
				continue;

#ifdef LOCAL_TRANSX
			if(road->owner != g_localP)
				continue;
#endif
			
			Vec3f pos = RoadPosition(x, z);

			screenpos.x = pos.x;
			screenpos.y = pos.y;
			screenpos.z = pos.z;
			screenpos.w = 1;
			
			screenpos.Transform(projmodlview);
			screenpos = screenpos / screenpos.w;
			screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
			screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;
			
			char st[32];
			st[0] = '\0';
			for(int i=0; i<RESOURCES; i++)
			{
				if(g_roadCost[i] <= 0.0f)
					continue;

				char add[16];
				sprintf(add, "%c%d/%d", g_resource[i].icon, (int)road->conmat[i], (int)g_roadCost[i]);
				strcat(st, add);
			}
			DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y+f->gheight, st, color);
		}

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CPowerline* powl = PowlAt(x, z);

			if(!powl->on)
				continue;

			if(powl->finished)
				continue;
			
#ifdef LOCAL_TRANSX
			if(powl->owner != g_localP)
				continue;
#endif

			Vec3f pos = PowerlinePosition(x, z);

			screenpos.x = pos.x;
			screenpos.y = pos.y + TILE_SIZE;
			screenpos.z = pos.z;
			screenpos.w = 1;
			
			screenpos.Transform(projmodlview);
			screenpos = screenpos / screenpos.w;
			screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
			screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;
			
			if(!PowlHasRoad(x, z))
			{
				DrawImage(g_hovertex[HOVER_NOROAD], screenpos.x-HOVER_SIZE/2, screenpos.y-HOVER_SIZE/2, screenpos.x+HOVER_SIZE/2, screenpos.y+HOVER_SIZE/2);

				DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, "No road", color);
			}

			screenpos.x = pos.x;
			screenpos.y = pos.y;
			screenpos.z = pos.z;
			screenpos.w = 1;
			
			screenpos.Transform(projmodlview);
			screenpos = screenpos / screenpos.w;
			screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
			screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;

			char st[32];
			st[0] = '\0';
			for(int i=0; i<RESOURCES; i++)
			{
				if(g_powerlineCost[i] <= 0.0f)
					continue;

				char add[16];
				sprintf(add, "%c%d/%d", g_resource[i].icon, (int)powl->conmat[i], (int)g_powerlineCost[i]);
				strcat(st, add);
			}
			DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y+f->gheight, st, color);
		}

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CPipeline* pipe = PipeAt(x, z);

			if(!pipe->on)
				continue;

			if(pipe->finished)
				continue;
			
#ifdef LOCAL_TRANSX
			if(pipe->owner != g_localP)
				continue;
#endif
			
			Vec3f pos = PipelinePhysPos(x, z);

			screenpos.x = pos.x;
			screenpos.y = pos.y + TILE_SIZE;
			screenpos.z = pos.z;
			screenpos.w = 1;
			
			screenpos.Transform(projmodlview);
			screenpos = screenpos / screenpos.w;
			screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
			screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;
			
			if(!PipeHasRoad(x, z))
			{
				DrawImage(g_hovertex[HOVER_NOROAD], screenpos.x-HOVER_SIZE/2, screenpos.y-HOVER_SIZE/2, screenpos.x+HOVER_SIZE/2, screenpos.y+HOVER_SIZE/2);

				DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, "No road", color);
			}

			screenpos.x = pos.x;
			screenpos.y = pos.y;
			screenpos.z = pos.z;
			screenpos.w = 1;
			
			screenpos.Transform(projmodlview);
			screenpos = screenpos / screenpos.w;
			screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
			screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;
			
			char st[32];
			st[0] = '\0';
			for(int i=0; i<RESOURCES; i++)
			{
				if(g_pipelineCost[i] <= 0.0f)
					continue;

				char add[16];
				sprintf(add, "%c%d/%d", g_resource[i].icon, (int)pipe->conmat[i], (int)g_pipelineCost[i]);
				strcat(st, add);
			}
			//sprintf(st, "netw=%d", pipe->netw);
			DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y+f->gheight, st, color);
		}
}

void DrawBStats(Matrix projmodlview)
{
	CBuilding* b;
	Vec3f pos, v;
	string status;
	Vec3f strafe = g_camera.Strafe();
	CPlayer* p = &g_player[g_localP];
	Vec4f screenpos;

	for(int i=0; i<BUILDINGS; i++)
	{
		b = &g_building[i];

		if(!b->on)
			continue;

		pos = b->pos;

		screenpos.x = pos.x;
		screenpos.y = pos.y;
		screenpos.z = pos.z;
		screenpos.w = 1;

		screenpos.Transform(projmodlview);
		screenpos = screenpos / screenpos.w;
		screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
		screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;

		CBuildingType* t = &g_buildingType[b->type];

		//if(u->hp < t->hitpoints)
		{
			const float w = t->hitpoints;
			const float h = 2;
				
			DrawSquare(1, 0, 0, 0.75f, screenpos.x - w/2, screenpos.y + 15, screenpos.x + w/2, screenpos.y + 15 + h);
			DrawSquare(0, 1, 0, 0.75f, screenpos.x - w/2, screenpos.y + 15, screenpos.x - w/2 + b->hp, screenpos.y + 15 + h);
		}
	}
}

bool CheckCanPlace(int btype, Vec3f vTile)
{
	if(BuildingCollides(btype, vTile))
		return false;

	if(!BuildingLevel(btype, vTile))
		return false;

	return true;
}

int NewBuilding()
{
	for(int i=0; i<BUILDINGS; i++)
		if(!g_building[i].on)
			return i;

	return -1;
}

int PlaceB(int player, int btype, Vec3f vTile)
{	
	if(!CheckCanPlace(btype, vTile))
		return -1;

	int i = NewBuilding();

	if(i < 0)
		return -1;

	g_building[i].set(btype, player, vTile);

	if(g_mode == PLAY)
		g_building[i].Allocate();
	else if(g_mode == EDITOR)
		g_building[i].finished = true;
	
	g_building[i].fillcollidercells();

	ClearPowerlines();
	ClearPipelines();
	RePow();
	RePipe();
	ReRoadNetw();
	
	CBuildingType* t = &g_buildingType[btype];
	ClearScenery(vTile.x, vTile.z, t->widthX*TILE_SIZE, t->widthZ*TILE_SIZE);

	if(g_mode == PLAY)
	{
		NewJob(GOINGTOCONJOB, i, -1);
	}

	return i;
}

void CBuilding::update()
{
	if(!finished)
		return;

	EmitParticles();

	if(framesago >= CYCLE_FRAMES)
		cycle();
	UpdateQueueU();
	
	process();	//needs to be called after cycle();

	framesago ++;
}

float CBuilding::netreq(int res)
{
	CBuildingType* t = &g_buildingType[type];
	
	float netrq = 0;

	if(finished)
	{
		float numfilled = 0;

		for(int res1=0; res1<RESOURCES; res1++)
		{
			CResource* r = &g_resource[res1];

			if(r->capacity)
				continue;

			if(t->output[res1] <= 0.0f)
				continue;

			float filled = quotafilled[res1] / t->output[res1];

			if(filled > 0.0f)
			{
				numfilled = filled;
			}
		}

		CPlayer* p = &g_player[owner];
		netrq = t->input[res] * prodquota - stock[res];

		if(!g_resource[res].capacity && res != LABOUR)
			netrq -= p->global[res];
		//netrq = t->input[res] * prodquota - stock[res];

		netrq -= t->input[res] * numfilled;
		float manufreq[RESOURCES];
		Zero(manufreq);

		for(int i=0; i<produ.size(); i++)
		{
			ManufJob *mj = &produ[i];
			CUnitType* ut = &g_unitType[mj->utype];
			netrq += ut->cost[res];

			for(int j=0; j<RESOURCES; j++)
			{
				if(j == res)
					continue;

				if(t->output[j] > 0.0f && ut->cost[j] > 0.0f)
				{
					manufreq[j] += ut->cost[j];
				}
			}
		}

		float reqprodlevel = 0;

		for(int j=0; j<RESOURCES; j++)
		{
			if(j == res)
				continue;

			if(t->output[j] > 0.0f && manufreq[j] > 0.0f)
			{
				manufreq[j] -= p->global[j];
				manufreq[j] -= stock[j];

				float prodlevel = manufreq[j] / t->output[j];

				if(prodlevel > reqprodlevel)
					reqprodlevel += prodlevel;
			}
		}

		netrq += reqprodlevel * t->input[res];
		
		//if(res == ORE && type == SMELTER)
		//if(res == ORE && type == SMELTER)
		/*if(type == FACTORY)
		{
			char msg[128];
			sprintf(msg, "(%s)req=%f=%f*%f - %f - %f - %f*%f", g_resource[res].name, netrq, t->input[res], prodquota, stock[res], p->global[res], t->input[res], numfilled);
			Chat(msg);
			g_log<<msg<<endl;
			g_log.flush();
		}*/
	}
	else
	{
		netrq = t->cost[res] - conmat[res];
	}

	return netrq;
}

void CBuilding::cycle()
{
	framesago = 0;

	CBuildingType* t = &g_buildingType[type];
	CPlayer* p = &g_player[owner];
	CUnit* u;
	CResource* r;
	/*
	bool prodscap = false;
	for(int i=0; i<RESOURCES; i++)
	{
		r = &g_resource[i];

		if(r->capacity && t->output[i] > 0.0f)
		{
			prodscap = true;
			break;
		}
	}

	bool quotanotmet = false;
	if(prodscap)
	{
		for(int i=0; i<RESOURCES; i++)
		{
			if(quotafilled[i] < t->output[i] * prodquota)
			{
				quotanotmet = true;
				break;
			}
		}
	}*/
	
	int thisID = BuildingID(this);

	char transx[32];
	transx[0] = '\0';

	float change[RESOURCES];
	Zero(change);

	if(type == APARTMENT)
	{
		for(int i=0; i<occupier.size(); i++)
		{
			u = &g_unit[occupier[i]];

			if(u->currency < p->price[HOUSING])
			{
				// evict
				u->home = -1;

				if(u->mode == GOINGTOREST || u->mode == RESTING)
					u->ResetMode();

				occupier.erase( occupier.begin() + i );
				i--;

				//if(owner == g_localP)
				Chat("Eviction");

				char msg[128];
				sprintf(msg, "eviction %f < %f", u->currency, p->price[HOUSING]);
				LogTransx(owner, 0.0f, msg);
			}
			else
			{
				u->currency -= p->price[HOUSING];
				p->global[CURRENC] += p->price[HOUSING];
				change[CURRENC] += p->price[HOUSING];
				recenth.produced[CURRENC] += p->price[HOUSING];
				p->recentph[HOUSING].earnings += p->price[HOUSING];

				LogTransx(owner, p->price[HOUSING], "rent");
			}
		}
	}

	if(t->output[HOUSING] > 0.0f)
	{
		recenth.consumed[HOUSING] = occupier.size();
	}

	cycleh.push_back(recenth);
	recenth.reset();

	if(owner == g_localP)
	{
		if(change[CURRENC] > 0.0f)
			TransxAppend(transx, CURRENC, change[CURRENC]);

		for(int i=0; i<RESOURCES; i++)
		{
			if(i == CURRENC)
				continue;
		
			if(owner == g_localP && change[i] != 0.0f)
				TransxAppend(transx, i, change[i]);
		}
	}

	if(transx[0] != '\0'
#ifdef LOCAL_TRANSX
		&& owner == g_localP
#endif
		)
		NewTransx(pos, transx);

	for(int i=0; i<RESOURCES; i++)
	{
		quotafilled[i] = 0;

		r = &g_resource[i];

		if(r->capacity && t->output[i] > 0.0f)
			stock[i] = 0;
	}

	RDelta(owner);
}

void UpdateBuildings()
{
	StartProfile(UPDATEBUILDINGS);

	for(int i=0; i<BUILDINGS; i++)
		if(g_building[i].on)
			g_building[i].update();

	EndProfile(UPDATEBUILDINGS);
}
