

#ifndef FIXMATH_H
#define FIXMATH_H

#include "../platform.h"

//#define SMALL_ISQRT

#ifdef SMALL_ISQRT

unsigned short isqrt(unsigned long a);

#else

inline int isqrt(uint32_t x);

#endif

#endif