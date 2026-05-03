#pragma once

class c_view_render_beams{
public:
  ALWAYSINLINE void draw_beam(void* info){
    assert(info != nullptr);
    utils::internal_virtual_call<void, void*>(base_interface->draw_beam_index, this, info);
  }

  ALWAYSINLINE void* create_beam_points(s_beam_info* info){
    assert(info != nullptr);
    return utils::internal_virtual_call<void*, s_beam_info*>(base_interface->create_beam_points_index, this, info);
  }
};