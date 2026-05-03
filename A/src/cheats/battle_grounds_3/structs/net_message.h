#pragma once

class c_net_message{
public:
  ALWAYSINLINE i32 get_group(){
    return utils::virtual_call<8, i32>(this);
  }

  ALWAYSINLINE i8* get_name(){
    return utils::virtual_call<9, i8*>(this);
  }
};
