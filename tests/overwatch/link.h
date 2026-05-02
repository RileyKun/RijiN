#pragma once

#if defined(DEV_MODE)
  #define DBG(_f, ...) { i8 buf[1024]; wsprintfA(buf, _f, ##__VA_ARGS__); OutputDebugStringA(buf); }
#else
  #define DBG(_f, ...) {}
#endif

// base
#include "../../base/link.h"

// includes
#include "settings.h"
#include "global.h"

#include "main_win.h"
#include "ow.h"
#include "ow_structs_misc.h"
#include "ow_structs_components.h"
#include "ow_hero_data.h"
#include "c_ow.h"
#include "c_state.h"
#include "c_local.h"
#include "c_hero.h"
#include "c_anim.h"
#include "c_aim.h"
#include "c_trig.h"
#include "c_offense.h"
#include "c_healer.h"
#include "c_esp.h"

#include "gcs_menu.h"