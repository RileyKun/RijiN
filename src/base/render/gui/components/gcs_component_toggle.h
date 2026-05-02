#pragma once

class gcs_component_toggle : public gcs_component_base{
  bool* ptr = nullptr;
public:

  ALWAYSINLINE void setup() override{
    set_size(vec3i(0.f, render->rescale(14)));
  }

  ALWAYSINLINE void set_bool(bool* new_ptr){
    ptr = new_ptr;
  }

  void think(vec3i pos, vec3i size) override{
    if(ptr == nullptr)
      return;

    if(is_in_rect() && input() & GCS_IN_M1_CLICK){
      *ptr = !*ptr;

      emit_click_sound();
    }
  }

  void draw_anywhere(vec3i pos, vec3i size){
    font()->draw(pos + vec3i(push_align_offset(), (size.y / 2) + render->rescale(1)), 16, name().c_str(), scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
  }

  void draw(vec3i pos, vec3i size){
    if(ptr == nullptr)
      return;

    vec3i checkbox_size = vec3i(size.y, size.y);
    vec3i box_pos = pos;

    if(*ptr){
      colour transparent_fill = scheme()->main;
      transparent_fill.a = 255.f * 0.25f;

      if(is_in_rect()){
        transparent_fill *= 1.25f;
        push_help_text_to_scene();
      }

      render->filled_rect(box_pos, checkbox_size, transparent_fill);
      render->outlined_rect(box_pos, checkbox_size, scheme()->main * 1.25f);

#if defined(IMGUI_API)
      render->tick_mark(box_pos + render->rescale(vec3i(2, 2)), scheme()->main * 1.25f, size.y - render->rescale(4));
#else
      u32 font_size = ((u32)14 - render->rescale(2)); // Rescale times by two to fit the mark in.

      render->push_clip_region(box_pos, checkbox_size);
      emoji()->draw(box_pos + (checkbox_size / 2), font_size, L" ✔️", scheme()->main * 1.25f, TSTYLE_NONE, TALIGN_CENTER, TALIGN_CENTER);
      render->pop_clip_region();
#endif
    }
    else{
      colour transparent_fill = scheme()->grey3;
      transparent_fill.a = 255.f * 0.25f;

      if(is_in_rect()){
        transparent_fill *= 1.10f;
        push_help_text_to_scene();
      }
      else
        transparent_fill *= 0.25f;

      render->filled_rect(box_pos, checkbox_size, transparent_fill);
      render->outlined_rect(box_pos, checkbox_size, scheme()->grey3);
    }
    //font()->draw(pos + vec3i(push_align_offset(), (size.y / 2) + 1), 16, name().c_str(), scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
  }
};