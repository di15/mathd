

#include "save.h"
#include "unit.h"
#include "building.h"
#include "map.h"
#include "main.h"
#include "tileset.h"
#include "player.h"
#include "powerline.h"
#include "pipeline.h"
#include "road.h"
#include "transaction.h"
#include "projectile.h"
#include "particle.h"
#include "selection.h"
#include "script.h"
#include "editor.h"
#include "chat.h"
#include "scenery.h"

const std::string currentDateTime() 
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

void SaveScenery(int i, FILE* fp)
{
	Scenery* s = &g_scenery[i];

	fwrite(&s->on, sizeof(bool), 1, fp);

	if(!s->on)
		return;

	/*
	bool on;
	int type;
	Vec3f pos;
	float yaw;
	*/
	
	fwrite(&s->type, sizeof(int), 1, fp);
	fwrite(&s->pos, sizeof(Vec3f), 1, fp);
	fwrite(&s->yaw, sizeof(float), 1, fp);
}

void ReadScenery(int i, FILE* fp)
{
	Scenery* s = &g_scenery[i];

	fread(&s->on, sizeof(bool), 1, fp);

	if(!s->on)
		return;
	
	fread(&s->type, sizeof(int), 1, fp);
	fread(&s->pos, sizeof(Vec3f), 1, fp);
	fread(&s->yaw, sizeof(float), 1, fp);
}

void SaveUnit(int i, FILE* fp)
{
	CUnit* u = &g_unit[i];
	
	fwrite(&u->on, sizeof(bool), 1, fp);

	if(!u->on)
		return;

	/*
	bool on;
	int owner;
	int type;
	Camera camera;
	int frame;
	int mode;
	Vec3f goal;
	Vec3f subgoal;
	deque<Vec3f> path;
	int step;
	int target;
	int target2;
	bool targetU;
	bool underOrder;
	int fuelStation;
	float currency;
	float fuel;
	float labour;
	int transportRes;
	float transportAmt;
	int driver;
	//long last;
	float hp;
	bool passive;
	float accum;
	Vec3f oldPos;
	int framesleft;
	int home;
	bool nopath;
	int frameslookjobago;
	int supplier;
	vector<TransportJob> bids;
	float reqAmt;
	int targtype;
	*/

	fwrite(&u->owner, sizeof(int), 1, fp);
	fwrite(&u->type, sizeof(int), 1, fp);
	fwrite(&u->camera, sizeof(Camera), 1, fp);
	fwrite(u->frame, sizeof(float), 2, fp);
	fwrite(&u->mode, sizeof(int), 1, fp);
	fwrite(&u->goal, sizeof(Vec3f), 1, fp);
	fwrite(&u->subgoal, sizeof(Vec3f), 1, fp);
	int path_size = u->path.size();
	fwrite(&path_size, sizeof(int), 1, fp);
	for(int j=0; j<path_size; j++)
		fwrite(&u->path[j], sizeof(Vec3f), 1, fp);
	fwrite(&u->step, sizeof(int), 1, fp);
	fwrite(&u->target, sizeof(int), 1, fp);
	fwrite(&u->target2, sizeof(int), 1, fp);
	fwrite(&u->targetU, sizeof(bool), 1, fp);
	fwrite(&u->underOrder, sizeof(bool), 1, fp);
	fwrite(&u->fuelStation, sizeof(int), 1, fp);
	fwrite(&u->currency, sizeof(float), 1, fp);
	fwrite(&u->fuel, sizeof(float), 1, fp);
	fwrite(&u->labour, sizeof(float), 1, fp);
	fwrite(&u->transportRes, sizeof(int), 1, fp);
	fwrite(&u->transportAmt, sizeof(float), 1, fp);
	fwrite(&u->driver, sizeof(int), 1, fp);
	//fwrite(&u->last, sizeof(long), 1, fp);
	fwrite(&u->hp, sizeof(float), 1, fp);
	fwrite(&u->passive, sizeof(bool), 1, fp);
	fwrite(&u->accum, sizeof(float), 1, fp);
	fwrite(&u->oldPos, sizeof(Vec3f), 1, fp);
	fwrite(&u->framesleft, sizeof(int), 1, fp);
	fwrite(&u->home, sizeof(int), 1, fp);
	fwrite(&u->nopath, sizeof(bool), 1, fp);
	fwrite(&u->frameslookjobago, sizeof(int), 1, fp);
	fwrite(&u->supplier, sizeof(int), 1, fp);
	fwrite(&u->reqAmt, sizeof(float), 1, fp);
	fwrite(&u->targtype, sizeof(int), 1, fp);
}

void ReadUnit(int i, FILE* fp)
{
	CUnit* u = &g_unit[i];
	
	fread(&u->on, sizeof(bool), 1, fp);

	if(!u->on)
		return;

	fread(&u->owner, sizeof(int), 1, fp);
	fread(&u->type, sizeof(int), 1, fp);
	fread(&u->camera, sizeof(Camera), 1, fp);
	fread(u->frame, sizeof(float), 2, fp);
	fread(&u->mode, sizeof(int), 1, fp);
	fread(&u->goal, sizeof(Vec3f), 1, fp);
	fread(&u->subgoal, sizeof(Vec3f), 1, fp);
	int path_size;
	fread(&path_size, sizeof(int), 1, fp);
	Vec3f step;
	for(int j=0; j<path_size; j++)
	{
		fread(&step, sizeof(Vec3f), 1, fp);
		u->path.push_back(step);
	}
	fread(&u->step, sizeof(int), 1, fp);
	fread(&u->target, sizeof(int), 1, fp);
	fread(&u->target2, sizeof(int), 1, fp);
	fread(&u->targetU, sizeof(bool), 1, fp);
	fread(&u->underOrder, sizeof(bool), 1, fp);
	fread(&u->fuelStation, sizeof(int), 1, fp);
	fread(&u->currency, sizeof(float), 1, fp);
	fread(&u->fuel, sizeof(float), 1, fp);
	fread(&u->labour, sizeof(float), 1, fp);
	fread(&u->transportRes, sizeof(int), 1, fp);
	fread(&u->transportAmt, sizeof(float), 1, fp);
	fread(&u->driver, sizeof(int), 1, fp);
	//fread(&u->last, sizeof(long), 1, fp);
	fread(&u->hp, sizeof(float), 1, fp);
	fread(&u->passive, sizeof(bool), 1, fp);
	fread(&u->accum, sizeof(float), 1, fp);
	fread(&u->oldPos, sizeof(Vec3f), 1, fp);
	fread(&u->framesleft, sizeof(int), 1, fp);
	fread(&u->home, sizeof(int), 1, fp);
	fread(&u->nopath, sizeof(bool), 1, fp);
	fread(&u->frameslookjobago, sizeof(int), 1, fp);
	fread(&u->supplier, sizeof(int), 1, fp);
	fread(&u->reqAmt, sizeof(float), 1, fp);
	fread(&u->targtype, sizeof(int), 1, fp);
}

void SaveCycleH(CCycleH* h, FILE* fp)
{
	/*
	float produced[RESOURCES];
	float consumed[RESOURCES];
	*/
	
	fwrite(h->produced, sizeof(float), RESOURCES, fp);
	fwrite(h->consumed, sizeof(float), RESOURCES, fp);
}

void ReadCycleH(CCycleH* h, FILE* fp)
{
	fread(h->produced, sizeof(float), RESOURCES, fp);
	fread(h->consumed, sizeof(float), RESOURCES, fp);
}

void SaveBuilding(int i, FILE* fp)
{
	CBuilding* b = &g_building[i];

	/*
	bool on;
	int owner;
	bool finished;
	int frame;
	int type;
	float hp;
	Vec3f pos;
	float stock[RESOURCES];
	float conmat[RESOURCES];	// construction material
	int pownetw;	//power network
	int pipenetw;	//pipe network
	vector<int> occupier;
	vector<int> build;
	vector<EmitterCounter> emitter;
	float prodquota;
	float quotafilled[RESOURCES];
	unsigned int framesago;
	vector<CCycleH> cycleh;
	CCycleH recenth;
	vector<int> roadnetw;
	int transporter[RESOURCES];
	bool forsale;
	float saleprice;
	*/

	fwrite(&b->on, sizeof(bool), 1, fp);

	if(!b->on)
		return;

	fwrite(&b->owner, sizeof(int), 1, fp);
	fwrite(&b->finished, sizeof(bool), 1, fp);
	fwrite(&b->frame, sizeof(int), 1, fp);
	fwrite(&b->type, sizeof(int), 1, fp);
	fwrite(&b->hp, sizeof(float), 1, fp);
	fwrite(&b->pos, sizeof(Vec3f), 1, fp);
	fwrite(b->stock, sizeof(float), RESOURCES, fp);
	fwrite(b->conmat, sizeof(float), RESOURCES, fp);
	fwrite(&b->pownetw, sizeof(int), 1, fp);
	fwrite(&b->pipenetw, sizeof(int), 1, fp);
	int num = b->occupier.size();
	fwrite(&num, sizeof(int), 1, fp);
	for(int j=0; j<num; j++)
		fwrite(&b->occupier[j], sizeof(int), 1, fp);
	num = b->produ.size();
	fwrite(&num, sizeof(int), 1, fp);
	for(int j=0; j<num; j++)
		fwrite(&b->produ[j], sizeof(ManufJob), 1, fp);
	fwrite(&b->prodquota, sizeof(float), 1, fp);
	fwrite(b->quotafilled, sizeof(float), RESOURCES, fp);
	fwrite(&b->framesago, sizeof(unsigned int), 1, fp);
	num = b->cycleh.size();
	fwrite(&num, sizeof(int), 1, fp);
	for(int j=0; j<num; j++)
		SaveCycleH(&b->cycleh[j], fp);
	SaveCycleH(&b->recenth, fp);
	num = b->roadnetw.size();
	fwrite(&num, sizeof(int), 1, fp);
	for(int j=0; j<num; j++)
		fwrite(&b->roadnetw[j], sizeof(int), 1, fp);
	fwrite(b->transporter, sizeof(int), RESOURCES, fp);
	fwrite(&b->forsale, sizeof(bool), 1, fp);
	fwrite(&b->saleprice, sizeof(float), 1, fp);
}

void ReadBuilding(int i, FILE* fp)
{
	CBuilding* b = &g_building[i];

	fread(&b->on, sizeof(bool), 1, fp);

	if(!b->on)
		return;

	fread(&b->owner, sizeof(int), 1, fp);
	fread(&b->finished, sizeof(bool), 1, fp);
	fread(&b->frame, sizeof(int), 1, fp);
	fread(&b->type, sizeof(int), 1, fp);
	fread(&b->hp, sizeof(float), 1, fp);
	fread(&b->pos, sizeof(Vec3f), 1, fp);
	fread(b->stock, sizeof(float), RESOURCES, fp);
	fread(b->conmat, sizeof(float), RESOURCES, fp);
	fread(&b->pownetw, sizeof(int), 1, fp);
	fread(&b->pipenetw, sizeof(int), 1, fp);
	int num;
	fread(&num, sizeof(int), 1, fp);
	for(int j=0; j<num; j++)
	{
		int occupier;
		fread(&occupier, sizeof(int), 1, fp);
		b->occupier.push_back(occupier);
	}
	fread(&num, sizeof(int), 1, fp);
	for(int j=0; j<num; j++)
	{
		ManufJob uprod;
		fread(&uprod, sizeof(ManufJob), 1, fp);
		b->produ.push_back(uprod);
	}
	fread(&b->prodquota, sizeof(float), 1, fp);
	fread(b->quotafilled, sizeof(float), RESOURCES, fp);
	fread(&b->framesago, sizeof(unsigned int), 1, fp);
	fread(&num, sizeof(int), 1, fp);
	for(int j=0; j<num; j++)
	{
		CCycleH h;
		ReadCycleH(&h, fp);
		b->cycleh.push_back(h);
	}
	ReadCycleH(&b->recenth, fp);
	num = b->roadnetw.size();
	fread(&num, sizeof(int), 1, fp);
	for(int j=0; j<num; j++)
	{
		int roadnetw;
		fread(&roadnetw, sizeof(int), 1, fp);
		b->roadnetw.push_back(roadnetw);
	}
	fread(b->transporter, sizeof(int), RESOURCES, fp);
	fread(&b->forsale, sizeof(bool), 1, fp);
	fread(&b->saleprice, sizeof(float), 1, fp);

	b->fillemitters();
}

void SaveRoad(int x, int z, FILE* fp)
{
	CRoad* r = RoadAt(x, z);

	/*
	bool on;
	int type;
	bool finished;
	int owner;
	float conmat[RESOURCES];
	CVertexArray vertexArray;
	int transporter[RESOURCES];
	*/

	fwrite(&r->on, sizeof(bool), 1, fp);

	if(!r->on)
		return;

	fwrite(&r->type, sizeof(int), 1, fp);
	fwrite(&r->finished, sizeof(bool), 1, fp);
	fwrite(&r->owner, sizeof(int), 1, fp);
	fwrite(r->conmat, sizeof(float), RESOURCES, fp);
	fwrite(r->transporter, sizeof(int), RESOURCES, fp);
}

void ReadRoad(int x, int z, FILE* fp)
{
	CRoad* r = RoadAt(x, z);

	fread(&r->on, sizeof(bool), 1, fp);

	if(!r->on)
		return;

	fread(&r->type, sizeof(int), 1, fp);
	fread(&r->finished, sizeof(bool), 1, fp);
	fread(&r->owner, sizeof(int), 1, fp);
	fread(r->conmat, sizeof(float), RESOURCES, fp);
	fread(r->transporter, sizeof(int), RESOURCES, fp);
}

void SavePowl(int x, int z, FILE* fp)
{
	CPowerline* p = PowlAt(x, z);

	/*
	bool on;
	int type;
	bool finished;
	int owner;
	float conmat[RESOURCES];
	int netw;	//power network
	CVertexArray vertexArray;
	int transporter[RESOURCES];
	*/
	
	fwrite(&p->on, sizeof(bool), 1, fp);

	if(!p->on)
		return;

	fwrite(&p->type, sizeof(int), 1, fp);
	fwrite(&p->finished, sizeof(bool), 1, fp);
	fwrite(&p->owner, sizeof(int), 1, fp);
	fwrite(p->conmat, sizeof(float), RESOURCES, fp);
	fwrite(&p->netw, sizeof(int), 1, fp);
	fwrite(p->transporter, sizeof(int), RESOURCES, fp);
}

void ReadPowl(int x, int z, FILE* fp)
{
	CPowerline* p = PowlAt(x, z);
	
	fread(&p->on, sizeof(bool), 1, fp);

	if(!p->on)
		return;

	fread(&p->type, sizeof(int), 1, fp);
	fread(&p->finished, sizeof(bool), 1, fp);
	fread(&p->owner, sizeof(int), 1, fp);
	fread(p->conmat, sizeof(float), RESOURCES, fp);
	fread(&p->netw, sizeof(int), 1, fp);
	fread(p->transporter, sizeof(int), RESOURCES, fp);
}

void SavePipe(int x, int z, FILE* fp)
{
	CPipeline* p = PipeAt(x, z);

	/*
	bool on;
	int type;
	bool finished;
	int owner;
	float conmat[RESOURCES];
	int netw;	//power network
	CVertexArray vertexArray;
	int transporter[RESOURCES];
	*/
	
	fwrite(&p->on, sizeof(bool), 1, fp);

	if(!p->on)
		return;

	fwrite(&p->type, sizeof(int), 1, fp);
	fwrite(&p->finished, sizeof(bool), 1, fp);
	fwrite(&p->owner, sizeof(int), 1, fp);
	fwrite(p->conmat, sizeof(float), RESOURCES, fp);
	fwrite(&p->netw, sizeof(int), 1, fp);
	fwrite(p->transporter, sizeof(int), RESOURCES, fp);
}

void ReadPipe(int x, int z, FILE* fp)
{
	CPipeline* p = PipeAt(x, z);
	
	fread(&p->on, sizeof(bool), 1, fp);

	if(!p->on)
		return;

	fread(&p->type, sizeof(int), 1, fp);
	fread(&p->finished, sizeof(bool), 1, fp);
	fread(&p->owner, sizeof(int), 1, fp);
	fread(p->conmat, sizeof(float), RESOURCES, fp);
	fread(&p->netw, sizeof(int), 1, fp);
	fread(p->transporter, sizeof(int), RESOURCES, fp);
}

void SavePlayer(int i, FILE* fp)
{
	CPlayer* p = &g_player[i];

	/*
	bool on;
	string username;
	float local[RESOURCES];	// used just for counting; cannot be used
	float global[RESOURCES];
	float price[RESOURCES];
	float reschange[RESOURCES];
	float wage[BUILDING_TYPES];
	float conwage;	// construction wage
	float truckwage;
	float uprice[UNIT_TYPES];
	int buframesago;
	float transportprice;
	vector<PriceH> priceh[RESOURCES];
	PriceH recentph[RESOURCES];
	int priceadjframes;
	float occupied[RESOURCES];	// used for capacity resources, just for the resource ticker
	vector<CCycleH> truckh;
	CCycleH recentth;
	int cycleframes;
	*/

	fwrite(&p->on, sizeof(bool), 1, fp);

	char tempname[32];
	strcpy(tempname, p->username.c_str());
	fwrite(tempname, sizeof(char), 32, fp);
	fwrite(&p->local, sizeof(float), RESOURCES, fp);
	fwrite(&p->global, sizeof(float), RESOURCES, fp);
	fwrite(&p->price, sizeof(float), RESOURCES, fp);
	fwrite(&p->reschange, sizeof(float), RESOURCES, fp);
	fwrite(&p->wage, sizeof(float), BUILDING_TYPES, fp);
	fwrite(&p->conwage, sizeof(float), 1, fp);
	fwrite(&p->truckwage, sizeof(float), 1, fp);
	fwrite(&p->ai, sizeof(bool), 1, fp);
	fwrite(&p->activity, sizeof(int), 1, fp);
	fwrite(&p->bbframesago, sizeof(int), 1, fp);
	fwrite(p->uprice, sizeof(float), UNIT_TYPES, fp);
	fwrite(&p->buframesago, sizeof(int), 1, fp);
	fwrite(&p->transportprice, sizeof(float), 1, fp);
	for(int j=0; j<RESOURCES; j++)
	{
	//p->priceh[j].clear();
		int numh = p->priceh[j].size();
		fwrite(&numh, sizeof(int), 1, fp);
		for(int k=0; k<numh; k++)
		{
			fwrite(&p->priceh[j][k], sizeof(PriceH), 1, fp);
		}
	}
	fwrite(p->recentph, sizeof(PriceH), RESOURCES, fp);
	fwrite(&p->priceadjframes, sizeof(int), 1, fp);
	fwrite(&p->occupied, sizeof(float), RESOURCES, fp);
	int numh = p->truckh.size();
	fwrite(&numh, sizeof(int), 1, fp);
	for(int j=0; j<numh; j++)
	{
		SaveCycleH(&p->truckh[j], fp);
	}
	SaveCycleH(&p->recentth, fp);
	fwrite(&p->cycleframes, sizeof(int), 1, fp);
}

void ReadPlayer(int i, FILE* fp)
{
	CPlayer* p = &g_player[i];
	
	fread(&p->on, sizeof(bool), 1, fp);

	char tempname[32];
	fread(tempname, sizeof(char), 32, fp);
	p->username = tempname;

	fread(&p->local, sizeof(float), RESOURCES, fp);
	fread(&p->global, sizeof(float), RESOURCES, fp);
	fread(&p->price, sizeof(float), RESOURCES, fp);
	fread(&p->reschange, sizeof(float), RESOURCES, fp);
	fread(&p->wage, sizeof(float), BUILDING_TYPES, fp);
	fread(&p->conwage, sizeof(float), 1, fp);
	fread(&p->truckwage, sizeof(float), 1, fp);
	fread(&p->ai, sizeof(bool), 1, fp);
	fread(&p->activity, sizeof(int), 1, fp);
	fread(&p->bbframesago, sizeof(int), 1, fp);
	fread(p->uprice, sizeof(float), UNIT_TYPES, fp);
	fread(&p->buframesago, sizeof(int), 1, fp);
	fread(&p->transportprice, sizeof(float), 1, fp);
	for(int j=0; j<RESOURCES; j++)
	{
		int numh;
		fread(&numh, sizeof(int), 1, fp);
		for(int k=0; k<numh; k++)
		{
			PriceH ph;
			fread(&ph, sizeof(PriceH), 1, fp);
			p->priceh[j].push_back(ph);
		}
	}
	fread(p->recentph, sizeof(PriceH), RESOURCES, fp);
	fread(&p->priceadjframes, sizeof(int), 1, fp);
	fread(&p->occupied, sizeof(float), RESOURCES, fp);
	int numh;
	fread(&numh, sizeof(int), 1, fp);
	for(int j=0; j<numh; j++)
	{
		CCycleH ch;
		ReadCycleH(&ch, fp);
		p->truckh.push_back(ch);
	}
	ReadCycleH(&p->recentth, fp);
	fread(&p->cycleframes, sizeof(int), 1, fp);

	//char msg[128];
	//sprintf(msg, "p%d truckwa=%f", i, p->truckwage);
	//Chat(msg);

	/*
		for(int j=0; j<RESOURCES; j++)
		{
			p->priceh[j].clear();
			p->recentph[j].earnings = 0;
			p->recentph[j].expenditures = 0;
			p->recentph[j].price = p->price[j];
		}

		p->truckh.clear();
		p->recentth.reset();*/
}

void SaveCondition(CCondition* c, FILE* fp)
{
	fwrite(c->name, sizeof(char), 16, fp);
	fwrite(&c->type, sizeof(int), 1, fp);
	fwrite(&c->met, sizeof(bool), 1, fp);
	fwrite(&c->target, sizeof(int), 1, fp);
	fwrite(&c->fval, sizeof(float), 1, fp);
	fwrite(&c->count, sizeof(int), 1, fp);
}

void ReadCondition(CCondition* c, FILE* fp)
{
	fread(c->name, sizeof(char), 16, fp);
	fread(&c->type, sizeof(int), 1, fp);
	fread(&c->met, sizeof(bool), 1, fp);
	fread(&c->target, sizeof(int), 1, fp);
	fread(&c->fval, sizeof(float), 1, fp);
	fread(&c->count, sizeof(int), 1, fp);
}

void SaveEffect(CEffect* e, FILE* fp)
{
	fwrite(e->name, sizeof(char), 16, fp);
	fwrite(&e->type, sizeof(int), 1, fp);
	int trigger = TriggerID(e->trigger);
	fwrite(&trigger, sizeof(int), 1, fp);
	int tvlen = strlen(e->textval.c_str())+1;
	fwrite(&tvlen, sizeof(int), 1, fp);
	fwrite(e->textval.c_str(), sizeof(char), tvlen, fp);
	fwrite(&e->imgdw, sizeof(int), 1, fp);
	fwrite(&e->imgdh, sizeof(int), 1, fp);
}

void ReadEffect(CEffect* e, vector<int>* triggerrefs, FILE* fp)
{
	fread(e->name, sizeof(char), 16, fp);
	fread(&e->type, sizeof(int), 1, fp);
	int ref;
	fread(&ref, sizeof(int), 1, fp);
	triggerrefs->push_back(ref);
	int tvlen = 0;
	fread(&tvlen, sizeof(int), 1, fp);
	char* tval = new char[tvlen];
	fread(tval, sizeof(char), tvlen, fp);
	e->textval = tval;
	delete [] tval;
	fread(&e->imgdw, sizeof(int), 1, fp);
	fread(&e->imgdh, sizeof(int), 1, fp);
}

void SaveTrigger(CTrigger* t, FILE* fp)
{
	fwrite(t->name, sizeof(char), 16, fp);
	fwrite(&t->enabled, sizeof(bool), 1, fp);
	int numc = t->conditions.size();
	int nume = t->effects.size();
	fwrite(&numc, sizeof(int), 1, fp);
	fwrite(&nume, sizeof(int), 1, fp);

	for(int i=0; i<numc; i++)
		SaveCondition(&t->conditions[i], fp);

	for(int i=0; i<nume; i++)
		SaveEffect(&t->effects[i], fp);
}

void ReadTrigger(CTrigger* t, vector<int>* triggerrefs, FILE* fp)
{
	fread(t->name, sizeof(char), 16, fp);
	fread(&t->enabled, sizeof(bool), 1, fp);
	int numc, nume;
	fread(&numc, sizeof(int), 1, fp);
	fread(&nume, sizeof(int), 1, fp);

	for(int i=0; i<numc; i++)
	{
		CCondition c;
		ReadCondition(&c, fp);
		t->conditions.push_back(c);
	}

	for(int i=0; i<nume; i++)
	{
		CEffect e;
		ReadEffect(&e, triggerrefs, fp);
		t->effects.push_back(e);
	}
}

void SaveHint(Hint* h, FILE* fp)
{
	int len = strlen(h->message.c_str())+1;
	fwrite(&len, sizeof(int), 1, fp);
	fwrite(h->message.c_str(), sizeof(char), len, fp);
	len = strlen(h->graphic.c_str())+1;
	fwrite(&len, sizeof(int), 1, fp);
	fwrite(h->graphic.c_str(), sizeof(char), len, fp);
	fwrite(&h->gwidth, sizeof(int), 1, fp);
	fwrite(&h->gheight, sizeof(int), 1, fp);
}

void ReadHint(Hint* h, FILE* fp)
{
	int len;
	fread(&len, sizeof(int), 1, fp);
	char* buff = new char[len];
	fread(buff, sizeof(char), len, fp);
	h->message = buff;
	delete [] buff;
	fread(&len, sizeof(int), 1, fp);
	buff = new char[len];
	fread(buff, sizeof(char), len, fp);
	h->graphic = buff;
	delete [] buff;
	fread(&h->gwidth, sizeof(int), 1, fp);
	fread(&h->gheight, sizeof(int), 1, fp);
}

void SaveSlot(int slot)
{
	g_log<<"saves\\slot%d.sav"<<endl;
	g_log.flush();
	
	char filename[32];
	sprintf(filename, "saves\\slot%d.sav", slot);
	char full[MAX_PATH+1];
	FullPath(filename, full);

	SaveMap(full);
}

void SaveMap(const char* filename)
{
	//char full[MAX_PATH+1];
	//GetCurrentDirectory(MAX_PATH, full);
	//strcat(full, "\\");
	//strcat(full, filename);

	FILE* fp = fopen(filename, "wb");

	char tag[] = {'D', 'M', 'D', 'M'};
	fwrite(tag, sizeof(char), 4, fp);

	char name[32];
	string datetime = currentDateTime();
	//sprintf(name, "@ %d", (int)GetTickCount());
	sprintf(name, "%s", datetime.c_str());
	fwrite(name, sizeof(char), 32, fp);
	
	fwrite(&g_camera, sizeof(Camera), 1, fp);

	fwrite(&g_hmap.m_widthX, sizeof(int), 1, fp);
	fwrite(&g_hmap.m_widthZ, sizeof(int), 1, fp);

	int x, z;
	for(x=0; x<=g_hmap.m_widthX; x++)
		for(z=0; z<=g_hmap.m_widthZ; z++)
		{
			fwrite(&g_hmap.m_heightpoints[ x + z*(g_hmap.m_widthX+1) ], sizeof(float), 1, fp);
		}

		
	g_log<<"heightmap save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	vector<string> tile_types;

	int i;
	int tt;
	bool found;
	for(x=0; x<=g_hmap.m_widthX; x++)
		for(z=0; z<=g_hmap.m_widthZ; z++)
		{
			tt = g_hmap.m_tiletype[ x + z*(g_hmap.m_widthX+1) ];
			found = false;
			for(i=0; i<tile_types.size(); i++)
			{
				if(stricmp(tile_types[i].c_str(), g_tiletype[tt].m_name) == 0)
				{
					//g_log<<"writing found tt ["<<tt<<"] = "<<g_tiletype[tt].m_name<<endl;
					//g_log.flush();

					found = true;
					break;
				}
			}

			if(found)
				continue;

			tile_types.push_back( string(g_tiletype[tt].m_name) );
		}
		
	int num_tt = tile_types.size();
	fwrite(&num_tt, sizeof(int), 1, fp);

	char tempname[32];
	for(i=0; i<num_tt; i++)
	{
		sprintf(tempname, "%s", tile_types[i].c_str());
		fwrite(tempname, sizeof(char), 32, fp);
	}

	
	g_log<<"tilenames save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	int ttindex;
	for(x=0; x<=g_hmap.m_widthX; x++)
		for(z=0; z<=g_hmap.m_widthZ; z++)
		{
			tt = g_hmap.m_tiletype[ x + z*(g_hmap.m_widthX+1) ];
			for(i=0; i<tile_types.size(); i++)
			{
				if(stricmp(tile_types[i].c_str(), g_tiletype[tt].m_name) == 0)
				{
					ttindex = i;
					break;
				}
			}

			fwrite(&ttindex, sizeof(int), 1, fp);
		}

		
	g_log<<"tiletypes size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;
		
	//fwrite(g_unit, sizeof(CUnit), UNITS, fp);
	for(i=0; i<UNITS; i++)
		SaveUnit(i, fp);

	
	g_log<<"units size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	for(i=0; i<BUILDINGS; i++)
		SaveBuilding(i, fp);

	g_log<<"buildings size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;
	
	for(i=0; i<PLAYERS; i++)
		SavePlayer(i, fp);
	//fwrite(g_player, sizeof(CPlayer), PLAYERS, fp);
	
	g_log<<"players save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	for(int i=0; i<PLAYERS; i++)
		fwrite(g_diplomacy[i], sizeof(bool), PLAYERS, fp);

	g_log<<"diplomacy save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;
	
	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			SaveRoad(x, z, fp);
	
	g_log<<"roads save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			SavePowl(x, z, fp);
	
	g_log<<"powls save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			SavePipe(x, z, fp);

	g_log<<"pipes save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	int numt = CountTriggers();
	fwrite(&numt, sizeof(int), 1, fp);

	for(int i=0; i<numt; i++)
		SaveTrigger(GetTrigger(i), fp);
	
	g_log<<"triggers save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	SaveHint(&g_lasthint, fp);
	
	g_log<<"hint save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	for(int i=0; i<SCENERY; i++)
		SaveScenery(i, fp);

	g_log<<"scenery save size = "<<ftell(fp)<<"bytes, "<<(ftell(fp)/1024/1024)<<"MB"<<endl;

	fclose(fp);
}

bool LoadSlot(int slot)
{
	g_log<<"load slot "<<slot<<endl;
	g_log.flush();

	char filename[32];
	sprintf(filename, "saves\\slot%d.sav", slot);
	char full[MAX_PATH+1];
	FullPath(filename, full);

	g_mode = PLAY;
	bool result = LoadMap(full);
	if(!result)
		g_mode = MENU;

	return result;
}


void MapName(const char* filename, char* slotname)
{
	//char full[MAX_PATH+1];
	//GetCurrentDirectory(MAX_PATH, full);
	//strcat(full, "\\");
	//strcat(full, filename);

	FILE* fp = fopen(filename, "rb");

	if(!fp)
		return;

	char tag[4];
	fread(tag, sizeof(char), 4, fp);

	if(tag[0] != 'D' ||  tag[1] != 'M' || tag[2] != 'D' || tag[3] != 'M')
	{
		fclose(fp);
		return;
	}

	fread(slotname, sizeof(char), 32, fp);
	fclose(fp);
}

string StripFile(string filepath)
{
	int lastof = filepath.find_last_of("/\\");
	if(lastof < 0)
		lastof = strlen(filepath.c_str());

	string stripped = filepath.substr(0, filepath.find_last_of("/\\"));
	return stripped;
}

void FullPath(const char* filename, char* full)
{
	char exepath[MAX_PATH+1];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	string path = StripFile(exepath);

	//char full[MAX_PATH+1];
	sprintf(full, "%s", path.c_str());

	char c = full[ strlen(full)-1 ];
	if(c != '\\' && c != '/')
		strcat(full, "\\");

	strcat(full, filename);
}

void FreeMap()
{
	SelectionFunc = NULL;
	FreeScript();
	ResetPlayers();

	int i;
	for(i=0; i<TRANSACTIONS; i++)
		g_transx[i].on = false;

	for(i=0; i<PARTICLES; i++)
		g_particle[i].on = false;

	for(i=0; i<PROJECTILES; i++)
		g_projectile[i].on = false;

	for(i=0; i<UNITS; i++)
	{
		if(!g_unit[i].on)
			continue;

		g_unit[i].destroy();
		g_unit[i].on = false;
	}
	
	LastNum("free map 1");

	for(i=0; i<BUILDINGS; i++)
	{
		if(!g_building[i].on)
			continue;

		g_building[i].destroy();
		g_building[i].on = false;
		g_building[i].finished = false;
	}

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			CRoad* road = RoadAt(x, z);
			if(road->on)
				road->destroy();
			PipeAt(x, z)->destroy();
			PowlAt(x, z)->destroy();
		}

	LastNum("free map 2");

	ClearScenery();
	FreeColliderCells();
	
	LastNum("free map 3");

	g_selection.clear();
	g_selection2.clear();

	int wx = g_hmap.m_widthX;
	int wz = g_hmap.m_widthZ;
	g_hmap.free();
	g_hmap.allocate(wx, wz);

	g_lasthint.message = "";
	g_lasthint.graphic = "gui\\transp.png";
}

bool LoadMap(const char* filename, bool test)
{
	//char full[MAX_PATH+1];
	//GetCurrentDirectory(MAX_PATH, full);
	//strcat(full, "\\");
	//strcat(full, filename);

	FILE* fp;
	
	if(!test)
	{
		fp = fopen(filename, "rb");
	}
	else
	{
		char full[MAX_PATH+1];
		FullPath(filename, full);
		fp = fopen(full, "rb");
	}

	if(!fp)
		return false;

	if(test)
	{
		fclose(fp);
		return true;
	}

	g_selP = g_localP;

	int i;

	FreeMap();

	char tag[4];
	fread(tag, sizeof(char), 4, fp);

	if(tag[0] != 'D' ||  tag[1] != 'M' || tag[2] != 'D' || tag[3] != 'M')
	{
		fclose(fp);
		return false;
	}

	char name[32];
	fread(name, sizeof(char), 32, fp);
	
	fread(&g_camera, sizeof(Camera), 1, fp);

	int wx, wz;
	fread(&wx, sizeof(int), 1, fp);
	fread(&wz, sizeof(int), 1, fp);

	g_log<<"freeing map to allocate "<<wx<<","<<wz<<endl;
	g_log.flush();

	g_hmap.free();

	g_log<<"allocating map "<<wx<<","<<wz<<endl;
	g_log.flush();

	g_hmap.allocate(wx, wz);

	g_log<<"read wx,z = "<<g_hmap.m_widthX<<","<<g_hmap.m_widthZ<<endl;
	g_log.flush();

	int x, z;
	for(x=0; x<=g_hmap.m_widthX; x++)
		for(z=0; z<=g_hmap.m_widthZ; z++)
		{
			fread(&g_hmap.m_heightpoints[ x + z*(g_hmap.m_widthX+1) ], sizeof(float), 1, fp);
		}

	int num_tt;
	fread(&num_tt, sizeof(int), 1, fp);

	g_log<<"loading num_tt = "<<num_tt<<endl;
	g_log.flush();

	vector<string> tile_types;
	vector<int> tile_type_indices;

	char ttname[32];
	for(i=0; i<num_tt; i++)
	{
		fread(ttname, sizeof(char), 32, fp);
		tile_types.push_back(string(ttname));

		//g_log<<"read tt ["<<i<<"] = "<<tile_types[i]<<endl;
		//g_log.flush();
	}
	
	int j, found;
	for(i=0; i<num_tt; i++)
	{
		found = 0;

		for(j=0; j<TILETYPES; j++)
			if(g_tiletype[j].on && stricmp(tile_types[i].c_str(), g_tiletype[j].m_name) == 0)
			{
				found = j;

				//g_log<<"found j ["<<j<<"] = "<<g_tiletype[j].m_name<<endl;
				//g_log.flush();

				break;
			}

		tile_type_indices.push_back(found);
	}

	int ttindex;
	for(x=0; x<=g_hmap.m_widthX; x++)
		for(z=0; z<=g_hmap.m_widthZ; z++)
		{
			fread(&ttindex, sizeof(int), 1, fp);
			g_hmap.m_tiletype[ x + z*(g_hmap.m_widthX+1) ] = tile_type_indices[ ttindex ];
		}

	g_hmap.remesh();
	g_hmap.retexture();
	
	for(i=0; i<UNITS; i++)
		ReadUnit(i, fp);

	for(i=0; i<BUILDINGS; i++)
		ReadBuilding(i, fp);

	for(i=0; i<PLAYERS; i++)
		ReadPlayer(i, fp);
	//fread(g_player, sizeof(CPlayer), PLAYERS, fp);
	
	for(int i=0; i<PLAYERS; i++)
		fread(g_diplomacy[i], sizeof(bool), PLAYERS, fp);
	
	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			ReadRoad(x, z, fp);

	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			MeshRoad(x, z);

	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			ReadPowl(x, z, fp);
	
	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			MeshPowl(x, z);
	
	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			ReadPipe(x, z, fp);
	
	for(x=0; x<g_hmap.m_widthX; x++)
		for(z=0; z<g_hmap.m_widthZ; z++)
			MeshPipe(x, z);

	int numt;
	fread(&numt, sizeof(int), 1, fp);

	CTrigger* prev = NULL;
	CTrigger* t;
	vector<int> triggerrefs;

	for(int i=0; i<numt; i++)
	{
		t = new CTrigger();

		if(i==0)
			g_scripthead = t;

		if(prev != NULL)
			prev->next = t;

		t->prev = prev;

		ReadTrigger(t, &triggerrefs, fp);

		prev = t;
	}

	int refnum = 0;
	for(t=g_scripthead; t; t=t->next)
	{
		for(int i=0; i<t->effects.size(); i++)
		{
			CEffect* e = &t->effects[i];
			e->trigger = GetTrigger(triggerrefs[refnum]);
			refnum++;
		}
	}

	ReadHint(&g_lasthint, fp);

	for(int i=0; i<SCENERY; i++)
		ReadScenery(i, fp);

	fclose(fp);

	FillColliderCells();
	
	if(g_mode == PLAY)
	{
		if(strlen(g_lasthint.message.c_str()) > 0)
		{
			GMessage(g_lasthint.message.c_str());
			GMessageG(g_lasthint.graphic.c_str(), g_lasthint.gwidth, g_lasthint.gheight);
		}

		OnLoadMap();

		if(g_player[g_localP].activity == ACTIVITY_NONE)
			GameOver("", g_localP);
	}

	//g_log<<"LOADED MAP"<<endl;
	//g_log.flush();

	/*
		//REMOVE THIS
		for(int i=0; i<UNITS; i++)
		{
			CUnit* u = &g_unit[i];
			if(!u->on)
				continue;
			if(u->type != TRUCK)
				continue;
			u->ResetMode();
			
		g_log<<"sTruck "<<i<<" mode = "<<u->mode<<endl;
		g_log.flush();
		}*/

	//RePipe();
	/*
	float uprice[UNIT_TYPES];

	for(int i=0; i<UNIT_TYPES; i++)
	{
		uprice[i] = 0;
		CUnitType* ut = &g_unitType[i];

		for(int j=0; j<RESOURCES; j++)
		{
			//float multiplier = 0.01f;
			float multiplier = 0.10f;

			if(j == LABOUR)
				//multiplier = 0.02f;
				multiplier = 0.20f;

			uprice[i] += ut->cost[j] * multiplier;
		}
		
		//uprice[i] += 0.1f;
		uprice[i] += 1.0f;
	}
	
	//uprice[CARLYLE] += 0.05f;
	//uprice[MECH] += 0.1f;
	uprice[CARLYLE] += 0.5f;
	uprice[MECH] += 1.0f;
	
	CPlayer* p;
	for(int i=0; i<PLAYERS; i++)
	{
		p = &g_player[i];

		p->conwage = 0.05f;
		p->truckwage = 0.01f;
		p->transportprice = 0.02f;
		for(int j=0; j<RESOURCES; j++)
			//p->price[j] = 0.01f;
			p->price[j] = 0.20f;
		p->price[CEMENT] = 0.01f;
		p->price[STONE] = 0.01f;
		p->price[HOUSING] = 0.08f;
		p->price[CRUDE] = 0.01f;
		p->price[ZETROL] = 0.01f;
		for(int j=0; j<BUILDING_TYPES; j++)
			//p->wage[j] = 0.02f;
			p->wage[j] = 0.10f;
		p->wage[REACTOR] = 0.02f;
	
		memcpy(p->uprice, uprice, sizeof(float)*UNIT_TYPES);
	}*/

	return true;
}