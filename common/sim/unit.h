

#ifndef UNIT_H
#define UNIT_H

#include <deque>
#include "3dmath.h"
#include "pathfinding.h"
#include "debug.h"
#include "resource.h"
#include "transportation.h"
#include "map.h"
#include "physics.h"

using namespace std;

class Shader;
class CSound;

class CUnitType
{
public:
	int model;
	float radius;
	float height;
	float speed;
	bool passable;
	float cost[RESOURCES];
	float hitpoints;
	float range;
	int shotdelay;
	float damage;
	unsigned int sprite;
	char name[32];
	bool walker;
	vector<EmitterPlace> deademitter;
	vector<CSound> atksnd;
	bool roaded;

	void draw(int frame, Vec3f pos, float yaw);
};

enum UNITTYPE{LABOURER, TRUCK, BATTLECOMP, CARLYLE, MECH, UNIT_TYPES};
extern CUnitType g_unitType[UNIT_TYPES];

enum MODE{NONE, GOINGTONORMJOB, NORMJOB, GOINGTOCONJOB, CONJOB, GOINGTOROADJOB, ROADJOB, 
		GOINGTOPOWLJOB, POWLJOB, GOINGTOPIPEJOB, PIPEJOB, GOINGTOSHOP, SHOPPING, GOINGTOREST, RESTING,
		GOINGTOTRUCK, DRIVING, GOINGTOSUPPLIER, GOINGTODEMANDERB, 
		GOINGTOREFUEL, REFUELING, ATDEMANDERB, ATSUPPLIER,
		GOINGTODEMROAD, GOINGTODEMPOWL, GOINGTODEMPIPE, ATDEMROAD, ATDEMPOWL, ATDEMPIPE};

#define LABOURER_LABOUR		10
#define LABOURER_FUEL		10
#define MULTIPLY_FUEL		(LABOURER_FUEL*2.0f)
#define LABOURER_CONSUMPTIONRATE		0.001f
#define WORK_DELAY			1000
#define DRIVE_WORK_DELAY	1000
#define TRUCK_CONSUMPTIONRATE		0.5f
#define TRUCK_FUEL			10
#define LOOKJOB_DELAY_MAX	(FRAME_RATE*5)
#define FUEL_DISTANCE		183		//(TILE_SIZE*2.0f)
#define DEATH_FRAMES		120

//10 fuel/tank / 0.5 fuel/consumption = 20 consumption/tank
//0.35^2 = 0.1225
//old: consumption every 64/0.1225 = 522.44897959183673469387755102041
//new: consumption every X/0.35 = 522.44897959183673469387755102041; X = 182.85714285714285714285714285714
//dunno 20/0.1225 = 163.26530612244897959183673469388

#define UNIT_FOV		30.0f

#define MIN_WORKING_AGE	16
#define WORKING_AGE		19
#define RETIREMENT_AGE	65
#define MAX_AGE			100

#define BODY_LOWER	0
#define BODY_UPPER	1

class CUnit
{
public:
	bool on;
	int owner;
	int type;
	Camera camera;
	float frame[2];
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
	vector<EmitterCounter> emitter;

	CUnit() { on = false; accum = 0; }
	//ColliderCell* collidercellat(int x, int z);
	void relocate();
	bool hidden();
	float pathlength();
	int stepbeforearrival();
	bool trystep(CTryStep *bestS, bool roadVeh, int ignoreBuilding, CUnit* ignoreUnit);
	bool trydiagstep(CTryStep *nextS, CTryStep* prevS, bool roadVeh, int ignoreBuilding, CUnit* ignoreUnit);
	bool canpath();
	void repath();
	void fillcollidercells();
	void freecollidercells();
	bool CheckConstructionAvailability();
	bool CheckNormalJobAvailability();
	bool CheckRoadAvailability();
	bool CheckPowerlineAvailability();
	bool CheckPipelineAvailability();
	bool CheckShopAvailability();
	bool CheckApartmentAvailability(bool* eviction);
	bool CheckIfArrived();
	void OnArrived();
	void GoToConJob();
	void DoConJob();
	void GoToNormJob();
	void DoNormJob();
	void GoToRoadJob();
	void DoRoadJob();
	void GoToPowerlineJob();
	void DoPowerlineJob();
	void GoToPipelineJob();
	void DoPipelineJob();
	void GoShop();
	void checkmultiply();
	void DoShop();
	void GoRest();
	void evict();
	void DoRest();
	void ArrivedAtTruck();
	bool CheckTruckAvailability();
	void GoToTruck();
	void DoDrive();
	void GoToDemander();
	void GoToSupplier();
	void GoToRefuel();
	void DoneRefueling();
	void DoneAtSupplier();
	void AtDemanderB();
	void AtDemRoad();
	void AtDemPowl();
	void AtDemPipe();
	void AtSupplier();
	void DoRefuel();
	bool Supply(int type);
	void ExchangeMomentum(int i);
	bool CanMove();
	void Movement();
	bool TryStep(Vec3f vPos, CUnit* ignoreUnit, int ignoreBuilding, bool roadVehicle);
	Vec3f trysteptoposition(int x, int z);
	bool Pathfind(bool nodelay=true);
	bool Approach();
	bool AtGoal();
	Vec3f EnemyPosition(int id, bool isU);
	bool canturn();
	void Aim(int* id, bool* isU);
	bool WithinRange(Vec3f p);
	bool WithinView(Vec3f p);
	void Shoot(int id, bool isU);
	void Damage(int id, bool isU, float damage);
	bool Collides2(bool checkroad);
	bool Collides(CUnit* ignoreUnit=NULL, int ignoreBuilding=-1);
	bool collidesfast(CUnit* ignoreUnit=NULL, int ignoreBuilding=-1);
	bool confirmcollision(int ctype, int ID, float eps=EPSILON);
	bool NeedFood();
	bool NeedRest();
	bool FindFood();
	bool FindRest();
	bool checkhomefaulty();
	bool FindJob();
	bool FindDemander();
	bool FindFuel();
	int FindSupplier(int r);
	bool CheckNeedCG();
	bool CheckNeedE();
	void Accum(float d);
	void Disembark();
	void UpdateLabourer();
	void UpdateTruck();
	void updatedead();
	void UpdateMilitary();
	bool CheckTargetAvailability();
	bool FindTarget();
	bool Shoot();
	void Update();
	void ResetPath()
	{
		path.clear();
		step = 0;
	}
	void ResetGoal() 
	{
		goal = subgoal = camera.Position();
		ResetPath();
	}
	void ResetMode();
	void ResetTarget()
	{
		target = -1;
		ResetMode();
	}
	void ResetLabourer()
	{
		frame[BODY_LOWER] = 0;
		fuel = LABOURER_FUEL;
		labour = LABOURER_LABOUR;
		currency = 0;
		hp = 1;
		ResetMode();
		if(home >= 0)
		{
			evict();
		}
	}
	void newjob() 
	{
		mode = NONE;
		ResetGoal();
	}
	void destroy();
	void Place(int t, int o, Vec3f p, int parnt, float ag, int male);
	bool IsMilitary()
	{
		if(type == LABOURER)
			return false;
		if(type == TRUCK)
			return false;
			/*
		CUnitType* t = &g_unitType[type];
		if(t->damage == 0.0f)
			return false;*/

		return true;
	}
	void GoHome();
	void draw();
	bool walking();
};

#define UNITS	1024
extern CUnit g_unit[UNITS];

void PlaceUAround(CUnit* u, float borderleft, float bordertop, float borderright, float borderbottom, bool checkroad);
void USprite(int type, char* file);
void UDefine(int type, const char* name, const char* file, Vec3f scale, Vec3f translate, float rad, float mov, bool pass, float hp, float rng, int sht, float dmg);
void InitUnits();
void LoadUnitSprites();
void UpdateUnits();
void DrawUnits();
void DrawUnitStatus(Matrix projmodlview);
void DrawUnitStats(Matrix projmodlview);
void DrawUnitStatus2();
int PlaceUnit(int type, int owner, Vec3f pos, int parent=-1, float age=-1, int male=-1);
int NewUnit();
int UnitID(CUnit* u);
void ClearAttackers(int id, bool isU);

#endif