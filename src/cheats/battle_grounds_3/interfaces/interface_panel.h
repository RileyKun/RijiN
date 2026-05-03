#pragma once

class c_interface_panel{
public:
  ALWAYSINLINE void set_mouse_input_enabled(u32 panel, bool enabled){
    if(panel == 0)
      return;

    utils::virtual_call<32, void, u32, bool>(this, panel, enabled);
  }

  ALWAYSINLINE i8* get_name(u32 panel){
    return utils::virtual_call<36, i8*, u32>(this, panel);
  }
};