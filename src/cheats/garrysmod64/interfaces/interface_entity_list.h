#pragma once

class c_interface_entity_list{
public:
  ALWAYSINLINE c_base_entity* get_entity(i32 index){
    return index >= 0 ? utils::virtual_call64<3, c_base_entity*, i32>(this, index) : nullptr;
  }

  ALWAYSINLINE c_base_entity* get_entity_handle(i32 handle){
    if(handle <= 0)
      return nullptr;

    return utils::virtual_call64<4, c_base_entity*, i32*>(this, &handle);
  }

  ALWAYSINLINE i32 get_highest_index(){
    return utils::virtual_call64<6, i32>(this);
  }
};