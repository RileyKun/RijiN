#pragma once

#define GCS_LABEL_FONT_SIZE 16
class gcs_component_label : public gcs_component_base{
  std::wstring text;
public:

  ALWAYSINLINE void set_text(std::wstring new_text){
    text = new_text;
  }

  void setup() override{
    set_size(vec3i(0.f, render->rescale(GCS_LABEL_FONT_SIZE)));
  }

  void think(vec3i pos, vec3i size) override{
  }

  void draw(vec3i pos, vec3i size){
    font()->draw(pos + vec3i(push_align_offset(), (size.y / 2) + 1), GCS_LABEL_FONT_SIZE, text.c_str(), scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
  }
};