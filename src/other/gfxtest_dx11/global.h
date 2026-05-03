#pragma once

class c_global{
public:

  // Hook original pointers
  void* d3d11_hook_trp           = 0;

  ID3D11RenderTargetView* target_view;
  ID3D11DeviceContext*    device_context;
};

CLASS_EXTERN(c_global, global);