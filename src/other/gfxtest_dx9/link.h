#pragma once
#include "../../base/link.h"

#if defined(DEV_MODE)
  typedef void (__cdecl* warning_fn)(const char* text, ...);
  #define DBG(_f, ...)  {static uptr tier0 = utils::get_module_handle(HASH("tier0.dll")); static uptr warning_ptr = utils::get_proc_address(tier0, HASH("Warning")); static warning_fn warning = (warning_fn)warning_ptr; warning(_f, ##__VA_ARGS__);}
#else
  #define DBG(_f, ...) {}
#endif



// includes
#include "global.h"