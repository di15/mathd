
#ifndef UTIL_H
#define UTIL_H

#include "platform.h"

void StripPathExtension(const char* n, char* o);
string ExePath();
string MakePathRelative(const char* full);

#endif