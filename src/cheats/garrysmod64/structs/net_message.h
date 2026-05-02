#pragma once

class c_net_message{
public:
  ALWAYSINLINE i32 get_group(){
    return utils::virtual_call64<8, i32>(this);
  }

  ALWAYSINLINE i8* get_name(){
    return utils::virtual_call64<9, i8*>(this);
  }
};
