

#define NUMPAD_FIGURES	3

extern int g_numPadPow;
extern float g_numPadVal;
extern bool g_numPadDecimal;
extern char g_numPadAfter[8];
extern char g_numPadBefore[8];
extern void (*g_numPadAccept)();
extern void (*g_numPadCancel)();

void Click_NextNumPad();
void Click_NumPadLeft();
void Click_NumPadRight();
void Click_NumPadPlus();
void Click_NumPadMinus();
void Click_NumPadAccept();
void Click_NumPadCancel();
void NumPadUnderscore();
void NumPadChange(float change);
void CloseNumPad();
void OpenNumPad(void (*accept)(), void (*cancel)(), float val, bool decimal, const char* beforestr, const char* afterstr);