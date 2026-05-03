#pragma once

class gcs_component_divider : public gcs_component_base{
public:

  void setup() override{
    set_size(vec3i(0.f, 14));
  }

  void think(vec3i pos, vec3i size) override{
  }

  void draw(vec3i pos, vec3i size){
    colour main_clr = scheme()->main;
    main_clr.w *= 0.75f;
    render->solid_line(vec3i(pos.x + push_align_offset(), pos.y + size.y / 2), vec3i(push_align_offset() + (pos.x + size.x), pos.y + size.y / 2), main_clr, false, 1.f);
  }
};