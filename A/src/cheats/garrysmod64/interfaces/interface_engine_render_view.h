#pragma once

class c_interface_engine_render_view{
public:

  ALWAYSINLINE void set_blend( float blend ){
    utils::virtual_call64<4, void, float>(this, blend);
  }

  ALWAYSINLINE float get_blend(){
    return utils::virtual_call64<5, float>(this);
  }

  ALWAYSINLINE void set_colour_modulation( colour clr ){
    float c[3];
    c[0] = clr.x != 0 ? (float)clr.x / 255.f : 0.f;
    c[1] = clr.y != 0 ? (float)clr.y / 255.f : 0.f;
    c[2] = clr.z != 0 ? (float)clr.z / 255.f : 0.f;
    utils::virtual_call64<6, void, float*>(this, c);
  }

  ALWAYSINLINE colour get_colour_modulation(){
    float c[3];
    utils::virtual_call64<7, void, float*>(this, &c[0]);

    return rgb(c[0] * 255, c[1] * 255, c[2] * 255);
  }

};