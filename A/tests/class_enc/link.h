#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <windows.h>

#include "typedefs.h"
#include "structs.h"

#define EXTERN extern "C"
#define EXPORT EXTERN __attribute((dllexport))
#define ALWAYSINLINE __attribute__((always_inline))

#include "crc32.h"
#include "xorptr.h"