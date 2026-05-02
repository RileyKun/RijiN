#pragma once
#include "../../base/link.h"

#if defined(DEV_MODE)
  typedef void (__cdecl* warning_fn)(const char* text, ...);
  #define DBG(_f, ...)  {static uptr tier0 = utils::get_module_handle(HASH("tier0.dll")); static uptr warning_ptr = utils::get_proc_address(tier0, HASH("Warning")); static warning_fn warning = (warning_fn)warning_ptr; warning(_f, ##__VA_ARGS__);}
#else
  #define DBG(_f, ...) {}
#endif

// includes
#include "utils.h"
#include "structs/structs.h"
#include "interfaces/interfaces.h"
#include "gui/settings.h"
#include "global.h"
#include "math.h"
#include "gui/gcs_info_panel.h"
#include "features/features.h"

#include "player_list_handler.h"

// GUI
#include "../../base/render/gui/gcs.h"

#include "gui/gcs_component_player_list_entry.h"
#include "gui/gcs_menu.h"


#include "misc.h"