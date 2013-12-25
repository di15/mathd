

enum EDTOOL{NOTOOL=-1, PLACETILE, RAISE, LOWER, SPREAD, DESTROY, PLACEUNIT, PLACESCENERY};

extern int g_edTool;
extern int g_selU;

float strToFloat(const char *s);
void Change_EdPRes(int param);
void Change_EdPWage(int param);
void Change_EdPPrice(int param);
void Click_ClosePMore();
void Click_EdPMore();
void Click_EdPMore2();
void Click_EdPMore3();
void Click_EdSave();
void Click_EdLoad();
void Click_Raise();
void Click_Lower();
void Click_Spread();
void Click_TileType(int param);
void EdApply();
void EdApplyUp();
void Change_EdUnitType();
void Change_EdCat();
void Change_Activity();
void Change_SelP();
void Change_ScT();
void Click_Destroy();