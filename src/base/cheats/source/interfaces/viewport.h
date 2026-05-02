#pragma once

class c_viewport_panel{
public:
  ALWAYSINLINE i8* get_name(){
    return utils::internal_virtual_call<i8*>(base_interface->vpp_get_name_index, this);
  }

  ALWAYSINLINE void set_data(void* kv){
    utils::internal_virtual_call<void, void*>(base_interface->vpp_set_data_index, this, kv);
  }

  ALWAYSINLINE void reset(){
    utils::internal_virtual_call<void>(base_interface->vpp_reset_index, this);
  }

  ALWAYSINLINE void update(){
    utils::internal_virtual_call<void>(base_interface->vpp_update_index, this);
  }

  ALWAYSINLINE bool needs_update(){
    return utils::internal_virtual_call<bool>(base_interface->vpp_needs_update_index, this);
  }

  ALWAYSINLINE bool has_input_elements(){
    return utils::internal_virtual_call<bool>(base_interface->vpp_has_input_elements_index, this);
  }

  ALWAYSINLINE void show_panel(bool state){
    utils::internal_virtual_call<void, bool>(base_interface->vpp_show_panel_index, this, state);
  }
};

class c_interface_viewport{
public:
  ALWAYSINLINE void show_background(bool state){
    utils::internal_virtual_call<void, bool>(base_interface->vp_show_background_index, this, state);
  }

  ALWAYSINLINE c_viewport_panel* find_panel_by_name(std::string name){
    return utils::internal_virtual_call<c_viewport_panel*, const i8*>(base_interface->vp_find_panel_by_name_index, this, name.c_str());
  }
};