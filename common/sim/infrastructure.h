

#ifndef INFRASTRUCTURE_H
#define INFRASTRUCTURE_H

#include "connectable.h"
#include "resources.h"
#include "../render/model.h"
#include "../render/vertexarray.h"
#include "../math/vec3i.h"

#define CON_ROAD	0
#define CON_POWL	1
#define CON_CRPIPE	2
#define CONDUITS	3

class ConduitTile;

class ConduitType
{
public:
	int conmat[RESOURCES];
	unsigned short netwoff;	//offset to network list in Building class
	int model[CONNECTION_TYPES][2];
	unsigned short maxforwincl;
	unsigned short maxsideincl;
	bool blconduct;

	ConduitTile* (*get)(int tx, int tz, bool plan);
	void (*checkplace)(int tx, int tz);
	Vec3f (*drawpos)(int tx, int tz);
	Vec2i (*physpos)(int tx, int tz);
	void (*draw)(int tx, int ty, bool plan);
	//void (*updplans)(char owner, Vec3f start, Vec3f end);
	//void (*clearplans)();
	//void (*renetw)();
	void (*place)(int tx, int tz, char owner, bool plan);
	void updplans(char owner, Vec3f start, Vec3f end);
	void clearplans();
	void renetw();
	void resetnetw();
	bool renetwb();
	void mergenetw(int A, int B);
	bool renetwtiles();

	ConduitType()
	{
		Zero(conmat);
		checkplace = NULL;
		drawpos = NULL;
		physpos = NULL;
		draw = NULL;
		//updplans = NULL;
		//renetw = NULL;
		place = NULL;
		//clearplans = NULL;
		get = NULL;
		blconduct = false;
	}
};

extern ConduitType g_cotype[CONDUITS];
void DefCo(char ctype, unsigned short netwoff, unsigned short maxforwincl, unsigned short maxsideincl, bool blconduct, 
	void (*checkplace)(int tx, int tz),
	Vec3f (*drawpos)(int tx, int tz),
	Vec2i (*physpos)(int tx, int tz),
	void (*draw)(int tx, int ty, bool plan),
	//void (*updplans)(char owner, Vec3f start, Vec3f end),
	//void (*clearplans)(),
	//void (*renetw)(),
	void (*place)(int tx, int tz, char owner, bool plan),
	ConduitTile* (*get)(int tx, int tz, bool plan));
void CoConMat(char ctype, char rtype, short ramt);

class ConduitTile
{
public:
	bool on;
	char conntype;
	bool finished;
	char owner;
	int conmat[RESOURCES];
	short netw;	//network
	VertexArray drawva;
	//bool inaccessible;
	short transporter[RESOURCES];
	Vec3f drawpos;
	//int maxcost[RESOURCES];

	ConduitTile();
	~ConduitTile();

	virtual char condtype();
	virtual int netreq(int res);
	virtual void destroy();
	virtual void allocate();
	virtual bool checkconstruction();
	virtual void fillcollider();
	virtual void freecollider();
};

#endif