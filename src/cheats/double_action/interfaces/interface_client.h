#pragma once

class c_interface_client{
public:
  ALWAYSINLINE bool write_user_cmd_delta_to_buffer(s_bf_write* buf, i32 from, i32 to, bool isnewcmd){
    return utils::virtual_call<23, bool, s_bf_write*, i32, i32, bool>(this, buf, from, to, isnewcmd);
  }
};
