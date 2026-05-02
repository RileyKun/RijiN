#pragma once

class c_interface_engine_model{
public:
  ALWAYSINLINE void force_material_override( void* material ){
    utils::virtual_call<1, void, void*, i32>(this, material, 0);
  }
};