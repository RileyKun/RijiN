#pragma once

#if defined(DEV_MODE)
  #define DBG(_f, ...) { i8 buf[1024]; wsprintfA(buf, _f, ##__VA_ARGS__); OutputDebugStringA(buf); }
#else
  #define DBG(_f, ...) {}
#endif

// base
#include "../../base/link.h"

// includes
#include "global.h"

#include "gcs_test.h"