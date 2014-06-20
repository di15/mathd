


#ifndef DEBUG_H
#define DEBUG_H

#include "platform.h"
#include "gui/richtext.h"

void LogRich(const RichText* rt);
void CheckNum(const char* num);
void LastNum(const char* l);
void CheckGLError(const char* file, int line);

#endif	//DEBUG_H