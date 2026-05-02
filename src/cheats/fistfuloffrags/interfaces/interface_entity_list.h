#pragma once

class c_interface_entity_list{
public:
  ALWAYSINLINE c_base_entity* get_entity(i32 index){
    assert(index >= 0);
    return utils::virtual_call<3, c_base_entity*, i32>(this, index);
  }

  ALWAYSINLINE c_base_entity* get_entity_handle(uptr handle){
    if(handle <= 0)
      return nullptr;

    return utils::virtual_call<4, c_base_entity*, uptr>(this, handle);
  }

  ALWAYSINLINE i32 get_highest_index(){
    return utils::virtual_call<6, i32>(this);
  }
};