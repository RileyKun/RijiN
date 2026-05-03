#pragma once

#if defined(LOADER_CLIENT)
#include "stb_image_write.h"
#endif


#if defined(RENDERING_MODE_D2D1)
  #define D3D_VERSION "D2D1"
#endif

#if defined(RENDERING_MODE_D3D9)
  #if defined(RENDERING_MODE_D3D9_NEW)
    #define D3D_VERSION "D3D9_NF"
  #else
    #define D3D_VERSION "D3D9_PR"
  #endif
#endif

#if defined(RENDERING_MODE_D3D11_IMGUI)
  #define D3D_VERSION "D3D11"
#endif

#if !defined(D3D_VERSION)
  #define D3D_VERSION "UNK"
#endif

#include "render_base.h"


#if defined(RENDERING_MODE_D2D1)
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#include "render_d2d1.h"
#endif

#if defined(RENDERING_MODE_D3D9)
#include <d3d9.h>
#include <D3dx9core.h>
#include "rijin_icon.h"
// dev test
#if defined(RENDERING_MODE_D3D9_NEW)
  #include "daisy.h"
  #include "render_d3d9.h"
#else
  #include "render_d3d9_previous.h"
#endif
#endif

#if defined(RENDERING_MODE_D3D11_IMGUI)
#include <d3d11.h>
#include <dxgi.h>

// imgui required
#include "imgui/font_link.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "render_d3d11_imgui.h"
#endif

#if defined(GCS)
#include "gui/gcs.h"
#endif

#if defined(RENDER_3D)
#include "render_debug.h"
#include "notifications.h"
#endif