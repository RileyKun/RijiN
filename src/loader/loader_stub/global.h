#pragma once

struct s_dxgi_offsets{
  bool x86 = false;
  u64 d3d9_reset    = 0;
  u64 d3d9_present  = 0;
  u64 d3d11_present = 0;
};

class c_global{
public:
  bool gmc_run = false;
  c_stub_data* stub_data          = nullptr;
  s_dxgi_offsets* dxgi_offset_data  = nullptr;
};

CLASS_EXTERN(c_global, global);