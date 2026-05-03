#pragma once

class gcs_component_text : public gcs_component_base{
  i32     text_padding      = 0;
  u32     font_size         = 12;
  vec3i   text_size;
  bool    force_recalculate = false;
public:

  ALWAYSINLINE void set_padding(float new_padding){
    text_padding = new_padding;
    force_recalculate = true;
  }

  ALWAYSINLINE void set_font_size(u32 new_font_size){
    font_size = new_font_size;
    force_recalculate = true;
  }

  ALWAYSINLINE void set_name(std::wstring new_name) override{
    info()->name = new_name;
    force_recalculate = true;
  }

  void setup() override{
  }

  void pre_run_child() override{
    if(!force_recalculate)
      return;

    text_size = font()->get_size(font_size, name().c_str(), nullptr);
    set_size(vec3i(0, text_size.y + (text_padding * 2)));

    force_recalculate = false;
  }

  void think(vec3i pos, vec3i size) override{
  }

  void draw(vec3i pos, vec3i size){
  }
};