#pragma once

class c_global{
public:

  // Hook original pointers
  void* get_system_info_hook_trp            = 0;
  void* dll_main_hook_trp                   = 0;
};

CLASS_EXTERN(c_global, global);