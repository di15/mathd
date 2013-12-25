
#include <vector>
#include "resource.h"
#include "building.h"
#include "unit.h"

using namespace std;

#define ACTIVITY_NONE		0
#define ACTIVITY_FULL		1
#define ACTIVITY_NOBUILDB	2
#define ACTIVITY_NOBUILDU	3
#define ACTIVITY_NOBUILDBU	4
#define AI_ACTIVITIES		5

extern const char* activitystr[];
extern const char* facnamestr[];
extern const char* facfullname[];
extern const char* faclogofile[];
extern const float facowncolor[][4];

//price history
class PriceH
{
public:
	float price;
	float expenditures;
	float earnings;

	void reset()
	{
		earnings = 0;
		expenditures = 0;
	}
};

class CPlayer
{
public:
	bool on;
	string username;
	float local[RESOURCES];	// used just for counting; cannot be used
	float global[RESOURCES];
	float price[RESOURCES];
	float reschange[RESOURCES];
	float wage[BUILDING_TYPES];
	float conwage;	// construction wage
	float truckwage;
	bool ai;
	int activity;
	int bbframesago;
	float uprice[UNIT_TYPES];
	int buframesago;
	float transportprice;
	vector<PriceH> priceh[RESOURCES];
	PriceH recentph[RESOURCES];
	int priceadjframes;
	float occupied[RESOURCES];	// used for capacity resources, just for the resource ticker
	vector<CCycleH> truckh;
	CCycleH recentth;	// truckh also includes currency subtractions from paying for resources for roads, powerlines, pipelines (where they can't be stored) but not buildings
	int cycleframes;
};

#define PLAYERS		6
extern CPlayer g_player[PLAYERS];
extern bool g_diplomacy[PLAYERS][PLAYERS];
extern float g_pColor[PLAYERS][4];	//player colors

extern int g_localP;
extern int g_selP;

void UpdatePlayers();
void InitPlayers();
void ResetPlayers();
void MakePeace(int a, int b);
void MakeWar(int a, int b);
void Bankrupt(int player, const char* reason);
void GameOver(const char* reason, int player);
void CheckGameOver(int who);
void LogTransx(const int player, const float moneych, const char* what);