

#include "sim.h"
#include "../platform.h"
#include "../texture.h"
#include "../render/heightmap.h"
#include "../render/model.h"
#include "resources.h"
#include "../gui/icon.h"

void Queue()
{
	/*
#define ICON_DOLLAR			0
#define ICON_LABOUR			1
#define ICON_HOUSING		2
#define ICON_FARMPRODUCT	3
#define ICON_FOOD			4
#define ICON_CHEMICALS		5
#define ICON_ELECTRONICS	6
#define ICON_RESEARCH		7
#define ICON_PRODUCTION		8
#define ICON_IRONORE		9
#define ICON_URANIUMORE		10
#define ICON_STEEL			11
#define ICON_CRUDEOIL		12
#define ICON_WSFUEL			13
#define ICON_STONE			14
#define ICON_CEMENT			15
#define ICON_ENERGY			16
#define ICON_ENRICHEDURAN	17
#define ICON_COAL			18
#define ICON_TIME			19
#define ICON_RETFUEL		20
#define ICONS				21
	*/

	DefineIcon(ICON_DOLLAR, "gui/icons/dollars.png", ":funds:");
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

	/*
#define RES_FUNDS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_FOOD			4
#define RES_CHEMICALS		5
#define RES_ELECTRONICS		6
#define RES_RESEARCH		7
#define RES_PRODUCTION		8
#define RES_IRONORE			9
#define RES_URANIUMORE		10
#define RES_STEEL			11
#define RES_CRUDEOIL		12
#define RES_WSFUEL			13
#define RES_RETFUEL			14
#define RES_STONE			15
#define RES_CEMENT			16
#define RES_ENERGY			17
#define RES_ENRICHEDURAN	18
#define RES_COAL			19
#define RESOURCES			20
	*/

	DefineRes(RES_FUNDS,		"Funds",				"",							ICON_DOLLAR, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_LABOUR,		"Labour",				"",							ICON_LABOUR, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_HOUSING,		"Housing",				"",							ICON_HOUSING, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_FARMPRODUCTS,	"Farm Products",		"Fertile",					ICON_FARMPRODUCT, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_CHEMICALS,	"Chemicals",			"",							ICON_CHEMICALS, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_ELECTRONICS,	"Electronics",			"",							ICON_ELECTRONICS, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_RESEARCH,		"Research",				"",							ICON_RESEARCH, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_PRODUCTION,	"Production",			"",							ICON_PRODUCTION, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_FOOD,			"Food",					"",							ICON_FOOD, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_IRONORE,		"Iron Ore",				"Iron Ore Deposit",			ICON_IRONORE, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_URANIUMORE,	"Uranium Ore",			"Uranium Ore Deposit",		ICON_URANIUMORE, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_STEEL,		"Steel",				"",							ICON_STEEL, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_CRUDEOIL,		"Crude Oil",			"Oil Deposit",				ICON_CRUDEOIL, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_WSFUEL,		"Wholesale Fuel",		"",							ICON_WSFUEL, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_RETFUEL,		"Retail Fuel",			"",							ICON_RETFUEL, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_STONE,		"Stone",				"Stone Deposit",			ICON_STONE, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_CEMENT,		"Cement",				"",							ICON_CEMENT, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_ENERGY,		"Energy",				"",							ICON_ENERGY, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_ENRICHEDURAN,	"Enriched Uranium",		"",							ICON_ENRICHEDURAN, false, false, true, 1.0f,1.0f,1.0f,1.0f);
	DefineRes(RES_COAL,			"Coal",					"Call Deposit",				ICON_COAL, false, false, true, 1.0f,1.0f,1.0f,1.0f);

	QueueTexture(&g_tiletexs[TILE_SAND], "textures/terrain/default/sand.jpg", false);
	QueueTexture(&g_tiletexs[TILE_GRASS], "textures/terrain/default/grass.jpg", false);
	QueueTexture(&g_tiletexs[TILE_SNOW], "textures/terrain/default/snow.jpg", false);
	QueueTexture(&g_tiletexs[TILE_ROCK], "textures/terrain/default/rock.jpg", false);
	QueueTexture(&g_tiletexs[TILE_CRACKEDROCK], "textures/terrain/default/crackedrock.jpg", false);

	//QueueModel(&themodel, "models/battlecomp/battlecomp.ms3d", Vec3f(0.1f,0.1f,0.1f) * 100 / 64, Vec3f(0,100,0));
}