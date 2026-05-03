#pragma once

class c_interface_physics_surface{
public:
  ALWAYSINLINE surface_data* get_surface_data(i32 index){
    return utils::virtual_call64<5, surface_data*, i32>(this, index);
  }
};