

#include <vector>

using namespace std;

#define CONDITION_NONE			0
#define CONDITION_ONLOADMAP		1
#define CONDITION_ONPLACEB		2
#define CONDITION_ONBUILDU		3
#define CONDITION_ONPLACEOTHERB	4
#define CONDITION_ONBUILDOTHERU	5
#define CONDITION_PRICESET		6
#define CONDITION_UNITCOUNT		7
#define CONDITION_CAPTUREB		8
#define CONDITION_MESSAGECLOSED	9
#define CONDITION_ROADINACCESSABILITY	10
#define CONDITION_ALLROADACCESSIBLE		11
#define CONDITION_ONFINISHEDB	12
#define CONDITION_ALLCONSTRUCTIONFINI	13

class CCondition
{
public:
	char name[16];
	int type;
	bool met;
	int target;
	float fval;
	int count;
	int owner;

	CCondition();
};

#define EFFECT_NONE				0
#define EFFECT_ENABTRIGGER		1
#define EFFECT_DISABTRIGGER		2
#define EFFECT_MESSAGE			3
#define EFFECT_VICTORY			4
#define EFFECT_LOSS				5
#define EFFECT_MESSAGEIMG		6
#define EFFECT_RESETCONDITIONS	7
#define EFFECT_EXECUTETRIGGER	8

class CTrigger;

class CEffect
{
public:
	char name[16];
	int type;
	CTrigger* trigger;
	string textval;
	int imgdw;
	int imgdh;

	CEffect();
};

class CTrigger
{
public:
	char name[16];
	bool enabled;

	vector<CCondition> conditions;
	vector<CEffect> effects;

	CTrigger* prev;
	CTrigger* next;

	CTrigger();
	bool checkallmet();	//returns true if map switched
	bool execute();	//returns true if map switched
	void resetconditions();
};

typedef CTrigger* pTrigger;
extern CTrigger* g_scripthead;

class Hint
{
public:
	string message;
	string graphic;
	int gwidth;
	int gheight;

	Hint();
	void reset();
};

extern Hint g_lasthint;

void OnPriceSet(int targ, float val);
void OnLoadMap();
void OnRoadInacc();
void OnAllRoadAc();
void OnCaptureB(int target);
void OnPlaceB(int target);
void OnBuildU(int target);
void OnCloseMessage();
void OnFinishedB(int target);
bool ConditionMet(int type, int target, float fval, int count, int owner);
void GMessage(const char* text);
void GMessageG(const char* tex, int w, int h);
void Click_CloseMessage();
void ReGUICondition();
void Click_ConSelB();
void Select_ConCaptB();
void Change_ConUCntType();
void Change_ConUCntOwner();
void Change_ConUCntCount(int dummy);
void Change_ConRes();
void Change_ConPrice(int dummy);
void Change_EffGSize(int dummy);
void CloseCondition();
void CloseEffect();
void Click_EffPickMPath();
void Click_EffPickGPath();
void Change_EffGPath(int dummy);
void Change_EffMPath(int dummy);
void Click_MessageR(int res);
void Change_Message();
int CountTriggers();
int TriggerID(CTrigger* trigger);
void Change_EffTrigger();
void Change_ConUnit();
void Change_ConBuild();
void Change_CondType();
void Change_EffType();
void Click_CloseCondition();
void Click_CloseEffect();
CCondition* GetChosenCondition();
CEffect* GetChosenEffect();
CTrigger* GetTrigger(int which);
void ReGUITrigger();
void ReGUIScript();
void Click_CloseUnique();
bool UniqueTrigger(const char* name);
bool UniqueEffect(const char* name);
bool UniqueCondition(const char* name);
void FreeScript();
CTrigger* GetScriptTail();
CTrigger* GetChosenTrigger();
void Click_DelTrigger();
void Click_EditTrigger();
void Click_AddTrigger();
void Click_RenameTrigger();
void Change_NewTrigger(int dummy);
void Change_Triggers();
void Click_CloseTrigger();
void Click_AddCondition();
void Click_AddAction();
void Click_RenCondition();
void Click_RenEffect();
void Click_EdCondition();
void Click_EdEffect();
void Click_DelCondition();
void Click_DelEffect();
void Change_TrigEnabled();
