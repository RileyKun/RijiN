#pragma once

class c_interface_panel{
public:
  ALWAYSINLINE void set_mouse_input_enabled(uptr panel, bool enabled){
    assert(panel > 0);

    STACK_CHECK_START;
    utils::internal_virtual_call<void, uptr, bool>(base_interface->set_mouse_input_enabled_index, this, panel, enabled);
    STACK_CHECK_END;
  }

  ALWAYSINLINE i8* get_name(uptr panel){
    assert(panel > 0);

    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i8*, uptr>(base_interface->panel_get_name_index, this, panel);
    STACK_CHECK_END;
    return r;
  }
};