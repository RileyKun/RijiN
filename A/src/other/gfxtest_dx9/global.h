#pragma once

class c_global{
public:
  // Hook original pointers
  void* d3d9_hook_trp = nullptr;
};

CLASS_EXTERN(c_global, global);