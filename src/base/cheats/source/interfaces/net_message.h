#pragma once

class c_net_message{
public:
  ALWAYSINLINE i32 get_group(){
    return utils::internal_virtual_call<i32>(base_interface->net_msg_get_group_index, this);
  }

  ALWAYSINLINE i8* get_name(){
    return utils::internal_virtual_call<i8*>(base_interface->net_msg_get_name_index, this);
  }
};
