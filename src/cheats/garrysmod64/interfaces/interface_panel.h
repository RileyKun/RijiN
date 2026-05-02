#pragma once

class c_interface_panel{
public:
  ALWAYSINLINE void set_mouse_input_enabled(i64 panel, bool enabled){
    if(!panel)
      return;

    utils::virtual_call64<32, void, i64, bool>(this, panel, enabled);
  }

  ALWAYSINLINE i8* get_name(i64 panel){
    return utils::virtual_call64<36, i8*, i64>(this, panel);
  }
};