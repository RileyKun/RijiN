#pragma once

class c_interface_engine_model{
public:
  ALWAYSINLINE void force_material_override( void* material ){
    utils::internal_virtual_call<void, void*, i32>(base_interface->force_material_override_index, this, material, 0);
  }
};