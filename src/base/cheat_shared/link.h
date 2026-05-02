#pragma once

// rebuild all required functions
#include "rebuild_core.h"

// gen
#include "gen.h"
#include "gen_internal.h"

// rebuilt sprintf
#if defined(RENDERING_MODE_D3D11_IMGUI) || defined(IMGUI_API)
#include "stb_sprintf.h"
#endif