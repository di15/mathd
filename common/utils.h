#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>

using namespace std;

#define CORRECT_SLASH '/'

#define ARRSZ(a)	(sizeof(a)/sizeof(a[0]))

extern ofstream g_log;

const string DateTime();
const string FileDateTime();
void FullPath(const char* filename, char* full);
string MakePathRelative(const char* full);
void ExePath(char* exepath);
string StripFile(string filepath);
void StripPathExtension(const char* n, char* o);
void StripExtension(char* filepath);
void StripPath(char* filepath);
void OpenLog(const char* filename, int version);
float StrToFloat(const char *s);
int HexToInt(const char* s);
int StrToInt(const char *s);
void CorrectSlashes(char* corrected);
void BackSlashes(char* corrected);
void ErrorMessage(const char* title, const char* message);
void InfoMessage(const char* title, const char* message);
void WarningMessage(const char* title, const char* message);
void OutOfMem(const char* file, int line);

long timeGetTime();
long GetTickCount();
long long GetTickCount64();
void Sleep(int ms);
