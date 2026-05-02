#pragma once

#define __CRT__NO_INLINE

#include "stdlib.h"
#include "stdint.h"
#include "math.h"

#ifndef imax
#define imax(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef imin
#define imin(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define VFUNC(p, n) ((*(uintptr_t**)(p))[n])
#define EXPORT extern "C" __attribute((dllexport))
#define NO_OPT __attribute((optimize("O0")))
#define INLINE __attribute((always_inline))

#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)

#define OPAD(size) uint8_t CAT(_pad_, __COUNTER__)[size]
#define OFIELD(off, field) struct { OPAD(off); field; };

#define DEBUG_BREAKS

#ifdef DEBUG_BREAKS
#define BREAK(crc) 	asm("int $3;" \
		"push %0;" \
		: \
		:"i"(__LINE__))
#else
#define BREAK(crc)
#endif

#include "stdlib.h"
#include "mathlib.h"