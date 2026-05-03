#pragma once

#define __CRT__NO_INLINE

// core includes
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <functional>
#include <windows.h>
#include <tlhelp32.h>
#include <x86intrin.h>
#include <psapi.h>
#include <Ntdef.h>

// additional includes
#include "typedefs.h"
#include "format.h"
#include "macros.h"
#include "errors.h"
#include "structs.h"
#include "dbg.h"
#include "protections/link.h"
#include "assert.h"
#include "license_object.h"
#include "input_system.h"
#include "map.h"

// hde
#include "hde/hde.h"

// library includes
#include "math/crc32.h"
#include "math/fnv1a.h"
#include "math/math.h"

#include "api/file_manager.h"
#include "api/registry.h"
#include "api/convert.h"
#include "api/string_utils.h"
#include "api/cpu.h"
#include "api/clipboard.h"

#include "shared_globally/link.h"
#include "cheat_shared/link.h"
#include "utils.h"

// requires math::time and input_system
#include "key_control.h"
#include "memory_manager.h"
#include "dynamic_array.h"

// network related
#if !defined(DISABLE_NETWORKING)
  #include "net_fragment.h"
  #include "net_structs.h"
  #include "cloud.h"
#endif

#include "exceptions.h"

#if defined(RENDERING_ENABLE) && !defined(FORCE_NO_RENDER)
#include "render/link.h"
#endif

#if defined(SOURCE_CHEAT)
#include "cheats/source/link.h"
#endif