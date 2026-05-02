#pragma once

class gcs_component_test : public gcs_component_base{
public:

  bool is_module() override{
    return true;
  }

  void setup() override{
    info()->module_size = 16;
  }

  void think(vec3i pos, vec3i size) override{
  }

  void draw(vec3i pos, vec3i size){
    render->outlined_rect(pos,size, rgb(0,255,0));
  }
};