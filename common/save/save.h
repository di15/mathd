

#include <time.h>

void FreeMap();
void FullPath(const char* filename, char* full);
void MapName(const char* filename, char* slotname);
void SaveSlot(int slot);
void SaveMap(const char* filename);
bool LoadSlot(int slot);
bool LoadMap(const char* filename, bool test=false);