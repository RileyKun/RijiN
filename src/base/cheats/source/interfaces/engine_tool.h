#pragma once

class c_interface_engine_tool{
public:
  ALWAYSINLINE void get_world_to_screen_matrix_for_view(s_view_setup* view_setup, matrix4x4* matrix){
    if(view_setup == nullptr || matrix == nullptr)
      return;

    utils::internal_virtual_call<void, s_view_setup*, matrix4x4*>(base_interface->get_world_to_screen_matrix_for_view_index, this, view_setup, matrix);
  }
};