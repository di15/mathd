

#include "sim.h"
#include "../platform.h"
#include "../texture.h"
#include "../render/heightmap.h"
#include "../render/model.h"
#include "resources.h"
#include "../gui/icon.h"
#include "buildingtype.h"
#include "country.h"
#include "unittype.h"
#include "../render/foliage.h"
#include "road.h"
#include "powerline.h"
#include "crudepipeline.h"
#include "../render/water.h"
#include "selection.h"
#include "../sound/sound.h"

long long g_simframe = 0;

// Queue all the game resources and define objects
void Queue()
{
#if 0
#define ICON_DOLLARS		0
#define ICON_PESOS			1
#define ICON_EUROS			2
#define ICON_POUNDS			3
#define ICON_FRANCS			4
#define ICON_YENS			5
#define ICON_RUPEES			6
#define ICON_ROUBLES		7
#define ICON_LABOUR			8
#define ICON_HOUSING		9
#define ICON_FARMPRODUCT	10
#define ICON_FOOD			11
#define ICON_CHEMICALS		12
#define ICON_ELECTRONICS	13
#define ICON_RESEARCH		14
#define ICON_PRODUCTION		15
#define ICON_IRONORE		16
#define ICON_URANIUMORE		17
#define ICON_STEEL			18
#define ICON_CRUDEOIL		19
#define ICON_WSFUEL			20
#define ICON_STONE			21
#define ICON_CEMENT			22
#define ICON_ENERGY			23
#define ICON_ENRICHEDURAN	24
#define ICON_COAL			25
#define ICON_TIME			26
#define ICON_RETFUEL		27
#define ICON_LOGS			28
#define ICON_LUMBER			29
#define ICON_WATER			30
#define ICONS				31
#endif
	
	DefineIcon(ICON_DOLLARS, "gui/icons/dollars.png", ":dollar:");
	DefineIcon(ICON_PESOS, "gui/icons/pesos.png", ":peso:");
	DefineIcon(ICON_EUROS, "gui/icons/euros.png", ":euro:");
	DefineIcon(ICON_POUNDS, "gui/icons/pounds.png", ":pound:");
	DefineIcon(ICON_FRANCS, "gui/icons/francs.png", ":franc:");
	DefineIcon(ICON_YENS, "gui/icons/yens.png", ":yen:");
	DefineIcon(ICON_RUPEES, "gui/icons/rupees.png", ":rupee:");
	DefineIcon(ICON_ROUBLES, "gui/icons/roubles.png", ":rouble:");
	DefineIcon(ICON_LABOUR, "gui/icons/labour.png", ":labour:");
	DefineIcon(ICON_HOUSING, "gui/icons/housing.png", ":housing:");
	DefineIcon(ICON_FARMPRODUCT, "gui/icons/farmproducts.png", ":farmprod:");
	DefineIcon(ICON_FOOD, "gui/icons/food.png", ":food:");
	DefineIcon(ICON_CHEMICALS, "gui/icons/chemicals.png", ":chemicals:");
	DefineIcon(ICON_ELECTRONICS, "gui/icons/electronics.png", ":electronics:");
	DefineIcon(ICON_RESEARCH, "gui/icons/research.png", ":research:");
	DefineIcon(ICON_PRODUCTION, "gui/icons/production.png", ":production:");
	DefineIcon(ICON_IRONORE, "gui/icons/ironore.png", ":ironore:");
	DefineIcon(ICON_URANIUMORE, "gui/icons/uraniumore.png", ":uraniumore:");
	DefineIcon(ICON_STEEL, "gui/icons/steel.png", ":steel:");
	DefineIcon(ICON_CRUDEOIL, "gui/icons/crudeoil.png", ":crudeoil:");
	DefineIcon(ICON_WSFUEL, "gui/icons/fuelwholesale.png", ":wsfuel:");
	DefineIcon(ICON_STONE, "gui/icons/stone.png", ":stone:");
	DefineIcon(ICON_CEMENT, "gui/icons/cement.png", ":cement:");
	DefineIcon(ICON_ENERGY, "gui/icons/energy.png", ":energy:");
	DefineIcon(ICON_ENRICHEDURAN, "gui/icons/uranium.png", ":enricheduran:");
	DefineIcon(ICON_COAL, "gui/icons/coal.png", ":coal:");
	DefineIcon(ICON_TIME, "gui/icons/time.png", ":time:");
	DefineIcon(ICON_RETFUEL, "gui/icons/fuelretail.png", ":retfuel:");
	DefineIcon(ICON_LOGS, "gui/icons/logs.png", ":logs:");
	DefineIcon(ICON_LUMBER, "gui/icons/lumber.png", ":lumber:");
	DefineIcon(ICON_WATER, "gui/icons/water.png", ":water:");

#if 0
#define RES_DOLLARS			0
#define RES_PESOS			1
#define RES_EUROS			2
#define RES_POUNDS			3
#define RES_FRANCS			4
#define RES_YENS			5
#define RES_RUPEES			6
#define RES_ROUBLES			7
#define RES_LABOUR			8
#define RES_HOUSING			9
#define RES_FARMPRODUCTS	10
#define RES_FOOD			11
#define RES_CHEMICALS		12
#define RES_ELECTRONICS		13
#define RES_RESEARCH		14
#define RES_PRODUCTION		15
#define RES_IRONORE			16
#define RES_URANIUMORE		17
#define RES_STEEL			18
#define RES_CRUDEOIL		19
#define RES_WSFUEL			20
#define RES_RETFUEL			21
#define RES_STONE			22
#define RES_CEMENT			23
#define RES_ENERGY			24
#define RES_ENRICHEDURAN	25
#define RES_COAL			26
#define RES_LOGS			27
#define RES_LUMBER			28
#define RES_WATER			29
#define RESOURCES			30
#endif
	
#if 0
	void DefineRes(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a)
#endif

	DefineRes(RES_DOLLARS,		"Dollars",				"",							ICON_DOLLARS,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_PESOS,		"Pesos",				"",							ICON_PESOS,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_EUROS,		"Euros",				"",							ICON_EUROS,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_POUNDS,		"Pounds",				"",							ICON_POUNDS,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_FRANCS,		"Francs",				"",							ICON_FRANCS,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_YENS,			"Yens",					"",							ICON_YENS,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_RUPEES,		"Rupees",				"",							ICON_RUPEES,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_ROUBLES,		"Roubles",				"",							ICON_ROUBLES,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_LABOUR,		"Labour",				"",							ICON_LABOUR,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_HOUSING,		"Housing",				"",							ICON_HOUSING,		true,	true,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_FARMPRODUCTS,	"Farm Products",		"Fertile",					ICON_FARMPRODUCT,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_CHEMICALS,	"Chemicals",			"",							ICON_CHEMICALS,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_ELECTRONICS,	"Electronics",			"",							ICON_ELECTRONICS,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_RESEARCH,		"Research",				"",							ICON_RESEARCH,		false,	false,	true,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_PRODUCTION,	"Production",			"",							ICON_PRODUCTION,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_FOOD,			"Food",					"",							ICON_FOOD,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_IRONORE,		"Iron Ore",				"Iron Ore Deposit",			ICON_IRONORE,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_URANIUMORE,	"Uranium Ore",			"Uranium Ore Deposit",		ICON_URANIUMORE,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_STEEL,		"Steel",				"",							ICON_STEEL,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_CRUDEOIL,		"Crude Oil",			"Oil Deposit",				ICON_CRUDEOIL,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_WSFUEL,		"Wholesale Fuel",		"",							ICON_WSFUEL,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_RETFUEL,		"Retail Fuel",			"",							ICON_RETFUEL,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_STONE,		"Stone",				"Stone Deposit",			ICON_STONE,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_CEMENT,		"Cement",				"",							ICON_CEMENT,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_ENERGY,		"Energy",				"",							ICON_ENERGY,		true,	true,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_ENRICHEDURAN,	"Enriched Uranium",		"",							ICON_ENRICHEDURAN,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_COAL,			"Coal",					"Call Deposit",				ICON_COAL,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_LOGS,			"Logs",					"",							ICON_LOGS,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_LUMBER,		"Lumber",				"",							ICON_LUMBER,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_WATER,		"Water",				"",							ICON_WATER,			false,	false,	false,	1.0f,1.0f,1.0f,1.0f);

	QueueTexture(&g_tiletexs[TILE_SAND], "textures/terrain/default/sand.jpg", false);
	QueueTexture(&g_tiletexs[TILE_GRASS], "textures/terrain/default/grass.png", false);
	QueueTexture(&g_tiletexs[TILE_SNOW], "textures/terrain/default/snow.png", false);
	QueueTexture(&g_tiletexs[TILE_ROCK], "textures/terrain/default/rock.png", false);
	QueueTexture(&g_tiletexs[TILE_ROCK_NORM], "textures/terrain/default/rock.norm.jpg", false);
	QueueTexture(&g_tiletexs[TILE_CRACKEDROCK], "textures/terrain/default/crackedrock.png", false);
	QueueTexture(&g_tiletexs[TILE_CRACKEDROCK_NORM], "textures/terrain/default/crackedrock.norm.jpg", false);
	//QueueTexture(&g_water, "textures/terrain/default/water.png", false);
	QueueTexture(&g_watertex[WATER_TEX_GRADIENT], "textures/terrain/default/water.gradient.png", false);
	QueueTexture(&g_watertex[WATER_TEX_DETAIL], "textures/terrain/default/water.detail.jpg", false);
	QueueTexture(&g_watertex[WATER_TEX_SPECULAR], "textures/terrain/default/water.spec.jpg", false);
	QueueTexture(&g_watertex[WATER_TEX_NORMAL], "textures/terrain/default/water.norm.jpg", false);
	
	QueueTexture(&g_circle, "gui/circle.png", true);

	//QueueModel(&themodel, "models/battlecomp/battlecomp.ms3d", Vec3f(0.1f,0.1f,0.1f) * 100 / 64, Vec3f(0,100,0));
	
#if 0
#define RES_DOLLARS			0
#define RES_PESOS			1
#define RES_EUROS			2
#define RES_POUNDS			3
#define RES_FRANCS			4
#define RES_YENS			5
#define RES_RUPEES			6
#define RES_ROUBLES			7
#endif

	DefineCountry(0, 1, 0, 0, 1, RichText("USA (Red)"), RES_DOLLARS);
	DefineCountry(1, 0, 1, 0, 1, RichText("Mexico (Green)"), RES_PESOS);
	DefineCountry(2, 0, 0, 1, 1, RichText("EU (Blue)"), RES_EUROS);
	DefineCountry(3, 1, 0, 1, 1, RichText("UK (Purple)"), RES_POUNDS);
	DefineCountry(4, 1, 1, 0, 1, RichText("Switzerland (Yellow)"), RES_FRANCS);
	DefineCountry(5, 0, 1, 1, 1, RichText("Japan (Cyan)"), RES_YENS);
	DefineCountry(6, 150.0f/255.0f, 249.0f/255.0f, 123.0f/255.0f, 1, RichText("India (Light Green)"), RES_RUPEES);
	DefineCountry(7, 3.0f/255.0f, 53.0f/255.0f, 0.0f/255.0f, 1, RichText("Russia (Dark Green)"), RES_ROUBLES);

#if 0
#define UNIT_LABOURER		0
#if 0
#define UNIT_BLIZZAIAMECH	1
#define UNIT_918132634MECH	2
#define UNIT_CALMMECH		3
#define UNIT_GIZAMECH		4
#define UNIT_ZENITHMECH		5
#define UNIT_M1A2TANK		6
#define UNIT_OPLOTTANK		7
#define UNIT_LAVAPC			8
#define UNIT_GEPARDAA		9
#define UNIT_TYPES			10
#else
#define UNIT_TRUCK			1
#define UNIT_SOLDIER		2
#define UNIT_CAR			3
#define UNIT_CONCTRUCK		4
#define UNIT_TANKERTRUCK	5
#define UNIT_CARGOSHIP		6
#define UNIT_LOGHARVESTER	7
#define UNIT_LOGFORWARDER	8
#define UNIT_TYPES			9
#endif
#endif
	
#if 0
	//int type, const char* texrelative, Vec3i size, Vec2i bilbsize, const char* name, int starthp, bool landborne, bool walker, bool roaded, bool seaborne, bool airborne
	//DefineUnit(UNIT_LABOURER, "billboards/units/labourer/smiley.png", Vec3i(50, 50, 50));
	DefineUnit(UNIT_LABOURER, "billboards/units/labourer/worker.png", Vec3i(125, 8/3*125, 125), Vec2i(125, 8/3*125), "Worker", 100, true, true, false, false, false);
#if 0
	DefineUnit(UNIT_BLIZZAIAMECH, "billboards/units/mech/blizzaia.png", Vec3i(375, 95/67.0f*375, 375), Vec2i(575, 95/67.0f*575), "Blizzaia Mech");
	DefineUnit(UNIT_918132634MECH, "billboards/units/mech/918132634.png", Vec3i(375, 351/330.0f*375, 375), Vec2i(775, 351/330.0f*775), "918 Mech");
	DefineUnit(UNIT_CALMMECH, "billboards/units/mech/Calm_Render_thumb.png", Vec3i(375, 540/554.0f*375, 375), Vec2i(775, 540/554.0f*775), "Calm Mech");
	DefineUnit(UNIT_GIZAMECH, "billboards/units/mech/Giza_wanzer_00.png", Vec3i(375, 850/850.0f*375, 375), Vec2i(575, 850/850.0f*575), "Giza Mech");
	DefineUnit(UNIT_ZENITHMECH, "billboards/units/mech/Zenith_wanzer_01.png", Vec3i(375, 850/850.0f*375, 375), Vec2i(675, 850/850.0f*675), "Zenith Mech");
	DefineUnit(UNIT_M1A2TANK, "billboards/units/tank/01809-03579.png", Vec3i(375, 769/1680.0f*375, 375), Vec2i(775, 769/1680.0f*775), "M1A2 Tank");
	DefineUnit(UNIT_OPLOTTANK, "billboards/units/tank/Oplot main battle tanks 1.png", Vec3i(375, 929/1444.0f*375, 375), Vec2i(575, 929/1444.0f*575), "Oplot Tank");
	DefineUnit(UNIT_LAVAPC, "billboards/units/tank/QAMR_vehicle.png", Vec3i(375, 1302/2308.0f*375, 375), Vec2i(600, 1302/2308.0f*600), "LAV APC");
	DefineUnit(UNIT_GEPARDAA, "billboards/units/gepardaa/gepardaa.png", Vec3i(500, 1503/1951.0f*500, 500), Vec2i(500, 1503/1951.0f*500), "Gepard Anti-Air");
#else
	DefineUnit(UNIT_TRUCK, "billboards/units/truck/truck.png", Vec3i(250, 767/964.0f*250, 250), Vec2i(400, 767/964.0f*400), "Truck", 1000, true, false, true, false, false);
	DefineUnit(UNIT_CAR, "billboards/units/car/car.png", Vec3i(250, 246/388.0f*250, 250), Vec2i(300, 246/388.0f*300), "Car", 500, true, false, true, false, false);
	DefineUnit(UNIT_SOLDIER, "billboards/units/soldier/soldier.png", Vec3i(125, 1551/1134.0f*125, 125), Vec2i(175, 1551/1134.0f*175), "Soldier", 500, true, true, false, false, false);
	DefineUnit(UNIT_CONCTRUCK, "billboards/units/conctruck.png", Vec3i(250, 767/964.0f*250, 250), Vec2i(400, 333/512.0f*400), "Concrete Truck", 1000, true, false, true, false, false);
	DefineUnit(UNIT_TANKERTRUCK, "billboards/units/tankertruck.png", Vec3i(250, 767/964.0f*250, 250), Vec2i(600, 161/512.0f*600), "Tanker Truck", 1000, true, false, true, false, false);
	DefineUnit(UNIT_CARGOSHIP, "billboards/units/cargoship.png", Vec3i(250, 767/964.0f*250, 250), Vec2i(800, 400/600.0f*800), "Cargo Ship", 5000, false, false, false, true, false);
	DefineUnit(UNIT_LOGHARVESTER, "billboards/units/logharvester.png", Vec3i(250, 767/964.0f*250, 250), Vec2i(500, 250/330.0f*500), "Log Harvester", 1000, true, false, false, false, false);
	DefineUnit(UNIT_LOGFORWARDER, "billboards/units/forwarder.png", Vec3i(250, 767/964.0f*250, 250), Vec2i(500, 443/640.0f*500), "Log Forwarder", 1000, true, false, false, false, false);
#endif
#endif
#if 0
	DefineUnit(UNIT_ROBOSOLDIER, "models/battlecomp2013/battlecomp.ms3d", Vec3f(1,1,1)*182.0f/70.0f, Vec3f(0,70,0)*182.0f/70.0f, Vec3i(125, 8/3*125, 125), "Robot Soldier", 100, true, true, false, false, false);
#elif 0
	DefineUnit(UNIT_ROBOSOLDIER, "models/battlecomp2013/battlecomp.ms3d", Vec3f(1,1,1)*8.0f/3.0f*125.0f/70.0f, Vec3f(0,70,0)*8.0f/3.0f*125.0f/70.0f, Vec3i(125, 8.0f/3.0f*125, 125), "Robot Soldier", 100, true, true, false, false, false, 5);
#elif 0
	DefineUnit(UNIT_ROBOSOLDIER, "models/battlecomp2013/battlecomp.ms3d", Vec3f(1,1,1)*182.0f/70.0f, Vec3f(0,67,0)*182.0f/70.0f, Vec3i(50, 182, 50), "Robot Soldier", 100, true, true, false, false, false, 5);
#elif 1
	DefineUnit(UNIT_ROBOSOLDIER, "models/battlecomp2011simp/battlecomp.ms3d", Vec3f(1,1,1)*182.0f/70.0f, Vec3f(0,0,0)*182.0f/70.0f, Vec3i(125, 250, 125), "Robot Soldier", 100, true, true, false, false, false, 6, true);
	DefineUnit(UNIT_LABOURER, "models/labourer/labourer.ms3d", Vec3f(1,1,1)*182.0f/70.0f, Vec3f(0,0,0)*182.0f/70.0f, Vec3i(125, 250, 125), "Labourer", 100, true, true, false, false, false, 6, true);
#else
	DefineUnit(UNIT_ROBOSOLDIER, "models/battlecomp2013/battlecomp.ms3d", Vec3f(1,1,1)*500.0f/70.0f, Vec3f(0,67,0)*500.0f/70.0f, Vec3i(50, 182, 50), "Robot Soldier", 100, true, true, false, false, false, 5);
#endif

#if 0
	DefineFoliage(FOLIAGE_TREE1, "billboards/foliage/img1380909552.png", Vec3i(500, 7/3*500, 500));
	DefineFoliage(FOLIAGE_TREE2, "billboards/foliage/img1380909741.png", Vec3i(500, 500, 500));
	DefineFoliage(FOLIAGE_TREE3, "billboards/foliage/tree_34_png_by_gd08-d30unt3.png", Vec3i(500, 500, 500));
#endif
#if 0
	DefineFoliage(FOLIAGE_BUSH1, "billboards/foliage/tree_png_by_dbszabo1-d4ftcdo.png", Vec3i(200, 739/900.0f*200, 200));
#endif
	DefineFoliage(FOLIAGE_TREE1, "models/spruce1/spruce1.ms3d", Vec3f(20,20,20), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
	DefineFoliage(FOLIAGE_TREE2, "models/spruce1/spruce1.ms3d", Vec3f(20,20,20), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
	DefineFoliage(FOLIAGE_TREE3, "models/spruce1/spruce1.ms3d", Vec3f(20,20,20), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);

	QueueModel(&g_forestmodel, "models/spruce1forest/spruce1.ms3d", Vec3f(20,20,20), Vec3f(0,0,0));

#if 0
#define BUILDING_NONE			-1
#define BUILDING_APARTMENT		0
#define BUILDING_HOUSE			1
#define BUILDING_STORE		2
#define BUILDING_FARM			3
#define BUILDING_GASSTATION		4
#define BUILDING_BANK			5
#define BUILDING_CARDEALERSHIP	6
#define BUILDING_CARFACTORY		7
#define BUILDING_MILITARYBASE	8
#define BUILDING_TRUCKFACTORY	9
#define BUILDING_CONCGARBTRFAC	10
#define BUILDING_TANKERTRFAC	11
#define BUILDING_CURREXCENTER	12
#define BUILDING_COALMINE		13
#define BUILDING_HARBOUR		14
#define BUILDING_LUMBERMILL		15
#define BUILDING_FOREQFACTORY	16
#define BUILDING_CEMENTPLANT	17
#define BUILDING_CHEMPLANT		18
#define BUILDING_COALPOW		19
#define BUILDING_ELECPLANT		20
#define BUILDING_IRONMINE		21
#define BUILDING_URANMINE		22
#define BUILDING_NUCPOW			23
#define BUILDING_OILREFINERY	24
#define BUILDING_OILWELL		25
#define BUILDING_RESEARCHFAC	26
#define BUILDING_STEELMILL		27
#define BUILDING_STONEQUARRY	28
#define BUILDING_URANENRICHPL	29
#define BUILDING_WATERTOWER		30
#define BUILDING_WATERPUMPSTN	31
#define BUILDING_OFFSHOREOILRIG	32
#endif

	DefineBuildingType(BUILDING_APARTMENT, "Apartment Building", Vec2i(2,1), "models/apartment1/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/apartment1/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_APARTMENT, RES_CEMENT, 5);
	BuildingConMat(BUILDING_APARTMENT, RES_LABOUR, 10);
	BuildingConMat(BUILDING_APARTMENT, RES_STEEL, 10);
	BuildingConMat(BUILDING_APARTMENT, RES_LUMBER, 10);
	BuildingInput(BUILDING_APARTMENT, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	BuildingOutput(BUILDING_APARTMENT, RES_HOUSING, 5);
	
	DefineBuildingType(BUILDING_HOUSE, "House", Vec2i(1,1), "models/house/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/house_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_HOUSE, RES_CEMENT, 5);
	BuildingConMat(BUILDING_HOUSE, RES_LABOUR, 10);
	BuildingConMat(BUILDING_HOUSE, RES_STEEL, 10);
	BuildingConMat(BUILDING_HOUSE, RES_LUMBER, 10);
	BuildingInput(BUILDING_HOUSE, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	BuildingOutput(BUILDING_HOUSE, RES_HOUSING, 1);
	
	DefineBuildingType(BUILDING_FARM, "Farm", Vec2i(1,1), "models/farm/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/farm_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_FARM, RES_CEMENT, 5);
	BuildingConMat(BUILDING_FARM, RES_LABOUR, 10);
	BuildingConMat(BUILDING_FARM, RES_STEEL, 10);
	BuildingConMat(BUILDING_FARM, RES_LUMBER, 10);
	BuildingInput(BUILDING_FARM, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	BuildingOutput(BUILDING_FARM, RES_FARMPRODUCTS, 5);
	
	DefineBuildingType(BUILDING_GASSTATION, "Gas Station", Vec2i(1,1), "models/gasstation/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/gasstation_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_GASSTATION, RES_CEMENT, 5);
	BuildingConMat(BUILDING_GASSTATION, RES_LABOUR, 10);
	BuildingConMat(BUILDING_GASSTATION, RES_STEEL, 10);
	BuildingInput(BUILDING_GASSTATION, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	BuildingOutput(BUILDING_GASSTATION, RES_RETFUEL, 5);
	
	DefineBuildingType(BUILDING_BANK, "Bank", Vec2i(1,1), "models/bank/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/bank_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_BANK, RES_CEMENT, 5);
	BuildingConMat(BUILDING_BANK, RES_LABOUR, 10);
	BuildingConMat(BUILDING_BANK, RES_STEEL, 10);
	BuildingInput(BUILDING_BANK, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_CARDEALERSHIP, "Car Dealership", Vec2i(1,1), "models/cardealership/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/cardealership_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_CARDEALERSHIP, RES_CEMENT, 5);
	BuildingConMat(BUILDING_CARDEALERSHIP, RES_LABOUR, 10);
	BuildingConMat(BUILDING_CARDEALERSHIP, RES_STEEL, 10);
	BuildingInput(BUILDING_CARDEALERSHIP, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_CARFACTORY, "Car Factory", Vec2i(1,1), "models/carfactory/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/carfactory_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_CARFACTORY, RES_CEMENT, 5);
	BuildingConMat(BUILDING_CARFACTORY, RES_LABOUR, 10);
	BuildingConMat(BUILDING_CARFACTORY, RES_STEEL, 10);
	BuildingInput(BUILDING_CARFACTORY, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_MILITARYBASE, "Military Base", Vec2i(1,1), "models/milbase/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/milbase_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_MILITARYBASE, RES_CEMENT, 5);
	BuildingConMat(BUILDING_MILITARYBASE, RES_LABOUR, 10);
	BuildingConMat(BUILDING_MILITARYBASE, RES_STEEL, 10);
	BuildingInput(BUILDING_MILITARYBASE, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_TRUCKFACTORY, "Truck Factory", Vec2i(1,1), "models/truckfactory/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/truckfactory_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_TRUCKFACTORY, RES_CEMENT, 5);
	BuildingConMat(BUILDING_TRUCKFACTORY, RES_LABOUR, 10);
	BuildingConMat(BUILDING_TRUCKFACTORY, RES_STEEL, 10);
	BuildingInput(BUILDING_TRUCKFACTORY, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_CONCGARBTRFAC, "Concrete, Garbage Truck Factory", Vec2i(1,1), "models/concgarbtrfac/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/concgarbtrfac_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_CONCGARBTRFAC, RES_CEMENT, 5);
	BuildingConMat(BUILDING_CONCGARBTRFAC, RES_LABOUR, 10);
	BuildingConMat(BUILDING_CONCGARBTRFAC, RES_STEEL, 10);
	BuildingInput(BUILDING_CONCGARBTRFAC, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_TANKERTRFAC, "Tanker Truck Factory", Vec2i(1,1), "models/tankertrfac/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/tankertrfac_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_TANKERTRFAC, RES_CEMENT, 5);
	BuildingConMat(BUILDING_TANKERTRFAC, RES_LABOUR, 10);
	BuildingConMat(BUILDING_TANKERTRFAC, RES_STEEL, 10);
	BuildingInput(BUILDING_TANKERTRFAC, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_CURREXCENTER, "Currency Exchange Center", Vec2i(1,1), "models/currexcenter/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/currexcenter_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_CURREXCENTER, RES_CEMENT, 5);
	BuildingConMat(BUILDING_CURREXCENTER, RES_LABOUR, 10);
	BuildingConMat(BUILDING_CURREXCENTER, RES_STEEL, 10);
	BuildingInput(BUILDING_CURREXCENTER, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);

	DefineBuildingType(BUILDING_COALMINE, "Coal Mine", Vec2i(1,1), "models/coalmine/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/coalmine_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_COAL);
	BuildingConMat(BUILDING_COALMINE, RES_CEMENT, 5);
	BuildingConMat(BUILDING_COALMINE, RES_LABOUR, 10);
	BuildingConMat(BUILDING_COALMINE, RES_STEEL, 10);
	BuildingInput(BUILDING_COALMINE, RES_ENERGY, 5);
	BuildingInput(BUILDING_COALMINE, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_HARBOUR, "Harbour", Vec2i(1,1), "models/harbour/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/harbour_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_COASTAL, RES_NONE);
	BuildingConMat(BUILDING_HARBOUR, RES_CEMENT, 5);
	BuildingConMat(BUILDING_HARBOUR, RES_LABOUR, 10);
	BuildingConMat(BUILDING_HARBOUR, RES_STEEL, 10);
	BuildingInput(BUILDING_HARBOUR, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_LUMBERMILL, "Lumber Mill", Vec2i(1,1), "models/lumbermill/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/lumbermill_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_LUMBERMILL, RES_CEMENT, 5);
	BuildingConMat(BUILDING_LUMBERMILL, RES_LABOUR, 10);
	BuildingConMat(BUILDING_LUMBERMILL, RES_STEEL, 10);
	BuildingInput(BUILDING_LUMBERMILL, RES_ENERGY, 5);
	BuildingInput(BUILDING_LUMBERMILL, RES_LOGS, 5);
	BuildingInput(BUILDING_LUMBERMILL, RES_WATER, 5);
	BuildingOutput(BUILDING_LUMBERMILL, RES_LUMBER, 5);
	
	DefineBuildingType(BUILDING_FOREQFACTORY, "Forestry Equipment Factory", Vec2i(1,1), "models/foreqfactory/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/foreqfactory_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_FOREQFACTORY, RES_CEMENT, 5);
	BuildingConMat(BUILDING_FOREQFACTORY, RES_LABOUR, 10);
	BuildingConMat(BUILDING_COALPOW, RES_STEEL, 10);
	BuildingInput(BUILDING_APARTMENT, RES_ENERGY, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_CEMENTPLANT, "Cement Plant", Vec2i(1,1), "models/cemplant/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/cemplant_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_CEMENTPLANT, RES_CEMENT, 5);
	BuildingConMat(BUILDING_CEMENTPLANT, RES_LABOUR, 10);
	BuildingConMat(BUILDING_CEMENTPLANT, RES_STEEL, 10);
	BuildingInput(BUILDING_CEMENTPLANT, RES_ENERGY, 5);
	BuildingInput(BUILDING_CEMENTPLANT, RES_STONE, 5);
	BuildingInput(BUILDING_CEMENTPLANT, RES_WATER, 5);
	BuildingOutput(BUILDING_CEMENTPLANT, RES_CEMENT, 5);
	
	DefineBuildingType(BUILDING_CHEMPLANT, "Chemical Plant", Vec2i(1,1), "models/chemplant/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/chemplant_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_CHEMPLANT, RES_CEMENT, 5);
	BuildingConMat(BUILDING_CHEMPLANT, RES_LABOUR, 10);
	BuildingConMat(BUILDING_CHEMPLANT, RES_STEEL, 10);
	BuildingInput(BUILDING_CHEMPLANT, RES_ENERGY, 5);
	BuildingInput(BUILDING_CHEMPLANT, RES_WATER, 5);
	BuildingOutput(BUILDING_CHEMPLANT, RES_CHEMICALS, 5);
	
	DefineBuildingType(BUILDING_COALPOW, "Coal-Fired Powerplant", Vec2i(1,1), "models/coalpow/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/coalpow_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_COALPOW, RES_CEMENT, 5);
	BuildingConMat(BUILDING_COALPOW, RES_LABOUR, 10);
	BuildingConMat(BUILDING_COALPOW, RES_STEEL, 10);
	BuildingInput(BUILDING_COALPOW, RES_COAL, 5);
	BuildingInput(BUILDING_APARTMENT, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_ELECPLANT, "Electronics Manufacturing Plant", Vec2i(1,1), "models/elecplant/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/elecplant_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_ELECPLANT, RES_CEMENT, 5);
	BuildingConMat(BUILDING_ELECPLANT, RES_LABOUR, 10);
	BuildingConMat(BUILDING_ELECPLANT, RES_STEEL, 10);
	BuildingInput(BUILDING_ELECPLANT, RES_ENERGY, 5);
	BuildingInput(BUILDING_ELECPLANT, RES_STEEL, 5);
	BuildingInput(BUILDING_ELECPLANT, RES_LABOUR, 5);
	BuildingInput(BUILDING_ELECPLANT, RES_WATER, 5);
	BuildingOutput(BUILDING_ELECPLANT, RES_ELECTRONICS, 5);

	//DefineBuildingType(BUILDING_FACTORY, "Factory", Vec2i(1,1), "models/factory/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/factory_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND);
	
	DefineBuildingType(BUILDING_IRONMINE, "Iron Mine", Vec2i(1,1), "models/ironmine/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/ironmine_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_IRONORE);
	BuildingConMat(BUILDING_IRONMINE, RES_CEMENT, 5);
	BuildingConMat(BUILDING_IRONMINE, RES_LABOUR, 10);
	BuildingConMat(BUILDING_IRONMINE, RES_STEEL, 10);
	BuildingInput(BUILDING_IRONMINE, RES_ENERGY, 5);
	BuildingInput(BUILDING_IRONMINE, RES_WATER, 5);
	BuildingOutput(BUILDING_IRONMINE, RES_IRONORE, 5);
	
	DefineBuildingType(BUILDING_URANMINE, "Uranium Mine", Vec2i(1,1), "models/uranmine/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/uranmine_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_URANIUMORE);
	BuildingConMat(BUILDING_URANMINE, RES_CEMENT, 5);
	BuildingConMat(BUILDING_URANMINE, RES_LABOUR, 10);
	BuildingConMat(BUILDING_URANMINE, RES_STEEL, 10);
	BuildingInput(BUILDING_URANMINE, RES_ENERGY, 5);
	BuildingInput(BUILDING_URANMINE, RES_WATER, 5);
	BuildingOutput(BUILDING_URANMINE, RES_URANIUMORE, 5);
	
	DefineBuildingType(BUILDING_NUCPOW, "Nuclear Powerplant", Vec2i(1,1), "models/nucpow/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/nucpow_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_NUCPOW, RES_CEMENT, 5);
	BuildingConMat(BUILDING_NUCPOW, RES_LABOUR, 10);
	BuildingConMat(BUILDING_NUCPOW, RES_STEEL, 10);
	BuildingInput(BUILDING_NUCPOW, RES_ENRICHEDURAN, 5);
	BuildingInput(BUILDING_NUCPOW, RES_WATER, 5);
	BuildingOutput(BUILDING_NUCPOW, RES_ENERGY, 500);
	
	DefineBuildingType(BUILDING_OILREFINERY, "Oil Refinery", Vec2i(1,1), "models/oilrefinery/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/oilrefinery_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_OILREFINERY, RES_CEMENT, 5);
	BuildingConMat(BUILDING_OILREFINERY, RES_LABOUR, 10);
	BuildingConMat(BUILDING_OILREFINERY, RES_STEEL, 10);
	BuildingInput(BUILDING_OILREFINERY, RES_CRUDEOIL, 5);
	BuildingInput(BUILDING_OILREFINERY, RES_WATER, 5);
	BuildingOutput(BUILDING_OILREFINERY, RES_WSFUEL, 5);
	
	DefineBuildingType(BUILDING_OILWELL, "Oil Well", Vec2i(1,1), "models/oilwell/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/oilwell_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_CRUDEOIL);
	BuildingConMat(BUILDING_OILWELL, RES_CEMENT, 5);
	BuildingConMat(BUILDING_OILWELL, RES_LABOUR, 10);
	BuildingConMat(BUILDING_OILWELL, RES_STEEL, 10);
	BuildingInput(BUILDING_OILWELL, RES_ENERGY, 5);
	BuildingInput(BUILDING_OILWELL, RES_WATER, 5);
	BuildingOutput(BUILDING_OILWELL, RES_CRUDEOIL, 5);
	
	DefineBuildingType(BUILDING_RESEARCHFAC, "Research Facility", Vec2i(1,1), "models/researchfac/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/researchfac_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_RESEARCHFAC, RES_CEMENT, 5);
	BuildingConMat(BUILDING_RESEARCHFAC, RES_LABOUR, 10);
	BuildingConMat(BUILDING_RESEARCHFAC, RES_STEEL, 10);
	BuildingInput(BUILDING_RESEARCHFAC, RES_ENERGY, 5);
	BuildingInput(BUILDING_RESEARCHFAC, RES_WATER, 5);
	BuildingOutput(BUILDING_RESEARCHFAC, RES_RESEARCH, 5);
	
	DefineBuildingType(BUILDING_STORE, "Store", Vec2i(2,1), "models/store1/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/store1/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_STORE, RES_CEMENT, 5);
	BuildingConMat(BUILDING_STORE, RES_LABOUR, 10);
	BuildingConMat(BUILDING_STORE, RES_STEEL, 10);
	BuildingInput(BUILDING_STORE, RES_ENERGY, 5);
	BuildingInput(BUILDING_STORE, RES_FARMPRODUCTS, 5);
	BuildingInput(BUILDING_STORE, RES_WATER, 5);
	BuildingOutput(BUILDING_STORE, RES_FOOD, 5);
	
	DefineBuildingType(BUILDING_STEELMILL, "Steel Mill", Vec2i(1,1), "models/steelmill/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/steelmill_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_STEELMILL, RES_CEMENT, 5);
	BuildingConMat(BUILDING_STEELMILL, RES_LABOUR, 10);
	BuildingConMat(BUILDING_STEELMILL, RES_STEEL, 10);
	BuildingInput(BUILDING_STEELMILL, RES_ENERGY, 5);
	BuildingInput(BUILDING_STEELMILL, RES_IRONORE, 5);
	BuildingInput(BUILDING_STEELMILL, RES_WATER, 5);
	BuildingOutput(BUILDING_STEELMILL, RES_STEEL, 5);
	
	DefineBuildingType(BUILDING_STONEQUARRY, "Stone Quarry", Vec2i(1,1), "models/stonequarry/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/stonequarry_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_STONE);
	BuildingConMat(BUILDING_STONEQUARRY, RES_CEMENT, 5);
	BuildingConMat(BUILDING_STONEQUARRY, RES_LABOUR, 10);
	BuildingConMat(BUILDING_STONEQUARRY, RES_STEEL, 10);
	BuildingInput(BUILDING_STONEQUARRY, RES_ENERGY, 5);
	BuildingInput(BUILDING_STONEQUARRY, RES_WATER, 5);
	BuildingOutput(BUILDING_STONEQUARRY, RES_STONE, 5);
	
	DefineBuildingType(BUILDING_URANENRICHPL, "Uranium Enrichment Plant", Vec2i(1,1), "models/uranenrichpl/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/uranenrichpl_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_URANENRICHPL, RES_CEMENT, 5);
	BuildingConMat(BUILDING_URANENRICHPL, RES_LABOUR, 10);
	BuildingConMat(BUILDING_URANENRICHPL, RES_STEEL, 10);
	BuildingInput(BUILDING_URANENRICHPL, RES_ENERGY, 5);
	BuildingInput(BUILDING_URANENRICHPL, RES_URANIUMORE, 5);
	BuildingInput(BUILDING_URANENRICHPL, RES_WATER, 5);
	BuildingOutput(BUILDING_URANENRICHPL, RES_ENRICHEDURAN, 5);
	
	DefineBuildingType(BUILDING_WATERTOWER, "Water Tower", Vec2i(1,1), "models/watertower/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/watertower_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BuildingConMat(BUILDING_WATERTOWER, RES_CEMENT, 5);
	BuildingConMat(BUILDING_WATERTOWER, RES_LABOUR, 10);
	BuildingConMat(BUILDING_WATERTOWER, RES_STEEL, 10);
	BuildingInput(BUILDING_WATERTOWER, RES_ENERGY, 5);
	BuildingOutput(BUILDING_WATERTOWER, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_WATERPUMPSTN, "Water Pumping Station", Vec2i(1,1), "models/waterpumpstn/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/waterpumpstn_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_COASTAL, RES_NONE);
	BuildingConMat(BUILDING_WATERPUMPSTN, RES_CEMENT, 5);
	BuildingConMat(BUILDING_WATERPUMPSTN, RES_LABOUR, 10);
	BuildingConMat(BUILDING_WATERPUMPSTN, RES_STEEL, 10);
	BuildingInput(BUILDING_WATERPUMPSTN, RES_ENERGY, 5);
	BuildingOutput(BUILDING_WATERPUMPSTN, RES_WATER, 5);
	
	DefineBuildingType(BUILDING_OFFSHOREOILRIG, "Offshore Oil Rig", Vec2i(1,1), "models/offshoreoilrig/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/offshoreoilrig_c/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_SEA, RES_CRUDEOIL);
	BuildingConMat(BUILDING_OFFSHOREOILRIG, RES_CEMENT, 5);
	BuildingConMat(BUILDING_OFFSHOREOILRIG, RES_LABOUR, 10);
	BuildingConMat(BUILDING_OFFSHOREOILRIG, RES_STEEL, 10);
	BuildingInput(BUILDING_OFFSHOREOILRIG, RES_LABOUR, 5);
	BuildingOutput(BUILDING_OFFSHOREOILRIG, RES_CRUDEOIL, 5);
	
	Zero(g_roadcost);
	g_roadcost[RES_LABOUR] = 1;
	g_roadcost[RES_CEMENT] = 1;
    
	DefineRoad(CONNECTION_NOCONNECTION, CONSTRUCTION, "models/road/1_c.ms3d");
	DefineRoad(CONNECTION_NORTH, CONSTRUCTION, "models/road/n_c.ms3d");
	DefineRoad(CONNECTION_EAST, CONSTRUCTION, "models/road/e_c.ms3d");
	DefineRoad(CONNECTION_SOUTH, CONSTRUCTION, "models/road/s_c.ms3d");
	DefineRoad(CONNECTION_WEST, CONSTRUCTION, "models/road/w_c.ms3d");
	DefineRoad(CONNECTION_NORTHEAST, CONSTRUCTION, "models/road/ne_c.ms3d");
	DefineRoad(CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/road/ns_c.ms3d");
	DefineRoad(CONNECTION_EASTSOUTH, CONSTRUCTION, "models/road/es_c.ms3d");
	DefineRoad(CONNECTION_NORTHWEST, CONSTRUCTION, "models/road/nw_c.ms3d");
	DefineRoad(CONNECTION_EASTWEST, CONSTRUCTION, "models/road/ew_c.ms3d");
	DefineRoad(CONNECTION_SOUTHWEST, CONSTRUCTION, "models/road/sw_c.ms3d");
	DefineRoad(CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/road/esw_c.ms3d");
	DefineRoad(CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/road/nsw_c.ms3d");
	DefineRoad(CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/road/new_c.ms3d");
	DefineRoad(CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/road/nes_c.ms3d");
	DefineRoad(CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/road/nesw_c.ms3d");
	DefineRoad(CONNECTION_NOCONNECTION, FINISHED, "models/road/1.ms3d");
	//DefineRoad(CONNECTION_NOCONNECTION, FINISHED, "models/road/flat.ms3d");
	DefineRoad(CONNECTION_NORTH, FINISHED, "models/road/n.ms3d");
	DefineRoad(CONNECTION_EAST, FINISHED, "models/road/e.ms3d");
	DefineRoad(CONNECTION_SOUTH, FINISHED, "models/road/s.ms3d");
	DefineRoad(CONNECTION_WEST, FINISHED, "models/road/w.ms3d");
	DefineRoad(CONNECTION_NORTHEAST, FINISHED, "models/road/ne.ms3d");
	DefineRoad(CONNECTION_NORTHSOUTH, FINISHED, "models/road/ns.ms3d");
	DefineRoad(CONNECTION_EASTSOUTH, FINISHED, "models/road/es.ms3d");
	DefineRoad(CONNECTION_NORTHWEST, FINISHED, "models/road/nw.ms3d");
	DefineRoad(CONNECTION_EASTWEST, FINISHED, "models/road/ew.ms3d");
	DefineRoad(CONNECTION_SOUTHWEST, FINISHED, "models/road/sw.ms3d");
	DefineRoad(CONNECTION_EASTSOUTHWEST, FINISHED, "models/road/esw.ms3d");
	DefineRoad(CONNECTION_NORTHSOUTHWEST, FINISHED, "models/road/nsw.ms3d");
	DefineRoad(CONNECTION_NORTHEASTWEST, FINISHED, "models/road/new.ms3d");
	DefineRoad(CONNECTION_NORTHEASTSOUTH, FINISHED, "models/road/nes.ms3d");
	DefineRoad(CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/road/nesw.ms3d");

	Zero(g_powlcost);
	g_powlcost[RES_LABOUR] = 1;
	g_powlcost[RES_STEEL] = 1;
    
	DefinePowl(CONNECTION_NOCONNECTION, CONSTRUCTION, "models/powerline/1_c.ms3d");
	DefinePowl(CONNECTION_NORTH, CONSTRUCTION, "models/powerline/n_c.ms3d");
	DefinePowl(CONNECTION_EAST, CONSTRUCTION, "models/powerline/e_c.ms3d");
	DefinePowl(CONNECTION_SOUTH, CONSTRUCTION, "models/powerline/s_c.ms3d");
	DefinePowl(CONNECTION_WEST, CONSTRUCTION, "models/powerline/w_c.ms3d");
	DefinePowl(CONNECTION_NORTHEAST, CONSTRUCTION, "models/powerline/ne_c.ms3d");
	DefinePowl(CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/powerline/ns_c.ms3d");
	DefinePowl(CONNECTION_EASTSOUTH, CONSTRUCTION, "models/powerline/es_c.ms3d");
	DefinePowl(CONNECTION_NORTHWEST, CONSTRUCTION, "models/powerline/nw_c.ms3d");
	DefinePowl(CONNECTION_EASTWEST, CONSTRUCTION, "models/powerline/ew_c.ms3d");
	DefinePowl(CONNECTION_SOUTHWEST, CONSTRUCTION, "models/powerline/sw_c.ms3d");
	DefinePowl(CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/powerline/esw_c.ms3d");
	DefinePowl(CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/powerline/nsw_c.ms3d");
	DefinePowl(CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/powerline/new_c.ms3d");
	DefinePowl(CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/powerline/nes_c.ms3d");
	DefinePowl(CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/powerline/nesw_c.ms3d");
	DefinePowl(CONNECTION_NOCONNECTION, FINISHED, "models/powerline/1.ms3d");
	DefinePowl(CONNECTION_NORTH, FINISHED, "models/powerline/n.ms3d");
	DefinePowl(CONNECTION_EAST, FINISHED, "models/powerline/e.ms3d");
	DefinePowl(CONNECTION_SOUTH, FINISHED, "models/powerline/s.ms3d");
	DefinePowl(CONNECTION_WEST, FINISHED, "models/powerline/w.ms3d");
	DefinePowl(CONNECTION_NORTHEAST, FINISHED, "models/powerline/ne.ms3d");
	DefinePowl(CONNECTION_NORTHSOUTH, FINISHED, "models/powerline/ns.ms3d");
	DefinePowl(CONNECTION_EASTSOUTH, FINISHED, "models/powerline/es.ms3d");
	DefinePowl(CONNECTION_NORTHWEST, FINISHED, "models/powerline/nw.ms3d");
	DefinePowl(CONNECTION_EASTWEST, FINISHED, "models/powerline/ew.ms3d");
	DefinePowl(CONNECTION_SOUTHWEST, FINISHED, "models/powerline/sw.ms3d");
	DefinePowl(CONNECTION_EASTSOUTHWEST, FINISHED, "models/powerline/esw.ms3d");
	DefinePowl(CONNECTION_NORTHSOUTHWEST, FINISHED, "models/powerline/nsw.ms3d");
	DefinePowl(CONNECTION_NORTHEASTWEST, FINISHED, "models/powerline/new.ms3d");
	DefinePowl(CONNECTION_NORTHEASTSOUTH, FINISHED, "models/powerline/nes.ms3d");
	DefinePowl(CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/powerline/nesw.ms3d");

	Zero(g_crpipecost);
	g_crpipecost[RES_LABOUR] = 1;
	g_crpipecost[RES_STEEL] = 1;
    
	DefineCrPipe(CONNECTION_NOCONNECTION, CONSTRUCTION, "models/crpipeline/1_c.ms3d");
	DefineCrPipe(CONNECTION_NORTH, CONSTRUCTION, "models/crpipeline/n_c.ms3d");
	DefineCrPipe(CONNECTION_EAST, CONSTRUCTION, "models/crpipeline/e_c.ms3d");
	DefineCrPipe(CONNECTION_SOUTH, CONSTRUCTION, "models/crpipeline/s_c.ms3d");
	DefineCrPipe(CONNECTION_WEST, CONSTRUCTION, "models/crpipeline/w_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHEAST, CONSTRUCTION, "models/crpipeline/ne_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/crpipeline/ns_c.ms3d");
	DefineCrPipe(CONNECTION_EASTSOUTH, CONSTRUCTION, "models/crpipeline/es_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHWEST, CONSTRUCTION, "models/crpipeline/nw_c.ms3d");
	DefineCrPipe(CONNECTION_EASTWEST, CONSTRUCTION, "models/crpipeline/ew_c.ms3d");
	DefineCrPipe(CONNECTION_SOUTHWEST, CONSTRUCTION, "models/crpipeline/sw_c.ms3d");
	DefineCrPipe(CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/crpipeline/esw_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/crpipeline/nsw_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/crpipeline/new_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/crpipeline/nes_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/crpipeline/nesw_c.ms3d");
	DefineCrPipe(CONNECTION_NOCONNECTION, FINISHED, "models/crpipeline/1.ms3d");
	DefineCrPipe(CONNECTION_NORTH, FINISHED, "models/crpipeline/n.ms3d");
	DefineCrPipe(CONNECTION_EAST, FINISHED, "models/crpipeline/e.ms3d");
	DefineCrPipe(CONNECTION_SOUTH, FINISHED, "models/crpipeline/s.ms3d");
	DefineCrPipe(CONNECTION_WEST, FINISHED, "models/crpipeline/w.ms3d");
	DefineCrPipe(CONNECTION_NORTHEAST, FINISHED, "models/crpipeline/ne.ms3d");
	DefineCrPipe(CONNECTION_NORTHSOUTH, FINISHED, "models/crpipeline/ns.ms3d");
	DefineCrPipe(CONNECTION_EASTSOUTH, FINISHED, "models/crpipeline/es.ms3d");
	DefineCrPipe(CONNECTION_NORTHWEST, FINISHED, "models/crpipeline/nw.ms3d");
	DefineCrPipe(CONNECTION_EASTWEST, FINISHED, "models/crpipeline/ew.ms3d");
	DefineCrPipe(CONNECTION_SOUTHWEST, FINISHED, "models/crpipeline/sw.ms3d");
	DefineCrPipe(CONNECTION_EASTSOUTHWEST, FINISHED, "models/crpipeline/esw.ms3d");
	DefineCrPipe(CONNECTION_NORTHSOUTHWEST, FINISHED, "models/crpipeline/nsw.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTWEST, FINISHED, "models/crpipeline/new.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTSOUTH, FINISHED, "models/crpipeline/nes.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/crpipeline/nesw.ms3d");

	g_ordersnd.clear();

	g_ordersnd.push_back(Sound("sounds\\aaa000\\gogogo.wav"));
	g_ordersnd.push_back(Sound("sounds\\aaa000\\moveout2.wav"));
	g_ordersnd.push_back(Sound("sounds\\aaa000\\spreadout.wav"));
	g_ordersnd.push_back(Sound("sounds\\aaa000\\wereunderattack3.wav"));
	//g_zpainSnd.push_back(Sound("sounds\\zpain.wav"));
}