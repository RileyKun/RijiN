#pragma once

class c_interface_engine_tool{
public:
  ALWAYSINLINE void get_world_to_screen_matrix_for_view(s_view_setup* view_setup, matrix4x4* matrix){
    assert(view_setup != nullptr);
    assert(matrix != nullptr);
    utils::virtual_call64<78, void, s_view_setup*, matrix4x4*>(this, view_setup, matrix);
  }
};