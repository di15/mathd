
#include "map.h"
#include "unit.h"
#include "model.h"
#include "main.h"
#include "font.h"
#include "chat.h"
#include "player.h"
#include "pathfinding.h"
#include "image.h"
#include "shader.h"
#include "namegen.h"
#include "transportation.h"
#include "gui.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "sound.h"

CUnitType g_unitType[UNIT_TYPES];
CUnit g_unit[UNITS];

int g_lastU = -1;

void UDefine(int type, const char* name, const char* file, bool walker, Vec3f scale, Vec3f translate, float rad, float heigh, float mov, bool pass, float hp, float rng, int sht, float dmg, bool roaded)
{
	CUnitType* t = &g_unitType[type];
	g_lastU = type;
	QueueModel(&t->model, file, scale, translate);
	t->radius = rad;
	t->height = heigh;
	t->speed = mov;
	t->passable = pass;
	t->hitpoints = hp;
	t->range = rng;
	t->shotdelay = sht;
	t->damage = dmg;
	strcpy(t->name, name);
	t->walker = walker;
	t->roaded = roaded;
}

void USprite(int type, char* file)
{
	CUnitType* t = &g_unitType[type];
	//CreateTexture(sprite, file);
	QueueTexture(&t->sprite, file, true);
}

void UAtkSnd(const char* filepath)
{
	CUnitType* t = &g_unitType[g_lastU];
	t->atksnd.push_back(CSound(filepath));
}

void UCost(int res, float value)
{
	CUnitType* t = &g_unitType[g_lastU];
	t->cost[res] = value;
}

void UDeadEm(int type, Vec3f offset)
{
	CUnitType* t = &g_unitType[g_lastU];
	t->deademitter.push_back(EmitterPlace(type, offset));
}

void CUnitType::draw(int frame, Vec3f pos, float yaw)
{
	g_model[model].draw(frame, pos, yaw);
	//g_model[model].draw(rand()%30, pos, yaw);
}

bool CUnit::hidden()
{
	if(mode == NORMJOB)
		return true;
	if(mode == CONJOB)
		return true;
	if(mode == PIPEJOB)
		return true;
	if(mode == ROADJOB)
		return true;
	if(mode == POWLJOB)
		return true;
	if(mode == DRIVING)
		return true;
	if(mode == RESTING)
		return true;
	if(mode == SHOPPING)
		return true;

	return false;
}

void CUnit::ResetMode()
{
	LastNum("resetmode 1");
	if(type == LABOURER)
	{
	LastNum("resetmode 1a");
		if(mode == NORMJOB)
			g_building[target].RemoveWorker(this);
		
		if(hidden())
			relocate();
	}
	else if(type == TRUCK)
	{
		if(mode == GOINGTOSUPPLIER 
			//|| mode == GOINGTOREFUEL 
			//|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
			)
		{
			if(supplier >= 0)
			{
				CBuilding* b = &g_building[supplier];
				b->transporter[transportRes] = -1;
			}
		}
		if((mode == GOINGTOSUPPLIER || mode == GOINGTODEMANDERB) && targtype == GOINGTODEMANDERB)
		{
			if(target >= 0)
			{
				CBuilding* b = &g_building[target];
				b->transporter[transportRes] = -1;
			}
		}
		else if((mode == GOINGTOSUPPLIER || mode == GOINGTODEMROAD) && targtype == GOINGTODEMROAD)
		{
			RoadAt(target, target2)->transporter[transportRes] = -1;
		}
		else if((mode == GOINGTOSUPPLIER || mode == GOINGTODEMPOWL) && targtype == GOINGTODEMPOWL)
		{
			PowlAt(target, target2)->transporter[transportRes] = -1;
		}
		else if((mode == GOINGTOSUPPLIER || mode == GOINGTODEMPIPE) && targtype == GOINGTODEMPIPE)
		{
			PipeAt(target, target2)->transporter[transportRes] = -1;
		}
		
		targtype = NONE;

		if(driver >= 0)
		{
		LastNum("resetmode 1b");
			g_unit[driver].Disembark();
			driver = -1;
		}
	}
	
	LastNum("resetmode 2");

	//transportAmt = 0;
	target = target2 = -1;
	supplier = -1;
	mode = NONE;
	ResetGoal();

	LastNum("resetmode 3");
}

bool CUnit::CanMove()
{
	if(type == TRUCK)
	{
		if(driver < 0)
			return false;

		if(mode == NONE)
			return false;

		if(fuel <= 0)
			return false;
	}

	return true;
}

void InitUnits()
{
	// 0.01 driver wage/payment
	// FUEL_DISTANCE = TILE_SIZE*2.0f = 16*2 = 32
	// float payments = driveframes * 1000.0f / FRAME_RATE / DRIVE_WORK_DELAY;
	// DRIVER_WORK_DELAY = 4000
	// driveframes/trip = payments/trip * DRIVER_WORK_DELAY * FRAME_RATE / 1000.0f = 720 frames/trip
	// 720 frames/trip * 0.35 units/frame = 252 units/trip
	// 252 units/trip / 16 units/tile = 15.75 tiles/trip
	// 252 units/trip / 32 units/consumption = 7.875 consumption/trip
	// 7.875 consumption/trip * 0.5f fuel/consumption = 3.9375 fuel/trip
	// 3.9375 fuel/trip * 0.38 $/fuel = 1.49625 $/trip (fuel cost)
	// 0.01 profit/payment * 6 payments/trip - 0.06 profit/trip
	// 1.49625 profit/trip = 

	// 0.001 food/frame/lab * 30frame/second = 0.03 food/second/lab
	// 0.03 food/second/lab * 60 seconds/minute = 1.8 food/minute/lab
	// 5 lab/shop * 1.8 food/minute/lab = 9.0 food/minute/shop
	// shop cplx: 9.0 food needed/minute / 10 food produced/minute = 0.9 production ratio
	// shop cplx: 16.0 food needed/minute / 18 food produced/minute = 0.9 production ratio
	// shop cplx: 35.0 food needed/minute / 39 food produced/minute = 0.9 production ratio
	// shop cplx: 0.9 ( 5 produce + 5 production + 3 electricity + 10 labour + 2 transport ) / minute = ( 4.5 produce + 4.5 production + 2.7 electricity + 9 labour + 1.8 transport ) / minute
	// farm: 4.5 produce needed/minute / 5 produce harvested/minute = 0.9 production ratio
	// farm: 0.9 ( 10 labour + 1 electricity + 1 chemicals + 1 transport) / minute = ( 9 labour + 0.9 electricity + 0.9 chemicals + 0.9 transport) / minute
	// chem plant: 3.65 chemicals needed/minute / 42 chemicals produced/minute = 0.09 production ratio
	// chem plant: 0.09 ( 10 labour + 10 electricity ) / minute = ( 0.9 labour + 0.9 electricity ) / minute
	// factory: 4.5 production needed/minute / 18 production generated/minute = 0.25 production ratio
	// factory: 0.25 ( 10 labour + 10 electricity + 3 chemicals + 3 metal + 2 transport ) / minute = ( 2.5 labour + 2.5 electricity + 0.75 chemicals + 0.75 metal + 0.5 transport ) / minute
	// smelter: 0.75 metal needed/minute / 5 metal produced/minute = 0.15 production ratio
	// smelter: 0.15 ( 10 labour + 10 electricity + 5 ore + 10 chemicals + 2 transport ) / minute = ( 1.5 labour + 1.5 electricity + 0.75 ore + 1.5 chemicals + 0.3 transport ) / minute
	// mine: 0.75 ore needed/minute / 5 ore produced/minute = 0.15 production ratio
	// mine: 0.15 ( 10 labour ) / minute = ( 1.5 labour ) / minute
	// 2.7+0.9+0.9+2.5+1.5 = 8.5 electricity split between reactor and combustor
	// reactor: 4.25 electricity needed/minute / 10 electricity generated/minute = 0.425 production ratio
	// reactor: 0.425 ( 10 labour + 1 uronium + 1 transport ) / minute = ( 4.25 labour + 0.425 uronium + 0.425 transport ) / minute
	// mine: 0.425 uronium needed/minute / 5 uronium produced/minute = 0.085 production ratio < 0.15
	// combustor: 4.25 electricity / 10 electricity generated/minute = 0.425 production ratio
	// combustor: 0.425 ( 10 labour + 1 cohl + 1 transport ) / minute = ( 4.25 labour + 0.425 cohl + 0.425 transport ) / minute
	// mine: 0.425 cohl needed/minute / 5 cohl produced/minute = 0.085 production ratio < 0.15
	// refinery: (4.35 transport/minute + X transport/minute) * 3.9375 fuel needed/trip / 21 fuel produced/minute = ~1 production ratio
	// refinery: (4.35+X)*0.5625*( 1 labour + 1 electricity + 1 chemicals + 3 crude + 1 transport) / minute = 1 ( 1 labour + 1 electricity + 1 chemicals ) / minute + X transport / minute
	// (4.35+X)*0.5625 transport / minute = X transport / minute => (1-0.5625)X = 4.35*0.5625 => X = 4.35*0.5625/(1-0.5625) = 2.446875/0.4375 = 5.5928571428571428571428571428571 
	// X = 1
	// derrick: (4.35+X)*0.5625*3 crude needed/minute / 3 crude generated/minute = ~1 production ratio
	// derrick: 1*( 10 labour ) / minute = ( 10 labour ) / minute

	// building labour needed/minute = 9+9+0.9+2.5+1.5+1.5+4.25+4.25+10 = 42.9 building labour / minute => 42.9 seconds building labour/minute / 60 seconds/building labour minute = 0.715 building labour minutes/minute
	// + 0.715 rest minutes/minute
	// 720 frames/trip / 30 frames/second / 60 seconds/minute = 0.4 drive minutes/trip
	// transport needed/minute = 1.8+0.9+0.5+0.3+0.425+0.425+1 =  5.35 transport/minute => 5.35 trips/minute * 0.4 drive minutes/trip = 2.14 drive minutes/minute
	// + 1.74/4 rest minutes/minute = 0.435 rest minutes/minute
	// 3.9375 refuel/trip * 1 seconds refueling/1 fuel * 4.35 trips/minute / 60 seconds/minute refueling = 0.28546875 refuel minutes/minute
	// avg walk length = 6 tiles/walk => 6 tiles/walk * 16 units/tile / 0.2 tiles/frame / 30 frames/second / 60 seconds/minute = 0.26666666666666666666666666666667 minutes/walk
	// 5.35 truck walks/minute + 10 building walks/minute  = 15.35 walks/minute * 0.26666666 minutes walking/walk = 4.0933231 minutes walking/minute
	// labourers required: 0.715 + 0.715 + 2.14 + 0.435 + 4.0933231 = 8.0983231 labourers/minute => 8 lab
	// trucks required: 2.14 + 0.28546875 = 2.42546875 => 3 trucks

	// 8/5*10 food/shop/minute = 16 food/shop/minute

	// 42.9 seconds building labour/minute + 2.14 drive minutes/minute * 60 drive seconds/minute * 1/4 labour/drive second = ( 42.9 + 32.1 ) labour/minute * 0.01 $/labour = 0.75 $/minute
	// 0.75 earned $/minute - 0.01 $/housing/minute * 8 lab = 0.67 $/minute
	// 0.67 $/minute / 16 food/minute = 0.041875 $/food

	// shop cplx: 0.041875 $/food * 14.4 food - $0.01 profit = $0.593 budget
	// shop cplx: $0.593 = 4.5 produce + 4.5 production + 2.7 electricity + 9 labour + 1.8 transport = 4.5 produce + 4.5 production + 2.7 electricity + $0.09 + 1.8 transport
	// shop cplx: $0.503 = 4.5 produce + 4.5 production + 2.7 electricity + 1.8 transport
	// $0.503 = (9 labour + 0.9 electricity + 0.9 chemicals + 0.9 transport + $0.01) + (2.5 labour + 2.5 electricity + 0.75 chemicals + 0.75 metal + 0.5 transport + $0.01) + (2.7 electricity) + (1.8 transport)
	// $0.501 = 11.5 labour + 6.1 electricity + 1.65 chemicals + 0.75 metal + 3.2 transport
	// $0.501 = 11.5 labour + 6.1 electricity + 2.15 chemicals + (1.5 labour + 1.5 electricity + 0.75 ore + 1.5 chemicals + 0.3 transport + $0.01) + 3.2 transport
	// $0.491 = 12.5 labour + 7.6 electricity + 0.75 ore + 3.65 chemicals + 3.5 transport
	// $0.491 = 12.5 labour + 7.6 electricity + 0.75 ore + (0.9 labour + 0.9 electricity + $0.01) + 3.5 transport
	// $0.481 = 13.4 labour + 8.5 electricity + 0.75 ore + 3.5 transport
	// $0.481 = 13.4 labour + ( 4.25 labour + 0.425 uronium + 0.425 transport + $0.01 ) + ( 4.25 labour + 0.425 cohl + 0.425 transport + $0.01 ) + 0.75 ore + 3.5 transport
	// $0.461 = 21.9 labour + 0.425 uronium + 0.425 cohl + 0.75 ore + 4.35 transport 
	// $0.461 - $0.219 = 0.425 uronium + 0.425 cohl + 0.75 ore + 4.35 trip * 3.9375 fuel/trip
	// $0.242 = 0.425 uronium + 0.425 cohl + 0.75 ore + 17.128125 fuel
	// $0.242 = 0.425 uronium + 0.425 cohl + 0.75 ore + 1 labour + 1 electricity + 1 chemicals + 3 crude
	// $0.242 = 0.425 uronium + 0.425 cohl + 0.75 ore + 1 labour + 0.5/10( 10 labour + 1 uronium + 1 transport +$0.01 ) + 0.5/10( 10 labour + 1 cohl + 1 tranposrt + $0.01 ) + 1/42( 10 labour + electricity ) + 3 crude
	// $0.242 = 0.425 uronium + 0.425 cohl + 0.75 ore + 1 labour + 0.5 labour + 0.5 uronium + $0.01 + 0.5 labour + 0.5 cohl + $0.01 + 10/42 labour + 10/42 electricity + 3 crude
	// $0.242 - $0.02  = (0.425+0.5) uronium + (0.425+0.5) cohl + 0.75 ore + (1+10/42+0.5+0.5) labour + 5/42/10 (  10 labour + 1 cohl ) + $0.01 + 5/42/10 ( 10 labour + 1 uronium ) + $0.01 + 3 crude
	// $0.222 = 0.925 uronium + 0.925 cohl + 0.75 ore + 2.238 labour + 0.119 labour + 0.0119 cohl + 0.119 labour + 0.0119 uronium + 3 crude
	// $0.222 = 2.476 labour + 1.044 uronium + 1.044 cohl + 0.75 ore + 3 crude
	// $0.19724 = 1.044 uronium + 1.044 cohl + 0.75 ore + 3 crude
	// $0.1924/5.838 ( 1.044 /uronium + 1.044 /cohl + 0.75 /ore + 3 /crude )
	// $0.03295649194929770469338814662556 ( 1.044 /uronium + 1.044 /cohl + 0.75 /ore 3 /crude )
	// $0.03440657759506680369989722507708/uronium + $0.03440657759506680369989722507708/cohl + $0.02471736896197327852004110996917/ore + $0.09886947584789311408016443987668/crude
	// reactor: ( $0.03440657759506680369989722507708/uronium*1uronium + 10 labour*$0.01/labour + 1 transport ) / 10 electricity
	// reactor: 
	// refinery: ( $0.09886947584789311408016443987668/crude*3 crude + 1 electricity + 1 chemicals + 1 transport ) / 21 fuel
	// chem plant: ( $0.01/labour*10 labour + 10 electricity ) / 42 chemicals
	// $0.03320616029934709868104743489958 / fuel
	// $0.03651558337737460047565214999942 / electricity
	// $0.02107513889937490487515527380939 / chemical
	// mine: 0.15 ( 10 labour ) / 0.75 ore, uronium, cohl + $0.01
	// mine: 0.15 ( $0.1 ) / 0.75 ore, uronium, cohl + $0.01 
	// mine: $0.03 / ore, uronium, cohl
	// smelter: 0.15 ( 10 labour + 10 electricity + 5 ore + 10 chemicals + 2 transport ) / 0.75 metal
	// smelter: 0.15 ( $0.10 + $0.3651558337737460047565214999942 + 5*$0.04 + 10*$0.02107513889937490487515527380939 + 2*3.9375*0.03320616029934709868104743489958 ) / 0.75 metal + $0.01
	// smelter: 0.15 ( $0.10 + $0.3651558337737460047565214999942 + $0.20 + 0.26149851235735840211324854983419 ) / 0.75 metal + $0.01
	// smelter: $0.19533086922622088137395400996568 / metal
	// factory: 0.25 ( 10 labour + 10 electricity + 3 chemicals + 3 metal + 2 transport ) / 4.5 production
	// factory: 0.25 ( $0.10 + 0.3651558337737460047565214999942 + 0.06322541669812471462546582142817 + 0.58599260767866264412186202989704 + 0.26149851235735840211324854983419 ) / 4.5 production + $0.01
	// factory: $0.08643735391710509808983877228631 / production
	// farm: 0.9 ( 10 labour + 1 electricity + 1 chemicals + 1 transport) / 4.5 produce + $0.01
	// farm: 0.9 ( $0.10 + 0.03651558337737460047565214999942 + 0.02107513889937490487515527380939 + 3.9375*0.03320616029934709868104743489958 ) / 4.5 produce + $0.01
	// farm: $0.06766799569108574128148633974518 / produce
	// shop cplx: 0.9 ( 5 produce + 5 production + 3 electricity + 10 labour + 2 transport ) / 16 consumer goods + $0.01
	// shop cplx: 0.9 ( 5*0.06766799569108574128148633974518 + 5*0.08643735391710509808983877228631 + 3*0.03651558337737460047565214999942 + $0.10 + 2*3.9375*0.03320616029934709868104743489958 ) / 16 consumer goods + $0.01
	// shop cplx: 0.9 ( 0.3383399784554287064074316987259 + 0.43218676958552549044919386143155 + 0.10954675013212380142695644999826 + 0.10 + 0.26149851235735840211324854983419 ) / 16 consumer goods + 0.01
	// shop cplx: $0.07983842559233704752232171899943 / consumer good
	// shop cplx: $0.041875 = 0.9 ( 0.3383399784554287064074316987259 + 0.43218676958552549044919386143155 + 0.10954675013212380142695644999826 + 0.10 + 0.26149851235735840211324854983419 ) / X consumer goods + 0.01
	// shop cplx: $0.031875 = 0.9 ( 0.3383399784554287064074316987259 + 0.43218676958552549044919386143155 + 0.10954675013212380142695644999826 + 0.10 + 0.26149851235735840211324854983419 ) / X consumer goods
	// shop cplx: X consumer goods = 0.9 ( 0.3383399784554287064074316987259 + 0.43218676958552549044919386143155 + 0.10954675013212380142695644999826 + 0.10 + 0.26149851235735840211324854983419 ) / 0.031875
	// $0.041875 / consumer good
	// shop cplx: X consumer goods = 35.05615088556526307002815698795
	// elec plant: ( 1 labour + 1 electricity + 1 metal + 1 chemicals ) / 25 electronics + $0.01
	// elec plant: ( $0.01 + 0.03651558337737460047565214999942 + 0.19533086922622088137395400996568 + 0.02107513889937490487515527380939 ) / 25 electronics + $0.01
	// elec plant: 0.02051686366011881546899045735098 / electronics
	// r&d fac: ( 1 labour + 1 electricity + 1 metal + 1 electronics + 1 chemicals ) / 1 research + $0.01
	// r&d fac: ( $0.01 + 0.03651558337737460047565214999942 + 0.19533086922622088137395400996568 + 0.02051686366011881546899045735098 + 0.02107513889937490487515527380939 ) / 1 research + $0.01
	// r&d fac: $0.29343845516308920219375189112547 / research
	// quarry: ( 10 labour ) / 5 stone + $0.01
	// quary: $0.03 / stone
	// cem plant: ( 10 labour + 5 stone ) / 5 cement + $0.01
	// cem plant: ( $0.10 + $0.15 ) / 5 + $0.01
	// cem plant: $0.06 / cement
	// truck: 3.9375 fuel/trip / 6 payments/trip * $0.03320616029934709868104743489958/fuel + $0.01
	// truck: $0.03179154269644653350943737915285 / payment
	// derrick: ( 10 labour ) / 3 crude + $0.01
	// derrick: $0.04333333333333333333333333333333 / crude

	// 

	//			Unit		Name				Model										Walker?		Scale					Translate	Radius		Height,			Speed	Pass.	HP	Range	Shot Delay		Dmg		Roaded
	UDefine(LABOURER,		"Labourer",			"models\\labourer\\labourer.ms3d",			true,	Vec3f(0.1f,0.1f,0.1f),	Vec3f(0,0,0),	MIN_RADIUS,	MIN_RADIUS*5,	0.2f,	false,	1,	0,		0,				0,		false);
	UDefine(TRUCK,			"Truck",			"models\\truck\\truck.ms3d",				false,	Vec3f(1,1,1),			Vec3f(0,0,0),	MIN_RADIUS,	MIN_RADIUS,		0.35f,	false,	1,	0,		0,				0,		true);
	UCost(PRODUCTION, 2);
	UCost(METAL, 1);
	UCost(ELECTRONICS, 1);
	UDefine(BATTLECOMP,		"Battle Computer",	"models\\battlecomp\\battlecomp.ms3d",		true,	Vec3f(0.1f,0.1f,0.1f),	Vec3f(0,0,0),	MIN_RADIUS,	MIN_RADIUS*5,	0.2f,	false,	100,64,		100,			1.0f,	false);
	UCost(PRODUCTION, 1);
	UCost(METAL, 1);
	UCost(ELECTRONICS, 1);
	UAtkSnd("sounds\\mp5shot.wav");
	UAtkSnd("sounds\\mp5shot2.wav");
	UAtkSnd("sounds\\mp5shot3.wav");
	UAtkSnd("sounds\\mp5shot4.wav");
	UAtkSnd("sounds\\mp5shot5.wav");
	UAtkSnd("sounds\\mp5shot6.wav");
	UDefine(CARLYLE,		"Carlyle Tank",		"models\\carlyle\\carlyle.ms3d",			false,	Vec3f(0.1f,0.1f,0.1f),	Vec3f(0,0,0),	MIN_RADIUS*2,MIN_RADIUS,	0.4f,	false,	100,64,		500,			10,		false);
	UCost(PRODUCTION, 2);
	UCost(METAL, 2);
	UDeadEm(FLAME, Vec3f(0,1.5f,0));
	UDeadEm(PLUME, Vec3f(0,1.5f,0));
	UCost(ELECTRONICS, 20);
	UDefine(MECH,			"Mech",				"models\\mech\\mech.ms3d",					true,	Vec3f(0.1f,0.1f,0.1f),	Vec3f(0,0,0),	MIN_RADIUS*2,MIN_RADIUS*3,	0.2f,	false,	100,64,		500,			16.5f,	false);
	UCost(PRODUCTION, 2);
	UCost(METAL, 2);
	UCost(ELECTRONICS, 2);
}

void LoadUnitSprites()
{
	USprite(LABOURER, "gui\\centerp\\labourericon.png");
	USprite(TRUCK, "gui\\centerp\\truckicon.png");
	USprite(BATTLECOMP, "gui\\centerp\\battlecompicon.png");
	USprite(CARLYLE, "gui\\centerp\\carlyleicon.png");
}

void UpdateUnits()
{
	StartProfile(UPDATEUNITS);

	g_freetrucks.clear();

	LastNum("pre u.upd");

	for(int i=0; i<UNITS; i++)
		if(g_unit[i].on)
		{
			char msg[128];
			sprintf(msg, "upd unit %s mode=%d", g_unitType[g_unit[i].type].name, g_unit[i].mode);
			LastNum(msg);
			g_unit[i].Update();
		}

	LastNum("pre mng trips");

	if(g_freetrucks.size() > 0)
		ManageTrips();
	
	LastNum("post mng trips");

	EndProfile(UPDATEUNITS);
}

void CUnit::draw() 
{ 
	if(owner >= 0)
	{
		const float* owncol = facowncolor[owner];
		glUniform4f(g_shader[g_curS].m_slot[SLOT::OWNCOLOR], owncol[0], owncol[1], owncol[2], owncol[3]);
	}
	g_unitType[type].draw(frame[BODY_LOWER], camera.Position(), camera.Yaw()); 
}

void DrawUnits()
{
	StartProfile(DRAWUNITS);

	int m;

	for(int i=0; i<UNITS; i++)
	{
		if(!g_unit[i].on)
			continue;

		m = g_unit[i].mode;

		if(m == CONJOB)
			continue;

		if(m == NORMJOB)
			continue;

		if(m == SHOPPING)
			continue;

		if(m == RESTING)
			continue;

		if(m == ROADJOB)
			continue;

		if(m == POWLJOB)
			continue;

		if(m == PIPEJOB)
			continue;

		if(m == DRIVING)
			continue;

		g_unit[i].draw();
	}

	EndProfile(DRAWUNITS);
}

void CUnit::Update() 
{
	//if(hp <= 0)
	{
		//on = false;
		//return;
	}
	/*
	int uID = UnitID(this);

	if(uID == 2)
	{
		g_log<<"u[2]type = "<<g_unitType[type].name<<endl;
		g_log<<"u[2]mode = "<<mode<<endl;
		g_log<<"u[2]target = "<<target<<endl;
		g_log.flush();
	}*/

	if(type == LABOURER)
		UpdateLabourer();
	else if(type == TRUCK)
		UpdateTruck();
	else
		UpdateMilitary();

	//if(type == LABOURER)
	//	Chat("upd lab");
		
	//if(hp <= 0.0f && type == LABOURER)
	//	Chat(" hp < 0 ");

	if(mode != DRIVING)
	{
		//if(type == LABOURER)
		//	Chat("mov lab");
		Movement();
	}
	/*
	if(uID == 2)
				{
					g_log<<"u[2]dodrive"<<endl;
					g_log.flush();

					
				//if(type == TRUCK)
					{
						char msg[128];
						//sprintf(msg, "offgoal=%f vw=%f speed=%f", Magnitude(camera.Position() - subgoal), Magnitude(camera.Position() - camera.View()), Magnitude(camera.Velocity()));
						//Vec3f vw = camera.View() - camera.Position();
						//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", Magnitude(camera.Position() - subgoal), vw.x, vw.y, vw.z, Magnitude(camera.Velocity()));
						Vec3f vel = g_unit[5].camera.Velocity();
						//Vec3f offg = subgoal - camera.Position();
						//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", Magnitude(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
						sprintf(msg, "velafter2mov=%f,%f,%f", vel.x, vel.y, vel.z);
						int unID = 5;

						g_log<<"u["<<unID<<"]: "<<msg<<endl;
						g_log.flush();

						Chat(msg);
					}
				}*/
}

void CUnit::destroy()
{
	LastNum("destroying unit 0");

	if(type == LABOURER)
	{
		ResetMode();
		LastNum("destroying unit 1");
		//CUnit* u;
		int thisID = UnitID(this);

		if(home >= 0)
		{
			CBuilding* b = &g_building[home];
			
			for(int i=0; i<b->occupier.size(); i++)
			{
				if(b->occupier[i] == thisID)
				{
					b->occupier.erase( b->occupier.begin() + i );
					break;
				}
			}

			home = -1;
		}
		LastNum("destroying unit 2");
	}
	else if(type == TRUCK)
	{
		ResetMode();
	}
	
	LastNum("destroying unit 3");

	freecollidercells();
	on = false;
	
	LastNum("destroying unit 4");
}

void CUnit::Place(int t, int o, Vec3f p, int parnt, float ag, int male)
{
	on = true;
	owner = o;
	type = t;
	camera.PositionCamera(p.x, p.y, p.z, p.x, p.y, p.z+1, 0, 1, 0);
	step = 0;
	ResetGoal();
	target2 = -1; 
	home = -1;
	nopath = false;
	frame[BODY_LOWER] = 0;
	frame[BODY_UPPER] = 0;
	accum = 0;
	frameslookjobago = LOOKJOB_DELAY_MAX;
	CUnitType *tt = &g_unitType[t];
	hp = tt->hitpoints;
	underOrder = false;
}

void DrawUnitStatus(Matrix projmodlview)
{
	CUnit* u;
	Vec3f pos, v;
	string status;
	string status2;
	int m;
	Vec3f strafe = g_camera.Strafe();
	CPlayer* p = &g_player[g_localP];
	Vec4f screenpos;
	CBuilding* b;
	CBuildingType* bt;

	//glDisable(GL_TEXTURE_2D);
	//glColor4f(1, 1, 1, 1);
	
	//Matrix projmodlview = modelmat;
	//projmodlview.postMultiply(viewmat);
	//projmodlview.postMultiply(projection);

	//Matrix projmodlview = projection;
	//projmodlview.postMultiply(viewmat);
	//projmodlview.postMultiply(modelmat);

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != LABOURER && u->type != TRUCK)
			continue;

		m = u->mode;

		if(m == NORMJOB || m == CONJOB || m == ROADJOB || m == POWLJOB || m == PIPEJOB || m == DRIVING)
			continue;

		if(u->type == LABOURER)
		{/*
			status = u->firstname;
			status += " ";
			status += u->lastname;
			*/
			
			if(u->NeedFood() && u->NeedRest())
			{
				status = "Need food and rest";

				//if(u->currency < p->price[CONSUMERGOODS] || u->currency < p->price[LABOUR])
				//	status.append(" (Broke)");
				//else 
					if(u->path.size() <= 0)
				{
					//status.append(" (Stuck)");
					status.append(" (Broke?)");
					//char msg[128];
					//sprintf(msg, "%s #%d", status.c_str(), i);
					//Chat(msg);
				}
			}
			else if(u->NeedFood())
			{
				status = "Need food";

				//if(u->currency < p->price[CONSUMERGOODS])
				//	status.append(" (Broke)");
				//else if(u->path.size() <= 0)
				{
					//status.append(" (Stuck)");
					status.append(" (Broke?)");
					//char msg[128];
					//sprintf(msg, "%s #%d", status.c_str(), i);
					//Chat(msg);
				}
			}
			else if(u->NeedRest())
			{
				status = "Need rest";

				//if(u->currency < p->price[HOUSING])
				//	status.append(" (Broke)");
				//else 
					if(u->path.size() <= 0)
				{
					//status.append(" (Stuck)");
					status.append(" (Broke?)");/*
					char msg[128];
					Vec3f dp = u->camera.Position()-u->goal;
					sprintf(msg, "%s #%d m=%d h=%d pos-goal=%f,%f,%f", status.c_str(), i, u->mode, u->home, dp.x, dp.y, dp.z);
					Chat(msg);*/
				}
			}
			else if(m == MODE::NONE)
			{
				//status = "";
				//status += g_resource[RESOURCE::LABOUR].icon;
				//status += "Unemployed";
				status = "Unemployed";
			}
			else if(m == MODE::GOINGTONORMJOB)
			{
				status = "To work (";

				b = &g_building[ u->target ];
				bt = &g_buildingType[ b->type ];

				status += bt->name;
				status += ")";
			}
			else if(m == MODE::GOINGTOCONJOB)
			{
				status = "To construction (";

				b = &g_building[ u->target ];
				bt = &g_buildingType[ b->type ];

				status += bt->name;
				status += ")";
			}
			else if(m == MODE::GOINGTOROADJOB)
				status = "To construction (Road)";
			else if(m == MODE::GOINGTOPOWLJOB)
				status = "To construction (Powerline)";
			else if(m == MODE::GOINGTOPIPEJOB)
				status = "To construction (Pipeline)";
			else if(m == MODE::GOINGTOTRUCK)
				status = "To work (truck driving)";
			else
				status = "";
			
			char add[32];
			//sprintf(add, "%c%0.2f %c%d/%d %c%0.1f/%d", g_resource[CURRENC].icon, u->currency, g_resource[LABOUR].icon, (int)u->labour, (int)LABOURER_LABOUR, g_resource[CONSUMERGOODS].icon, u->fuel, (int)LABOURER_FUEL);
			sprintf(add, "%c%0.2f %c%d/%d %c%0.1f", g_resource[CURRENC].icon, u->currency, g_resource[LABOUR].icon, (int)u->labour, (int)LABOURER_LABOUR, g_resource[CONSUMERGOODS].icon, u->fuel, (int)MULTIPLY_FUEL);
			status2 = add;

			//char msg[128];
			//sprintf(msg, "%d", i);
			//status = msg;
		}
		else if(u->type == TRUCK)
		{
			if(u->driver < 0 && (u->mode == GOINGTODEMANDERB || u->mode == GOINGTOSUPPLIER || u->mode == GOINGTOREFUEL || u->mode == GOINGTODEMROAD || u->mode == GOINGTODEMPIPE || u->mode == GOINGTODEMPOWL))
				status = "Awaiting driver";
			else if(u->mode == GOINGTOREFUEL)
				status = "Going to refuel";
			else if(u->mode == GOINGTODEMANDERB)
			{
				status = "To demander (";

				b = &g_building[ u->target ];
				bt = &g_buildingType[ b->type ];

				status += bt->name;
				status += ")";
			}
			else if(u->mode == GOINGTODEMROAD)
			{
				status = "To demander (Road)";
			}
			else if(u->mode == GOINGTODEMPOWL)
			{
				status = "To demander (Powerline)";
			}
			else if(u->mode == GOINGTODEMPIPE)
			{
				status = "To demander (Pipeline)";
			}
			else if(u->mode == GOINGTOSUPPLIER)
			{
				status = "To supplier (";

				b = &g_building[ u->supplier ];
				bt = &g_buildingType[ b->type ];

				status += bt->name;
				status += ")";
			}
			else if(u->mode == ATDEMANDERB)
				status = "At demander";
			else if(u->mode == ATSUPPLIER)
				status = "At supplier";
			else if(u->NeedFood())
			{
				status = "Need fuel";
				if(u->path.size() <= 0)
					status += " (Stuck)";
			}
			else
				status = "";

			char add[16];
			sprintf(add, " %c%0.1f/%d", g_resource[ZETROL].icon, u->fuel, (int)TRUCK_FUEL);
			status2 = add;

			if(u->transportAmt > 0.0f)
			{
				//char add[16];
				sprintf(add, " %c%d", g_resource[u->transportRes].icon, (int)u->transportAmt);
				status2 += add;
			}
			
			//char msg[128];
			//sprintf(msg, "%d", i);
			//status = msg;
		}

		pos = u->camera.Position();

		screenpos.x = pos.x;
		screenpos.y = pos.y;
		screenpos.z = pos.z;
		screenpos.w = 1;

		screenpos.Transform(projmodlview);
		screenpos = screenpos / screenpos.w;
		screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
		screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;

		/*
		pos.Transform(projmodlview);
		pos.x = (pos.x * 0.5f + 0.5f) * g_width;
		pos.y = (pos.y * 0.5f + 0.5f) * g_height;*/

		const float color[] = {1.0f, 1.0f, 1.0f, STATUS_ALPHA};

		DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, status.c_str(), color);
		DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y+g_font[MAINFONT8].gheight, status2.c_str(), color);

		//screenpos = Unproject(pos);
		//glDrawText(MAINFONT16, screenpos.x, screenpos.y, status.c_str());
		
		/*
		glPushMatrix();

		glTranslatef(pos.x, pos.y + 10, pos.z);
		glRotatef(g_camera.Pitch(), strafe.x, strafe.y, strafe.z);
		glRotatef(180.0f + g_camera.Yaw(), 0, 1, 0);
		glScalef(2.0f, 2.0f, 2.0f);

		//glDraw3DText(MAINFONT32, status.c_str());

		glPopMatrix();*/
	}
	
	//glColor4f(1, 1, 1, 1);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);

	//glEnable(GL_TEXTURE_2D);
}

void DrawUnitStats(Matrix projmodlview)
{
	CUnit* u;
	Vec3f pos, v;
	string status;
	Vec3f strafe = g_camera.Strafe();
	CPlayer* p = &g_player[g_localP];
	Vec4f screenpos;

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		pos = u->camera.Position();

		screenpos.x = pos.x;
		screenpos.y = pos.y;
		screenpos.z = pos.z;
		screenpos.w = 1;

		screenpos.Transform(projmodlview);
		screenpos = screenpos / screenpos.w;
		screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
		screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;

		if(u->IsMilitary())
		{
			CUnitType* t = &g_unitType[u->type];

			//if(u->hp < t->hitpoints)
			{
				const float w = t->hitpoints;
				const float h = 1;
				
				DrawSquare(1, 0, 0, 0.75f, screenpos.x - w/2, screenpos.y + 5, screenpos.x + w/2, screenpos.y + 5 + h);

				if(u->hp > 0.0f)
					DrawSquare(0, 1, 0, 0.75f, screenpos.x - w/2, screenpos.y + 5, screenpos.x - w/2 + u->hp, screenpos.y + 5 + h);
			}
		}
	}
}


void DrawUnitStatus2()
{
	CUnit* u;
	Vec3f p, v;
	string status;
	int m;

	glColor4f(1, 1, 1, 1);

	for(int i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		p = u->camera.Position();
		//v = u->camera.Velocity();

		//char msg[128];
		//sprintf(msg, "[%d].pos = (%f,%f,%f) .vel = (%f,%f,%f)", i, p.x, p.y, p.z, v.x, v.y, v.z);
		//sprintf(msg, "[%d].yaw = %f", i, u->camera.Yaw());
		//Chat(msg);

		glPushMatrix();

		glTranslatef(p.x, p.y + 10, p.z);
		glRotatef(180.0f + g_camera.Yaw(), 0, 1, 0);
		glScalef(5.0f, 5.0f, 5.0f);

		m = u->mode;

		if(m == NONE)
			status = "None";
		else if(m == GOINGTONORMJOB)
			status = "GoingToNormJob";
		else if(m == NORMJOB)
			status = "NormJob";
		else if(m == GOINGTOCONJOB)
			status = "GoingToConJob";
		else if(m == CONJOB)
			status = "ConJob";
		else if(m == GOINGTOROADJOB)
			status = "GoingToRoadJob";
		else if(m == ROADJOB)
			status = "RoadJob";
		else if(m == GOINGTOPOWLJOB)
			status = "GoingToPowlJob";
		else if(m == POWLJOB)
			status = "PowlJob";
		else if(m == GOINGTOPIPEJOB)
			status = "GoingToPipeJob";
		else if(m == PIPEJOB)
			status = "PipeJob";
		else if(m == GOINGTOSUPPLIER)
			status = "GoingToSupplier";
		else if(m == GOINGTODEMANDERB)
			status = "GoingToDemander";
		else if(m == GOINGTOREFUEL)
			status = "GoingToRefuel";

		//glDraw3DText(MAINFONT32, status.c_str());

		glPopMatrix();
	}

	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

int NewUnit()
{
	for(int i=0; i<UNITS; i++)
		if(!g_unit[i].on)
			return i;

	return -1;
}

int UnitID(CUnit* u)
{
	for(int i=0; i<UNITS; i++)
		if(&g_unit[i] == u)
			return i;

	return -1;
}

int PlaceUnit(int type, int owner, Vec3f pos, int parent, float age, int male)
{
	int i = NewUnit();
	if(i < 0)
		return -1;

	if(type == LABOURER)
		owner = -1;

	g_unit[i].Place(type, owner, pos, parent, age, male);
	return i;
}

void PlaceUAround(CUnit* u, float borderleft, float bordertop, float borderright, float borderbottom, bool checkroad)
{
	CUnitType* t = &g_unitType[u->type];
	float r = t->radius;
	float r2 = r*2.0f;
	
	float left;
	float top;
	float right;
	float bottom;
	float x, z;

	if(checkroad)
	{
		for(int shell=0; shell<max(g_hmap.m_widthX, g_hmap.m_widthZ); shell++)
		{
			left = borderleft - r - r2*shell;
			top = bordertop - r - r2*shell;
			right = borderright + r + r2*shell;
			bottom = borderbottom + r + r2*shell;
		
			// bottom row
			for(x=left, z=bottom; x<=right-r2; x+=r2)
			{
				u->camera.MoveTo(Vec3f(x, Bilerp(x, z), z));
				if(!u->Collides2(true))
				{
					goto donepos;
				}
			}
		
			// right column
			for(x=right, z=bottom; z>=top+r2; z-=r2)
			{
				u->camera.MoveTo(Vec3f(x, Bilerp(x, z), z));
				if(!u->Collides2(true))
				{
					goto donepos;
				}
			}

			// top row
			for(x=right, z=top; x>=left+r2; x-=r2)
			{
				u->camera.MoveTo(Vec3f(x, Bilerp(x, z), z));
				if(!u->Collides2(true))
				{
					goto donepos;
				}
			}
		
			// left column
			for(x=left, z=top; z<=bottom-r2; z+=r2)
			{
				u->camera.MoveTo(Vec3f(x, Bilerp(x, z), z));
				if(!u->Collides2(true))
				{
					goto donepos;
				}
			}
		}
	}

	for(int shell=0; shell<max(g_hmap.m_widthX, g_hmap.m_widthZ); shell++)
	{
		left = borderleft - r - r2*shell;
		top = bordertop - r - r2*shell;
		right = borderright + r + r2*shell;
		bottom = borderbottom + r + r2*shell;
		
		// bottom row
		for(x=left, z=bottom; x<=right-r2; x+=r2)
		{
			u->camera.MoveTo(Vec3f(x, Bilerp(x, z), z));
			if(!u->Collides2(false))
			{
				goto donepos;
			}
		}
		
		// right column
		for(x=right, z=bottom; z>=top+r2; z-=r2)
		{
			u->camera.MoveTo(Vec3f(x, Bilerp(x, z), z));
			if(!u->Collides2(false))
			{
				goto donepos;
			}
		}

		// top row
		for(x=right, z=top; x>=left+r2; x-=r2)
		{
			u->camera.MoveTo(Vec3f(x, Bilerp(x, z), z));
			if(!u->Collides2(false))
			{
				goto donepos;
			}
		}
		
		// left column
		for(x=left, z=top; z<=bottom-r2; z+=r2)
		{
			u->camera.MoveTo(Vec3f(x, Bilerp(x, z), z));
			if(!u->Collides2(false))
			{
				goto donepos;
			}
		}
	}

	return;

donepos:
	u->ResetGoal();
	u->fillcollidercells();
	//return;
}